/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:51:13 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/12 14:48:46 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###
Client::Client()
{
	_time = std::time(NULL);
}

Client::Client(int server_fd)
{
	_time = std::time(NULL);
	initClientSocket(server_fd);
}

Client::Client(const Client &other) : Socket(other) // Chama explicitamente o copy contructor de Socket
{
	*this = other;
}

Client	&Client::operator=(const Client& other)
{
	_time = other.getTime();
	return *this;
}

Client::~Client()
{
	
}

// ### PUBLIC METHODS ###

Client *Client::initClientSocket(int server_fd)
{
    // Aloca memória para o endereço do cliente
    sockaddr_storage addr; // Usando sockaddr_storage para suportar IPv4 e IPv6
    socklen_t addr_len = sizeof(addr);

    // Aceita a conexão
    _socket_fd = accept(server_fd, (sockaddr *)&addr, &addr_len);
    if (_socket_fd == -1) {
        std::cerr << "Erro ao aceitar nova conexão" << std::endl;
        return NULL;
    }

    // Configura o socket do cliente
    setEvent(EPOLLIN, _socket_fd);

    // Configura o endereço do cliente
    addrinfo *_res = new addrinfo(); // Aloca memória para addrinfo
    _res->ai_addr = (sockaddr *)new sockaddr_storage(addr); // Copia o endereço do cliente
    _res->ai_addrlen = addr_len; // Armazena o tamanho do endereço do cliente

    // Configura a família de endereços com base no tipo de endereço recebido
    if (addr.ss_family == AF_INET) {
        _res->ai_family = AF_INET; // IPv4
    } else if (addr.ss_family == AF_INET6) {
        _res->ai_family = AF_INET6; // IPv6
    } else {
        std::cerr << "Família de endereços desconhecida ao aceitar conexão" << std::endl;
        delete _res;
        return NULL;
    }

    // Adicionar o non-blocking ao socket do cliente
    int flags = fcntl(_socket_fd, F_GETFL, 0);
    fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);

    int keepalive = 1; // Ativa o keepalive para o socket do cliente
    setsockopt(_socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)); // Permite reutilizar o endereço

	// printServer(client_fd); // Imprime as informações do servidor
    return this;
}

bool Client::checkTimeout() const
{
    std::time_t curr_time = std::time(NULL);
    // Verifica se o tempo decorrido desde a última atividade é maior que o timeout
    if (curr_time - _time > 10)
        return true;
    else
        return false;
}

// ### GETTERS ###
std::time_t	Client::getTime() const
{
	return _time;
}

// ### SETTERS ###
void	Client::setTime(std::time_t time)
{
	_time = time;
}