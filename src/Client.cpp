/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:51:13 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/31 03:35:06 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###
Client::Client() : _state(RECEIVING), _fileFd(-1), _fileSize(0), _bytesSent(0)
{
	_time = std::time(NULL);
}

Client::Client(int server_fd) : _state(RECEIVING), _fileFd(-1), _fileSize(0), _bytesSent(0)
{
	_time = std::time(NULL);
	_response = NULL;
	_request = NULL;
	initClientSocket(server_fd);
}

Client::Client(const Client &other) : Socket(other), _time(other.getTime()), _fileFd(-1), _fileSize(0), _bytesSent(0)
{
	_response = NULL;
	_request = NULL;
	_state = other._state;
}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		Socket::operator=(other);

		if (_fileFd != -1)
			close(_fileFd);
		_time = other._time;
		_state = other._state;
		_fileFd = -1;
		_fileSize = 0;
		_bytesSent = 0;
		if (_response)
		{
			delete _response;
			_response = NULL;
		}
		if (_request)
		{
			delete _request;
			_request = NULL;
		}
	}
	return *this;
}

Client::~Client()
{
	if (_fileFd != -1)
	{
		close(_fileFd);
		_fileFd = -1; // analisar se eh realmente necessario
	}
	if (_response)
		delete _response;
	if (_request)
		delete _request;
}

// ### PUBLIC METHODS ###

Client *Client::initClientSocket(int server_fd)
{
	// Aloca memória para o endereço do cliente
	sockaddr_storage addr; // Usando sockaddr_storage para suportar IPv4 e IPv6
	socklen_t addr_len = sizeof(addr);

	// Aceita a conexão
	_socket_fd = accept(server_fd, (sockaddr *)&addr, &addr_len);
	if (_socket_fd == -1)
	{
		std::cerr << "Erro ao aceitar nova conexão" << std::endl;
		return NULL;
	}

	// Configura o socket do cliente
	setEvent(EPOLLIN, _socket_fd);

	// Configura o endereço do cliente
	addrinfo *_res = new addrinfo();						// Aloca memória para addrinfo
	_res->ai_addr = (sockaddr *)new sockaddr_storage(addr); // Copia o endereço do cliente
	_res->ai_addrlen = addr_len;							// Armazena o tamanho do endereço do cliente

	// Configura a família de endereços com base no tipo de endereço recebido
	if (addr.ss_family == AF_INET)
	{
		_res->ai_family = AF_INET; // IPv4
	}
	else if (addr.ss_family == AF_INET6)
	{
		_res->ai_family = AF_INET6; // IPv6
	}
	else
	{
		std::cerr << "Família de endereços desconhecida ao aceitar conexão" << std::endl;
		delete _res;
		return NULL;
	}

	// Adicionar o non-blocking ao socket do cliente
	int flags = fcntl(_socket_fd, F_GETFL, 0);
	fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);

	int keepalive = 1;																 // Ativa o keepalive para o socket do cliente
	setsockopt(_socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)); // Permite reutilizar o endereço

	// printServer(client_fd); // Imprime as informações do servidor
	return this;
}

RequestProcessingState Client::getProcessingState() const
{
	return _state;
}

void Client::setProcessingState(RequestProcessingState state)
{
	_state = state;
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
std::time_t Client::getTime() const
{
	return _time;
}

const int &Client::getFileFd() const
{
	return _fileFd;
}

const size_t &Client::getFileSize() const
{
	return _fileSize;
}

const size_t &Client::getBytesSent() const
{
	return _bytesSent;
}

// ### SETTERS ###
void Client::setTime(std::time_t time)
{
	_time = time;
}

void Client::setFileFd(int fd)
{
	_fileFd = fd;
}

void Client::setFileSize(size_t size)
{
	_fileSize = size;
}

void Client::setBytesSent(size_t bytes)
{
	_bytesSent = bytes;
}

// Adicionar método público para reset de streaming
void Client::resetFileStreaming()
{
	if (_fileFd != -1)
	{
		close(_fileFd);
		_fileFd = -1;
	}
	_fileSize = 0;
	_bytesSent = 0;
}

// Adicionar método para verificar se está streaming
bool Client::isFileStreaming() const
{
	return (_fileFd != -1 && _state == STREAMING);
}

// Adicionar método para verificar progresso
double Client::getStreamingProgress() const
{
	if (_fileSize == 0)
		return 0.0;
	return (double)_bytesSent / (double)_fileSize;
}