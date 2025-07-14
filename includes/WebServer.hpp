/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:43 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/14 21:20:16 by brunhenr         ###   ########.fr       */
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
		std::map<int, int> _client_to_server_map; // associar client fd com server fd	

		int									_epoll_fd; // armazena o fd do epoll para usar nas funcoes
		std::map<int, Server *>		_servers_map; // mapeia uma porta especifica para servidores (pensar em vecrtor de servers depois)
		std::vector<Client *>				_clients_vec;
		struct epoll_event					*_events; // é usado como buffer, recebe os eventos que aconteceram nos descritores monitorados
		char 								_buffer[BUFFER_SIZE]; // buffer para leitura de dados
		std::vector<Configuration>			_configurations; // armazenamos as configurações do servidor

	public:
		WebServer();
		WebServer(const WebServer &other);
		WebServer &operator=(const WebServer &other);
		~WebServer();

		WebServer(const std::vector<Configuration>& configs);

		int									initEpoll(void);
		void								registerEpollSocket(Socket *socket);
		void								handleNewClient(int server_fd);

		// ### GETTERS ###
		int									getEpollFd() const;
		std::map<int, Server *>		getServersMap() const;
		std::vector<Client *>				getClientsVec() const;
		struct epoll_event					*getEvents() const;
		char								*getBuffer();
		const std::vector<Configuration>&	getConfigurations() const;


		// ### SETTERS ###
		void								setEpollFd(int epoll_fd);
		void								setServersMap(const std::map<int, Server *> &servers_map);
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

		//Configuration* findConfigForRequest(const HttpRequest& request);
		Configuration*	findConfigForRequest(const HttpRequest& request, const int& server_fd);
		int				getServerFdForClient(int client_fd);
};

#endif