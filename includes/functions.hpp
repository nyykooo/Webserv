/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:14:08 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/26 13:35:03 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS
# define FUNCTIONS

# include "headers.hpp"


int			execMethod(Client *client);
int			findLocation(Client *client);

// ### TIMESTAMP LOGS ###
std::string							setTimeStamp();
void								printLog(std::string message, const char *color);

#endif