/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:45:14 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/19 20:27:46 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "headers.hpp"

class Client : public Socket {
	private:
		std::time_t				_time;
		RequestProcessingState	_state;
		int						_fileFd; // usado para o streaming de arquivos
		size_t					_fileSize; // usado para o streaming de arquivos
		size_t					_bytesSent; // usado para o streaming de arquivos

	public:
		Client();
		Client(int server_fd);
		Client(const Client& other);
		Client &operator=(const Client& other);
		~Client();

		// ### PUBLIC METHODS ###
		Client *initClientSocket(int server_fd);
		bool	checkTimeout() const;
		RequestProcessingState getProcessingState() const;
    	void setProcessingState(RequestProcessingState state);
		
		// ### GETTERS ###
		std::time_t	getTime() const;
		const int& 		getFileFd() const;
		const size_t& 	getFileSize() const;
		const size_t& 	getBytesSent() const;

		// ### SETTERS ###
		void	setTime(std::time_t time);
		void	setFileFd(int fd);
		void	setFileSize(size_t size);
		void	setBytesSent(size_t bytes);

		// ### ATRIBUTES ###
		HttpResponse*	sendResponse;
		HttpRequest*	_request;

		void	resetFileStreaming(); // zerar tudo e settar fd pra -1
		bool	isFileStreaming() const; // verifica se está streaming
		double	getStreamingProgress() const; // verifica progresso
};

#endif