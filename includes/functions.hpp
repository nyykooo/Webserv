/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:14:08 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/15 15:17:51 by discallow        ###   ########.fr       */
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
std::string get_http_date();
void		printLog(std::string message, const char *color);

#endif