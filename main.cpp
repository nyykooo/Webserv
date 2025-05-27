/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/05/27 18:31:47 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/headers.hpp"

int main(void)
{   
	// struct addrinfo hints, *res; // hints eh um struct que contém as informações de configuração do socket
	// int socket_fd; // socket file descriptor
	// struct sockaddr addr; // addr é um struct que contém o endereço do socket

    HTTPServer server;

    server.initServer();
    server.printServer();
    server.startServer();

	// memset(&hints, 0, sizeof(hints));
	// hints.ai_family = AF_UNSPEC; // AF_INET ou AF_INET6 também serve
	// hints.ai_socktype = SOCK_STREAM;
	// hints.ai_flags = AI_PASSIVE; // para bind()

	// // Obtém informações de endereço
    // if (getaddrinfo("127.0.0.0", "8080", &hints, &res) != 0) {
    //     std::cerr << "Erro em getaddrinfo" << std::endl;
    //     return 1;
    // }

	// AF_INTET6 stands for ipv6
	// SOCK_STREAM stands for TCP sockets
	// 0 means the default protocol, which is TCP for SOCK_STREAM
    // Cria o socket
    // socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // if (socket_fd == -1) {
    //     std::cerr << "Erro ao criar o socket" << std::endl;
    //     freeaddrinfo(res);
    //     return 1;
    // }
	
    // Faz o bind
    // if (bind(socket_fd, res->ai_addr, res->ai_addrlen) == -1) {
    //     std::cerr << "Erro ao fazer bind" << std::endl;
    //     close(socket_fd);
    //     freeaddrinfo(res);
    //     return 1;
    // }

	// // Converte o endereço para string legível
    // char ip_str[INET6_ADDRSTRLEN];
    // void *addr_ptr;

    // if (res->ai_family == AF_INET) { // IPv4
    //     struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    //     addr_ptr = &(ipv4->sin_addr);
    // } else if (res->ai_family == AF_INET6) { // IPv6
    //     struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    //     addr_ptr = &(ipv6->sin6_addr);
    // } else {
    //     std::cerr << "Família de endereços desconhecida" << std::endl;
    //     close(socket_fd);
    //     freeaddrinfo(res);
    //     return 1;
    // }

    // // Converte o endereço binário para string
    // inet_ntop(res->ai_family, addr_ptr, ip_str, sizeof(ip_str));
    // std::cout << "Endereço IP: " << ip_str << std::endl;

	
	// listen(socket_fd, 10); // 10 é o backlog, ou seja, o número máximo de conexões pendentes
	// std::cout << "Servidor iniciado na porta 8080" << std::endl;
	// while (1)
	// {
	// 	socklen_t addr_len = sizeof(addr);
	// 	int client_fd = accept(socket_fd, &addr, &addr_len); // aceita uma conexão
	// 	if (client_fd < 0)
	// 	{
	// 		std::cerr << "Erro ao aceitar conexão" << std::endl;
	// 		continue; // continua o loop se houver erro
	// 	}
		
	// 	std::cout << "Cliente conectado" << std::endl;
		
	// 	// Aqui você pode ler e escrever no socket do cliente usando client_fd
	// 	close(client_fd); // fecha o socket do cliente após o uso
	// 	break;
	// }

    // // Libera recursos
    // freeaddrinfo(res);
    // close(socket_fd);
}