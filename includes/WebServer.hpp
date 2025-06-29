/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:43 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/26 21:11:35 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

# include "headers.hpp"

class WebServer
{
	private:
		int	_epoll_fd; // armazena o fd do epoll para usar nas funcoes
		std::map<std::string, HttpServer *> _servers_map; // mapeia uma porta especifica para servidores (pensar em vecrtor de servers depois)
		std::vector<Socket *>	_clients_vec;
		std::vector<struct epoll_event>	_events; // é usado como buffer, recebe os eventos que aconteceram nos descritores monitorados
		char 							_buffer[BUFFER_SIZE]; // buffer para leitura de dados

	public:
		WebServer();
		WebServer(const WebServer &other);
		WebServer &operator=(const WebServer &other);
		~WebServer();

		int		initEpoll(void);
		void	registerEpollSocket(Socket *socket);
		void	startServer();
		int	receiveData(int client_fd);
};

#endif