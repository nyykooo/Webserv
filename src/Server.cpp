/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 16:32:10 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###

Server::Server()
{
	// inicializa o socket do servidor com as configuracoes corretas
	initServerSocket("8080");
	std::cout << "Servidor Http iniciado fd : " << _socket_fd << std::endl;
}

// CONSTRUCTOR WITH PORT
Server::Server(const std::string &port)
{
	// inicializa o socket do servidor com as configuracoes corretas
	initServerSocket(port);
	std::cout << "🌐 Servidor Http iniciado fd : " << _socket_fd << " 🌐" << std::endl;
}

Server::Server(const Server &other) {
	*this = other; // Chama o operador de atribuição
}

Server &Server::operator=(const Server &other) {
	if (this != &other) {
		_client_fds = other._client_fds;
	}
	return *this;
}

Server::~Server() {
	std::cout << "‼️ WARNING: Server is down! ‼️" << std::endl;
}

// ### INIT SERVER SOCKET ###
void	Server::initServerSocket(std::string port)
{
	// Configura o socket hints e res
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // Use my IP. Isso é adeqiado para o servidor, pois ele irá escutar em todas as interfaces de rede disponíveis.
	_hints = hints; // Armazena as hints no socket

	// Obtém informações de endereço
    if (getaddrinfo(NULL, port.c_str(), &hints, &res) != 0) {
        std::cerr << "Erro em getaddrinfo" << std::endl;
    }
	_res = res; // Armazena o resultado de getaddrinfo no socket

	_socket_fd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
	if (_socket_fd == -1)
	{
		std::cerr << "Erro ao criar o socket" << std::endl;
		freeaddrinfo(_res);
		return ;
	}

	// Torna o server socket nao bloqueante
	int flags = fcntl(_socket_fd, F_GETFL, 0);
	fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);

	// seta as opcoes do socket para reutilizar o endereco
	int reuse = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	// Faz o bind
	if (bind(_socket_fd, _res->ai_addr, _res->ai_addrlen) == -1) {
		perror("Erro ao fazer bind");
		std::cerr << "Erro ao fazer bind: " << errno << std::endl;
		close(_socket_fd);
		freeaddrinfo(_res);
		return ;
	}

	// Escuta por conexões
	if (listen(_socket_fd, 10) == -1) {
		std::cerr << "Erro ao escutar no socket" << std::endl;
		close(_socket_fd);
		freeaddrinfo(_res);
		return ;
	}

	// // Registra o server socket na epoll para monitorar
	setEvent(EPOLLIN, _socket_fd);
}