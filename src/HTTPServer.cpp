/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/05/29 20:16:59 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###

HTTPServer::HTTPServer() {
	std::cout << "Iniciando o servidor HTTP..." << std::endl;
	// Cria o socket epoll
	_epoll_fd = epoll_create(0);


	// Cria o socket
	_server_fd.setSocketFd( socket(_server_fd.getRes()->ai_family, _server_fd.getRes()->ai_socktype, _server_fd.getRes()->ai_protocol));
	if (_server_fd.getSocketFd() == -1) {
		std::cerr << "Erro ao criar o socket" << std::endl;
		freeaddrinfo(_server_fd.getRes());
		return ;
	}

	// Faz o bind
	if (bind(_server_fd.getSocketFd(), _server_fd.getRes()->ai_addr, _server_fd.getRes()->ai_addrlen) == -1) {
		std::cerr << "Erro ao fazer bind" << std::endl;
		close(_server_fd.getSocketFd());
		freeaddrinfo(_server_fd.getRes());
		return ;
	}
}

HTTPServer::HTTPServer(const HTTPServer &other) {
	*this = other; // Chama o operador de atribuição
}

HTTPServer &HTTPServer::operator=(const HTTPServer &other) {
	if (this != &other) {
		_server_fd = other._server_fd;
		_epoll_fd = other._epoll_fd;
		_client_fds = other._client_fds;
	}
	return *this;
}

HTTPServer::~HTTPServer() {
}

// // ### INIT SERVER ###
// void HTTPServer::initServer()
// {
// 	// Cria o socket
// 	_server_fd;

//     // // Cria o socket
// 	// _server_fd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
//     // if (_server_fd == -1) {
//     //     std::cerr << "Erro ao criar o socket" << std::endl;
//     //     freeaddrinfo(_res);
//     //     return;
//     // }

//     // // Faz o bind
//     // if (bind(_server_fd, _res->ai_addr, _res->ai_addrlen) == -1) {
//     //     std::cerr << "Erro ao fazer bind" << std::endl;
//     //     close(_server_fd);
//     //     freeaddrinfo(_res);
//     //     return ;
//     // }
// }

// ### PRINT SERVER INFO ###
void HTTPServer::printServer()
{

	// Converte o endereço para string legível
    char ip_str[INET6_ADDRSTRLEN];
    void *addr_ptr;

    if (_server_fd.getRes()->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)_server_fd.getRes()->ai_addr;
        addr_ptr = &(ipv4->sin_addr);
    } else if (_server_fd.getRes()->ai_family == AF_INET6) { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)_server_fd.getRes()->ai_addr;
        addr_ptr = &(ipv6->sin6_addr);
    } else {
        std::cerr << "Família de endereços desconhecida" << std::endl;
        close(_server_fd.getSocketFd());
        freeaddrinfo(_server_fd.getRes());
        return ;
    }

    // Converte o endereço binário para string
    inet_ntop(_server_fd.getRes()->ai_family, addr_ptr, ip_str, sizeof(ip_str));
    std::cout << "Endereço IP: " << ip_str << std::endl;
}

// ### START SERVER ###
void HTTPServer::startServer()
{
	// Escuta por conexões
	if (listen(_server_fd.getSocketFd(), 10) == -1) {
		std::cerr << "Erro ao escutar no socket" << std::endl;
		close(_server_fd.getSocketFd());
		freeaddrinfo(_server_fd.getRes());
		return ;
	}
	std::cout << "Servidor iniciado e escutando na porta 8080" << std::endl;

	// Aceita novas conexões
	while (true) {
		Socket *client_fd = new Socket();
		socklen_t addr_len = sizeof(client_fd->getAddress());
		int socket = accept(_server_fd.getSocketFd(), &client_fd->getAddress(), &addr_len);
		client_fd->setSocketFd(socket); 
		if (client_fd->getSocketFd() == -1) {
			std::cerr << "Erro ao aceitar nova conexão" << std::endl;
			continue;
		}
		_client_fds.push_back(client_fd);
		handleNewClient(_client_fds.back());
	}
}

// ### HANDLE NEW CLIENT ###
void	HTTPServer::handleNewClient(Socket *client)
{
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client->getSocketFd(), &client->getEvent());
	std::cout << "Novo cliente conectado - _client_fd: " << client->getSocketFd() << std::endl;
}