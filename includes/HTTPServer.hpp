/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 12:10:32 by ncampbel          #+#    #+#             */
/*   Updated: 2025/05/29 20:16:47 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

# include "headers.hpp"

// Declaração antecipada da classe Socket
class Socket;

class HTTPServer {
	private:
		// struct sockaddr		_addr;
		// int					_server_fd;
		// std::vector<int>	_client_fds;
		int					_epoll_fd; // armazena o fd do epoll para usar nas funcoes
		Socket				_server_fd;
		std::vector<Socket *>	_client_fds;
	public:
		HTTPServer();
		HTTPServer(const HTTPServer &other);
		HTTPServer &operator=(const HTTPServer &other);
		~HTTPServer();

	void	initServer();
	void	printServer();
	void	startServer();
	void	handleNewClient(Socket *client);
};

#endif