/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/07 13:44:46 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###

HttpServer::HttpServer()
{
	std::cout << "Iniciando o servidor Http..." << std::endl;

	// Cria o socket epoll mantido pelo kernel para armazenar o conjunto de descritores a serem monitorados
	_epoll_fd = epoll_create(1);
	if (_epoll_fd == -1)
	{
		std::cerr << "Erro ao criar epoll" << std::endl;
		return ;
	}
	
	// Pegar as flags do epoll_fd e adicionar a flag FD_CLOEXEC
	int epoll_flags = fcntl(_epoll_fd, F_GETFD);
	if (epoll_flags == -1)
	{
		std::cerr << "Erro ao obter flags do epoll_fd" << std::endl;
		close(_epoll_fd);
		return ;
	}
	epoll_flags |= FD_CLOEXEC;
	if (fcntl(_epoll_fd, F_SETFD, epoll_flags) == -1)
	{
		std::cerr << "Erro ao definir flags do epoll_fd" << std::endl;
		close(_epoll_fd);
		return ;
	}

	_events.resize(MAX_EVENTS); // Redimensiona o vetor de eventos para o tamanho máximo

	// Cria o socket
	_server_fd.setSocketFd( socket(_server_fd.getRes()->ai_family, _server_fd.getRes()->ai_socktype, _server_fd.getRes()->ai_protocol));
	if (_server_fd.getSocketFd() == -1) {
		std::cerr << "Erro ao criar o socket" << std::endl;
		freeaddrinfo(_server_fd.getRes());
		return ;
	}

	// Torna o server socket nao bloqueante
	int flags = fcntl(_server_fd.getSocketFd(), F_GETFL, 0);
	fcntl(_server_fd.getSocketFd(), F_SETFL, flags | O_NONBLOCK);

	// Faz o bind
	if (bind(_server_fd.getSocketFd(), _server_fd.getRes()->ai_addr, _server_fd.getRes()->ai_addrlen) == -1) {
		std::cerr << "Erro ao fazer bind" << std::endl;
		close(_server_fd.getSocketFd());
		freeaddrinfo(_server_fd.getRes());
		return ;
	}

	// Escuta por conexões
	if (listen(_server_fd.getSocketFd(), 10) == -1) {
		std::cerr << "Erro ao escutar no socket" << std::endl;
		close(_server_fd.getSocketFd());
		freeaddrinfo(_server_fd.getRes());
		return ;
	}

	// Registra o server socket na epoll para monitorar
	_server_fd.setEvent(EPOLLIN, _server_fd.getSocketFd());
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _server_fd.getSocketFd(), &_server_fd.getEvent()) == -1) {
		std::cerr << "Erro ao adicionar o socket ao epoll" << std::endl;
		freeaddrinfo(_server_fd.getRes());
		return ;
	}
}

HttpServer::HttpServer(const HttpServer &other) {
	*this = other; // Chama o operador de atribuição
}

HttpServer &HttpServer::operator=(const HttpServer &other) {
	if (this != &other) {
		_server_fd = other._server_fd;
		_epoll_fd = other._epoll_fd;
		_client_fds = other._client_fds;
	}
	return *this;
}

HttpServer::~HttpServer() {
}

// ### PRINT SERVER INFO ###
void HttpServer::printServer()
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
void HttpServer::startServer()
{
	// Aceita novas conexões
	while (true) {
		// Espera por eventos
		int event_count = epoll_wait(_epoll_fd, _events.data(), MAX_EVENTS, -1);
		if (event_count == -1) {
			std::cerr << "Erro no epoll_wait" << std::endl;
			continue;
		}

		for (int i = 0; i < event_count; ++i)
		{
			if (_events[i].data.fd == _server_fd.getSocketFd())
			{
				handleNewClient();
			} else {
				receiveData(_events[i].data.fd);
			}
		}
		
	}
}

// ### RECEIVE DATA FROM CLIENT ###
void	HttpServer::receiveData(int client_fd)
{
	// Recebe dados do cliente
	ssize_t bytes_received = recv(client_fd, _buffer, BUFFER_SIZE - 1, 0);
	if (bytes_received > 0) {
		_buffer[bytes_received] = '\0'; // Garante que o buffer é uma string
		// std::cout << "Dados recebidos do cliente " << client_fd << ": \n" << _buffer << std::endl;


		// parsing bruno -> retorna objeto ha HttpRequest com as informacoes que precisamos
		// parsing da estrutura HttpRequest para processar a requisicao valida ou nao


		// logica para realizar o send depois
		// // Envio de resposta padrao ao cliente sem processamento de requisicao
		const char *html_body = 
			"<!DOCTYPE html>"
			"<html lang=\"en\">"
			"<head>"
			"<meta charset=\"UTF-8\">"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			"<title>Bem-vindo</title>"
			"<style>"
			"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
			"h1 { color: #333; }"
			"</style>"
			"</head>"
			"<body>"
			"<h1>Bem-vindo ao WebServr de ncampbel, bruhenr dioalexa</h1>"
			"</body>"
			"</html>";

		size_t content_length = strlen(html_body);

		const char *response_header = 
			"Http/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: ";
			// "Set-Cookie: sessionId=1234567890; Path=/; HttpOnly\r\n";

		char response[1024];
		snprintf(response, sizeof(response), "%s%zu\r\n\r\n%s", response_header, content_length, html_body);
		// ssize_t sent_bytes = send(_events[i].data.fd, response, strlen(response), 0);
		ssize_t sent_bytes = send(client_fd, response, strlen(response), 0);

		if (sent_bytes == -1) {
			std::cerr << "Erro ao enviar resposta ao cliente " << client_fd << std::endl;
		} else {
			std::cout << "Resposta enviada ao cliente " << client_fd << ": \n";
		}

		
		// Configurar o socket para monitorar eventos de leitura (EPOLLIN) novamente
		struct epoll_event ev;
		ev.events = EPOLLIN; // EPOLLIN para leitura
		ev.data.fd = client_fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
			std::cerr << "Erro ao modificar socket para EPOLLIN: " << client_fd << std::endl;
			close(client_fd);
			return;
		}
	} else if (bytes_received == 0) {
		std::cout << "Cliente desconectado - _client_fd: " << client_fd << std::endl;
		close(client_fd);
		// Remove o cliente do vetor e do epoll
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		for (std::vector<Socket *>::iterator it = _client_fds.begin(); it != _client_fds.end(); ++it) {
			if ((*it)->getSocketFd() == client_fd) {
				delete *it; // Libera a memória do socket
				_client_fds.erase(it); // Remove o socket do vetor
				break; // Sai do loop após encontrar e remover o cliente
			}
		}
	} else {
		std::cerr << "Erro ao receber dados do cliente " << client_fd << std::endl;
	}
}

// ### HANDLE NEW CLIENT ###
void	HttpServer::handleNewClient()
{
	// Nova conexao
	Socket *client_fd = new Socket();
	socklen_t addr_len = sizeof(client_fd->getAddress());
	int socket = accept(_server_fd.getSocketFd(), &client_fd->getAddress(), &addr_len);
	std::cout << "\n### Novo socket aceito: " << socket << "\n\n";
	client_fd->setSocketFd(socket);
	client_fd->setEvent(EPOLLIN, client_fd->getSocketFd());
	if (client_fd->getSocketFd() == -1) {
		std::cerr << "Erro ao aceitar nova conexão" << std::endl;
		return;
	}
	// Adiciona o novo socket no vector e no epoll
	_client_fds.push_back(client_fd);
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd->getSocketFd(), &client_fd->getEvent());
}