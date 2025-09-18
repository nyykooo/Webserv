/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brunhenr <brunhenr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 14:40:07 by brunhenr          #+#    #+#             */
/*   Updated: 2025/09/20 17:47:59 by brunhenr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

HttpRequest::HttpRequest()
	: _parseStatus(200),
	  _sessions(NULL),
	  _uploadSize(0),
	  _config(NULL),
	  session(NULL)
{}

HttpRequest::HttpRequest(const std::string &request_text, Configuration *config, std::vector<SessionData> *sessions) : _parseStatus(200), _sessions(sessions), _uploadSize(0), _config(config)
{
	parse(request_text);
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &other)
	: _parseStatus(other._parseStatus),
	  _parseError(other._parseError),
	  method(other.method),
	  path(other.path),
	  version(other.version),
	  headers(other.headers),
	  body(other.body),
	  _config(other._config)
{}

void HttpRequest::parse(const std::string &request_text)
{
	std::istringstream	stream(request_text);
	std::string			request_line;

	if (std::getline(stream, request_line) && !request_line.empty())
	{
		parse_requestline(request_line);
		std::stringstream ss;
		ss << "HTTP Request line parsed: " << method << " " << path << " " << version;
		printLog(ss.str(), WHITE, std::cout);
		if (_parseStatus != 200)
			return;
	}
	else
	{
		setParseError(400, "Empty or invalid request line");
			return;
	}
	parse_headers(stream);
	parseCookies();
	if (_parseStatus != 200)
			return;

	if (stream.peek() != EOF)
	{
		parseBody(stream);
	}
}

void HttpRequest::parse_requestline(const std::string &request_line)
{
	std::istringstream 	stream(request_line);
	std::string			method, path, version, further;

	stream >> method >> path >> version >> further;
	if (method.empty() || path.empty() || version.empty() || !further.empty())
	{
		setParseError(400, "Invalid HTTP request line format");
			return;
	}
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		setParseError(405, "Method not allowed: " + method);
			return;
	}
	if (version != "HTTP/1.1")
	{
		setParseError(505, "HTTP version not supported: " + version);
			return;
	}
	if (path.length() > MAX_URI_LENGTH)
	{
		setParseError(414, "URI too long");
			return;
	}
	this->method = method;
	this->path = path;
	this->version = version;
}

void HttpRequest::parse_headers(std::istringstream &stream)
{
	std::string header_line;

	while (std::getline(stream, header_line) && !header_line.empty())
	{
		if (!header_line.empty() && header_line[header_line.size() - 1] == '\r')
			header_line.erase(header_line.size() - 1);

		if (header_line.empty())
			break;
			
		if (!header_line.empty() && std::isspace(header_line[0]))
		{
			setParseError(400, "Line folding is forbidden in HTTP headers");
				return;
		}
		size_t colon_pos = header_line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string header_name = header_line.substr(0, colon_pos);
			std::string header_value = header_line.substr(colon_pos + 1);
			if (header_name.length() > MAX_HEADER_NAME_LENGTH)
			{
				setParseError(400, "Header name too long");
					return;
			}
			if (header_value.length() > MAX_HEADER_VALUE_LENGTH)
			{
				setParseError(400, "Header value too long");
					return;
			}
			header_name.erase(header_name.find_last_not_of(" \t") + 1);
			header_value.erase(0, header_value.find_first_not_of(" \t"));
			headers[header_name] = header_value;
			if (headers.size() > MAX_HEADERS_COUNT)
			{
				setParseError(400, "Too many headers");
					return;
			}
		}
		else
		{
			setParseError(400, "Invalid header format");
				return;
		}
	}
}

void HttpRequest::parseBody(std::istringstream &stream)
{
	if (!_config)
	{
		setParseError(500, "Configuration required for request parsing");
			return;
	}
	std::map<std::string, std::string>::const_iterator contentLengthIt = headers.find("Content-Length");
	if (method == "POST")
	{
		if (contentLengthIt == headers.end())
		{
			setParseError(411, "Length required: POST requests must include Content-Length header");
				return;
		}
	}
	if (contentLengthIt != headers.end())
	{
		if (!isValidContentLengthFormat(contentLengthIt->second))
		{
			setParseError(400, "Invalid Content-Length format");
				return;
		}
		long contentLength = std::atol(contentLengthIt->second.c_str());
		if (contentLength < 0)
		{
			setParseError(400, "Invalid Content-Length: negative value");
				return;
		}
		if (contentLength > _config->getRequestSize())
		{
			setParseError(413, "Content-Length exceeds server limit");
				return;
		}
	}
	std::string body_str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	body = body_str;
	if (body.length() > static_cast<size_t>(_config->getRequestSize()))
	{
		setParseError(413, "Request body too large");
			return;
	}
	if (contentLengthIt != headers.end())
	{
		long contentLength = std::atol(contentLengthIt->second.c_str());
		if (static_cast<long>(body.length()) != contentLength)
		{
			setParseError(400, "Body size doesn't match Content-Length header");
				return;
		}
	}
}

bool HttpRequest::isValidContentLengthFormat(const std::string &value)
{
	if (value.empty())
		return false;

	for (size_t i = 0; i < value.length(); ++i)
	{
		if (!std::isdigit(value[i]))
			return false;
	}
	if (value.length() > 19)
		return false;
	if (value.length() == 19 && value > "9223372036854775807")
		return false;
	return true;
}

void HttpRequest::setParseError(int status, const std::string &error)
{
	_parseStatus = status;
	_parseError = error;
}

bool HttpRequest::hasParseError() const
{
	return (_parseStatus != 200);
}

int HttpRequest::getParseStatus() const
{
	return _parseStatus;
}

const std::string &HttpRequest::getParseError() const
{
	return _parseError;
}

const std::string &HttpRequest::getUploadPath() const
{
	return _uploadPath;
}

size_t HttpRequest::getUploadSize() const
{
	return _uploadSize;
}

void HttpRequest::setUploadPath(const std::string &path)
{
	_uploadPath = path;
}
void HttpRequest::setUploadSize(size_t size)
{
	_uploadSize = size;
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

const std::map<std::string, std::string>& HttpRequest::getCookies() const {
	return (_cookies);
}

void	HttpRequest::setCookies(const std::string& key, const std::string& value) {
	_cookies[key] = value;
}

const std::map<std::string, std::string>&	HttpRequest::getHeaders() const {
	return (headers);
}

const std::string&	HttpRequest::getBody() const {
	return (body);
}

const std::string& HttpRequest::getMethod() const {
	return (method);
}

const std::string& HttpRequest::getPath() const {
	return (path);
}

const std::string& HttpRequest::getVersion() const {
	return (version);
}