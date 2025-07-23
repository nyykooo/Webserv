/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timeStamp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:13:55 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/21 19:37:03 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

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