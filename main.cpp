/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/02 17:47:25 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/headers.hpp"
static void printHL(void)
{
    char hostname[256];
	std::string link;
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent* host = gethostbyname(hostname);
        if (host != NULL) {
            struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
			std::string addr(inet_ntoa(*addr_list[0]));
			link = "http://" + addr + ":8080";
        }
    }

    std::cout << "Machine IP URL: " << "\033]8;;" << link << "\033\\"
              << link
              << "\033]8;;\033\\" << std::endl;
    std::cout << "localhost URL:" << "\033]8;;127.0.0.1:8080\033\\"
              << "http://127.0.0.1:8080"
              << "\033]8;;\033\\" << std::endl;
    return;
}

int main(void)
{
	printHL();
	WebServer webServer;
	webServer.startServer();
}