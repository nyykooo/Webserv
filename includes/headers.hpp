/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 18:00:37 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/22 17:03:48 by discallow        ###   ########.fr       */
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
# include <sys/wait.h>		// includes waitpid, etc.
# include <netdb.h>			// includes getaddrinfo(), getnameinfo(), etc.
# include <sys/types.h>		// includes types like sockaddr, socklen_t, etc.
# include <sys/stat.h>		// includes stat and S_ISxxx macros
# include <sys/epoll.h>		// includes epoll_create(), epoll_ctl(), epoll_wait(), etc.
# include <fcntl.h>			// includes fcntl(), open(), O_RDONLY, O_WRONLY, etc.
# include <cstdlib>			// includes std::exit(), std::atoi(), std::atof(), etc.
# include <ctime>			// includes std::time_t and std::time()
# include <dirent.h>		// includes opendir(), readdir(), closedir(), etc.
# include <sys/wait.h>		// includes waitpid(), WNOHANG, etc.
# include <map>
# include <set>
# include <stdexcept>
# include <climits>


# include "macros.hpp"
# include "enums.hpp"
# include "Socket.hpp"
# include "Block.hpp"
# include "Configuration.hpp"
# include "LocationBlock.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "WebServer.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "ErrorPageRule.hpp"
# include "functions.hpp"
# include "SessionData.hpp"
# include "ErrorPages.hpp"
# include "Signals.hpp"

// utils 
std::string toLower(const std::string &str);
std::string removeSlashes(std::string path);

#endif