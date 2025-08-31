/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 14:40:07 by brunhenr          #+#    #+#             */
/*   Updated: 2025/08/31 04:41:33 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

HttpRequest::HttpRequest() : method(""), path(""), version(""), headers(), body("")
{}

static bool ft_isspace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}


void HttpRequest::parse_headers(std::istringstream &stream)
{
	std::string header_line;
	
	while (std::getline(stream, header_line) /* && !header_line.empty() */)
	{
		// remove os /r do final de cada linha
		if (header_line[header_line.size() - 1] == '\r')
			header_line.erase(header_line.size() - 1);

		if (ft_isspace(header_line[0]))
		{
			throw std::invalid_argument("Sintax error. Line folding is forbidden in HTTP headers.");
		}	
		size_t colon_pos = header_line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string header_name = header_line.substr(0, colon_pos);
			if (header_name.length() > 1024)
			{
				throw std::invalid_argument("Header name too long: " + header_name);
			}
			std::string header_value = header_line.substr(colon_pos + 1);
			if (header_value.length() > 8192)
			{
				throw std::invalid_argument("Header value too long: " + header_value);
			}
			
			header_name.erase(header_name.find_last_not_of(" \t") + 1);
			header_value.erase(0, header_value.find_first_not_of(" \t"));
			//verificar a questão das listas nos headers e da consolidação de valores em listas
			
			this->headers[header_name] = header_value;
			if (headers.size() > 64) // 64 é o tamanho máximo de um header HTTP
			{
				throw std::invalid_argument("Too many headers");
			}
		}
	}
}


void HttpRequest::parse_requestline(const std::string& request_line)
{
	std::istringstream stream(request_line);
	std::string method, path, version, further;

	stream >> method >> path >> version >> further;
	if (method.empty() || path.empty() || version.empty() || !further.empty())
	{
		throw std::invalid_argument("Invalid HTTP request line");
	}
	else if (method != "GET" && method != "POST" && method != "DELETE") //Avaliar a inclusão do PUT?
	{
		throw std::invalid_argument("Unsupported or non-existent HTTP method: " + method);
	}
	else if (version != "HTTP/1.1" && version != "HTTP/1.0")// se a version for mais moderna, rodar com o HTTP/1.1? E HTTP/1.0 é possível?
	{
		throw std::invalid_argument("Unsupported or non-existent HTTP version: " + version);
	}
	this->method = method;
	this->path = path;
	this->version = version;
}

void HttpRequest::parseBody(std::istringstream &stream)
{
	std::string body_str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	body = body_str;
	if (body.length() > 131072)
		throw std::invalid_argument("Body too long");
}

std::string generateRandomSessionId(size_t index) {
	std::stringstream ss;
	ss << index;
	std::string str;
	ss >> str;
	return ("session_" + str);
}

void HttpRequest::checkCreatedSessions(const std::string& cookiesLine) {
	std::stringstream ss(cookiesLine);
	std::string token;
	while (std::getline(ss, token, ';')) {
		// Remove spaces
		size_t start = token.find_first_not_of(" \t");
		size_t end = token.find_last_not_of(" \t");
		std::string pair;
		if (start != std::string::npos)
			pair = token.substr(start, end - start + 1);
		// Split by '='
		size_t eqPos = pair.find('=');
		if (eqPos != std::string::npos) {
			std::string key = pair.substr(0, eqPos);
			std::string value = pair.substr(eqPos + 1);
			if (key == "session_id" || key == "theme") {
				_cookies[key] = value;
			}
		}
	}
	// check if session already exists or create a new one
	std::map<std::string, std::string>::iterator sidIt = _cookies.find("session_id");
	std::map<std::string, std::string>::iterator themeIt = _cookies.find("theme");

	if (sidIt != _cookies.end()) {
		// Look for existing session_id
		bool found = false;
		for (size_t i = 0; i < _sessions->size(); ++i) {
			if ((*_sessions)[i].getSessionId() == sidIt->second) {
				found = true;
				session = &((*_sessions)[i]);
				if (themeIt != _cookies.end())
					(*_sessions)[i].setTheme(themeIt->second);
				break;
			}
		}
		if (!found) {
			SessionData newSession;
			newSession.setSessionId(generateRandomSessionId(_sessions->size()));
			if (themeIt != _cookies.end())
				newSession.setTheme(themeIt->second);
			_sessions->push_back(newSession);
			session = &(_sessions->back());
		}
	} else if (themeIt != _cookies.end()) {
		// No session_id, but theme present → create new session with generated ID
		SessionData newSession;
		newSession.setSessionId(generateRandomSessionId(_sessions->size()));
		_cookies["session_id"] = newSession.getSessionId();
		newSession.setTheme(themeIt->second);
		_sessions->push_back(newSession);
		session = &(_sessions->back());
	}	
}

void HttpRequest::parseCookies() {
	std::map<std::string, std::string>::const_iterator it = getHeaders().begin();
	while (it != getHeaders().end()) {
		if (it->first == "Cookie")
			break ;
		it++;
	}
	if (it == getHeaders().end()) {
		SessionData newSession;
		newSession.setSessionId(generateRandomSessionId(_sessions->size()));
		_cookies["session_id"] = newSession.getSessionId();
		_sessions->push_back(newSession);
		session = &(_sessions->back());
		return ;
	}
	checkCreatedSessions(it->second);
}

void HttpRequest::parse(const std::string &request_text)
{
	std::istringstream	stream(request_text);
	std::string			request_line;

	if (std::getline(stream, request_line) && !request_line.empty())
	{
		parse_requestline(request_line);
		std::stringstream ss;
		ss << "HTTP Request line parsed: " << method << " " << path << " " << version;
		printLog(ss.str(), WHITE);
	}
	
	parse_headers(stream);
	if (stream.peek() != EOF) // a peek retorna o próximo caractere sem removê-lo do stream
		parseBody(stream);
	parseCookies();
}

HttpRequest::HttpRequest(const std::string& request_text, std::vector<SessionData>* sessions): _sessions(sessions)
{
	parse(request_text);
}

HttpRequest::HttpRequest(const HttpRequest& other) : method(other.method), path(other.path), version(other.version),
	  headers(other.headers), body(other.body)
{}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		method = other.method;
		path = other.path;
		version = other.version;
		headers = other.headers;
		body = other.body;
	}
	return *this;
}

HttpRequest::~HttpRequest()
{}

// GETTERS
const std::string& HttpRequest::getMethod() const
{
	return (method);
}

const std::string& HttpRequest::getPath() const
{
	return (path);
}

const std::string& HttpRequest::getVersion() const
{
	return (version);
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
	return (headers);
}

const std::string& HttpRequest::getBody() const
{
	return (body);
}

const std::map<std::string, std::string>& HttpRequest::getCookies() const {
	return (_cookies);
}

void	HttpRequest::setCookies(const std::string& key, const std::string& value) {
	_cookies[key] = value;
}