/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/13 16:38:33 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

Server::Server(const std::string &ip, const std::string &port) : Socket(ip, port) //constructor da classe base
{
    initServerSocket();
}

Server::Server() : Socket("0.0.0.0", "8080") // Valores padrão para IP e porta
{
	initServerSocket();
	std::cout << "Servidor Http iniciado fd : " << _socket_fd << std::endl;
}

// O constructor apenas com a porta
Server::Server(const std::string &port) : Socket("0.0.0.0", port)
{
	initServerSocket();
	std::cout << "🌐 Servidor Http iniciado fd : " << _socket_fd << " 🌐" << std::endl;
}

Server::Server(const Server &other) : Socket(other)
{
	_client_fds = other._client_fds;
}

Server &Server::operator=(const Server &other) {
	if (this != &other)
	{
		Socket::operator=(other); // Para copiarmos a parte do objeto referente ao Socket
		_client_fds = other._client_fds;
	}
	return *this;
}

Server::~Server() {
	std::cout << "‼️ WARNING: Server is down! ‼️" << std::endl;
}

// ### INIT SERVER SOCKET ###
void	Server::initServerSocket()
{
	// Cria socket usando addrinfo já configurado
	_socket_fd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
	if (_socket_fd == -1)
	{
		throw std::runtime_error("socket creation failed");
	}
	// Torna o server socket não bloqueante
	int flags = fcntl(_socket_fd, F_GETFL, 0);
	fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);
	int opt = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(_socket_fd, _res->ai_addr, _res->ai_addrlen) == -1)
	{
		close(_socket_fd);
		throw std::runtime_error("bind failed on " + _ip + ":" + _port);
	}
    if (listen(_socket_fd, SOMAXCONN) == -1)
	{
		close(_socket_fd);
		throw std::runtime_error("listen failed");
	}
    std::cout << "Server listening on " << _ip << ":" << _port << std::endl;
	// Registra o server socket na epoll para monitorar
	setEvent(EPOLLIN, _socket_fd);
}