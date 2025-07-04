/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:19 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/04 15:02:23 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

WebServer::WebServer() {
	initEpoll();

	// _events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo
	_events.resize(MAX_EVENTS); // Aloca memória para o vetor de eventosams

	HttpServer *server1 = new HttpServer("8080");
	HttpServer *server2 = new HttpServer("8081");
	
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server1->getServerSocket()->getSocketFd(), &server1->getServerSocket()->getEvent());
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server2->getServerSocket()->getSocketFd(), &server2->getServerSocket()->getEvent());

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

// ### START SERVER ###
void WebServer::startServer()
{
	// // Aceita novas conexões
	while (true) {
		// Espera por eventos
		int event_count = epoll_wait(_epoll_fd, _events.data(), MAX_EVENTS, -1);
		if (event_count == -1) {
			std::cerr << "Erro no epoll_wait" << std::endl;
			return;
		}

		for (int i = 0; i < event_count; ++i)
		{
			bool server_found = false;
			// verifica se o evento corresponde a um server (conexao nova)
			std::map<std::string, HttpServer *>::iterator it;
			for (it = _servers_map.begin(); it != _servers_map.end(); ++it)
			{
				if (_events[i].data.fd == it->second->getServerSocket()->getSocketFd())
				{
					int new_client = it->second->handleNewClient();
					if (new_client == -1)
					{
						std::cerr << "Erro ao aceitar nova conexão" << std::endl;
						continue; // Continua para o próximo evento
					}
					std::cout << "✅ Novo cliente conectado - new_client_fd: " << new_client << " ✅" << std::endl;
					// Adiciona o novo socket no vector e no epoll
					struct epoll_event client_event;
					client_event.events = EPOLLIN; // Monitorar eventos de leitura (EPOLLIN) e usar o modo edge-triggered (EPOLLET)
					client_event.data.fd = new_client;
					int res = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, new_client, &client_event);
					if (res == -1)
					{
						std::cerr << "Erro ao adicionar o socket do cliente ao epoll" << std::endl;
						close(new_client);
						continue; // Continua para o próximo evento
					}
					server_found = true;
					break; // Sai do loop após encontrar o servidor correspondente
				}
			}
			if (!server_found)
			{
				std::cout << "Evento recebido de um cliente existente, processando dados..." << std::endl;
				int status = receiveData(_events[i].data.fd);

				if (status == 0)
				{
					// criar metodo para desconectar o cliente
					std::cout << "❌ Cliente desconectado - _client_fd: " << _events[i].data.fd << " ❌" << std::endl;
					close(_events[i].data.fd);
					// Remove o cliente do vetor e do epoll
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL);
					// QUANDO CRIAR O METODO PARA DESCONEXAO LIDAR COM VAZAMENTO DE MEMORIA
					// for (std::vector<Socket *>::iterator it = _client_fds.begin(); it != _client_fds.end(); ++it) {
					// 	if ((*it)->getSocketFd() == client_fd) {
					// 		delete *it; // Libera a memória do socket
					// 		_client_fds.erase(it); // Remove o socket do vetor
					// 		break ; // Sai do loop após encontrar e remover o cliente
					// 	}
					// }
				}
			}
		}
	}
}


// ### RECEIVE DATA FROM CLIENT ###
int	WebServer::receiveData(int client_fd)
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
			return 0;
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
			send(client_fd, response.c_str(), response.size(), 0);
		}
		return 1;
}

// ### GETTERS ###
int WebServer::getEpollFd() const {
	return _epoll_fd;
}

std::map<std::string, HttpServer *> WebServer::getServersMap() const {
	return _servers_map;
}

std::vector<Socket *> WebServer::getClientsVec() const {
	return _clients_vec;
}

std::vector<struct epoll_event> WebServer::getEvents() const {
	return _events;
}

char *WebServer::getBuffer() {
	return _buffer;
}

// ### SETTERS ###
void WebServer::setEpollFd(int epoll_fd) {
	_epoll_fd = epoll_fd;
}

void WebServer::setServersMap(const std::map<std::string, HttpServer *> &servers_map) {
	_servers_map = servers_map;
}

void WebServer::setClientsVec(const std::vector<Socket *> &clients_vec) {
	_clients_vec = clients_vec;
}

void WebServer::setEvents(std::vector<struct epoll_event> events) {
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
