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
		std::stringstream									_logger;
		std::map<int, std::string>							_partial_requests;
		int													_epoll_fd;
		std::map<int, Server *>								_servers_map;
		std::vector<Client *>								_clients_vec;
		std::map<int, std::pair<std::string, std::string> >	_client_to_server_map;
		std::vector<Configuration>							_configurations;
		struct epoll_event									*_events;
		std::vector<SessionData *>							*_sessions;
		Client												*_currentClient;
		Server												*_server;
		
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


		// ### TESTANDO STARTSERVER DENTRO DA WEBSERVER ###
		void	startServer();
		void	lookForTimeouts();
		void	handleEvents(int event_count);
		void	treatExistingClient(int i);
		void	setClientTime(int client_fd);
		void	sendData(int client_fd);
		int		receiveData(int client_fd);
		bool	tryConnection(int i);
		void	deleteClient(int fd, int event);


		void	handleEpollInEvent(int i);
		void	handleEpollOutEvent(int i);
		void	handleEpollErrEvent(int i);
		void	handleEpollHupEvent(int i);
		void	handleEpollRdHupEvent(int i);

		// ### AFTER REQUEST PARSING ###
		Configuration* 						findConfigForRequestFast(const std::string& rawRequest, int client_fd);
		bool								continueLargeFileStreaming();


		// ### streaming ###
		int 								extractContentLength(const std::string& headers);
		std::string							extractHostHeaderSimple(const std::string &rawRequest);
		void								handleClientInput(int i);
		void								handleClientOutput(int i);

		Client								*findFd(int fd);
		void								execCgiOp(int i, Client *client);
		void								execFStreamOp(int i, Client *client);

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