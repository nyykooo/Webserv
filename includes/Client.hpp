/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:45:14 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/22 12:24:32 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "headers.hpp"

class Client : public Socket {
	private:
		std::time_t _time;

	public:
		Client();
		Client(int server_fd);
		Client(const Client& other);
		Client &operator=(const Client& other);
		~Client();

		// ### PUBLIC METHODS ###
		Client *initClientSocket(int server_fd);
		bool	checkTimeout() const;
		HttpResponse*	sendResponse;

		// ### GETTERS ###
		std::time_t	getTime() const;

		// ### SETTERS ###
		void	setTime(std::time_t time);

};

#endif