/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:45:14 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/27 17:36:39 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "headers.hpp"

class Client : public Socket {
	private:
		std::time_t						_time;
		RequestProcessingState			_state;
		std::string 					_originalHeaders;
		int 							_fileFd;
		size_t 							_fileSize;
		size_t 							_bytesSent;
		int 							_uploadFd;
		size_t 							_uploadSize;
		size_t 							_uploadReceived;
		std::string						_uploadPath;
		bool							_firstRequest;
		int								_epoll_fd;

	public:
		// ### ATRIBUTES ###
		HttpResponse*					_response;
		HttpRequest*					_request;
		Server*							_server;

		Client();
		Client(int server_fd);
		Client(const Client& other);
		Client &operator=(const Client& other);
		~Client();

		// ### PUBLIC METHODS ###
		Client 							*initClientSocket(int server_fd);
		bool							checkTimeout() const;
		RequestProcessingState	 		getProcessingState() const;
    	void 							setProcessingState(RequestProcessingState state);
		void							resetFileStreaming();
		bool							isFileStreaming() const;
		double							getStreamingProgress() const;
		
		// ### GETTERS ###
		std::time_t 					getTime() const;
		const int	 					&getFileFd() const;
		const size_t 					&getFileSize() const;
		const size_t 					&getBytesSent() const;
		const int 						&getUploadFd() const;
		const size_t 					&getUploadSize() const;
		const size_t 					&getUploadReceived() const;
		const std::string 				&getUploadPath() const;
		const std::string 				&getOriginalHeaders() const;
		bool							getFirstRequest();
		int								getEpollFd() const;

		// ### SETTERS ###
		void							setTime(std::time_t time);
		void							setFileFd(int fd);
		void							setFileSize(size_t size);
		void							setBytesSent(size_t bytes);
		void 							setUploadFd(int uploadFd);
		void 							setUploadSize(size_t uploadSize);
		void 							setUploadReceived(size_t uploadReceived);
		void 							setUploadPath(std::string uploadPath);
		void 							setOriginalHeaders(const std::string &headers);
		void							setFirstRequest(bool value);
		void							setEpollFd(int fd);

};

#endif