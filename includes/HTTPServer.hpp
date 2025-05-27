/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 12:10:32 by ncampbel          #+#    #+#             */
/*   Updated: 2025/05/27 18:31:01 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

# include "headers.hpp"

/* class Socket {
	private:
		int				_fd;
		struct sockaddr	_addr;
	public:
		[...]
}*/

class HTTPServer {
	private:
		struct sockaddr		_addr;
		int					_server_fd;
		std::vector<int>	_client_fds;
		struct addrinfo		_hints, *_res;
		// Socket				_server_fd;
		// std::vector<Socket>	_client_fds;
	public:
		HTTPServer();
		HTTPServer(const HTTPServer &other);
		HTTPServer &operator=(const HTTPServer &other);
		~HTTPServer();

	void	initServer();
	void	printServer();
	void	startServer();
	void	handleNewClient(int client_fd);
};

#endif