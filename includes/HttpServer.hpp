/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 12:10:32 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 15:03:03 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Http_SERVER_HPP
#define Http_SERVER_HPP

# include "headers.hpp"

// Declaração antecipada da classe Socket
class Socket;

class HttpServer {
	private:
		Socket							*_server_fd; // mudar pra server_socket para ficar mais claro
		std::vector<Socket *>			_client_fds;
		
	public:
		HttpServer();
		HttpServer(const std::string &port);
		HttpServer(const HttpServer &other);
		HttpServer &operator=(const HttpServer &other);
		~HttpServer();

		void	initEpoll();
		void	initServerSocket(std::string port);
		Socket	*initClientSocket();
		void	printServer(Socket *socket);
		void	startServer();
		int		handleNewClient();
		void	receiveData(int client_fd);

		// ### GETTERS ###
		Socket *getServerSocket() const;
};

#endif