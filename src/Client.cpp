/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:51:13 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 15:38:26 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### ORTHODOX CANONICAL FORM ###
Client::Client()
{
	_time = std::time(NULL);
}

Client::Client(const Client &other)
{
	*this = other;
}

Client	&Client::operator=(const Client& other)
{
	_time = other.getTime();
}

// ### GETTERS ###
std::time_t	Client::getTime() const
{
	return _time;
}

// ### SETTERS ###
void	Client::setTime(std::time_t time)
{
	_time = time;
}