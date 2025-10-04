/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 18:24:43 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/04 19:52:43 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

# include "headers.hpp"

// pre definition of Server and Client
class Server;
class Client;
class Configuration;
class SessionData;

class WebServer
{
	private:
	std::map<int, std::string>							_partial_requests; // Eh um buffer universal, que todas as requisicoes passarao, feito para armazenar requisições parciais
	int													_epoll_fd; // armazena o fd do epoll para usar nas funcoes
	std::map<int, Server *>								_servers_map; // mapeia uma porta especifica para servidores (pensar em vecrtor de servers depois)
	std::vector<Client *>								_clients_vec;
	std::map<int, std::pair<std::string, std::string> >	_client_to_server_map; // mapeia o fd do cliente para o ip:port do servidor
	std::vector<Configuration>							_configurations; // armazena as configurações do servidor
	struct epoll_event									*_events; // é usado como buffer, recebe os eventos que aconteceram nos descritores monitorados
	char 												_buffer[BUFFER_SIZE]; // buffer para leitura de dados
	std::vector<SessionData *>							*_sessions;
	
	bool 								isRequestComplete(const std::string &data);
	int 								extractContentLength(const std::string& headers);
	std::string							extractHostHeaderSimple(const std::string &rawRequest);
	bool								isLargeFileRequest(Client *client);
	std::string							getContentType(const std::string& filePath);
	void								handleClientInput(Client *client, int i);
	void								handleClientOutput(Client *client, int i);
	void								logStreamingError(int client_fd, const std::string& operation, const std::string& details = "");
	void								logStreamingInfo(int client_fd, const std::string& message);
	int									startLargePostUpload(Client* client, size_t body_start, int content_length);
	int									continueLargePostUpload(Client* client, const char* buffer, size_t length);
	int									finishLargePostUpload(Client* client);


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

		// ### AFTER REQUEST PARSING ###
		int									getServerFdForClient(int client_fd);
		Configuration* 						findConfigForRequestFast(const std::string& rawRequest, int client_fd);
		bool								startLargeFileStreaming(Client* client);
		bool								continueLargeFileStreaming(Client* client);

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