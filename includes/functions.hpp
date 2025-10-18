/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:14:08 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/27 21:17:24 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS
# define FUNCTIONS

# include "headers.hpp"

// ### HTTP METHODS ###
int			findLocation(Client *client);

// ### TIMESTAMP LOGS ###
std::string	setTimeStamp();
std::string get_http_date();
void		printLog(std::string message, const char *color, std::ostream &stream);

// ### UTILS ### 
std::string toLower(const std::string &str);
std::string removeSlashes(std::string path);

std::string getContentType(const std::string &filePath);

#endif