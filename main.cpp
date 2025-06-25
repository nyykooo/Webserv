/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/06/25 19:46:30 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/headers.hpp"

int main(void)
{
    HttpServer server1("8080");
    HttpServer server2("80");

    while (true)
    {
        server1.startServer();
        server2.startServer();
    }
}