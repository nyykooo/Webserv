/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/05 16:54:10 by discallow        ###   ########.fr       */
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

int	main(int argc, char** argv)
{
	const char	*configFile;

	if (argc == 1)
	{
		configFile = "default.config";
		std::cout << YELLOW << "Using default configuration file: " << configFile << RESET << std::endl;
	}
	else if (argc == 2)
		configFile = argv[1];
	else
	{
		std::cerr << RED << "Wrong input. Usage: ./webserv [configuration_file]" << RESET << std::endl;
		return (1);
	}
	std::vector<Configuration>	configVector;
	try	{
		setup(configFile, configVector);
		WebServer webServer(configVector);
		webServer.startServer();
		printHL();
	}
	catch (const std::exception& e) {
		printLog(e.what(), RED, std::cerr);
		return (1);
	}
}