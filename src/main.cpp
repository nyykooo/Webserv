/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/15 21:42:33 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

int	main(int argc, char** argv)
{
	const char	*configFile;

	switch (argc)
	{
		case 1:
			configFile = DEFAULT_CONFIG_FILE;
			std::cout << YELLOW << "Using default configuration file: " << configFile << RESET << std::endl;
			break;
		case 2:
			configFile = argv[1];
			std::cout << YELLOW << "Using configuration file: " << argv[1] << RESET << std::endl;
			break;
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