/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 15:53:12 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/06 17:11:25 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "headers.hpp"

class Socket
{
	private:
		int					_socket_fd; // armazena o file descriptor do socket
		struct sockaddr		_addr; // armazena o endereço do socket
		struct epoll_event	_event; // armazena o evento do epoll para usar nas funcoes.
		struct addrinfo		_hints, *_res; // armazena as informações de endereço para o socket
		
	public:
		Socket();
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
		~Socket();

	// ### GETTERS ###
		int					getSocketFd();
		struct sockaddr		&getAddress();
		struct epoll_event	&getEvent();
		struct addrinfo		*getRes();
		struct addrinfo		*getHints();

	// ### SETTERS ###
		void				setSocketFd(int fd);
		void				setAddress(struct sockaddr addr);
		void				setEvent(int event_flags, int fd);
		void				setRes(struct addrinfo *res);
		void				setHints(struct addrinfo hints);
};

#endif