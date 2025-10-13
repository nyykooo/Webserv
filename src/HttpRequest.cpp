/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 14:40:07 by brunhenr          #+#    #+#             */
/*   Updated: 2025/10/13 20:46:30 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

HttpRequest::HttpRequest()
	: _parseStatus(200),
	  _uploadSize(0),
	  _chunked(false),
	_sessions(NULL),
	_requestComplete(false),
	_config(NULL),
	session(NULL)
{}

HttpRequest::HttpRequest(const std::string &request_text, Configuration *config, std::vector<SessionData *> *sessions) : _parseStatus(200), _uploadSize(0), 
				_chunked(false),  _sessions(sessions), _requestComplete(false), _config(config)  {
	parse(request_text);
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &other)
	: _parseStatus(other._parseStatus),
	  method(other.method),
	  path(other.path),
	  version(other.version),
	  headers(other.headers),
	  _body(other._body),
	  _requestComplete(other._requestComplete),
	  _config(other._config)
{}

bool	HttpRequest::chunkedRequestCompleted(const std::string& str) {
	size_t pos = 0;

	while (true) {
		_bodyBuffer = str;
		// Find the CRLF that terminates the chunk-size line
		size_t crlf = _bodyBuffer.find("\r\n", pos);
		if (crlf == std::string::npos)
			return (false); // incomplete line
		std::string sizeStr = _bodyBuffer.substr(pos, crlf - pos);
		

        // Convert hex size to integer
		char *endptr = NULL;
		long chunkSize = strtol(sizeStr.c_str(), &endptr, 16);
		if (*endptr != '\0' || chunkSize < 0) {
			_parseStatus = 400;
			_requestComplete = true;
            return (true); // invalid hex size
		}
		pos = crlf + 2; // move after "\r\n"
		// Chunk size 0 means end of body
		if (chunkSize == 0) {
			
			// Verify it ends with "\r\n"
			if (_bodyBuffer.size() < pos + 2) {
				return true;
			}
				
			if (_bodyBuffer.substr(pos, 2) != "\r\n") {
				_parseStatus = 400;
				return true;
            }
            _bodyBuffer.erase(0, pos + 2); // remove parsed part
            _requestComplete = true;
            return true; // fully parsed
        }
		if (_bodyBuffer.size() < pos + chunkSize + 2)
			return false; // wait for rest of chunk
        // Append the actual data to the de-chunked body
		_body.append(_bodyBuffer, pos, chunkSize);
		pos += chunkSize;
		// After chunk data, there must be a CRLF
		if (_bodyBuffer.substr(pos, 2) != "\r\n") {
			_parseStatus = 400;
			_requestComplete = true;
			return true;
		}
		pos += 2;
	}
	return (true);
}

void HttpRequest::parse(const std::string &request_text)
{
	std::istringstream	stream(request_text);
	std::string			request_line;

	//std::cout << YELLOW << request_text << RESET << std::endl;
	if (_chunked && !chunkedRequestCompleted(stream.str())) // to keep parsing the chunk request and don't check the other headers that come ONLY in first recv call
		return ;
	else if (_chunked == false) {
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
			setParseStatus(400);
				return;
		}
		parse_headers(stream);
		if (_parseStatus != 200)
			return;
		parseCookies();
		std::map<std::string, std::string>::const_iterator transferEnconding = headers.find("Transfer-Encoding");
		if (transferEnconding == headers.end())
			return ;
		if (transferEnconding->second != "chunked") {
			_parseStatus = 400;
			return ;
		}
		_chunked = true;
		return ;
	}
	if (_parseStatus != 200)
		return;
	if (_chunked) {
		std::istringstream ss(_body);
		parseBody(ss);
	}
	else if (stream.peek() != EOF)
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
		setParseStatus(400);
			return;
	}
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		setParseStatus(405);
			return;
	}
	if (version != "HTTP/1.1")
	{
		setParseStatus(505);
			return;
	}
	if (path.length() > MAX_URI_LENGTH)
	{
		setParseStatus(414);
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
			setParseStatus(400);
				return;
		}
		size_t colon_pos = header_line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string header_name = header_line.substr(0, colon_pos);
			std::string header_value = header_line.substr(colon_pos + 1);
			if (header_name.length() > MAX_HEADER_NAME_LENGTH)
			{
				setParseStatus(400);
					return;
			}
			if (header_value.length() > MAX_HEADER_VALUE_LENGTH)
			{
				setParseStatus(400);
					return;
			}
			header_name.erase(header_name.find_last_not_of(" \t") + 1);
			header_value.erase(0, header_value.find_first_not_of(" \t"));
			headers[header_name] = header_value;
			if (headers.size() > MAX_HEADERS_COUNT)
			{
				setParseStatus(400);
					return;
			}
		}
		else
		{
			setParseStatus(400);
				return;
		}
	}
}

void HttpRequest::parseBody(std::istringstream &stream)
{
	if (!_config)
	{
		setParseStatus(500);
			return;
	}
	std::map<std::string, std::string>::const_iterator contentLengthIt = headers.find("Content-Length");
	std::map<std::string, std::string>::const_iterator transferEnconding = headers.find("Transfer-Encoding");
	if (contentLengthIt == headers.end() && transferEnconding == headers.end())
	{
		setParseStatus(411);
		return ;
	}
	if (contentLengthIt != headers.end())
	{
		if (!isValidContentLengthFormat(contentLengthIt->second))
		{
			setParseStatus(400);
				return;
		}
		long contentLength = std::atol(contentLengthIt->second.c_str());
		if (contentLength < 0)
		{
			setParseStatus(400);
				return;
		}
		if (contentLength > _config->getRequestSize())
		{
			setParseStatus(413);
				return;
		}
	}
	std::string body_str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	_body = body_str;
	if (_body.length() > static_cast<size_t>(_config->getRequestSize()))
	{
		setParseStatus(413);
			return;
	}
	if (contentLengthIt != headers.end())
	{
		long contentLength = std::atol(contentLengthIt->second.c_str());
		if (static_cast<long>(_body.length()) != contentLength)
		{
			setParseStatus(400);
			return ;
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

void HttpRequest::setParseStatus(int status)
{
	_parseStatus = status;
}

bool HttpRequest::hasParseError() const
{
	return (_parseStatus != 200);
}

int HttpRequest::getParseStatus() const
{
	return _parseStatus;
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
			if ((*_sessions)[i]->getSessionId() == sidIt->second) {
				found = true;
				if (themeIt != _cookies.end())
					(*_sessions)[i]->setTheme(themeIt->second);
				(*_sessions)[i]->setTime(std::time(NULL));
				session = ((*_sessions)[i]);
				break;
			}
		}
		if (!found) {
			SessionData *newSession = new SessionData();
			newSession->setSessionId(generateRandomSessionId(_sessions->size()));
			if (themeIt != _cookies.end())
				newSession->setTheme(themeIt->second);
			_sessions->push_back(newSession);
			session = (_sessions->back());
		}
	} else if (themeIt != _cookies.end()) {
		// No session_id, but theme present → create new session with generated ID
		SessionData *newSession = new SessionData();
		newSession->setSessionId(generateRandomSessionId(_sessions->size()));
		_cookies["session_id"] = newSession->getSessionId();
		newSession->setTheme(themeIt->second);
		_sessions->push_back(newSession);
		session = (_sessions->back());
	}	
}

void HttpRequest::parseCookies() {
	std::map<std::string, std::string>::const_iterator it = getHeaders().begin();
	while (it != getHeaders().end()) {
		if (it->first == "Cookie") {
			checkCreatedSessions(it->second);
			break ;
		}
		it++;
	}
	if (it == getHeaders().end()) {
		SessionData *newSession = new SessionData();
		newSession->setSessionId(generateRandomSessionId(_sessions->size()));
		_cookies["session_id"] = newSession->getSessionId();
		_sessions->push_back(newSession);
		session = (_sessions->back());
		return ;
	}
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
	return (_body);
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
bool HttpRequest::getChunked() const {
	return _chunked;
}

void HttpRequest::setChunked(bool stat){
	_chunked = stat;
}

bool	HttpRequest::isRequestCompleted() const {
	return (_requestComplete);
}