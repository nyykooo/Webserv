/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:14:08 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/19 18:42:28 by ncampbel         ###   ########.fr       */
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

// ### UTILS ### 
std::string toLower(const std::string &str);
std::string removeSlashes(std::string path);

#endif