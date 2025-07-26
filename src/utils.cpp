/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:13:55 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/26 19:45:19 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"


// ### LOCATION HANDLERS ###

int	handleLocation(Client *client, LocationBlock loc)
{
	std::vector<std::string> methods = loc.getMethods();
	HttpRequest	*req = client->_request;
	std::string	reqMethod = req->getMethod();
	
	std::vector<std::string>::iterator it;
	for (it = methods.begin(); it != methods.end(); ++it)
	{
		if ((*it).compare(reqMethod) == 0)
			return 1;
	}
	return 0;
}

int	findLocation(Client *client)
{
	std::string path = client->_request->getPath();
	std::vector<LocationBlock> loc = client->_request->_config->locations;

	std::vector<LocationBlock>::iterator it;
	for (it = loc.begin(); it != loc.end(); ++it)
	{
		if (path.compare((*it).getLocation()) == 0)
			return handleLocation(client, (*it));
	}
	return 200; // handleSBRoot
}

// ### TIMESTAMP LOGS ###
std::string setTimeStamp()
{
	std::time_t timestamp = std::time(&timestamp);
	std::tm *timeinfo = std::localtime(&timestamp);

	std::ostringstream oss;
	oss << (1900 + timeinfo->tm_year) 
	<< "-" << std::setfill('0') << std::setw(2) << (1 + timeinfo->tm_mon) 
	<< "-" << std::setfill('0') << std::setw(2) << (timeinfo->tm_mday) 
	<< "T" << std::setfill('0') << std::setw(2) << (timeinfo->tm_hour)
	<< ":" << std::setfill('0') << std::setw(2) << (timeinfo->tm_min)
	<< ":" << std::setfill('0') << std::setw(2) << (timeinfo->tm_sec);

	return oss.str();
}

void	printLog(std::string message, const char	*color)
{
	std::string time = setTimeStamp();
	std::cout << "[" + time + "]: " << color << message << RESET << std::endl;
}