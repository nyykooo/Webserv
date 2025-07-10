/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/10 21:08:57 by ncampbel         ###   ########.fr       */
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
    std::cout << "localhost URL:" << "\033]8;;http://127.0.0.1:8080\033\\"
              << "http://127.0.0.1:8080"
              << "\033]8;;\033\\" << std::endl;
    return;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << RED << "Wrong input. Correct usage: ./webserv [configuration_file]" << RESET << std::endl;
        return (1);
    }
    std::vector<Configuration> configFile;
    
    try {
      setup(argv[1], configFile);
    }
    catch (const Configuration::WrongConfigFileException& e) {
      std::cerr << RED << e.what() << RESET << std::endl;
    }
	printHL();
	WebServer webServer;
	webServer.startServer();
}