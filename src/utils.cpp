/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:13:55 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/26 13:41:46 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### HTTP METHODS ###

static int	handleGET(Client *client)
{
	static_cast<void>(client);
	std::cout << "GET detected" << std::endl;
	return 200;
}

int	execMethod(Client *client)
{
	HttpRequest	*req = client->_request;
	std::string	method = req->getMethod();

	if (method == "GET")
		return handleGET(client);
	return 400;
}

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
