/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 12:10:32 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/13 18:50:13 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Http_SERVER_HPP
#define Http_SERVER_HPP

# include "headers.hpp"

// Declaração antecipada da classe Socket
class Socket;

class HttpServer {
	private:
		int								_epoll_fd; // armazena o fd do epoll para usar nas funcoes
		Socket							*_server_fd; // mudar pra server_socket para ficar mais claro
		std::vector<Socket *>			_client_fds;
		std::vector<struct epoll_event>	_events; // é usado como buffer, recebe os eventos que aconteceram nos descritores monitorados
		char 							_buffer[BUFFER_SIZE]; // buffer para leitura de dados
		
	public:
		HttpServer();
		HttpServer(const HttpServer &other);
		HttpServer &operator=(const HttpServer &other);
		~HttpServer();

		void	initEpoll();
		void	initServerSocket();
		Socket	*initClientSocket();
		void	printServer();
		void	startServer();
		void	handleNewClient();
		void	receiveData(int client_fd);
};

#endif