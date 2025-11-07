#include "../includes/headers.hpp"

extern volatile sig_atomic_t g_shutdown;

// ### STATIC FUNCTIONS ###
Client *findClient(int client_fd, std::vector<Client *> &client_vec)
{
	std::vector<Client *>::iterator it;
	for (it = client_vec.begin(); it != client_vec.end(); ++it)
	{
		if ((*it)->getSocketFd() == client_fd
	)
			return (*it);
	}
	return (NULL);
}

Client *findOwner(int fd, std::vector<Client *> &client_vec)
{
	std::vector<Client *>::iterator it;
	for (it = client_vec.begin(); it != client_vec.end(); ++it)
	{
		if ((*it)->getSocketFd() == fd 
			|| (*it)->_response->getCgiInput() == fd 
			|| (*it)->_response->getCgiOutput() == fd)
			return (*it);
	}
	return (NULL);
}

void static closeCgiFd(Client *client, int fd)
{
	// Invalidate fd references in response
	if (client->_response->getCgiInput() == fd)
		client->_response->setCgiInput(-1);
	if (client->_response->getCgiOutput() == fd)
		client->_response->setCgiOutput(-1);
	
	epoll_ctl(client->getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}

WebServer::WebServer()
{
	// THIS CLASS WILL NEVER BE DEFAULT CONSTRUCTED
}

WebServer::WebServer(const std::vector<Configuration> conf) : _epoll_fd(-1), _configurations(conf)
{
try {
	initEpoll();
	_events = new struct epoll_event[MAX_EVENTS];

	std::set<std::pair<std::string, std::string> >::const_iterator it;
	for (it = _configurations[0].getAllHosts().begin(); it != conf[0].getAllHosts().end(); ++it)
	{
		Server *server = new Server(it->first, it->second);
		if (server)
		{
			registerEpollSocket(server);
			_servers_map[server->getSocketFd()] = server;
			_logger << "WebServer >> WebServer >> Server initialzed on IP and PORT: " << server->getIp() << ":" << server->getPort();
			printLog(_logger, WHITE, std::cout, true);
		}
		else
		{
			_logger << "WebServer >> WebServer >> Error initializing server on port: " << it->first;
			printLog(_logger, RED, std::cerr, true);
			throw WebServerErrorException(_logger.str());
		}
	}
	_sessions = new std::vector<SessionData *>;
    }
    catch (...) {
		if (_events)
			delete[] _events;
		if (this->getEpollFd() != -1)
			close(this->getEpollFd());
		for (std::map<int, Server *>::iterator it = _servers_map.begin(); it != _servers_map.end(); ++it)
		{
			close(it->second->getSocketFd());
			delete it->second;
		}
		_servers_map.clear();
		throw;
    }
}

WebServer::WebServer(const WebServer &other)
{
	_epoll_fd = other._epoll_fd;
	_servers_map = other._servers_map;
	_clients_vec = other._clients_vec;
	_client_to_server_map = other._client_to_server_map;
	_events = other._events;
	_configurations = other._configurations;
}

WebServer &WebServer::operator=(const WebServer &other)
{
	if (this != &other)
	{
		_epoll_fd = other._epoll_fd;
		_servers_map = other._servers_map;
		_clients_vec = other._clients_vec;
		_client_to_server_map = other._client_to_server_map;
		_events = other._events;
		_configurations = other._configurations;
	}
	return *this;
}

WebServer::~WebServer()
{
	for (std::vector<Client *>::iterator it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
	{
		close((*it)->getSocketFd());
		delete *it;
	}
	_clients_vec.clear();

	_client_to_server_map.clear();

	for (std::map<int, Server *>::iterator it = _servers_map.begin(); it != _servers_map.end(); ++it)
	{
		close(it->second->getSocketFd());
		delete it->second;
	}
	_servers_map.clear();

	if (_events)
		delete[] _events;
	if (_epoll_fd != -1)
		close(_epoll_fd);
	if (_sessions)
	{
		std::vector<SessionData *>::iterator it;
		for (it = _sessions->begin(); it < _sessions->end(); ++it)
			delete (*it);
		_sessions->clear();
		delete _sessions;
	}
}

int WebServer::initEpoll(void)
{
	_epoll_fd = epoll_create(1);
	if (_epoll_fd == -1)
	{
		_logger << "WebServer >> initEpoll >> Error while creating epoll";
		printLog(_logger, RED, std::cerr, true);
		return -1;
	}

	// pensar numa forma de modularizar e reutilizar esse trecho de codigo abaixo
	// Pegar as flags do _epoll_fd e adicionar a flag FD_CLOEXEC
	int epoll_flags = fcntl(_epoll_fd, F_GETFD);
	if (epoll_flags == -1)
	{
		_logger << "WebServer >> initEpoll >> Error obtaining flags from _epoll_fd";
		printLog(_logger, RED, std::cerr, true);
		close(_epoll_fd);
		return -1;
	}
	epoll_flags |= FD_CLOEXEC;
	epoll_flags |= O_NONBLOCK;
	if (fcntl(_epoll_fd, F_SETFD, epoll_flags) == -1)
	{
		_logger << "WebServer >> initEpoll >> Error defining flags from _epoll_fd";
		printLog(_logger, RED, std::cerr, true);
		close(_epoll_fd);
		return -1;
	}

	return _epoll_fd;
}

void WebServer::registerEpollSocket(Socket *socket)
{
	// Registra o server socket na epoll para monitorar
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket->getSocketFd(), &socket->getEvent()) == -1)
	{
		_logger << "WebServer >> registerEpollSocket >> Error to add socket to epoll";
		printLog(_logger, RED, std::cerr, true);
		freeaddrinfo(socket->getRes());
		return;
	}
}

// ### HANDLE NEW CLIENT ###
void WebServer::handleNewClient(int server_fd)
{
	Client *client_socket = NULL;
	try
	{
		client_socket = new Client(server_fd);
		_clients_vec.push_back(client_socket);

		Server *server = _servers_map.find(server_fd)->second;
		_client_to_server_map[client_socket->getSocketFd()] = std::make_pair(server->getIp(), server->getPort()); // novidade, possível approach
		client_socket->_server = server;
		_logger << "WebServer >> handleNewClient >> New client conected - client_fd: " << client_socket->getSocketFd();
		printLog(_logger, WHITE, std::cout, true);
		registerEpollSocket(client_socket);
		client_socket->setEpollFd(_epoll_fd);
	}
	catch (const std::exception &e)
	{
		delete client_socket;
		_logger << "WebServer >> handleNewClient >> Error creating new client: " << e.what();
		printLog(_logger, RED, std::cerr, true);
		return;
	}
}

// ### GETTERS ###
int WebServer::getEpollFd() const
{
	return _epoll_fd;
}

std::map<int, Server *> WebServer::getServersMap() const
{
	return _servers_map;
}

std::vector<Client *> WebServer::getClientsVec() const
{
	return _clients_vec;
}

struct epoll_event *WebServer::getEvents() const
{
	return _events;
}

const std::vector<Configuration> &WebServer::getConfigurations() const
{
	return _configurations;
}

// ### SETTERS ###
void WebServer::setEpollFd(int epoll_fd)
{
	_epoll_fd = epoll_fd;
}

void WebServer::setServersMap(const std::map<int, Server *> &servers_map)
{
	_servers_map = servers_map;
}

void WebServer::setClientsVec(const std::vector<Client *> &clients_vec)
{
	_clients_vec = clients_vec;
}

void WebServer::setEvents(struct epoll_event *events)
{
	_events = events;
}

// ### TESTANDO STARTSERVER DENTRO DA WEBSERVER ###
bool WebServer::tryConnection(int i)
{
	if (_servers_map.find(_events[i].data.fd) != _servers_map.end())
	{
			handleNewClient(_events[i].data.fd);
			return true;
	}
	return false;
}

int WebServer::extractContentLength(const std::string &headers_lower)
{
	size_t content_length_pos = headers_lower.find("content-length:");
	if (content_length_pos == std::string::npos)
		return -1;

	size_t pos = content_length_pos + 15; // strlen("content-length:")

	while (pos < headers_lower.length() && (headers_lower[pos] == ' ' || headers_lower[pos] == '\t'))
		pos++;

	size_t start = pos;

	while (pos < headers_lower.length() && std::isdigit(headers_lower[pos]))
		pos++;

	if (pos == start)
		return -1;
	std::string length_str = headers_lower.substr(start, pos - start);
	return std::atoi(length_str.c_str());
}

int WebServer::receiveData(int client_fd)
{
	std::string	newData;
	char 		buffer[BUFFER_SIZE];
	ssize_t		bytes;

	bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytes <= 0)
		return (-1);
	buffer[bytes] = '\0';
	newData = buffer;
	if (newData.empty())
		return -1;
	Configuration *config = findConfigForRequestFast(newData, client_fd);
	if (!config)
	{
		_logger << "WebServer >> receiveData >> Configuration not found" << client_fd;
		printLog(_logger, RED, std::cerr, true);
		_partial_requests.erase(client_fd);
		return -1;
	}
	try
	{
		// verifica se o client tem req
		if (_currentClient->_request != NULL)
		{
			if (_currentClient->_request->RequestCompleted())
				_currentClient->setFirstRequest(true);
			if (_currentClient->getFirstRequest())
			{
				delete _currentClient->_request;
				_currentClient->_request   = new HttpRequest(newData, config, _sessions);
				_currentClient->setFirstRequest(false);
			}
			else
				_currentClient->_request->parse(newData);
			if (!_currentClient->_request->RequestCompleted())
				return (0);
		}
		else {
			_currentClient->_request = new HttpRequest(newData, config, _sessions);
			if (!_currentClient->_request->RequestCompleted())
				return (0);
		}
	}
	catch (const std::exception &e)
	{
		_partial_requests.erase(client_fd);
		_logger << "WebServer >> receiveData >> Error parsing HTTP request: " << e.what();
		printLog(_logger, RED, std::cerr, true);
		return -1;
	}
	return (1);
}
static bool sendResponseToClient(Client *client)
{
	std::stringstream _logger;
	client->_response->setResponse(client->_response->checkStatusCode());
	std::string response = client->_response->getResponse();
	const char *buf = response.c_str();
	size_t size = client->_response->getResponse().size();
	size_t totalSent = client->_response->getFilePos();
	int bytesToSend = size - totalSent;
	int sentBytes = send(client->getSocketFd(), buf + totalSent, bytesToSend, 0);
	if (sentBytes < 0)
	{
		_logger << "WebServer >> sendResponseToClient >> Error sending body to client - client_fd: " << client->getSocketFd();
		printLog(_logger, RED, std::cerr, true);
		return true;
	}
	totalSent += sentBytes;

	_logger << "WebServer >> sendResponseToClient >> Data sent to the client - client_fd: " << client->getSocketFd() << " " << totalSent;
	printLog(_logger, WHITE, std::cout, true);
	client->_response->setFilePos(totalSent);
	return (client->_response->getFilePos() >= size);
}

void WebServer::sendData(int client_fd)
{
	Client *client = findClient(client_fd, _clients_vec);

	if (client->_response != NULL)
		delete client->_response;

	client->_response = new HttpResponse(client);
}

void WebServer::setClientTime(int client_fd)
{
	std::vector<Client *>::iterator it;

	for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
		if ((*it)->getSocketFd() == client_fd)
			return (*it)->setTime(std::time(NULL));;
}

void WebServer::deleteClient(int fd, int event)
{
	std::vector<Client *>::iterator it;
	for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
	{
		if ((*it)->getSocketFd() == fd)
		{
			_logger << "WebServer >> deleteClient >> Client disconected - client_fd: " << (*it)->getSocketFd();
			printLog(_logger, GREEN, std::cout, true);
			if (event == 0)
			{
				int x;
				x = epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
				if (x == -1)
				{
					_logger << "WebServer >> deleteClient >> Error removing from EPOLL";
					printLog(_logger, RED, std::cerr, true);
				}
			}
			close((*it)->getSocketFd());
			delete *it;	
			it = _clients_vec.erase(it);
			_client_to_server_map.erase(fd);
			_partial_requests.erase(fd);
			return;
		}
	}
}

void static readFromCgi(Client* client, uint32_t events) {
	std::stringstream _logger;
    int fd = client->_response->getCgiOutput();

    if (events & EPOLLIN) {
        char buf[4096];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0)
            client->_response->setResponseCgi(buf, n);
    }

    // The CGI process ended and closes their side of the pipe communication
    if (events & (EPOLLRDHUP | EPOLLHUP)) {
		_logger << "WebServer >> readFromCgi >> inside EPOLLHUP or EPOLLRDHUP events";
		printLog(_logger, WHITE, std::cout, true);
        // flush any remaining data one last time
        char buf[4096];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0)
            client->_response->setResponseCgi(buf, n);
		closeCgiFd(client, fd);
		client->_response->checkCgiProcess();
        client->setProcessingState(SEND_DATA);
    }
}

void static writeToCgi(Client *client)
{
    int fd = client->_response->getCgiInput();
    if (fd < 0) return;

    const std::string &body = client->_request->getBody();
    size_t sent = client->_response->getBodySent();

    ssize_t n = write(fd, body.c_str() + sent, body.size() - sent);
    if (n > 0)
        client->_response->setBodySent(sent + n);
    else if (n == -1)
		return closeCgiFd(client, fd);

    // Only closes and remove from epoll if all body is sent to CGI
    if (client->_response->getBodySent() >= body.size())
		closeCgiFd(client, fd);
}

void WebServer::handleEpollErrEvent(int i)
{
    int fd = _events[i].data.fd;

    if (_currentClient->getSocketFd() == fd) {
        return deleteClient(fd, 1);
    }

    if (_currentClient->_response->getCgiInput() == fd
        || _currentClient->_response->getCgiOutput() == fd) 
	{
		closeCgiFd(_currentClient, fd);
        _currentClient->_response->setResStatus(500);
        _currentClient->setProcessingState(SEND_DATA);
        return;
    }
	epoll_ctl(_currentClient->getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}
void WebServer::handleEpollHupEvent(int i)
{
    int fd = _events[i].data.fd;

    // EPOLLHUP: fully closed. If it's client socket, delete client.
    if (_currentClient->getSocketFd() == fd) {
        deleteClient(fd, 1);
        return;
    }
    // If it's CGI output (child stdout) treat as EOF: read remaining and finalize
    if (_currentClient->_response->getCgiOutput() == fd) {
        // Try to drain any remaining data once (readFromCgi should handle EOF and cleanup)
        readFromCgi(_currentClient, _events[i].events);

        // After EOF, move client to sending state if not already
        if (_currentClient->getProcessingState() != SEND_DATA)
            _currentClient->setProcessingState(SEND_DATA);
        return;
    }
    // If it's CGI input (child stdin) a HUP is unusual, simply close and mark as failed
    if (_currentClient->_response->getCgiInput() == fd) {
		closeCgiFd(_currentClient, fd);
        _currentClient->_response->setResStatus(500);
        _currentClient->setProcessingState(SEND_DATA);
        return;
    }
    // Generic: remove and close unknown fd
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}
void WebServer::handleEpollRdHupEvent(int i)
{
	int fd = _events[i].data.fd;
    _logger.str(""); _logger.clear();

    // RDHUP: peer closed its write end (half-close). For sockets we can still send a response.
    if (_currentClient->getSocketFd() == fd) {
		// simple continues the treatment
        return;
    }

    // For CGI output FD: reader sees EOF soon. Drain and finalize (readFromCgi should handle)
    if (_currentClient->_response->getCgiOutput() == fd) {
        // read any remaining data and finalize
        readFromCgi(_currentClient, _events[i].events);

        if (_currentClient->getProcessingState() != SEND_DATA)
            _currentClient->setProcessingState(SEND_DATA);
        return;
    }

    // For CGI input FD: the child closed reading end (we can't send more). Close and mark 500.
    if (_currentClient->_response->getCgiInput() == fd) {
		closeCgiFd(_currentClient, fd);
        _currentClient->_response->setResStatus(500);
        _currentClient->setProcessingState(SEND_DATA);
        return;
    }

    // Fallback: remove & close
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}
void WebServer::handleEpollInEvent(int i)
{
    int fd = _events[i].data.fd;

    // If event is a client socket: forward to existing client input handler
    if (_currentClient && _currentClient->getSocketFd() == fd)
       return handleClientInput(i);
    // If event is a CGI output fd: read from it
    if (_currentClient && _currentClient->_response->getCgiOutput() == fd)
       return readFromCgi(_currentClient, _events[i].events);

    // Unknown FD: log and remove
    _logger << "WebServer >> handleEpollInEvent >> EPOLLIN on unknown fd: " << fd;
    printLog(_logger, RED, std::cerr, true);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}
void WebServer::handleEpollOutEvent(int i)
{
    int fd = _events[i].data.fd;
	// If event is client socket: write data to client (existing handler)
    if (_currentClient->getSocketFd() == fd) 
        return handleClientOutput(i);

    // If event is CGI input fd: pipe more request-body into CGI stdin
    if (_currentClient->_response->getCgiInput() == fd){
       return writeToCgi(_currentClient);}

    _logger << "WebServer >> handleEpollOutEvent >> EPOLLOUT on unknown fd: " << fd;
    printLog(_logger, RED, std::cerr, true);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

void WebServer::treatExistingClient(int i)
{
	_currentClient = findOwner(_events[i].data.fd, _clients_vec); // update all places that call findClient to use this new method
	if (!_currentClient)
		return ;
	uint32_t flags = _events[i].events;
	bool hasEpollIn = flags & EPOLLIN;
	bool hasEpollOut = flags & EPOLLOUT;
	bool hasEpollHup = flags & EPOLLHUP;
	bool hasEpollRdHup = flags & EPOLLRDHUP;
	bool hasEpollErr = flags & EPOLLERR;

	if (hasEpollErr)
		return handleEpollErrEvent(i);
	if (hasEpollHup)
		return handleEpollHupEvent(i);
	if (hasEpollRdHup)
		return handleEpollRdHupEvent(i);
	if (hasEpollIn)
		return handleEpollInEvent(i);
	if (hasEpollOut)
		return handleEpollOutEvent(i);
}

void WebServer::handleEvents(int event_count)
{
	for (int i = 0; i < event_count; ++i)
	{
		// check if this event is a new connection or not
		bool server_found = tryConnection(i);
		if (!server_found)
		{
			treatExistingClient(i);
		}
	}
}

void WebServer::lookForTimeouts()
{
	std::vector<Client *>::iterator it = _clients_vec.begin();
	while (it != _clients_vec.end())
	{
		if ((*it)->checkTimeout())
		{
			_logger << "WebServer >> lookForTimeouts >> Client timeout detected - client_fd: " << (*it)->getSocketFd();
			printLog(_logger, WHITE, std::cout, true);
			int fd = (*it)->getSocketFd();
			if ((*it)->getProcessingState() == CGI_PROCESSING)
				(*it)->_response->terminateCgiProcess();
			else if ((*it)->getProcessingState() == RECEIVING_LARGE)
				(*it)->_response->setResStatus(408); // Request Timeout
			else
				(*it)->_response->setResStatus(503); // Request Timeout
			sendResponseToClient(*it);
			deleteClient(fd, 0);
			it = _clients_vec.begin();
		}
		else
		{
			++it;
		}
	}
	std::vector<SessionData *>::iterator sit = _sessions->begin();
	while (sit != _sessions->end())
	{
		if ((*sit)->checkTimeout())
		{
			_logger << "WebServer >> lookForTimeouts >> Session timeout reached - sessionId " << (*sit)->getSessionId();
			printLog(_logger, WHITE, std::cout, true);
			delete (*sit);
        	sit = _sessions->erase(sit);
		}
		else
		{
			++sit;
		}
	}
}

void WebServer::startServer()
{
	while (!g_shutdown)
	{
		// Espera por novos eventos
		int event_count = epoll_wait(_epoll_fd, _events, MAX_EVENTS, 100);
		if (event_count < 0)
		{
			if (errno == EINTR) // Interrompido por sinal
				continue;
			_logger << "WebServer >> startServer >> Erro no epoll_wait" << strerror(errno) << std::endl;
			printLog(_logger, RED, std::cerr, true);
			return;
		}
		handleEvents(event_count);
		lookForTimeouts();
	}
	_logger << "WebServer >> startServer >> Shutdown signal received";
	printLog(_logger, YELLOW, std::cout, true);
}

static bool checkHostType(const std::string &host)
{
	size_t colonPos = host.find_first_of(':');
	if (colonPos != std::string::npos)
		return true;
	else
		return false;
}

static bool checkForServerName(const std::vector<std::string> &server_names, std::string host)
{
	for (std::vector<std::string>::const_iterator name_it = server_names.begin(); name_it != server_names.end(); ++name_it)
		if (host == *name_it)
			return (true);
	return false;
}

static Configuration *lookForConfigurations(bool numeric_host, std::string host,
											std::map<int, std::pair<std::string, std::string> >::const_iterator client_it,
											std::vector<Configuration> &configs)
{
	Configuration *defaultConfig = NULL;
	for (std::vector<Configuration>::iterator config_it = configs.begin(); config_it != configs.end(); ++config_it)
	{
		const std::set<std::pair<std::string, std::string> > &hosts = config_it->getHost();
		std::set<std::pair<std::string, std::string> >::iterator host_it = hosts.find(client_it->second); // Verifica se o pair é encontrado no set de hosts
		if (host_it == hosts.end())
			continue;
		if (host_it->first == client_it->second.first && host_it->second == client_it->second.second)
		{
			if (defaultConfig == NULL)
				defaultConfig = &(*config_it);
			if (numeric_host)
				return defaultConfig;
			if (checkForServerName(config_it->getServerName(), host))
				return &(*config_it);
		}
	}
	return (defaultConfig);
}

Configuration *WebServer::findConfigForRequestFast(const std::string &rawRequest, int client_fd)
{
	std::string host = extractHostHeaderSimple(rawRequest);
	bool numeric_host = checkHostType(host);
	std::map<int, std::pair<std::string, std::string> >::const_iterator client_it = _client_to_server_map.find(client_fd);
	return lookForConfigurations(numeric_host, host, client_it, _configurations);
}

// ### PRIVATE METHODS ###

void WebServer::handleClientInput(int i)
{
	if (_currentClient->getProcessingState() == RECEIVING)
	{
		int data = receiveData(_events[i].data.fd);
		if (data == -1)
		{
			deleteClient(_events[i].data.fd, 0);
			return;
		}
		else if (data == 0)
		{
			setClientTime(_events[i].data.fd);
			return;
		}
		if (data == 1)
		{
			_currentClient->setProcessingState(PROCESSING);
			_events[i].events = EPOLLOUT;
			epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]);
			setClientTime(_events[i].data.fd);
		}
	}
	else
	{
		_logger << "WARNING: Client " << _currentClient->getSocketFd() << " with the state " << _currentClient->getProcessingState() << " received EPOLLIN (expected: RECEIVING)";
		printLog(_logger, RED, std::cerr, true);
		_currentClient->setProcessingState(RECEIVING);
	}
}

void WebServer::handleClientOutput(int i)
{
	
	switch (_currentClient->getProcessingState())
	{
	case PROCESSING:
		_logger << "WebServer >> handleClientOutput >> starting response PROCESSING for client_fd: " << _currentClient->getSocketFd();
		printLog(_logger, CYAN, std::cout, true);
		sendData(_events[i].data.fd);
		if (_currentClient->getProcessingState() == PROCESSING)
			_currentClient->setProcessingState(SEND_DATA);
		else if (_currentClient->getProcessingState() == STREAMING)
		{
			_logger << "WebServer >> handleClientOutput >> starting reponse STREAMING for client_fd: " << _currentClient->getSocketFd();
			printLog(_logger, CYAN, std::cout, true);
			sendResponseToClient(_currentClient);
		}
		break;

	case STREAMING: // Streaming of large files
		if (!continueLargeFileStreaming())
		{
			_logger << "WebServer >> handleClientOutput >> response streaming finished for client_fd: " << _currentClient->getSocketFd();
			printLog(_logger, CYAN, std::cout, true);
			_events[i].events = EPOLLIN;
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]) == 0)
				_currentClient->setProcessingState(RECEIVING);
			else
			{
				_logger << "WebServer >> handleClientOutput >> Error modifying client event " << _currentClient->getSocketFd() << " para EPOLLIN";
				printLog(_logger, RED, std::cerr, true);
				deleteClient(_events[i].data.fd, 0);
				return ;
			}
			_currentClient->setTime(std::time(NULL));
		}
		break;

	case COMPLETED:
			_logger << "WebServer >> handleClientOutput >> request treatment COMPLETED for client_fd: " << _currentClient->getSocketFd();
			printLog(_logger, CYAN, std::cout, true);
			_events[i].events = EPOLLIN;
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]) == 0)
				_currentClient->setProcessingState(RECEIVING);
			else
			{
				_logger << "WebServer >> handleClientOutput >> Error modifying client event " << _currentClient->getSocketFd() << " to EPOLLIN";
				printLog(_logger, RED, std::cerr, true);
				deleteClient(_events[i].data.fd, 0);
				return ;
			}
			_currentClient->setTime(std::time(NULL));
			break;
	case SEND_DATA:
			_logger << "WebServer >> hanldeClientOutput >> SEND_DATA to client_fd: " << _currentClient->getSocketFd();
			printLog(_logger, CYAN, std::cout, true);
			if (sendResponseToClient(_currentClient))
			{
				_currentClient->setBytesSent(0);
				_currentClient->_response->setFilePos(0);
				_currentClient->setProcessingState(COMPLETED);
			}
			break;
	case CGI_PROCESSING:
		_currentClient->_response->checkCgiProcess();
		break;
	case CGI_COMPLETED: // avaliar a possibilidade de tirar esse processing state talvez nao seja mais usado
		_currentClient->setProcessingState(SEND_DATA);
		break;
	case RECEIVING:
		_logger << "WebServer >> handleClientOutput >> Warning: EPOLLOUT to client in RECEIVING state";
		printLog(_logger, RED, std::cerr, true);
		break;
	default:
		_logger << "WebServer >> handleClientOutput >> Invalid state: " << _currentClient->getProcessingState() << " to client " << _currentClient->getSocketFd();
		printLog(_logger, RED, std::cerr, true);
		_currentClient->setProcessingState(COMPLETED);
		break;
	}
}

bool WebServer::continueLargeFileStreaming()
{
	const size_t CHUNK_SIZE = 8192;
	char buffer[CHUNK_SIZE];

	std::ifstream &file = _currentClient->_response->getFileStream();

	if (!file.is_open()) {
		_logger << "WebServer >> continueLargeFileStreaming >> Error while openning file for client_fd: " << _currentClient->getSocketFd() ;
		printLog(_logger, RED, std::cerr, true);
		return false;
	}

	file.read(buffer, CHUNK_SIZE);
	std::streamsize bytesRead = file.gcount();

	if (bytesRead <= 0) {
		if (file.bad())
		{
			_logger << "WebServer >> continueLargeFileStreaming >> Error while reading file for client_fd: " << _currentClient->getSocketFd() ;
			printLog(_logger, RED, std::cerr, true);
		}
		_currentClient->resetFileStreaming();
		return false;
	}

	ssize_t bytesSent = send(_currentClient->getSocketFd(), buffer, bytesRead, 0);
	if (bytesSent <= 0) {
		_logger << "WebServer >> continueLargeFileStreaming >> Error while sending data for client_fd: " << _currentClient->getSocketFd() ;
		printLog(_logger, RED, std::cerr, true);
		_currentClient->resetFileStreaming();
		return false;
	}

	size_t newFilePos = _currentClient->_response->getFilePos() + bytesSent;
	_currentClient->_response->setFilePos(newFilePos);

	return (newFilePos < _currentClient->_response->getContentLength());
}

std::string WebServer::extractHostHeaderSimple(const std::string &rawRequest)
{
	size_t header_end = rawRequest.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return "";
	std::string headers = rawRequest.substr(0, header_end);
	std::string headers_lower = toLower(headers);
	size_t host_pos = headers_lower.find("host:");
	if (host_pos == std::string::npos)
		return "";
	size_t pos = host_pos + 5;
	while (pos < headers_lower.length() && (headers_lower[pos] == ' ' || headers_lower[pos] == '\t'))
		pos++;
	size_t start = pos;
	while (pos < headers.length() && headers[pos] != '\r' && headers[pos] != '\n')
		pos++;
	if (pos == start)
		return "";
	std::string host = headers.substr(start, pos - start);
	while (!host.empty() && (host[host.length() - 1] == ' ' || host[host.length() - 1] == '\t'))
	{
		host.erase(host.length() - 1);
	}
	return host;
}

// ######### EXCEPTION #########

const char *WebServer::WebServerErrorException::what() const throw()
{
	return (_message.c_str());
}
