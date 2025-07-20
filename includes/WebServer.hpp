/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:43 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/19 23:47:35 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

# include "headers.hpp"

// pre definition of Server and Client
class Server;
class Client;
class Configuration;

class WebServer
{
	private:
		int													_epoll_fd; // armazena o fd do epoll para usar nas funcoes
		std::map<int, Server *>								_servers_map; // mapeia uma porta especifica para servidores (pensar em vecrtor de servers depois)
		std::vector<Client *>								_clients_vec;
		std::map<int, std::pair<std::string, std::string> >	_client_to_server_map; // mapeia o fd do cliente para o ip:port do servidor
		std::vector<Configuration>							_configurations; // armazena as configurações do servidor
		struct epoll_event									*_events; // é usado como buffer, recebe os eventos que aconteceram nos descritores monitorados
		char 												_buffer[BUFFER_SIZE]; // buffer para leitura de dados

	public:
		WebServer();
		WebServer(std::vector<Configuration> conf);
		WebServer(const WebServer &other);
		WebServer &operator=(const WebServer &other);
		~WebServer();

		int									initEpoll(void);
		void								registerEpollSocket(Socket *socket);
		void								handleNewClient(int server_fd);

		// ### GETTERS ###
		int									getEpollFd() const;
		std::map<int, Server *>				getServersMap() const;
		std::vector<Client *>				getClientsVec() const;
		struct epoll_event					*getEvents() const;
		char								*getBuffer();
		const std::vector<Configuration>	&getConfigurations() const;

		// ### SETTERS ###
		void								setEpollFd(int epoll_fd);
		void								setServersMap(const std::map<int, Server *> &servers_map);
		void								setClientsVec(const std::vector<Client *> &clients_vec);
		void								setEvents(struct epoll_event *events);
		void								setBuffer(const char *buffer);

		// ### SERVER LOOP ###
		void								startServer();
		void								handleEvents(int event_count);
		void								treatExistingClient(int i);
		void								setClientTime(int client_fd);
		void								sendData(int client_fd);
		int									receiveData(int client_fd);
		bool								tryConnection(int i);
		void								deleteClient(int fd);

		// ### TIMEOUT ###
		void								lookForTimeouts();

		// ### AFTER REQUEST PARSING ###
		int									getServerFdForClient(int client_fd);
		Configuration						*findConfigForRequest(const HttpRequest& request, int client_fd);

		// ### EXCEPTION ###
		class WebServerErrorException: public std::exception {
			private:
				std::string	_message;
			public:
				WebServerErrorException(const std::string& message): _message("WebServer error detected: " + message) {}
				virtual ~WebServerErrorException() throw() {};
				const char* what() const throw();
		};
};

#endif