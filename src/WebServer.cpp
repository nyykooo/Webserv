/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:19 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/23 18:47:51 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"\

WebServer::WebServer()
{
	// THIS CLASS WILL NEVER BE DEFAULT CONSTRUCTED
}	

WebServer::WebServer(const std::vector<Configuration> conf) : _configurations(conf)
{
	initEpoll();
	_events = new struct epoll_event[MAX_EVENTS];

	// _events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo
	_events = new struct epoll_event[MAX_EVENTS]; // Aloca memória para o vetor de eventos

	std::set<std::pair<std::string, std::string> >::const_iterator it;
	for (it = conf[0].getAllHosts().begin(); it != conf[0].getAllHosts().end(); ++it) 
	{
		Server *server = new Server(it->first, it->second);
		if (server) {
			registerEpollSocket(server);
			_servers_map[server->getSocketFd()] = server; // Armazena o servidor no mapa usando o socket fd como chave
			std::stringstream ss;
			ss << "Servidor iniciado no Ip/Port: " << server->getIp() << "/" << server->getPort();
			printLog(ss.str(), WHITE);
		} else {
			std::stringstream ss;
			ss << "Erro ao inicializar o servidor na porta: " << it->first;
			throw WebServerErrorException(ss.str());
		}
	}
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
	for (std::vector<Client *>::iterator it = _clients_vec.begin(); it != _clients_vec.end(); ++it) {
		close((*it)->getSocketFd());
		delete *it;
	}
	_clients_vec.clear();

	_client_to_server_map.clear();

	for (std::map<int, Server*>::iterator it = _servers_map.begin(); it != _servers_map.end(); ++it) {
		close(it->second->getSocketFd());
		delete it->second;
	}
	_servers_map.clear();

	if (_events)
		delete[] _events;
	if (_epoll_fd != -1)
		close(_epoll_fd);
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
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket->getSocketFd(), &socket->getEvent()) == -1) {
		std::cerr << "Erro ao adicionar o socket ao epoll" << std::endl;
		freeaddrinfo(socket->getRes());
		return ;
	}
}

// ### HANDLE NEW CLIENT ###
void	WebServer::handleNewClient(int server_fd)
{
	// inicializa o socket do cliente
	Client *client_socket = new Client(server_fd);
	if (!client_socket) { // new nunca retorna NULL em c++?, verificar
		std::cerr << "Erro ao inicializar o socket do cliente" << std::endl;
		return ;
	}
	_clients_vec.push_back(client_socket);

	Server *server = _servers_map.find(server_fd)->second;
	_client_to_server_map[client_socket->getSocketFd()] = std::make_pair(server->getIp(), server->getPort()); // novidade, possível approach

	std::stringstream ss;
	ss << "Novo cliente conectado - client_fd: " << client_socket->getSocketFd();
	printLog(ss.str(), WHITE);
	registerEpollSocket(client_socket);
}

// ### GETTERS ###
int WebServer::getEpollFd() const {
	return _epoll_fd;
}

std::map<int, Server *> WebServer::getServersMap() const {
	return _servers_map;
}

std::vector<Client *> WebServer::getClientsVec() const {
	return _clients_vec;
}

struct epoll_event *WebServer::getEvents() const {
	return _events;
}

char *WebServer::getBuffer() {
	return _buffer;
}

const std::vector<Configuration>& WebServer::getConfigurations() const
{
	return _configurations;
}

// ### SETTERS ###
void WebServer::setEpollFd(int epoll_fd) {
	_epoll_fd = epoll_fd;
}

void WebServer::setServersMap(const std::map<int, Server *> &servers_map) {
	_servers_map = servers_map;
}

void WebServer::setClientsVec(const std::vector<Client *> &clients_vec) {
	_clients_vec = clients_vec;
}

void WebServer::setEvents(struct epoll_event *events) {
	_events = events;
}

void WebServer::setBuffer(const char *buffer) {
	if (buffer) {
		strncpy(_buffer, buffer, BUFFER_SIZE - 1);
		_buffer[BUFFER_SIZE - 1] = '\0'; // Garante que o buffer esteja null-terminated
	} else {
		_buffer[0] = '\0'; // Limpa o buffer se o ponteiro for nulo
	}
}

// ### TESTANDO STARTSERVER DENTRO DA WEBSERVER ###
bool WebServer::tryConnection(int i)
{
    if(_servers_map.find(_events[i].data.fd) != _servers_map.end())
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

int WebServer::receiveData(int client_fd)
{
    ssize_t bytes = recv(client_fd, _buffer, BUFFER_SIZE - 1, 0);
    // se bytes for -1 significa que houve um erro
    if (bytes == -1)
    {
        return -1;
    }
    // se bytes for 0 significa que houve desconexao
    else if (bytes == 0)
    {
        return -1;
    }
    else
    {
        // lidar com a leitura e envio de resposta
		_buffer[bytes] = '\0';  // Garante null-termination
        std::string requestBuffer(_buffer);

		try
		{
			std::vector<Client *>::iterator it;
			for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it) {
				std::cout << "client_fd: " << client_fd << " socket_fd: " << (*it)->getSocketFd() << std::endl;
				if ((*it)->getSocketFd() == client_fd)
					break ;
			}
			HttpRequest request(requestBuffer);
			Configuration* config = findConfigForRequest(request, client_fd);
			if (!config)
			{
				std::cerr << "Configuração não encontrada para servidor" << std::endl;
				return -1;
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error parsing HTTP request: " << e.what() << std::endl;
			return 1; 
		}
    }
    return 1;
}

void WebServer::sendData(int client_fd)
{
    // resposta padrao
    std::string responseBody = 
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>Bem-vindo</title>"
    "<style>"
    "body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
    "h1 { color: #333; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Bem-vindo ao WebServr de ncampbel, bruhenr dioalexa</h1>"
    "</body>"
    "</html>";
    
    std::ostringstream header;
    header << "HTTP/1.1 200 OK\r\n";
    header << "Content-Type: text/html\r\n";
    header << "Content-Length: " << responseBody.size() << "\r\n";
    header << "\r\n";

    std::string response = header.str() + responseBody;
	std::vector<Client *>::iterator it;
	for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it) {
		std::cout << "client_fd: " << client_fd << " socket_fd: " << (*it)->getSocketFd() << std::endl;
		if ((*it)->getSocketFd() == client_fd)
			break ;
	}
	(*it)->sendResponse = new HttpResponse;
	(*it)->sendResponse->setResponse(response);
	//std::cout << (*it)->sendResponse->getResponse() << std::endl;

	// std::cerr << RED << "||" << (*it)->sendResponse.getResponse() << "||" << RESET << std::endl;

    // envia a resposta ao cliente
	const char *buf = (*it)->sendResponse->getResponse().c_str();
	size_t size = (*it)->sendResponse->getResponse().size();
    int sent = send(client_fd, buf, size, 0);
	//int sent = send(client_fd, response.c_str(), response.size(), 0);
    if (sent == -1)
    {
        return;
    }
    else
	{
		std::stringstream ss;
		ss << "Dados enviados ao cliente - client_fd: " << client_fd;
		printLog(ss.str(), WHITE);
	}
    
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
			printLog(ss.str(), RED);
			close((*it)->getSocketFd());
			delete *it; // Libera a memória do cliente
			it = _clients_vec.erase(it); // Remove o cliente do vetor
			epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);

			_client_to_server_map.erase(fd); // para remover o elemento do mapeamento
			return;
		}
	}
}

void WebServer::treatExistingClient(int i)
{
	if (_events[i].events == EPOLLIN)
	{
		int data = receiveData(_events[i].data.fd);
		if (data == -1)
        {
            deleteClient(_events[i].data.fd);
            return;
        }
        _events[i].events = EPOLLOUT; // Muda o evento para EPOLLOUT após receber dados
        epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]);
        setClientTime(_events[i].data.fd);
    }
    else if (_events[i].events == EPOLLOUT)
    {
        sendData(_events[i].data.fd);
        _events[i].events = EPOLLIN; // Muda o evento de volta para EPOLLIN após enviar dados
        epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _events[i].data.fd, &_events[i]);
        setClientTime(_events[i].data.fd);
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
        // else
        //     break; // não é necessário remover esse break?
    }
}

// void WebServer::lookForTimeouts()
// {
//     std::vector<Client *>::iterator it;
//     for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
//     {
//         if ((*it)->checkTimeout())
//         {
//             deleteClient((*it)->getSocketFd()); // Modifica o vetor durante iteração?
//         }
//         if (it == _clients_vec.end()) // Nunca será verdadeiro
//             break;
//     }
// }

void WebServer::lookForTimeouts()
{
    std::vector<Client *>::iterator it = _clients_vec.begin();
    while (it != _clients_vec.end())
    {
        if ((*it)->checkTimeout())
        {
            int fd = (*it)->getSocketFd();
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
		if (event_count == -1) {
			std::cerr << "Erro no epoll_wait" << std::endl;
			return;
		}

        try
        {
            handleEvents(event_count);
            lookForTimeouts();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

// ### FIND CONFIG FOR REQUEST ###

static bool checkHostType(const std::string &host)
{
	size_t colonPos = host.find_first_of(':');
	if (colonPos != std::string::npos)
		return true;
	else
		return false;
}

static std::string getHostFromRequest(const HttpRequest& request)
{
	std::map<std::string, std::string>::const_iterator host_it = request.getHeaders().find("Host");
	if (host_it == request.getHeaders().end())
		return NULL; // verificar esse handle
	else
		return host_it->second;
}

static bool checkForServerName(const std::vector<std::string> & server_names, std::string host)
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
std::vector<Configuration> configs)
{
	Configuration* defaultConfig = NULL;
    // Procurar configuração que corresponde ao host e servidor
	for (std::vector<Configuration>::iterator config_it = configs.begin(); config_it != configs.end(); ++config_it)
	{
		const std::set<std::pair<std::string, std::string> >& hosts = config_it->getHost();
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

Configuration* WebServer::findConfigForRequest(const HttpRequest& request, int client_fd)
{
	// pega o header host da request
	std::string host = getHostFromRequest(request);

	// verifica o tipo do host obtido
	bool numeric_host = checkHostType(host);
	
	// Pegar o server pelo client_fd
	std::map<int, std::pair<std::string, std::string> >::const_iterator client_it = _client_to_server_map.find(client_fd);
	
	// obtem a configuracao baseado nos valores extraidos
	return (lookForConfigurations(numeric_host, host, client_it, _configurations));
}

// ### EXCEPTION ###

const char* WebServer::WebServerErrorException::what() const throw() {
	return (_message.c_str());
}
