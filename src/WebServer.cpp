/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:19 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 15:00:08 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

WebServer::WebServer() {
	initEpoll();

	// _events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo
	_events = new struct epoll_event[MAX_EVENTS]; // Aloca memória para o vetor de eventos

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

void WebServer::setServersMap(const std::map<std::string, HttpServer *> &servers_map) {
	_servers_map = servers_map;
}

void WebServer::setClientsVec(const std::vector<Socket *> &clients_vec) {
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
