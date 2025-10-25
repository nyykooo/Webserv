#include "../includes/headers.hpp"

// ### STATIC FUNCTIONS ###
Client *findClient(int client_fd, std::vector<Client *> &client_vec)
{
	std::vector<Client *>::iterator it;
	for (it = client_vec.begin(); it != client_vec.end(); ++it)
	{
		if ((*it)->getSocketFd() == client_fd)
			return (*it);
	}
	// return (*it);
	return (NULL);
}

WebServer::WebServer()
{
	// THIS CLASS WILL NEVER BE DEFAULT CONSTRUCTED
}

WebServer::WebServer(const std::vector<Configuration> conf) : _configurations(conf)
{
	initEpoll();
	_events = new struct epoll_event[MAX_EVENTS]; // Aloca memória para o vetor de eventos

	std::set<std::pair<std::string, std::string> >::const_iterator it;
	for (it = _configurations[0].getAllHosts().begin(); it != conf[0].getAllHosts().end(); ++it)
	{
		Server *server = new Server(it->first, it->second);
		if (server)
		{
			registerEpollSocket(server);
			_servers_map[server->getSocketFd()] = server; // Armazena o servidor no mapa usando o socket fd como chave
			_logger << "Servidor iniciado no Ip/Port: " << server->getIp() << "/" << server->getPort();
			printLogNew(_logger, WHITE, std::cout, true);
		}
		else
		{
			_logger << "Erro ao inicializar o servidor na porta: " << it->first;
			printLogNew(_logger, RED, std::cerr, true);
			throw WebServerErrorException(_logger.str());
		}
	}
	_sessions = new std::vector<SessionData *>;
}

WebServer::WebServer(const WebServer &other) // problemático pois compartilha ponteiros
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
		delete _sessions;
	if (_sessions)
	{
		for (size_t i = 0; i < _sessions->size(); ++i)
			delete (*_sessions)[i];
		_sessions->clear();
		delete _sessions;
	}
}

int WebServer::initEpoll(void)
{
	// Cria o socket epoll mantido pelo kernel para armazenar o conjunto de descritores a serem monitorados
	_epoll_fd = epoll_create(1);
	if (_epoll_fd == -1)
	{
		_logger << "Erro ao criar epoll";
		printLogNew(_logger, RED, std::cerr, true);
		return -1;
	}

	// pensar numa forma de modularizar e reutilizar esse trecho de codigo abaixo
	// Pegar as flags do _epoll_fd e adicionar a flag FD_CLOEXEC
	int epoll_flags = fcntl(_epoll_fd, F_GETFD);
	if (epoll_flags == -1)
	{
		_logger << "Erro ao obter flags do _epoll_fd";
		printLogNew(_logger, RED, std::cerr, true);
		close(_epoll_fd);
		return -1;
	}
	epoll_flags |= FD_CLOEXEC;
	epoll_flags |= O_NONBLOCK;
	if (fcntl(_epoll_fd, F_SETFD, epoll_flags) == -1)
	{
		_logger << "Erro ao definir flags do _epoll_fd";
		printLogNew(_logger, RED, std::cerr, true);
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
		_logger << "Erro ao adicionar o socket ao epoll";
		printLogNew(_logger, RED, std::cerr, true);
		freeaddrinfo(socket->getRes());
		return;
	}
}

// ### HANDLE NEW CLIENT ###
void WebServer::handleNewClient(int server_fd)
{
	Client *client_socket = new Client(server_fd);
	if (!client_socket)
	{
		_logger << "Erro ao inicializar o socket do cliente";
		printLogNew(_logger, RED, std::cerr, true);
		return;
	}
	_clients_vec.push_back(client_socket);

	Server *server = _servers_map.find(server_fd)->second;
	_client_to_server_map[client_socket->getSocketFd()] = std::make_pair(server->getIp(), server->getPort()); // novidade, possível approach
	client_socket->_server = server;
	_logger << "Novo cliente conectado - client_fd: " << client_socket->getSocketFd();
	printLogNew(_logger, WHITE, std::cout, true);
	registerEpollSocket(client_socket);
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
		try
		{
			handleNewClient(_events[i].data.fd);
			return true; // Conexão aceita com sucesso
		}
		catch (const std::exception &e)
		{
			_logger << "Error parsing HTTP request: " << e.what();
			printLogNew(_logger, RED, std::cerr, true);
			return true;
		}
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
	std::cout << GREEN << newData << RESET << std::endl;
	if (newData.empty())
		return -1;
	Configuration *config = findConfigForRequestFast(newData, client_fd);
	if (!config)
	{
		_logger << "Configuração não encontrada para cliente " << client_fd;
		printLog(_logger.str(), RED, std::cout);
		_logger.str("");
		_logger.clear();
		_partial_requests.erase(client_fd);
		return -1;
	}
	try
	{
		// encontra o client
		Client *client = findClient(client_fd, _clients_vec);
		// verifica se o client tem req
		if (client->_request != NULL)
		{
			if (client->_request->RequestCompleted())
				client->setFirstRequest(true);
			if (client->getFirstRequest())
			{
				delete client->_request;
				client->_request   = new HttpRequest(newData, config, _sessions);
				client->setFirstRequest(false);
			}
			else
				client->_request->parse(newData);
			if (!client->_request->RequestCompleted())
				return (0);
		}
		else {
			client->_request = new HttpRequest(newData, config, _sessions);
			if (!client->_request->RequestCompleted())
				return (0);
		}
	}
	catch (const std::exception &e)
	{
		_partial_requests.erase(client_fd);
		_logger << "Error parsing HTTP request: " << e.what();
		printLog(_logger.str(), RED, std::cout);
		_logger.str("");
		_logger.clear();
		return -1;
	}
	return (1);
}
static bool sendResponseToClient(Client *client)
{
	std::stringstream _logger;
	client->_response->setResponse(client->_response->checkStatusCode());
	const char *buf = client->_response->getResponse().c_str();
	size_t size = client->_response->getResponse().size();
	size_t totalSent = client->_response->getFilePos();
	int bytesToSend = size - totalSent;
	std::cout << "RESPONSE STATUS >> " << client->_response->getResponse() << std::endl;
	int sentBytes = send(client->getSocketFd(), buf + totalSent, bytesToSend, 0);
	std::cout << YELLOW << "A ENVIAR: " << buf << RESET << std::endl;
	if (sentBytes < 0)
	{
		_logger << "Erro ao enviar corpo ao cliente - client_fd: " << client->getSocketFd();
		printLog(_logger.str(), RED, std::cout);
		_logger.str("");
		_logger.clear();
		return true;
	}
	totalSent += sentBytes;

	_logger << "Dados enviados ao cliente - client_fd: " << client->getSocketFd() << " " << totalSent;
	printLog(_logger.str(), WHITE, std::cout);
	_logger.str("");
	_logger.clear();
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
			
			_logger << "Cliente desconectado - client_fd: " << (*it)->getSocketFd();
			printLog(_logger.str(), RED, std::cout);
			_logger.str("");
			_logger.clear();
			if (event == 0)
			{
				int x;
				x = epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
				if (x == -1)
				{
					_logger << "ERRO AO REMOVER DA EPOLL";
					printLog(_logger.str(), RED, std::cerr);	
					_logger.str("");
					_logger.clear();
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

void WebServer::treatExistingClient(int i)
{
	Client *client = findClient(_events[i].data.fd, _clients_vec);

	if (!client)
		return ;

	if (_events[i].events == EPOLLIN)
		handleClientInput(client, i);
	else if (_events[i].events == EPOLLOUT)
		handleClientOutput(client, i);
	else if (_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
		// Cliente desconectou ou houve erro
		_logger << "Conexão fechada ou erro na conexão com o cliente - client_fd: " << client->getSocketFd();
		printLog(_logger.str(), RED, std::cout);
		_logger.str("");
		_logger.clear();
		deleteClient(_events[i].data.fd, 1);
	}
}
void WebServer::handleEvents(int event_count)
{
	for (int i = 0; i < event_count; ++i)
	{
		// verifica se o evento corresponde a um server (conexao nova)
		bool server_found = tryConnection(i);
		if (!server_found)
		{
			treatExistingClient(i);
		}
	}
}

void WebServer::lookForTimeouts()
{
	// Verificar se o cgi esta em execucao e matar o processo caso positivo
	std::vector<Client *>::iterator it = _clients_vec.begin();
	while (it != _clients_vec.end())
	{
		if ((*it)->checkTimeout())
		{
			_logger << "Client timeout detected - client_fd: " << (*it)->getSocketFd();
			printLog(_logger.str(), RED, std::cout);
			_logger.str("");
			_logger.clear();
			int fd = (*it)->getSocketFd();
			// mata o processo cgi caso exista e esteja em execucao
			if ((*it)->getProcessingState() == CGI_PROCESSING)
				(*it)->_response->terminateCgiProcess();
			else if ((*it)->getProcessingState() == RECEIVING_LARGE)
				(*it)->_response->setResStatus(408); // Request Timeout
			else
				(*it)->_response->setResStatus(503); // Request Timeout
				// (*it)->_response->terminateLargeUpload();
			sendResponseToClient(*it); // envia a resposta antes de deletar o cliente
			deleteClient(fd, 0);
			it = _clients_vec.begin(); // Reinicia a iteração
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
			_logger << "Session timeout reached - sessionId " << (*sit)->getSessionId();
			printLog(_logger.str(), RED, std::cout);
			_logger.str("");
			_logger.clear();
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
	while (true)
	{
		// Espera por novos eventos
		int event_count = epoll_wait(_epoll_fd, _events, MAX_EVENTS, 100);
		if (event_count == -1)
		{
			std::cerr << "Erro no epoll_wait" << std::endl;
			return;
		}

		try
		{
			handleEvents(event_count);
			//lookForTimeouts();
		}
		catch (const std::exception &e)
		{
			// std::cerr << "Internal Server Error: " <<  e.what() << '\n';
			std::cerr << e.what() << '\n';
		}
	}
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
	{
		if (host == *name_it)
		{
			std::cout << "Server name encontrado: " << *name_it << std::endl;
			return true;
		}
	}
	return false;
}

static Configuration *lookForConfigurations(bool numeric_host, std::string host,
											std::map<int, std::pair<std::string, std::string> >::const_iterator client_it,
											std::vector<Configuration> &configs)
{
	Configuration *defaultConfig = NULL;
	// Procurar configuração que corresponde ao host e servidor
	for (std::vector<Configuration>::iterator config_it = configs.begin(); config_it != configs.end(); ++config_it)
	{
		const std::set<std::pair<std::string, std::string> > &hosts = config_it->getHost();
		std::set<std::pair<std::string, std::string> >::iterator host_it = hosts.find(client_it->second); // Verifica se o pair é encontrado no set de hosts
		if (host_it == hosts.end())
			continue;
		// entender first como ip e second como porta
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


void WebServer::handleClientInput(Client *client, int i)
{
	if (client->getProcessingState() == RECEIVING)
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
			client->setProcessingState(PROCESSING);
			_events[i].events = EPOLLOUT;
			epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]);
			setClientTime(_events[i].data.fd);
		}
	}
	else
	{
		_logger << "AVISO: Cliente " << client->getSocketFd() << " com estado " << client->getProcessingState() << " recebeu EPOLLIN (esperado: RECEIVING)";
		printLogNew(_logger, RED, std::cerr, true);
		client->setProcessingState(RECEIVING);
	}
}

void WebServer::handleClientOutput(Client *client, int i)
{
	
	switch (client->getProcessingState())
	{
	case PROCESSING:
		_logger << "WebServer >> handleClientOutput >> starting response PROCESSING for client_fd: " << client->getSocketFd();
		printLogNew(_logger, CYAN, std::cout, true);
		sendData(_events[i].data.fd);
		if (client->getProcessingState() == PROCESSING)
			client->setProcessingState(SEND_DATA);
		else if (client->getProcessingState() == STREAMING)
		{
			_logger << "WebServer >> handleClientOutput >> starting reponse STREAMING for client_fd: " << client->getSocketFd();
			printLogNew(_logger, CYAN, std::cout, true);
			sendResponseToClient(client); // this case will only send headers to warn the client about the content type and size of the req file
		}
		break;

	case STREAMING: // Streaming of large files
		if (!continueLargeFileStreaming(client))
		{
			_logger << "WebServer >> handleClientOutput >> response streaming finished for client_fd: " << client->getSocketFd();
			printLogNew(_logger, CYAN, std::cout, true);
			_events[i].events = EPOLLIN;
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]) == 0)
				client->setProcessingState(RECEIVING);
			else
			{
				_logger << "Erro ao modificar evento do cliente " << client->getSocketFd() << " para EPOLLIN";
				printLogNew(_logger, RED, std::cerr, true);
				deleteClient(_events[i].data.fd, 0);
				return ;
			}
			client->setTime(std::time(NULL));
		}
		break;

	case COMPLETED:
			_logger << "WebServer >> handleClientOutput >> request treatment COMPLETED for client_fd: " << client->getSocketFd();
			printLogNew(_logger, CYAN, std::cout, true);
			// sendResponseToClient(client);
			_events[i].events = EPOLLIN;
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]) == 0)
				client->setProcessingState(RECEIVING);
			else
			{
				_logger << "Erro ao modificar evento do cliente " << client->getSocketFd() << " para EPOLLIN";
				printLogNew(_logger, RED, std::cerr, true);
				deleteClient(_events[i].data.fd, 0);
				return ;
			}
			client->setTime(std::time(NULL));
			break;

	case SEND_DATA:
			_logger << "WebServer >> hanldeClientOutput >> sending data to client_fd: " << client->getSocketFd();
			printLogNew(_logger, CYAN, std::cout, true);
			if (sendResponseToClient(client))
				client->setProcessingState(COMPLETED);
			break;

	case CGI_PROCESSING:
		// client->_response->startResponse();
		client->_response->checkCgiProcess();
		if (client->getProcessingState() == CGI_COMPLETED)
		{
			// client->_response->setResponse(client->_response->checkStatusCode()); // garante que o response esta atualizado
			client->setProcessingState(SEND_DATA);
		}
		break;

	case RECEIVING: // apenas para testar, pois se o event eh EPOLLOUT, o estado deve ser COMPLETED
		_logger << "Warning: EPOLLOUT para cliente em estado RECEIVING";
		printLogNew(_logger, RED, std::cerr, true);
		break;

	default:
		_logger << "Estado inválido: " << client->getProcessingState() << " para cliente " << client->getSocketFd();
		printLogNew(_logger, RED, std::cerr, true);
		client->setProcessingState(COMPLETED);
		break;
	}
}

// std::streamsize HttpResponse::readFileChunk(char *buffer, std::streamsize size)
// {
//     _file.read(buffer, size);
//     return _file.gcount();
// }
bool WebServer::continueLargeFileStreaming(Client *client)
{
    const size_t CHUNK_SIZE = 8192;
    char buffer[CHUNK_SIZE];

    std::ifstream &file = client->_response->getFileStream();

    if (!file.is_open()) {
		_logger << "WebServer >> continueLargeFileStreaming >> Error while openning file for client_fd: " << client->getSocketFd() ;
		printLogNew(_logger, RED, std::cerr, true);
        return false;
    }

    file.read(buffer, CHUNK_SIZE);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead <= 0) {
        if (file.bad())
		{
			_logger << "WebServer >> continueLargeFileStreaming >> Error while reading file for client_fd: " << client->getSocketFd() ;
			printLogNew(_logger, RED, std::cerr, true);
		}
        client->resetFileStreaming();
        return false;
    }

    ssize_t bytesSent = send(client->getSocketFd(), buffer, bytesRead, 0);
    if (bytesSent <= 0) {
		_logger << "WebServer >> continueLargeFileStreaming >> Error while sending data for client_fd: " << client->getSocketFd() ;
		printLogNew(_logger, RED, std::cerr, true);
        client->resetFileStreaming();
        return false;
    }

    // Atualiza o progresso total do arquivo (pode ser menor que bytesRead)
    size_t newFilePos = client->_response->getFilePos() + bytesSent;
    client->_response->setFilePos(newFilePos);

    return (newFilePos < client->_response->getContentLength());
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
