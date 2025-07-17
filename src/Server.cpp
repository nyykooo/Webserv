/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/14 23:14:45 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

Server::Server() : Socket(), _ip("localhost"), _port("8080") // Valores padrão para IP e porta
{
	initServerSocket(_ip, _port);
	std::cout << "Servidor Http iniciado fd : " << _socket_fd << std::endl;
}

// CONSTRUCTOR WITH PORT
Server::Server(const std::string &ip, const std::string &port) : Socket(), _ip(ip), _port(port)
{
	// inicializa o socket do servidor com as configuracoes corretas
	initServerSocket(ip, port);
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
void	Server::initServerSocket(std::string ip, std::string port)
{
	// Configura o socket hints e res
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_NUMERICHOST;
	_hints = hints; // Armazena as hints no socket

	// Obtém informações de endereço
	std::cout << "Inicializando o servidor em " << ip << ":" << port << std::endl;
    if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &res) != 0) {
        std::cerr << "Erro em getaddrinfo" << std::endl;
    }
	_res = res; // Armazena o resultado de getaddrinfo no socket

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
		throw std::runtime_error("bind failed on " + ip + ":" + port);
	}
    if (listen(_socket_fd, SOMAXCONN) == -1)
	{
		close(_socket_fd);
		throw std::runtime_error("listen failed");
	}
    std::cout << "Server listening on " << ip << ":" << port << std::endl;
	// Registra o server socket na epoll para monitorar
	setEvent(EPOLLIN, _socket_fd);
}

// ### GETTERS ###

std::string Server::getIp() const {
	return _ip;
}

std::string Server::getPort() const {
	return _port;
}

// ### SETTERS ###

void Server::setIp(const std::string &ip) {
	_ip = ip;
}

void Server::setPort(const std::string &port) {
	_port = port;
}
