/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 15:53:12 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/13 16:48:27 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "headers.hpp"

class Socket
{
	protected:
		int					_socket_fd; // armazena o file descriptor do socket
		struct epoll_event	_event; // armazena o evento do epoll para usar nas funcoes.
		struct addrinfo		_hints, *_res; // é uma struct auxiliar que tb armazena o ip e porta
		std::string			_ip;
		std::string			_port; // esses atributos darão muito jeito pra simplificar o code
		
	public:
		Socket();
		Socket(const std::string& ip, const std::string& port);  // NOVO construtor
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
		~Socket();

	// ### GETTERS ###
		int					getSocketFd();
		struct epoll_event	&getEvent();
		struct addrinfo		*getRes();
		struct addrinfo		*getHints();

		const std::string& getIp() const { return _ip; } //passar a implementaÇão para o .cpp
		const std::string& getPort() const { return _port; }

	// ### SETTERS ###
		void				setSocketFd(int fd);
		void				setEvent(int event_flags, int fd);
		void				setRes(struct addrinfo *res);
		void				setHints(struct addrinfo hints);
		void				setIpPort(const std::string& ip, const std::string& port);  // NOVO

		void updateIpPortFromAddrinfo();
		void initAddrinfo(const std::string& ip, const std::string& port);  // NOVO
};

#endif