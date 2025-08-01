/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:45:14 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/25 17:47:07 by ncampbel         ###   ########.fr       */
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
		HttpRequest*	_request;

		// ### GETTERS ###
		std::time_t	getTime() const;

		// ### SETTERS ###
		void	setTime(std::time_t time);

};

#endif