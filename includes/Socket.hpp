/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 15:53:12 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 15:43:50 by ncampbel         ###   ########.fr       */
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
		struct addrinfo		_hints, *_res; // armazena as informações de endereço para o socket
		
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
};

#endif