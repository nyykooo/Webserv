/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/27 21:21:13 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ######### LIFE CYCLE #########

Server::Server() : Socket(), _ip("localhost"), _port("8080") // Valores padrão para IP e porta
{
	initServerSocket(_ip, _port);
}

Server::Server(const std::string &ip, const std::string &port) : Socket(), _ip(ip), _port(port)
{
	initServerSocket(ip, port);
}


Server::Server(const Server &other) : Socket(other)
{
	_client_fds = other._client_fds;
}

Server &Server::operator=(const Server &other) {
	if (this != &other)
	{
		Socket::operator=(other);
		_client_fds = other._client_fds;
	}
	return *this;
}

Server::~Server() {
	std::stringstream ss;
	ss << "‼️ WARNING: Server is down! ‼️\n";
	printLog(ss.str(), RED, std::cout);
}

// ######### INIT SERVER SOCKET #########
void	Server::initServerSocket(std::string ip, std::string port)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPV4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_NUMERICHOST; // Makes sure it's a numeric host (IPV4), avoiding DNS lookups (faster)
	_hints = hints;

    if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &res) != 0) {
        std::cerr << "Erro em getaddrinfo" << std::endl;
    }
	_res = res;

	_socket_fd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
	if (_socket_fd == -1)
	{
		throw Socket::SocketErrorException("failed to create server socket on " + ip + ":" + port);
	}
	int flags = fcntl(_socket_fd, F_GETFL, 0);
	fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);
	int opt = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(_socket_fd, _res->ai_addr, _res->ai_addrlen) == -1)
	{
		close(_socket_fd);
		throw Socket::SocketErrorException("failed to bind server socket on " + ip + ":" + port);
	}
    if (listen(_socket_fd, SOMAXCONN) == -1)
	{
		close(_socket_fd);
		throw Socket::SocketErrorException("failed to listen on server socket on " + ip + ":" + port);
	}
	setEvent(EPOLLIN, _socket_fd);
}

// ######### GETTERS #########

std::string Server::getIp() const {
	return _ip;
}

std::string Server::getPort() const {
	return _port;
}

// ######### SETTERS #########

void Server::setIp(const std::string &ip) {
	_ip = ip;
}

void Server::setPort(const std::string &port) {
	_port = port;
}
