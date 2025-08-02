/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   macros.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 20:55:39 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/02 13:57:57 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MACROS_HPP
#define MACROS_HPP

# define MAX_EVENTS 100 // Número máximo de eventos que o epoll pode monitorar
# define BUFFER_SIZE 4096 // Tamanho do buffer para leitura de dados


// ### OUTPUT COLORS ###
# define RED	"\033[31m"
# define GREEN	"\033[32m"
# define BLUE	"\033[34m"
# define WHITE	"\033[37m"
# define BLACK	"\033[30m"
# define YELLOW	"\033[33m"
# define CYAN	"\033[36m"
# define GRAY	"\033[90m"
# define RESET	"\033[0m"

# define CRLF "\r\n"

# define HTTP_200 "200 OK"
# define ERROR_404 "404 Not Found"
# define ERROR_413 "413 Request Entity Too Large"
#endif