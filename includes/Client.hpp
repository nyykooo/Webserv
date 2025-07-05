/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:45:14 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/05 14:50:40 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "headers.hpp"

class Client : private Socket {
	private:
		std::time_t _time;

	public:
		Client();
		Client(const Client& other);
		Client &operator=(const Client& other);
		~Client();

		// ### GETTERS ###
		std::time_t	getTime() const;

		// ### SETTERS ###
		void	setTime(std::time_t time);
};

#endif