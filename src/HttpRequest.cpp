#include "../includes/headers.hpp"

HttpRequest::HttpRequest() : _parseStatus(200), _uploadSize(0), _chunked(false), _sessions(NULL), _requestComplete(false), _config(NULL), session(NULL) {

}

HttpRequest::HttpRequest(const std::string &request_text, Configuration *config, std::vector<SessionData *> *sessions) : _parseStatus(200), _contentLength(0)
	, _headersCompleted(false), _uploadSize(0), _chunkedRequestSize(0), _chunked(false),  _sessions(sessions)
	, _requestComplete(false), _headersParsed(false), _config(config) {
	parse(request_text);
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &other)
	: _parseStatus(other._parseStatus),
	  _method(other._method),
	  _path(other._path),
	  _version(other._version),
	  _headers(other._headers),
	  _body(other._body),
	  _requestComplete(other._requestComplete),
	  _config(other._config)
{}

// ######### VALIDATORS #########

bool	HttpRequest::chunkedRequestCompleted(const std::string& str) {
	_bodyBuffer += str;
	size_t pos = 0;
	while (true) {
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
		_chunkedRequestSize += chunkSize;
		pos = crlf + 2; // move after "\r\n"
		// Chunk size 0 means end of body
		if (chunkSize == 0) {
			// Verify it ends with "\r\n"
			if (_bodyBuffer.size() < pos + 2)
				return (false);
			if (_bodyBuffer.substr(pos, 2) != "\r\n") {
				_parseStatus = 400;
				return (true);
			}
			_bodyBuffer.erase(0, pos + 2); // remove parsed part
			_requestComplete = true;
			return (true); // fully parsed
		}
		if (_bodyBuffer.size() < pos + chunkSize + 2)
			return (false); // wait for rest of chunk
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
		_bodyBuffer.erase(0, pos);
		pos = 0;
	}
	return (true);
}

void HttpRequest::checkHeaders(std::string& str) {
	if (_headersCompleted)
		return ;
	_headersNotParsed += str;
	size_t pos = _headersNotParsed.find("\r\n\r\n");
	if (pos == std::string::npos)
		return ;
	str = _headersNotParsed.substr(pos + 4);
	_headersNotParsed = _headersNotParsed.substr(0, pos);
	_headersCompleted = true;
}

void HttpRequest::checkChunkedRequest() {
	std::map<std::string, std::string>::const_iterator transferEnconding = _headers.find("Transfer-Encoding");
	if (transferEnconding != _headers.end()) {
		if (transferEnconding->second != "chunked") {
			_parseStatus = 400;
			_requestComplete = true;
			return ;
		}
		_chunked = true;
	}
}

bool HttpRequest::checkContentLength() {
	std::map<std::string, std::string>::const_iterator contentLengthIt = _headers.find("Content-Length");
	std::map<std::string, std::string>::const_iterator transferEnconding = _headers.find("Transfer-Encoding");

	if (_body.size() > 0 && contentLengthIt == _headers.end() && transferEnconding == _headers.end())
	{
		_parseStatus = 411;
		return (false);
	}
	else if (contentLengthIt != _headers.end() && transferEnconding != _headers.end()) {
		_parseStatus = 400;
		return (false);
	}
	if (contentLengthIt != _headers.end())
	{
		if (!isValidContentLengthFormat(contentLengthIt->second))
		{
			_parseStatus = 400;
			return (false);
		}
		_contentLength = std::atol(contentLengthIt->second.c_str());
		if (_contentLength < 0) {
			_parseStatus = 400;
			return (false);
		}	
		else if (_contentLength == 0) {
			_requestComplete = true;
			return (true);
		}	
	}
	return (true);
}

bool HttpRequest::isValidContentLengthFormat(const std::string &value)
{
	if (value.empty())
		return (false);
	for (size_t i = 0; i < value.length(); ++i)
	{
		if (!std::isdigit(value[i]))
			return (false);
	}
	errno = 0;
	if (std::strtol(value.c_str(), NULL, 10) > LONG_MAX || errno == ERANGE)
		return (false);
	return (true);
}

// ######### PARSERS #########

void HttpRequest::parse(const std::string &request_text)
{
	std::string			request_line;
	std::string			temp = request_text;

	checkHeaders(temp);
	if (!_headersCompleted)
		return ;
	checkChunkedRequest();
	if (_parseStatus == 400)
		return ;
	if (_chunked && !chunkedRequestCompleted(temp)) // to keep parsing the chunk request
		return ;	
	parse_headers();
	if (_parseStatus != 200) {
		_requestComplete = true;
		return ;
	}
	if (!_chunked) {
		_body.append(temp);
		parseBody();
	}
}

void HttpRequest::parseBody()
{
	if (!checkContentLength()) {
		_requestComplete = true;
		return ;
	}
	if (_contentLength != 0 && _body.size() < static_cast<size_t>(_contentLength))
		return ;
	_requestComplete = true;
	if (_parseStatus != 200)
		return ;
	parseCookies();
}

void HttpRequest::parse_requestline(const std::string &request_line)
{
	std::istringstream 	stream(request_line);
	std::string			method, path, version, further;

	stream >> method >> path >> version >> further;
	if (method.empty() || path.empty() || version.empty() || !further.empty())
	{
		_parseStatus = 400;
		return;
	}
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		_parseStatus = 405;
		return;
	}
	if (version != "HTTP/1.1")
	{
		_parseStatus = 505;
		return;
	}
	if (path.length() > MAX_URI_LENGTH)
	{
		_parseStatus = 414;
		return;
	}
	this->_method = method;
	this->_path = path;
	this->_version = version;
}

void HttpRequest::parse_headers()
{
	if (_headersParsed)
		return ;
	std::istringstream	stream(_headersNotParsed);
	std::string			header_line;

	if (std::getline(stream, header_line) && !header_line.empty()) {
			parse_requestline(header_line);
			std::stringstream ss;
			ss << "HTTP Request line parsed: " << _method << " " << _path << " " << _version;
			printLog(ss.str(), WHITE, std::cout);
			if (_parseStatus != 200)
				return ;
		}
	else {
		_parseStatus = 400;
		return;
	}
	while (std::getline(stream, header_line))
	{
		if (header_line.empty())
			break ;
		if (!header_line.empty() && header_line[header_line.size() - 1] == '\r')
			header_line.erase(header_line.size() - 1);
		if (header_line.empty())
			break;
			
		if (!header_line.empty() && std::isspace(header_line[0])) {
			_parseStatus = 400;
			return;
		}
		size_t colon_pos = header_line.find(':');
		if (colon_pos == std::string::npos) {
			_parseStatus = 400;
			return;
		}
		std::string header_name = header_line.substr(0, colon_pos);
		std::string header_value = header_line.substr(colon_pos + 1);
		if (header_name.length() > MAX_HEADER_NAME_LENGTH) {
			_parseStatus = 400;
			return;
		}
		if (header_value.length() > MAX_HEADER_VALUE_LENGTH) {
			_parseStatus = 400;
			return;
		}
		header_name.erase(header_name.find_last_not_of(" \t") + 1);
		header_value.erase(0, header_value.find_first_not_of(" \t"));
		_headers[header_name] = header_value;
		if (_headers.size() > MAX_HEADERS_COUNT) {
			_parseStatus = 400;
			return;
		}
	}
	_headersParsed = true;	
}

// ######### COOKIES && SESSIONS VALIDATORS #########

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

// ######### GETTERS #########

const std::map<std::string, std::string>& HttpRequest::getCookies() const {
	return (_cookies);
}

const std::map<std::string, std::string>&	HttpRequest::getHeaders() const {
	return (_headers);
}

const std::string&	HttpRequest::getBody() const {
	return (_body);
}

const std::string& HttpRequest::getMethod() const {
	return (_method);
}

const std::string& HttpRequest::getPath() const {
	return (_path);
}

const std::string& HttpRequest::getVersion() const {
	return (_version);
}
bool HttpRequest::getChunked() const {
	return _chunked;
}

bool	HttpRequest::RequestCompleted() const {
	return (_requestComplete);
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

long HttpRequest::getContentLength() const {
	return (_contentLength);
}

// ######### SETTERS #########

void	HttpRequest::setCookies(const std::string& key, const std::string& value) {
	_cookies[key] = value;
}


void HttpRequest::setChunked(bool stat){
	_chunked = stat;
}

void HttpRequest::setParseStatus(int status)
{
	_parseStatus = status;
}

void HttpRequest::setUploadPath(const std::string &path)
{
	_uploadPath = path;
}
void HttpRequest::setUploadSize(size_t size)
{
	_uploadSize = size;
}