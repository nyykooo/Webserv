/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:45:14 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/14 23:12:35 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "headers.hpp"

// Declaração antecipada da classe Client
class Client;

class Server : public Socket {
	private:
		std::vector<Client *>	_client_fds;
		std::string				_ip;
		std::string				_port;

	public:
		Server();
		Server(const Server& other);
		Server(const std::string &ip, const std::string &port);
		Server &operator=(const Server& other);
		~Server();

		// ### PUBLIC METHODS ###
		void	initServerSocket(std::string ip, std::string port);
		Socket	*initClientSocket();
		void	printServer(Socket *socket);
		int		handleNewClient();

		// ### GETTERS ###
		std::vector<Client *>	getTime() const;
		std::string				getIp() const;
		std::string				getPort() const;

		// ### SETTERS ###
		void	setTime(std::vector<Client *> time);
		void	setIp(const std::string &ip);
		void	setPort(const std::string &port);
};

#endif