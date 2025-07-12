/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:19 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/07 18:55:52 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

WebServer::WebServer() {
	initEpoll();

	// _events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo
	_events = new struct epoll_event[MAX_EVENTS]; // Aloca memória para o vetor de eventos

	Server *server1 = new Server("8080");
	Server *server2 = new Server("8081");
	
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server1->getSocketFd(), &server1->getEvent());
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server2->getSocketFd(), &server2->getEvent());

	_servers_map["8080"] = server1;
	_servers_map["8081"] = server2;
}

WebServer::WebServer(const WebServer &other) {
	_epoll_fd = other._epoll_fd;
	_servers_map = other._servers_map;
	_clients_vec = other._clients_vec;
	_events = other._events;
}

WebServer &WebServer::operator=(const WebServer &other) {
	if (this != &other) {
		_epoll_fd = other._epoll_fd;
		_servers_map = other._servers_map;
		_clients_vec = other._clients_vec;
		_events = other._events;
	}
	return *this;
}

WebServer::~WebServer() {
	_clients_vec.clear();
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
	socket->setEvent(EPOLLIN, socket->getSocketFd());
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
	if (!client_socket) {
		std::cerr << "Erro ao inicializar o socket do cliente" << std::endl;
		return ;
	}
	_clients_vec.push_back(client_socket);
	std::cout << "✅ Novo cliente conectado - new_client_socket: " << _clients_vec.back()->getSocketFd() << " ✅" << std::endl;
	registerEpollSocket(client_socket);
}

// ### GETTERS ###
int WebServer::getEpollFd() const {
	return _epoll_fd;
}

std::map<std::string, Server *> WebServer::getServersMap() const {
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

// ### SETTERS ###
void WebServer::setEpollFd(int epoll_fd) {
	_epoll_fd = epoll_fd;
}

void WebServer::setServersMap(const std::map<std::string, Server *> &servers_map) {
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
    std::map<std::string, Server *>::iterator it;
    for (it = _servers_map.begin(); it != _servers_map.end(); ++it)
    {
        if(_events[i].data.fd == it->second->getSocketFd())
        {
			try
			{
				handleNewClient(it->second->getSocketFd());
				return true; // Conexão aceita com sucesso
			}
			catch (const std::exception &e)
			{
				std::cerr << "Error parsing HTTP request: " << e.what() << std::endl;
				return 1; 
			}
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
        std::string requestBuffer(_buffer);

        try
        {
            HttpRequest request(requestBuffer);
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
        std::cout << "✅ Dados enviados para o cliente - client_fd: " << client_fd << " ✅" << std::endl;
    
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
            std::cout << "❌ Cliente desconectado - client_fd: " << (*it)->getSocketFd() << " ❌" << std::endl;
            close((*it)->getSocketFd());
            delete *it; // Libera a memória do cliente
            it = _clients_vec.erase(it); // Remove o cliente do vetor
            epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
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
        else
            break;
    }
}

void WebServer::lookForTimeouts()
{
    std::vector<Client *>::iterator it;
    for (it = _clients_vec.begin(); it != _clients_vec.end(); ++it)
    {
        if ((*it)->checkTimeout())
        {
            deleteClient((*it)->getSocketFd());
        }
        if (it == _clients_vec.end())
            break;
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