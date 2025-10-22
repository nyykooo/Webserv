/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/22 17:42:13 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

int	main(int argc, char** argv)
{
	const char	*configFile;

	switch (argc)
	{
		case 1:
			configFile = "default.config";
			std::cout << YELLOW << "Using default configuration file: " << configFile << RESET << std::endl;
			break ;
		case 2:
			configFile = argv[1];
			break ;
		default:
			std::cerr << RED << "Wrong input. Usage: ./webserv [configuration_file]" << RESET << std::endl;
			return (1);
	}

	std::vector<Configuration>	configVector;
	try	{
		setup(configFile, configVector);
		WebServer webServer(configVector);
		webServer.startServer();
	}
	catch (const std::exception& e) {
		printLog(e.what(), RED, std::cerr);
		return (1);
	}
}