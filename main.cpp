/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/07 13:44:46 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/headers.hpp"

int main(void)
{
    HttpServer server;

    server.printServer();
    server.startServer();
}