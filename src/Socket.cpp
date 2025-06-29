/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 17:16:05 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/29 10:47:01 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###
Socket::Socket() :_socket_fd(-1) {

	// std::cout << "Iniciando o socket..." << std::endl;

    memset(&_event, 0, sizeof(_event)); // Zero-initialize _event
	_res = NULL; // Inicializa _res como nullptr
}

Socket::Socket(const Socket &other) {
	*this = other; // Chama o operador de atribuição
}

Socket &Socket::operator=(const Socket &other) {
	if (this != &other) {
		_socket_fd = other._socket_fd;
		_event = other._event;
		_hints = other._hints;
		_res = other._res;
	}
	return *this;
}

Socket::~Socket() {
	if (_socket_fd != -1) {
		close(_socket_fd);
	}
	if (_res) {
		freeaddrinfo(_res);
	}
	std::cout << "Socket destruído e recursos liberados." << std::endl;
}

// ### GETTERS ###
int Socket::getSocketFd() {
	return _socket_fd;
}

struct epoll_event &Socket::getEvent() {
	return _event;
}

struct addrinfo *Socket::getRes()
{
	return _res;
}

struct addrinfo *Socket::getHints() {
	return &_hints;
}

// ### SETTERS ###
void Socket::setSocketFd(int fd) {
	_socket_fd = fd;
}

void Socket::setEvent(int event_flags, int fd)
{
	_event.events = event_flags; // Define os eventos que o epoll irá monitorar
	_event.data.fd = fd; // Define o file descriptor associado ao evento
}

void Socket::setRes(struct addrinfo *res) {
	if (_res) {
		freeaddrinfo(_res); // Libera o recurso anterior, se necessário
	}
	_res = res;
}

void Socket::setHints(struct addrinfo hints) {
	_hints = hints;
}

