/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 18:57:04 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 15:37:50 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###

HttpServer::HttpServer()
{
	std::cout << "Iniciando o servidor Http..." << std::endl;

	// inicializa o socket do servidor com as configuracoes corretas
	initServerSocket("8080");
	std::cout << "Servidor Http iniciado fd : " << _server_fd->getSocketFd() << std::endl;
}

// CONSTRUCTOR WITH PORT
HttpServer::HttpServer(const std::string &port)
{
	std::cout << "Iniciando o servidor Http na porta " << port << "..." << std::endl;

	// inicializa o socket do servidor com as configuracoes corretas
	initServerSocket(port);
	std::cout << "🌐 Servidor Http iniciado fd : " << _server_fd->getSocketFd() << " 🌐" << std::endl;
}

HttpServer::HttpServer(const HttpServer &other) {
	*this = other; // Chama o operador de atribuição
}

HttpServer &HttpServer::operator=(const HttpServer &other) {
	if (this != &other) {
		_server_fd = other._server_fd;
		// _epoll_fd = other._epoll_fd;
		// _client_fds = other._client_fds;
	}
	return *this;
}

HttpServer::~HttpServer() {
	delete _server_fd; // Libera a memória do socket do servidor
	std::vector<Socket *>::iterator it = _client_fds.begin();
	while (it != _client_fds.end()) {
		delete *it; // Libera a memória de cada socket do cliente
		it = _client_fds.erase(it); // Remove o socket do vetor e avança o iterador
	}
	std::cout << "‼️ WARNING: HttpServer is down! ‼️" << std::endl;
}

// ### PRINT SERVER INFO ###
void HttpServer::printServer(Socket *socket)
{

	// Converte o endereço para string legível
    char ip_str[INET6_ADDRSTRLEN];
    void *addr_ptr;

    if (socket->getRes()->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)socket->getRes()->ai_addr;
        addr_ptr = &(ipv4->sin_addr);
    } else if (socket->getRes()->ai_family == AF_INET6) { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)socket->getRes()->ai_addr;
        addr_ptr = &(ipv6->sin6_addr);
    } else {
        std::cerr << "Família de endereços desconhecida" << std::endl;
        close(socket->getSocketFd());
        freeaddrinfo(socket->getRes());
        return ;
    }

    // Converte o endereço binário para string
    inet_ntop(socket->getRes()->ai_family, addr_ptr, ip_str, sizeof(ip_str));
    std::cout << "Endereço IP: " << ip_str << std::endl;
}

// ### HANDLE NEW CLIENT ###
int	HttpServer::handleNewClient()
{
	// inicializa o socket do cliente
	Socket *client_fd = initClientSocket();
	if (!client_fd) {
		std::cerr << "Erro ao inicializar o socket do cliente" << std::endl;
		return client_fd->getSocketFd(); // Retorna um socket inválido
	}

	
	_client_fds.push_back(client_fd);
	return client_fd->getSocketFd(); // Retorna o fd do cliente para ser usado no epoll
}

// ### INIT SERVER SOCKET ###
void	HttpServer::initServerSocket(std::string port)
{
	// Cria o socket
	_server_fd = new Socket();

	// Configura o socket hints e res
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // Use my IP. Isso é adeqiado para o servidor, pois ele irá escutar em todas as interfaces de rede disponíveis.
	_server_fd->setHints(hints); // Armazena as hints no socket

	// Obtém informações de endereço
    if (getaddrinfo(NULL, port.c_str(), &hints, &res) != 0) {
        std::cerr << "Erro em getaddrinfo" << std::endl;
    }
	_server_fd->setRes(res); // Armazena o resultado de getaddrinfo no socket

	_server_fd->setSocketFd(socket(_server_fd->getRes()->ai_family, _server_fd->getRes()->ai_socktype, _server_fd->getRes()->ai_protocol));
	if (_server_fd->getSocketFd() == -1)
	{
		std::cerr << "Erro ao criar o socket" << std::endl;
		freeaddrinfo(_server_fd->getRes());
		return ;
	}

	// Torna o server socket nao bloqueante
	int flags = fcntl(_server_fd->getSocketFd(), F_GETFL, 0);
	fcntl(_server_fd->getSocketFd(), F_SETFL, flags | O_NONBLOCK);

	// seta as opcoes do socket para reutilizar o endereco
	int reuse = 1;
	setsockopt(_server_fd->getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	// Faz o bind
	if (bind(_server_fd->getSocketFd(), _server_fd->getRes()->ai_addr, _server_fd->getRes()->ai_addrlen) == -1) {
		perror("Erro ao fazer bind");
		std::cerr << "Erro ao fazer bind: " << errno << std::endl;
		close(_server_fd->getSocketFd());
		freeaddrinfo(_server_fd->getRes());
		return ;
	}

	// Escuta por conexões
	if (listen(_server_fd->getSocketFd(), 10) == -1) {
		std::cerr << "Erro ao escutar no socket" << std::endl;
		close(_server_fd->getSocketFd());
		freeaddrinfo(_server_fd->getRes());
		return ;
	}

	// // Registra o server socket na epoll para monitorar
	_server_fd->setEvent(EPOLLIN, _server_fd->getSocketFd());
}

Socket *HttpServer::initClientSocket()
{
    Socket *client_fd = new Socket();

    // Aloca memória para o endereço do cliente
    sockaddr_storage addr; // Usando sockaddr_storage para suportar IPv4 e IPv6
    socklen_t addr_len = sizeof(addr);

    // Aceita a conexão
    int socket = accept(_server_fd->getSocketFd(), (sockaddr *)&addr, &addr_len);
    if (socket == -1) {
        std::cerr << "Erro ao aceitar nova conexão" << std::endl;
        delete client_fd; // Libera a memória do socket se não for usado
        return NULL;
    }

    // Configura o socket do cliente
    client_fd->setSocketFd(socket);
    int events = EPOLLIN | EPOLLET; // Monitorar eventos de leitura (EPOLLIN) e usar o modo edge-triggered (EPOLLET)
    client_fd->setEvent(events, client_fd->getSocketFd());

    // Configura o endereço do cliente
    addrinfo *res = new addrinfo(); // Aloca memória para addrinfo
    res->ai_addr = (sockaddr *)new sockaddr_storage(addr); // Copia o endereço do cliente
    res->ai_addrlen = addr_len; // Armazena o tamanho do endereço do cliente

    // Configura a família de endereços com base no tipo de endereço recebido
    if (addr.ss_family == AF_INET) {
        res->ai_family = AF_INET; // IPv4
    } else if (addr.ss_family == AF_INET6) {
        res->ai_family = AF_INET6; // IPv6
    } else {
        std::cerr << "Família de endereços desconhecida ao aceitar conexão" << std::endl;
        delete client_fd;
        delete res;
        return NULL;
    }

    client_fd->setRes(res); // Armazena o resultado no socket do cliente

    // Adicionar o non-blocking ao socket do cliente
    int flags = fcntl(client_fd->getSocketFd(), F_GETFL, 0);
    fcntl(client_fd->getSocketFd(), F_SETFL, flags | O_NONBLOCK);

    int keepalive = 1; // Ativa o keepalive para o socket do cliente
    setsockopt(client_fd->getSocketFd(), SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)); // Permite reutilizar o endereço

	// printServer(client_fd); // Imprime as informações do servidor
    return client_fd;
}

// ### GETTERS ###
Socket *HttpServer::getServerSocket() const {
	return _server_fd;
}