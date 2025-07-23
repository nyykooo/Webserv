/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 15:53:12 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/21 19:08:26 by ncampbel         ###   ########.fr       */
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
		
	public:
		Socket();
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
		~Socket();

	// ### GETTERS ###
		int					getSocketFd();
		struct epoll_event	&getEvent();
		struct addrinfo		*getRes();
		struct addrinfo		*getHints();

	// ### SETTERS ###
		void				setSocketFd(int fd);
		void				setEvent(int event_flags, int fd);
		void				setRes(struct addrinfo *res);
		void				setHints(struct addrinfo hints);

	// ### EXCEPTION ###
		class SocketErrorException: public std::exception {
			private:
				std::string	_message;
			public:
				SocketErrorException(const std::string& message): _message("Socket error detected: " + message) {}
				virtual ~SocketErrorException() throw() {};
				const char* what() const throw();
		};
};

#endif