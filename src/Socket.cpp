/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 17:16:05 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/13 17:35:33 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

Socket::Socket() :_socket_fd(-1)
{
	memset(&_event, 0, sizeof(_event));
	memset(&_hints, 0, sizeof(_hints));
	_ip = "";
	_port = "";
	_res = NULL;
}

void	Socket::setIpPort(const std::string& ip, const std::string& port)
{
    _ip = ip;
    _port = port;
    initAddrinfo(ip, port); // faz novo procedimento com getaddrinfo
}

void	Socket::initAddrinfo(const std::string& ip, const std::string& port)
{
	_hints.ai_family = AF_INET;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_PASSIVE;
	if (_res)
	{
		freeaddrinfo(_res);
		_res = NULL;
	}
	int status = getaddrinfo(ip.c_str(), port.c_str(), &_hints, &_res);
	if (status != 0)
	{
		throw std::runtime_error("getaddrinfo error: " + std::string(gai_strerror(status)));
	}
    // Atualiza strings IP e porta (opcional, já temos nos parâmetros)
    updateIpPortFromAddrinfo();
}

void	Socket::updateIpPortFromAddrinfo()
{
	if (_res && _res->ai_family == AF_INET)
	{
		char ip_str[INET_ADDRSTRLEN];
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)_res->ai_addr;
		inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str));
		_ip = ip_str;
		std::stringstream ss;
		ss << ntohs(ipv4->sin_port);
		_port = ss.str();
	}
}

Socket::Socket(const std::string& ip, const std::string& port) : _socket_fd(-1), _res(NULL), _ip(ip), _port(port)
{
	memset(&_hints, 0, sizeof(_hints));
	memset(&_event, 0, sizeof(_event));
	initAddrinfo(ip, port);
}

Socket::Socket(const Socket &other) : _socket_fd(other._socket_fd), _ip(other._ip), _port(other._port)
{
	memset(&_hints, 0, sizeof(_hints));
	memset(&_event, 0, sizeof(_event));
	_res = NULL;
	_hints = other._hints;
	_event = other._event;
	if (!_ip.empty() && !_port.empty())
	{
		initAddrinfo(_ip, _port);
	}
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
		_ip = other._ip;
		_port = other._port;
        if (!_ip.empty() && !_port.empty())
		{
			initAddrinfo(_ip, _port);
		}
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

