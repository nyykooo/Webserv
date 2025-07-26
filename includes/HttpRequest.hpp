/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/07/26 18:31:29 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <string>
#include <sstream> //para std::istringstream
#include <iostream>
#include <map>
#include <cstdio> //para EOF

class HttpRequest
{
	private:
		// Attributes
		std::string method;
		std::string path;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;

		// Private methods
		void parse(const std::string &request_text);
		void parse_requestline(const std::string &request_line);
		void parse_headers(std::istringstream &stream);
		void parseBody(std::istringstream &stream);

	public:
		// Constructor
		HttpRequest();
		HttpRequest(const std::string& request_text);
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		~HttpRequest();
		
		// utils

		Configuration* _config;

		// getters
		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getVersion() const;
		const std::map<std::string, std::string> &getHeaders() const;
		const std::string& getBody() const;
};