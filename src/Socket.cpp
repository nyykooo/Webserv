/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 17:16:05 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/23 19:22:25 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ######### LIFE CYCLE #########

Socket::Socket() :_socket_fd(-1)
{
	memset(&_event, 0, sizeof(_event));
	memset(&_hints, 0, sizeof(_hints));
	_res = NULL;
}

Socket::Socket(const Socket &other) : _socket_fd(other._socket_fd)
{
	memset(&_hints, 0, sizeof(_hints));
	memset(&_event, 0, sizeof(_event));
	_res = NULL;
	_hints = other._hints;
	_event = other._event;
}

Socket &Socket::operator=(const Socket &other)
{
	if (this != &other)
	{
		if (_res)
		{
			freeaddrinfo(_res);
			_res = NULL;
		}
		_socket_fd = other._socket_fd;
		_event = other._event;
		_hints = other._hints;
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

// ######### GETTERS #########

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

// ######### SETTERS #########
void Socket::setSocketFd(int fd) {
	_socket_fd = fd;
}

void Socket::setEvent(int event_flags, int fd)
{
	_event.events = event_flags;
	_event.data.fd = fd;
}

void Socket::setRes(struct addrinfo *res) {
	if (_res) {
		freeaddrinfo(_res);
	}
	_res = res;
}

void Socket::setHints(struct addrinfo hints) {
	_hints = hints;
}

// ######### EXCEPTION #########

const char* Socket::SocketErrorException::what() const throw() {
	return _message.c_str();
}
