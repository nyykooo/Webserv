/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/09/01 04:33:32 by discallow        ###   ########.fr       */
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
		std::vector<SessionData>*			_sessions;
		std::map<std::string, std::string>	_cookies;

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
		HttpRequest(const std::string& request_text, Configuration* config, std::vector<SessionData>* sessions);
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		~HttpRequest();

		// utils
		Configuration*	_config;
		SessionData*	session;

		// getters
		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getVersion() const;
		const std::map<std::string, std::string> &getHeaders() const;
		const std::string& getBody() const;
		const std::map<std::string, std::string>&	getCookies() const;

		void	setCookies(const std::string& key, const std::string& value);

    	bool hasParseError() const;
    	int getParseStatus() const;
    	const std::string& getParseError() const;
};

#endif