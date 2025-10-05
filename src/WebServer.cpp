/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:19 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/05 00:53:52 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	//_events = new struct epoll_event[MAX_EVENTS];

	// _events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo
	_events = new struct epoll_event[MAX_EVENTS]; // Aloca memória para o vetor de eventos

	std::set<std::pair<std::string, std::string> >::const_iterator it;
	for (it = _configurations[0].getAllHosts().begin(); it != conf[0].getAllHosts().end(); ++it)
	{
		Server *server = new Server(it->first, it->second);
		if (server)
		{
			registerEpollSocket(server);
			_servers_map[server->getSocketFd()] = server; // Armazena o servidor no mapa usando o socket fd como chave
			std::stringstream ss;
			ss << "Servidor iniciado no Ip/Port: " << server->getIp() << "/" << server->getPort();
			printLog(ss.str(), WHITE, std::cout);
		}
		else
		{
			std::stringstream ss;
			ss << "Erro ao inicializar o servidor na porta: " << it->first;
			throw WebServerErrorException(ss.str());
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
		std::cerr << "Erro ao criar epoll" << std::endl;
		return -1;
	}

	// pensar numa forma de modularizar e reutilizar esse trecho de codigo abaixo
	// Pegar as flags do _epoll_fd e adicionar a flag FD_CLOEXEC
	int epoll_flags = fcntl(_epoll_fd, F_GETFD);
	if (epoll_flags == -1)
	{
		std::cerr << "Erro ao obter flags do _epoll_fd" << std::endl;
		close(_epoll_fd);
		return -1;
	}
	epoll_flags |= FD_CLOEXEC;
	epoll_flags |= O_NONBLOCK;
	if (fcntl(_epoll_fd, F_SETFD, epoll_flags) == -1)
	{
		std::cerr << "Erro ao definir flags do _epoll_fd" << std::endl;
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
		std::cerr << "Erro ao adicionar o socket ao epoll" << std::endl;
		freeaddrinfo(socket->getRes());
		return;
	}
}

// ### HANDLE NEW CLIENT ###
void WebServer::handleNewClient(int server_fd)
{
	// inicializa o socket do cliente
	Client *client_socket = new Client(server_fd);
	if (!client_socket)
	{ // new nunca retorna NULL em c++?, verificar
		std::cerr << "Erro ao inicializar o socket do cliente" << std::endl;
		return;
	}
	_clients_vec.push_back(client_socket);

	Server *server = _servers_map.find(server_fd)->second;
	_client_to_server_map[client_socket->getSocketFd()] = std::make_pair(server->getIp(), server->getPort()); // novidade, possível approach
	client_socket->_server = server;
	std::stringstream ss;
	ss << "Novo cliente conectado - client_fd: " << client_socket->getSocketFd();
	printLog(ss.str(), WHITE, std::cout);
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

char *WebServer::getBuffer()
{
	return _buffer;
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

void WebServer::setBuffer(const char *buffer)
{
	if (buffer)
	{
		strncpy(_buffer, buffer, BUFFER_SIZE - 1);
		_buffer[BUFFER_SIZE - 1] = '\0'; // Garante que o buffer esteja null-terminated
	}
	else
	{
		_buffer[0] = '\0'; // Limpa o buffer se o ponteiro for nulo
	}
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
			std::cerr << "Error parsing HTTP request: " << e.what() << std::endl;
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
	{
		pos++;
	}

	size_t start = pos;

	while (pos < headers_lower.length() && std::isdigit(headers_lower[pos]))
	{
		pos++;
	}

	if (pos == start)
	{
		return -1;
	}
	std::string length_str = headers_lower.substr(start, pos - start);
	return std::atoi(length_str.c_str());
}

int WebServer::receiveData(int client_fd)
{
	ssize_t bytes = recv(client_fd, _buffer, BUFFER_SIZE - 1, 0);
	if (bytes <= 0)
		return -1;

	_buffer[bytes] = '\0';
	std::string newData(_buffer);

	// Proteção contra DOS
	if (_partial_requests[client_fd].size() + newData.size() > MAX_ABSOLUTE_REQUEST_SIZE)
	{
		// Criar um erro 413 "Request entity too large"
		_partial_requests.erase(client_fd);
		return -1;
	}

	_partial_requests[client_fd] += newData;

	// VALIDATION
	size_t header_end = _partial_requests[client_fd].find("\r\n\r\n");
	if (header_end != std::string::npos)
	{
		// is post?
		bool isPost = _partial_requests[client_fd].rfind("POST ", 0) == 0;

		if (isPost)
		{
			std::string headers = _partial_requests[client_fd].substr(0, header_end);
			std::string headers_lower = toLower(headers);
			Configuration *config = findConfigForRequestFast(_partial_requests[client_fd], client_fd);

			// Content-Length é obrigatório para POST
			if (headers_lower.find("content-length:") == std::string::npos)
			{
				// Criar um 411 Lenght is required
				_partial_requests.erase(client_fd);
				return -1;
			}

			// Verificar limites de Content-Length
			int cl = extractContentLength(headers_lower);

			// Validar contra limite da configuração
			if (config && cl > config->getRequestSize())
			{
				// Criar um 413 Content-Length exceeds server limit
				_partial_requests.erase(client_fd);
				return -1;
			}

			if (cl > MAX_MEMORY_FILE_SIZE)
			{
				Client *client = findClient(client_fd, _clients_vec);
				if (!client)
				{
					_partial_requests.erase(client_fd);
					return -1;
				}
				return startLargePostUpload(client, header_end + 4, cl);
			}
		}
	}

	if (!isRequestComplete(_partial_requests[client_fd]))
		return 0; // Aguarda mais dados

	Configuration *config = findConfigForRequestFast(_partial_requests[client_fd], client_fd);
	if (!config)
	{
		std::stringstream ss;
		ss << "Configuração não encontrada para cliente " << client_fd;
		printLog(ss.str(), RED, std::cout);
		_partial_requests.erase(client_fd);
		return -1;
	}
	HttpRequest *request = NULL;
	try
	{
		request = new HttpRequest(_partial_requests[client_fd], config, _sessions);
	}
	catch (const std::exception &e)
	{
		_partial_requests.erase(client_fd);
		std::stringstream ss;
		ss << "Error parsing HTTP request: " << e.what();
		printLog(ss.str(), RED, std::cout);
		return -1;
	}

	_partial_requests.erase(client_fd);

	Client *client = findClient(client_fd, _clients_vec);
	if (client->_request != NULL)
		delete client->_request;

	client->_request = request;
	return (1);
}
static void sendResponseToClient(Client *client)
{
	std::stringstream ss;
	client->_response->setResponse(client->_response->checkStatusCode());
	const char *buf = client->_response->getResponse().c_str();
	size_t size = client->_response->getResponse().size();
	size_t totalSent = 0;

	while (totalSent < size)
	{
		int sent = send(client->getSocketFd(), buf + totalSent, size - totalSent, 0);
		if (sent < 0)
		{
			std::cout << RED << "errno: " << strerror(errno) << RESET << std::endl;
			ss << "Erro ao enviar corpo ao cliente - client_fd: " << client->getSocketFd();
			printLog(ss.str(), RED, std::cout);
			return;
		}
		totalSent += sent;
	}
	ss << "Dados enviados ao cliente - client_fd: " << client->getSocketFd();
	printLog(ss.str(), WHITE, std::cout);
}

void WebServer::sendData(int client_fd)
{
	Client *client = findClient(client_fd, _clients_vec);

	if (client->_response)
		delete client->_response;

	client->_response = new HttpResponse(client); // mudar esse construtor para um metodo para evitar multiplas alocacoes de memoria aqui (pode dar problemas)
	// [NCC] a ideia eh inicializar o httpresponse antes, nao executar a logica dele, para assim poder rodar a isLarge sem duplicar a execucao de logica do HttpResponse
	if (isLargeFileRequest(client)) // verifica se é um arquivo grande
	{
		client->setProcessingState(PROCESSING_LARGE); // Mudamos o estado para PROCESSING_LARGE para ser tratado no próximo ciclo
		return;
	}
	client->_response->startResponse(); // Executa a lógica de resposta do HttpResponse

	// client->setProcessingState(PROCESSING); // Comportamento atual // Precisa desse set de processing? o case para entrar na sendData ja eh processing
	// const char *buf = client->_response->getResponse().c_str(); // HttpResponse poderia ter um metodo para ter um buffer em const char * e outro size_t
	// size_t size = client->_response->getResponse().size();
	// int sent = send(client_fd, buf, size, 0);
	// if (sent == -1)
	// {
	// 	ss << "Erro ao enviar dados ao cliente - client_fd: " << client_fd;
	// 	printLog(ss.str(), RED);
	// 	return;
	// }
	// else
	// {
	// 	ss << "Dados enviados ao cliente - client_fd: " << client_fd;
	// 	printLog(ss.str(), WHITE);
	// }
	// client->setProcessingState(COMPLETED);
}

void WebServer::setClientTime(int client_fd)
{
	std::vector<Client *>::iterator it;

	for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
	{
		if ((*it)->getSocketFd() == client_fd)
		{
			(*it)->setTime(std::time(NULL));
			return;
		}
	}
}

void WebServer::deleteClient(int fd)
{
	std::vector<Client *>::iterator it;
	for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
	{
		if ((*it)->getSocketFd() == fd)
		{
			std::stringstream ss;
			ss << "Cliente desconectado - client_fd: " << (*it)->getSocketFd();
			printLog(ss.str(), RED, std::cout);
			int x;
			x = epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
			if (x == -1)
				std::cerr << "ERRO AO REMOVER DA EPOLL" << std::endl;
			close((*it)->getSocketFd());
			delete *it;					 // Libera a memória do cliente
			it = _clients_vec.erase(it); // Remove o cliente do vetor

			_client_to_server_map.erase(fd); // para remover o elemento do mapeamento
			_partial_requests.erase(fd);	 // para remover o elemento do buffer
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
			std::stringstream ss;
			ss << "Client timeout detected - client_fd: " << (*it)->getSocketFd();
			printLog(ss.str(), RED, std::cout);
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
			deleteClient(fd);
			it = _clients_vec.begin(); // Reinicia a iteração
		}
		else
		{
			++it;
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
			lookForTimeouts();
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
bool WebServer::isRequestComplete(const std::string &data)
{
	size_t header_end = data.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return false;

	std::string headers = data.substr(0, header_end);
	std::string headers_lower = toLower(headers); // Converte uma vez

	// Transfer-Encoding: chunked
	if (headers_lower.find("transfer-encoding: chunked") != std::string::npos)
		return data.find("0\r\n\r\n") != std::string::npos;

	// Content-Length
	if (headers_lower.find("content-length:") != std::string::npos)
	{
		int content_length = extractContentLength(headers_lower);
		if (content_length >= 0)
		{
			size_t body_start = header_end + 4;
			return (data.length() >= body_start + content_length);
		}
	}

	return true;
}

void WebServer::handleClientInput(Client *client, int i)
{
	if (client->getProcessingState() == RECEIVING)
	{
		int data = receiveData(_events[i].data.fd);
		if (data == -1)
		{
			deleteClient(_events[i].data.fd);
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
			_events[i].events = EPOLLOUT; // Nos mudamos para o evento de WRITE, quer dizer que o client esta pronto para enviar dados
			epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]);
			setClientTime(_events[i].data.fd);
		}
	}
	else if (client->getProcessingState() == RECEIVING_LARGE)
	{
		ssize_t bytes = recv(client->getSocketFd(), _buffer, BUFFER_SIZE - 1, 0);
		if (bytes <= 0)
		{
			if (client->getUploadFd() != -1)
			{
				close(client->getUploadFd());
				std::remove(client->getUploadPath().c_str());
			}
			deleteClient(client->getSocketFd());
			return;
		}

		int result = continueLargePostUpload(client, _buffer, bytes);
		if (result == 1)
		{
			client->setProcessingState(PROCESSING);
			_events[i].events = EPOLLOUT;
			epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client->getSocketFd(), &_events[i]);
		}
		else if (result == -1)
			deleteClient(client->getSocketFd());

		setClientTime(client->getSocketFd());
	}
	else
	{ // se o mano client esta com EPOLLIN siginidca que ele da pronto pra ser lido. E o status do processamento da req no server eh RECEIVING.
		std::cerr << "AVISO: Cliente " << client->getSocketFd()
				  << " com estado " << client->getProcessingState()
				  << " recebeu EPOLLIN (esperado: RECEIVING)" << std::endl;
		// Reset para estado seguro. Podemos explorar mais.
		client->setProcessingState(RECEIVING);
	}
}

void WebServer::handleClientOutput(Client *client, int i)
{
	std::stringstream ss;
	switch (client->getProcessingState())
	{
	case PROCESSING:
		sendData(_events[i].data.fd);
		if (client->getProcessingState() == PROCESSING)
			client->setProcessingState(COMPLETED);
		break;

	case PROCESSING_LARGE: // Processamento de arquivos grandes
		if (startLargeFileStreaming(client))
			client->setProcessingState(STREAMING);
		else
			client->setProcessingState(COMPLETED);
		break;

	case STREAMING: // Streaming de arquivos grandes
		if (!continueLargeFileStreaming(client))
			client->setProcessingState(COMPLETED);
		break;

	case COMPLETED:
			sendResponseToClient(client);
			_events[i].events = EPOLLIN;
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]) == 0)
				client->setProcessingState(RECEIVING);
			else
			{
				ss << "Erro ao modificar evento do cliente " << client->getSocketFd() << " para EPOLLIN";
				printLog(ss.str(), RED, std::cerr);
				deleteClient(_events[i].data.fd);
				return ;
			}
			client->setTime(std::time(NULL));
			break;

	case CGI_PROCESSING:
		// client->_response->startResponse();
		client->_response->checkCgiProcess();
		if (client->getProcessingState() == CGI_COMPLETED)
		{
			// client->_response->setResponse(client->_response->checkStatusCode()); // garante que o response esta atualizado
			client->setProcessingState(COMPLETED);
		}
		break;

	case RECEIVING: // apenas para testar, pois se o event eh EPOLLOUT, o estado deve ser COMPLETED
		std::cerr << "Warning: EPOLLOUT para cliente em estado RECEIVING" << std::endl;
		break;

	default:
		std::cerr << "Estado inválido: " << client->getProcessingState() << " para cliente " << client->getSocketFd() << std::endl;
		client->setProcessingState(COMPLETED);
		break;
	}
}

bool WebServer::isLargeFileRequest(Client *client)
{
	const HttpRequest *request = client->_request;
	// eh soh para GETs!
	if (request->getMethod() != "GET")
	{
		return false;
	}

	// imitando a handleGet
	std::string newRoot = removeSlashes(client->_response->getConfig().getRoot());
	std::string locPath = removeSlashes(client->_response->getFullPath());
	if (!newRoot.empty())
		newRoot = "/" + newRoot;
	std::string fileName = newRoot + "/" + locPath;

	struct stat st;
	if (stat(fileName.c_str(), &st) == -1)
	{
		// Arquivo não existe - não é large file
		return false;
	}

	// S_ISREG verifica se o arquivo é um "regular file"
	// st.st_mode contém informações sobre o tipo do arquivo

	// Retorna:
	// true  = É arquivo regular (dados normais)
	// false = NÃO é arquivo regular (diretório, link, device, etc.)
	// eh possivel explandir essa macro e fazer de forma manual. Mas nao acredito ser um problema de norme da 42.
	if (!S_ISREG(st.st_mode))
		return false;

	return (st.st_size > MAX_MEMORY_FILE_SIZE);
}

std::string WebServer::getContentType(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos)
	{
		return "application/octet-stream";
	}

	std::string extension = filePath.substr(dotPos + 1);

	if (extension == "html" || extension == "htm")
		return "text/html";
	if (extension == "css")
		return "text/css";
	if (extension == "js")
		return "application/javascript";
	if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	if (extension == "png")
		return "image/png";
	if (extension == "gif")
		return "image/gif";
	if (extension == "pdf")
		return "application/pdf";
	if (extension == "mp4")
		return "video/mp4";
	if (extension == "mp3")
		return "audio/mpeg";
	if (extension == "txt")
		return "text/plain";

	return "application/octet-stream";
}

bool WebServer::startLargeFileStreaming(Client *client)
{
	// Depois encapsular essa logica do file path
	// HttpResponse tempResponse(client->_request, client->_request->_config);
	HttpResponse tempResponse(client);
	std::string newRoot = removeSlashes(tempResponse.getConfig().getRoot());
	std::string locPath = removeSlashes(tempResponse.getFullPath());
	if (!newRoot.empty())
		newRoot = "/" + newRoot;
	std::string fileName = newRoot + "/" + locPath;

	struct stat st;
	if (stat(fileName.c_str(), &st) == -1)
	{
		logStreamingError(client->getSocketFd(), "stat arquivo", fileName);
		return false;
	}

	// aqui a diferenca em relacao ao processamento normal eh que abriremos o file para leitura sequencial. Evitamos aquela copia total para a memoria
	int fd = open(fileName.c_str(), O_RDONLY);
	if (fd < 0)
	{
		logStreamingError(client->getSocketFd(), "abertura arquivo", fileName);
		return false;
	}

	client->setFileFd(fd); // mantem aberto para leitura sequencial e ao fim eh fechado pela resetFileStreaming do client
	client->setFileSize(st.st_size);
	client->setBytesSent(0);

	std::ostringstream info;
	info << "iniciando streaming de " << fileName << " (" << st.st_size << " bytes)";
	logStreamingInfo(client->getSocketFd(), info.str());

	std::ostringstream headers;
	headers << "HTTP/1.1 200 OK\r\n";
	headers << "Content-Type: " << getContentType(fileName) << "\r\n";
	headers << "Content-Length: " << st.st_size << "\r\n";
	headers << "\r\n";

	std::string headerStr = headers.str();
	ssize_t headersSent = send(client->getSocketFd(), headerStr.c_str(), headerStr.length(), 0);

	if (headersSent < 0)
	{
		logStreamingError(client->getSocketFd(), "envio headers");
		client->resetFileStreaming();
		return false;
	}
	else if (headersSent == 0)
	{
		logStreamingError(client->getSocketFd(), "send headers retornou 0", "cliente possivelmente desconectado");
		client->resetFileStreaming();
		return false;
	}
	else if (headersSent < static_cast<ssize_t>(headerStr.length()))
	{
		std::ostringstream warning;
		warning << "headers enviados parcialmente: " << headersSent << "/" << headerStr.length() << " bytes";
		logStreamingInfo(client->getSocketFd(), warning.str());
	}
	else
	{
		logStreamingInfo(client->getSocketFd(), "headers enviados com sucesso");
	}

	return (true);
}

bool WebServer::continueLargeFileStreaming(Client *client)
{
	const size_t CHUNK_SIZE = 8192;
	char buffer[CHUNK_SIZE];

	ssize_t bytesRead = read(client->getFileFd(), buffer, CHUNK_SIZE);
	if (bytesRead <= 0)
	{
		if (bytesRead < 0)
			logStreamingError(client->getSocketFd(), "read");
		else
			logStreamingInfo(client->getSocketFd(), "streaming completo");
		client->resetFileStreaming();
		return false;
	}

	size_t totalSent = 0;
	while (totalSent < static_cast<size_t>(bytesRead))
	{
		ssize_t bytesSent = send(client->getSocketFd(),
								 buffer + totalSent,
								 bytesRead - totalSent,
								 0);

		if (bytesSent <= 0)
		{
			logStreamingError(client->getSocketFd(), "send");
			client->resetFileStreaming();
			return false;
		}
		totalSent += bytesSent;
	}
	client->setBytesSent(client->getBytesSent() + totalSent);
	return (client->getBytesSent() < client->getFileSize());
}

void WebServer::logStreamingError(int client_fd, const std::string &operation, const std::string &details)
{
	std::cerr << "[STREAMING ERROR] Cliente " << client_fd
			  << " - " << operation;
	if (!details.empty())
	{
		std::cerr << ": " << details;
	}
	std::cerr << " (" << strerror(errno) << ")" << std::endl;
}

void WebServer::logStreamingInfo(int client_fd, const std::string &message)
{
	std::cout << "[STREAMING INFO] Cliente " << client_fd
			  << " - " << message << std::endl;
}
// ### EXCEPTION ###
const char *WebServer::WebServerErrorException::what() const throw()
{
	return (_message.c_str());
}

std::string WebServer::extractHostHeaderSimple(const std::string &rawRequest)
{
	size_t header_end = rawRequest.find("\r\n\r\n");
	if (header_end == std::string::npos)
	{
		return "";
	}
	std::string headers = rawRequest.substr(0, header_end);
	std::string headers_lower = toLower(headers);
	size_t host_pos = headers_lower.find("host:");
	if (host_pos == std::string::npos)
	{
		return "";
	}
	size_t pos = host_pos + 5;
	while (pos < headers_lower.length() && (headers_lower[pos] == ' ' || headers_lower[pos] == '\t'))
	{
		pos++;
	}
	size_t start = pos;
	while (pos < headers.length() && headers[pos] != '\r' && headers[pos] != '\n')
	{
		pos++;
	}
	if (pos == start)
	{
		return "";
	}
	std::string host = headers.substr(start, pos - start);
	while (!host.empty() && (host[host.length() - 1] == ' ' || host[host.length() - 1] == '\t'))
	{
		host.erase(host.length() - 1);
	}
	return host;
}

int WebServer::startLargePostUpload(Client *client, size_t body_start, int content_length)
{
	Configuration *config = findConfigForRequestFast(_partial_requests[client->getSocketFd()], client->getSocketFd());
	if (!config)
	{
		_partial_requests.erase(client->getSocketFd());
		return -1;
	}

	std::string uploadDir = config->getUploadDirectory();
	if (uploadDir.empty())
		uploadDir = "/tmp/webserv_uploads";

	struct stat st;
	if (stat(uploadDir.c_str(), &st) == -1)
	{
		if (mkdir(uploadDir.c_str(), 0755) == -1)
		{
			_partial_requests.erase(client->getSocketFd());
			return -1;
		}
	}
	else if (!S_ISDIR(st.st_mode))
	{
		_partial_requests.erase(client->getSocketFd());
		return -1;
	}

	std::ostringstream tempname;
	tempname << uploadDir << "/upload_" << time(NULL) << "_" << rand() % 10000 << ".tmp";
	std::string tempPath = tempname.str();

	int uploadFd = open(tempPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (uploadFd == -1)
	{
		_partial_requests.erase(client->getSocketFd());
		return -1;
	}

	std::string originalHeaders = _partial_requests[client->getSocketFd()].substr(0, body_start);
	client->setOriginalHeaders(originalHeaders);
	client->setUploadFd(uploadFd);
	client->setUploadSize(content_length);
	client->setUploadReceived(0);
	client->setUploadPath(tempPath);
	client->setProcessingState(RECEIVING_LARGE);

	std::string &data = _partial_requests[client->getSocketFd()];
	size_t body_length = data.length() - body_start;

	if (body_length > 0)
	{
		const char *body_data = data.c_str() + body_start;
		ssize_t bytes_written = write(uploadFd, body_data, body_length);

		if (bytes_written > 0)
		{
			client->setUploadReceived(client->getUploadReceived() + bytes_written);
		}
	}

	_partial_requests.erase(client->getSocketFd());

	if (client->getUploadReceived() >= client->getUploadSize())
	{
		return finishLargePostUpload(client);
	}

	std::stringstream ss;
	ss << "Iniciando upload grande: " << tempPath
	   << " (" << client->getUploadReceived() << "/"
	   << client->getUploadSize() << " bytes)";
	printLog(ss.str(), WHITE, std::cout);

	return 0; // Continuar recebendo
}

int WebServer::continueLargePostUpload(Client *client, const char *buffer, size_t length)
{
	if (!client || client->getProcessingState() != RECEIVING_LARGE)
	{
		return -1;
	}

	// quanto ainda precisamos receber?
	size_t remaining = client->getUploadSize() - client->getUploadReceived();
	size_t to_write = (length > remaining) ? remaining : length;

	if (to_write > 0)
	{
		ssize_t bytes_written = write(client->getUploadFd(), buffer, to_write);
		if (bytes_written < 0)
		{
			std::stringstream ss;
			ss << "Erro ao escrever no arquivo de upload: " << strerror(errno);
			printLog(ss.str(), RED, std::cout);

			close(client->getUploadFd());
			std::remove(client->getUploadPath().c_str());
			return -1;
		}

		client->setUploadReceived(client->getUploadReceived() + bytes_written);

		std::stringstream ss;
		ss << "Upload progres: " << client->getUploadReceived()
		   << "/" << client->getUploadSize() << " bytes";
		printLog(ss.str(), WHITE, std::cout);
	}

	if (client->getUploadReceived() >= client->getUploadSize())
	{
		printLog("Upload complete, finishing..", WHITE, std::cout);
		return finishLargePostUpload(client);
	}

	return 0; // Continuar recebendo
}

int WebServer::finishLargePostUpload(Client *client)
{
	close(client->getUploadFd());
	client->setUploadFd(-1);

	std::stringstream ss;
	ss << "Upload finalizado: " << client->getUploadPath()
	   << " (" << client->getUploadSize() << " bytes)";
	printLog(ss.str(), GREEN, std::cout);

	// 3. Criar um HttpRequest com informações do upload
	Configuration *config = findConfigForRequestFast("POST / HTTP/1.1\r\nHost: " +
														 client->_server->getIp() + ":" +
														 client->_server->getPort(),
													 client->getSocketFd());
	if (!config)
	{
		std::remove(client->getUploadPath().c_str());
		return -1;
	}

	// 4. Criar um HttpRequest para o upload
	HttpRequest *request = NULL;
	try
	{
		// Cria um request básico com method="POST"
		std::string originalHeaders = client->getOriginalHeaders();
		// Garantir que Content-Length reflete o tamanho real do upload
		size_t clPos = originalHeaders.find("Content-Length:");
		if (clPos != std::string::npos)
		{
			// Encontrar final da linha
			size_t lineEnd = originalHeaders.find("\r\n", clPos);
			if (lineEnd != std::string::npos)
			{
				// Substituir a linha Content-Length
				std::string before = originalHeaders.substr(0, clPos);
				std::string after = originalHeaders.substr(lineEnd);
				std::stringstream newCL;
				newCL << "Content-Length: " << client->getUploadSize();
				originalHeaders = before + newCL.str() + after;
			}
		}

		request = new HttpRequest(originalHeaders, config, _sessions);

		// Adicionar o caminho do arquivo de upload
		request->setUploadPath(client->getUploadPath());
		request->setUploadSize(client->getUploadSize());
	}
	catch (const std::exception &e)
	{
		std::stringstream error_ss;
		error_ss << "Erro ao criar request: " << e.what();
		printLog(error_ss.str(), RED, std::cout);
		std::remove(client->getUploadPath().c_str());
		return -1;
	}

	// 5. Configurar o cliente
	if (client->_request)
	{
		delete client->_request;
	}
	client->_request = request;

	// 6. Mudar para processamento normal
	client->setProcessingState(PROCESSING);

	return 1; // Pronto para processar
}
