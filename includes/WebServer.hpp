/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:43 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/07 18:29:08 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

# include "headers.hpp"

// pre definition of Server and Client
class Server;
class Client;

class WebServer
{
	private:
		int									_epoll_fd; // armazena o fd do epoll para usar nas funcoes
		std::map<std::string, Server *>	_servers_map; // mapeia uma porta especifica para servidores (pensar em vecrtor de servers depois)
		std::vector<Client *>				_clients_vec;
		struct epoll_event					*_events; // é usado como buffer, recebe os eventos que aconteceram nos descritores monitorados
		char 								_buffer[BUFFER_SIZE]; // buffer para leitura de dados

	public:
		WebServer();
		WebServer(const WebServer &other);
		WebServer &operator=(const WebServer &other);
		~WebServer();

		int									initEpoll(void);
		void								registerEpollSocket(Socket *socket);
		void								handleNewClient(int server_fd);

		// ### GETTERS ###
		int									getEpollFd() const;
		std::map<std::string, Server *>		getServersMap() const;
		std::vector<Client *>				getClientsVec() const;
		struct epoll_event					*getEvents() const;
		char								*getBuffer();

		// ### SETTERS ###
		void								setEpollFd(int epoll_fd);
		void								setServersMap(const std::map<std::string, Server *> &servers_map);
		void								setClientsVec(const std::vector<Client *> &clients_vec);
		void								setEvents(struct epoll_event *events);
		void								setBuffer(const char *buffer);


		// ### TESTANDO STARTSERVER DENTRO DA WEBSERVER ###
		void startServer();
		void lookForTimeouts();
		void handleEvents(int event_count);
		void treatExistingClient(int i);
		void setClientTime(int client_fd);
		void sendData(int client_fd);
		int receiveData(int client_fd);
		bool tryConnection(int i);
		void deleteClient(int fd);
		
};

#endif