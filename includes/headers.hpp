/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 18:00:37 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 17:53:49 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERS_HPP
#define HEADERS_HPP

# include <iostream>
# include <iomanip>			// includes std::setw(), std::setfill(), etc.
# include <fcntl.h>			// includes fcntl(), O_NONBLOCK, etc.
# include <fstream>			// includes std::ifstream, std::ofstream, std::fstream, etc.
# include <sstream>			// includes std::stringstream, std::istringstream, etc.
# include <string>			// includes std::string, std::to_string(), etc.
# include <vector>			// includes std::vector, std::sort(), etc.
# include <unistd.h>		// includes close(), read(), write(), etc.
# include <arpa/inet.h>		// includes inet_ntop(), inet_pton(), etc.
# include <cstring>			// includes memset(), memcpy(), etc.
# include <sys/socket.h>	// includes socket(), bind(), accept(), connect(), etc.
# include <netdb.h>			// includes getaddrinfo(), getnameinfo(), etc.
# include <sys/types.h>		// includes types like sockaddr, socklen_t, etc.
# include <sys/epoll.h>		// includes epoll_create(), epoll_ctl(), epoll_wait(), etc.
# include <fcntl.h>			// includes fcntl(), open(), O_RDONLY, O_WRONLY, etc.
# include <cstdlib>			// includes std::exit(), std::atoi(), std::atof(), etc.
# include <ctime>			// includes std::time_t and std::time()

# include "macros.hpp"
# include "Socket.hpp"
# include "HttpRequest.hpp"
# include "WebServer.hpp"
# include "Client.hpp"
# include "Server.hpp"

// remover depois!!!! servem para printar o hyperlink no terminal
# include <netdb.h>
# include <arpa/inet.h>

#endif