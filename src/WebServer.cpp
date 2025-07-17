/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:19 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/16 19:28:30 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"\

WebServer::WebServer() // CORRIGIR DEFAULT CONSTRUCTOR PARA NOVO SERVERS_MAP
{
	initEpoll();

	_events = new struct epoll_event[MAX_EVENTS];

	Server *server1 = new Server("0.0.0.0", "8080");
	Server *server2 = new Server("0.0.0.0", "8081");
	
	_servers_map[server1->getSocketFd()] = server1;
	_servers_map[server2->getSocketFd()] = server2;

	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server1->getSocketFd(), &server1->getEvent());
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server2->getSocketFd(), &server2->getEvent());
}	

WebServer::WebServer(const std::vector<Configuration> conf) : _configurations(conf)
{
	initEpoll();
	_events = new struct epoll_event[MAX_EVENTS];

	// _events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo
	_events = new struct epoll_event[MAX_EVENTS]; // Aloca memória para o vetor de eventos

    std::set<std::pair<std::string, std::string> >::const_iterator it;
    for (it = conf[0].getAllHosts().begin(); it != conf[0].getAllHosts().end(); ++it) {
        Server *server = new Server(it->first, it->second);
        if (server) {
            registerEpollSocket(server);
            _servers_map[server->getSocketFd()] = server; // Armazena o servidor no mapa usando o socket fd como chave
			std::stringstream ss;
			ss << "Servidor iniciado no Ip/Port: " << server->getIp() << "/" << server->getPort();
			printLog(ss.str());
        } else {
            std::cerr << "Erro ao inicializar o servidor na porta: " << it->first << std::endl;
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

	_client_to_server_map[client_socket->getSocketFd()] = server_fd; // novidade, possível approach

	std::stringstream ss;
	ss << "Novo cliente conectado - client_fd: " << client_socket->getSocketFd();
	printLog(ss.str());
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
            return false; 
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

		// Qual servidor recebeu a requisição?
		int server_fd = getServerFdForClient(client_fd);
		if (server_fd == -1) {
			std::cerr << "Servidor não encontrado para cliente " << client_fd << std::endl;
			return -1;
		}
		try
		{
			HttpRequest request(requestBuffer);
			Configuration* config = findConfigForRequest(request, server_fd);
			if (!config)
			{
				std::cerr << "Configuração não encontrada para servidor " << server_fd << std::endl;
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

    // envia a resposta ao cliente
    int sent = send(client_fd, response.c_str(), response.size(), 0);
    if (sent == -1)
    {
        return;
    }
    else
	{
		std::stringstream ss;
		ss << "Dados enviados ao cliente - client_fd: " << client_fd;
		printLog(ss.str());
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
			printLog(ss.str());
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


int WebServer::getServerFdForClient(int client_fd)
{
	std::map<int, int>::iterator it = _client_to_server_map.find(client_fd);
	if (it != _client_to_server_map.end())
		return it->second;
	return -1;
}

Configuration* WebServer::findConfigForRequest(const HttpRequest& request, const int& server_fd)
{
	if (server_fd < 0)
	{
		std::cerr << "FD de servidor inválido: " << server_fd << std::endl;
		return NULL;
	}
	std::map<std::string, std::string>::const_iterator host_it = request.getHeaders().find("Host");
	if (host_it == request.getHeaders().end())
	{
		return NULL; // verificar esse handle
	}
	std::string host = host_it->second;

	// Tira a porta, se vier junto
	size_t colonPos = host.find(':');
	if (colonPos != std::string::npos)
		host = host.substr(0, colonPos);
	Configuration* defaultConfig = NULL;
    // Pegar o server diretamente pelo fd
	std::map<int, Server *>::iterator server_it = _servers_map.find(server_fd);
	if (server_it == _servers_map.end())
		return NULL;
	Server* server = server_it->second;
	std::string serverIp = server->getIp();
	std::string serverPort = server->getPort();

    // Procurar configuração que corresponde ao host e servidor
	for (std::vector<Configuration>::iterator config_it = _configurations.begin(); config_it != _configurations.end(); ++config_it)
	{
		const std::set<std::pair<std::string, std::string> >& hosts = config_it->getHost();
		for (std::set<std::pair<std::string, std::string> >::const_iterator host_it = hosts.begin(); host_it != hosts.end(); ++host_it)
		{
            // Comparar diretamente com o ip e porta do servidor
			if (host_it->first == serverIp && host_it->second == serverPort)
			{
				if (defaultConfig == NULL)
                    defaultConfig = &(*config_it);
                // Avaliar o server name agora
				const std::vector<std::string>& serverNames = config_it->getServerName();
				for (std::vector<std::string>::const_iterator name_it = serverNames.begin(); name_it != serverNames.end(); ++name_it)
				{
					if (host == *name_it)
					{
						return &(*config_it);
					}
				}
			}
		}
	}
	return (defaultConfig);
}

// ### TIMESTAMP LOGS ###
std::string WebServer::setTimeStamp()
{
	std::time_t timestamp = std::time(&timestamp);
	std::tm *timeinfo = std::localtime(&timestamp);

	std::ostringstream oss;
	oss << (1900 + timeinfo->tm_year) 
	<< "-" << std::setfill('0') << std::setw(2) << (1 + timeinfo->tm_mon) 
	<< "-" << std::setfill('0') << std::setw(2) << (timeinfo->tm_mday) 
	<< "T" << std::setfill('0') << std::setw(2) << (timeinfo->tm_hour)
	<< ":" << std::setfill('0') << std::setw(2) << (timeinfo->tm_min)
	<< ":" << std::setfill('0') << std::setw(2) << (timeinfo->tm_sec);

	return oss.str();
}

void	WebServer::printLog(std::string message)
{
	std::string time = setTimeStamp();
	std::cout << "[" + time + "]: " + message << std::endl;
}