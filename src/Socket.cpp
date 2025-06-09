/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 17:16:05 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/06 18:28:21 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###
Socket::Socket() :_socket_fd(-1) {

	std::cout << "Iniciando o socket..." << std::endl;

	memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_INET; // IPv4
	_hints.ai_socktype = SOCK_STREAM; // TCP
	_hints.ai_flags = AI_PASSIVE; // Use my IP. Isso é adeqiado para o servidor, pois ele irá escutar em todas as interfaces de rede disponíveis.

	// Obtém informações de endereço
    if (getaddrinfo("127.0.0.0", "8080", &_hints, &_res) != 0)
	{
        std::cerr << "Erro em getaddrinfo" << std::endl;
    }
}

Socket::Socket(const Socket &other) {
	*this = other; // Chama o operador de atribuição
}

Socket &Socket::operator=(const Socket &other) {
	if (this != &other) {
		_socket_fd = other._socket_fd;
		_addr = other._addr;
		_event = other._event;
		_hints = other._hints;
		if (_res) {
			freeaddrinfo(_res); // Libera o recurso anterior, se necessário
		}
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
}

// ### GETTERS ###
int Socket::getSocketFd() {
	return _socket_fd;
}

struct sockaddr &Socket::getAddress() {
	return _addr;
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

void Socket::setAddress(struct sockaddr addr) {
	_addr = addr;
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

