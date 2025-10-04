/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/10/04 19:52:37 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <sstream> //para std::istringstream
#include <iostream>
#include <map>
#include <cstdio> //para EOF

#define MAX_URI_LENGTH 4096
#define MAX_HEADER_NAME_LENGTH 1024
#define MAX_HEADER_VALUE_LENGTH 8192
#define MAX_HEADERS_COUNT 100

class SessionData;

class HttpRequest
{
	private:
		// Attributes
		int									_parseStatus;
		std::string							_parseError;
		std::string							method;
		std::string							path;
		std::string							version;
		std::map<std::string, std::string> 	headers;
		std::string							body;
		std::vector<SessionData *>*			_sessions;
		std::map<std::string, std::string>	_cookies;
		std::string 						_uploadPath;
		size_t 								_uploadSize;

		// Private methods
    	bool isValidContentLengthFormat(const std::string &value);
		void parse(const std::string &request_text);
		void parse_requestline(const std::string &request_line);
		void parse_headers(std::istringstream &stream);
		void parseBody(std::istringstream &stream);
		void setParseError(int status, const std::string& error);
		void parseCookies();
		void checkCreatedSessions(const std::string& cookiesLine);

	public:
		// Constructor
		HttpRequest();
		HttpRequest(const std::string& request_text, Configuration* config, std::vector<SessionData *>* sessions);
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		~HttpRequest();

		// public atributes
		Configuration*	_config;
		SessionData*	session;

		// getters
		const std::string 							&getMethod() const;
		const std::string 							&getPath() const;
		const std::string 							&getVersion() const;
		const std::map<std::string, std::string> 	&getHeaders() const;
		const std::string 							&getBody() const;
		const std::map<std::string, std::string>	&getCookies() const;
		const std::string							&getUploadPath() const;
		size_t 										getUploadSize() const;
		int 										getParseStatus() const;
		const std::string 							&getParseError() const;
		bool 										hasParseError() const;

		// setters
		void setUploadPath(const std::string &path);
		void setUploadSize(size_t size);
		void setCookies(const std::string& key, const std::string& value);
};

#endif