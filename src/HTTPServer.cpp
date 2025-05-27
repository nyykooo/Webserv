/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/05/27 18:35:03 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###

HTTPServer::HTTPServer()
	: _server_fd(-1) {
	memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_INET; // IPv4
	_hints.ai_socktype = SOCK_STREAM; // TCP
	_hints.ai_flags = AI_PASSIVE; // Use my IP

	// Obtém informações de endereço
    if (getaddrinfo("localhost", "8080", &_hints, &_res) != 0) {
        std::cerr << "Erro em getaddrinfo" << std::endl;
    }
}

HTTPServer::HTTPServer(const HTTPServer &other) {
	*this = other; // Chama o operador de atribuição
}

HTTPServer &HTTPServer::operator=(const HTTPServer &other) {
	if (this != &other) {
		_server_fd = other._server_fd;
		_addr = other._addr;
		_hints = other._hints;
		if (_res) {
			delete _res;
		}
		_res = new addrinfo(*other._res);
		_client_fds = other._client_fds;
	}
	return *this;
}

HTTPServer::~HTTPServer() {
	if (_server_fd != -1) {
		close(_server_fd);
	}
	if (_res) {
		freeaddrinfo(_res);
	}
	
	
	for (std::vector<int>::iterator it = _client_fds.begin(); it != _client_fds.end(); ++it) {
		close(*it);
	}
}

// ### INIT SERVER ###
void HTTPServer::initServer()
{
    // Cria o socket
	_server_fd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
    if (_server_fd == -1) {
        std::cerr << "Erro ao criar o socket" << std::endl;
        freeaddrinfo(_res);
        return;
    }

    // Faz o bind
    if (bind(_server_fd, _res->ai_addr, _res->ai_addrlen) == -1) {
        std::cerr << "Erro ao fazer bind" << std::endl;
        close(_server_fd);
        freeaddrinfo(_res);
        return ;
    }
}

// ### PRINT SERVER INFO ###
void HTTPServer::printServer()
{

	// Converte o endereço para string legível
    char ip_str[INET6_ADDRSTRLEN];
    void *addr_ptr;

    if (_res->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)_res->ai_addr;
        addr_ptr = &(ipv4->sin_addr);
    } else if (_res->ai_family == AF_INET6) { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)_res->ai_addr;
        addr_ptr = &(ipv6->sin6_addr);
    } else {
        std::cerr << "Família de endereços desconhecida" << std::endl;
        close(_server_fd);
        freeaddrinfo(_res);
        return ;
    }

    // Converte o endereço binário para string
    inet_ntop(_res->ai_family, addr_ptr, ip_str, sizeof(ip_str));
    std::cout << "Endereço IP: " << ip_str << std::endl;
}

// ### START SERVER ###
void HTTPServer::startServer()
{
	// Escuta por conexões
	if (listen(_server_fd, 10) == -1) {
		std::cerr << "Erro ao escutar no socket" << std::endl;
		close(_server_fd);
		freeaddrinfo(_res);
		return ;
	}
	std::cout << "Servidor iniciado e escutando na porta 8080" << std::endl;

	// Aceita novas conexões
	while (true) {
		socklen_t addr_len = sizeof(_addr);
		int client_fd = accept(_server_fd, &_addr, &addr_len);
		if (client_fd == -1) {
			std::cerr << "Erro ao aceitar nova conexão" << std::endl;
			continue;
		}
		_client_fds.push_back(client_fd);
		handleNewClient(_client_fds.back());
	}
}

// ### HANDLE NEW CLIENT ###
void	HTTPServer::handleNewClient(int client_fd)
{
	std::cout << "Novo cliente conectado - _client_fd: " << client_fd << std::endl;
}