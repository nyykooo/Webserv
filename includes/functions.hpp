/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:14:08 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/25 19:09:14 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS
# define FUNCTIONS

# include "headers.hpp"

// ### HTTP METHODS ###
int			execMethod(Client *client);
int			findLocation(Client *client);

// ### TIMESTAMP LOGS ###
std::string	setTimeStamp();
void		printLog(std::string message, const char *color);

#endif