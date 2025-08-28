/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/08/28 21:48:02 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <string>
#include <sstream> //para std::istringstream
#include <iostream>
#include <map>
#include <cstdio> //para EOF

class SessionData;

class HttpRequest
{
	private:
		// Attributes
		std::string method;
		std::string path;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
		std::vector<SessionData>*	_sessions;
		std::map<std::string, std::string>	_cookies;

		// Private methods
		void parse(const std::string &request_text);
		void parse_requestline(const std::string &request_line);
		void parse_headers(std::istringstream &stream);
		void parseBody(std::istringstream &stream);
		void parseCookies();
		void checkCreatedSessions(const std::string& cookiesLine);

	public:
		// Constructor
		HttpRequest();
		HttpRequest(const std::string& request_text, std::vector<SessionData>* sessions);
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
};