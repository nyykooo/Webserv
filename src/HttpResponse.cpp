#include "../includes/headers.hpp"

HttpResponse::HttpResponse() : _resStatus(-1), _useNewLocation(false), _pipeIn(-1), _pipeOut(-1), _cgiPid(-1), _conf(NULL), _req(NULL), _filePos(0), _rawUpload(false) {}


HttpResponse::~HttpResponse() {
	if (_file.is_open())
		_file.close();
}

void	HttpResponse::setHttpStatus(int status) {
	std::map<int, std::string>::iterator itt = _statusTexts.find(status);
	if (itt != _statusTexts.end())
		_httpStatus = itt->second;
	else
	{
		std::ostringstream oss;
		oss << _resStatus;
		_httpStatus = oss.str();
	}
}

void	HttpResponse::setCgiCookies(const std::string& cookie) {
	_cgiCookies.push_back(cookie);
}

void	HttpResponse::setCgiContentType(std::string& type) {
	if (_cgiContentType == "") {
		std::stringstream	ss(type);
		std::string			str;
		ss >> str;
		if (ss >> str)
			_cgiContentType = str;
		_cgiParsedHeaders.push_back(type + CRLF);
	}
}

void	HttpResponse::setCgiLocation(const std::string& location) {
	if (_cgiLocation == "") {
		_cgiLocation = location;
		_cgiParsedHeaders.push_back(location + CRLF);
	}
}

static std::string readFd(int fd)
{
	const size_t bufSize = 4096;
	char buffer[bufSize];
	std::string result;
	ssize_t bytesRead;

	while (true)
	{
		bytesRead = read(fd, buffer, bufSize);
		if (bytesRead > 0)
		{
			result.append(buffer, bytesRead);
		}
		else if (bytesRead == 0)
		{
			break;
		}
		else
		{
			std::stringstream ss;
			ss << "Error reading fd: (" << fd << ")";
			printLog(ss.str(), RED, std::cout);
			return "";
		}
	}

	return result;
}

void HttpResponse::checkCgiProcess()
{
	int doneProcess;
	int childrenStatus;

	doneProcess = waitpid(_cgiPid, &childrenStatus, WNOHANG); // permite verificar se o processo filho terminou sem bloquear o servidor
	if (doneProcess == 0)
	{
		// filho ainda esta correndo
		return;
	}
	else if (doneProcess == _cgiPid)
	{
		// filho terminou
		_client->setProcessingState(CGI_COMPLETED);
		int exitStatus = WEXITSTATUS(childrenStatus);
		if (exitStatus == 0)
		_resStatus = 200; // OK
		else
			_resStatus = 500; // Internal Server Error
		_response = readFd(_pipeOut); // Lê a saída do CGI
		if (_response == "")
			_resStatus = 500; // Internal Server Error
		std::stringstream ss;
		ss << "CGI process finished:\n\t-PID: " << _cgiPid << ";\n\t-Status: " << _resStatus;
		printLog(ss.str(), WHITE, std::cout);
		close(_pipeIn);
		close(_pipeOut);
		_pipeIn = -1;  // Reset pipeIn
		_pipeOut = -1; // Reset pipeOut
		_cgiPid = -1;  // Reset cgiPid
		return;		   // Retorna para não bloquear o servidor
	}
}

void HttpResponse::forkExecCgi(std::string interpreter)
{
	_fileName = removeSlashes(_fileName);

	char *args[3];
	int pipeInput[2];
	int pipeOutput[2];

	std::string _script_name = _scriptNameNico.find_last_of('?') != std::string::npos
									? _scriptNameNico.substr(0, _scriptNameNico.find_last_of('?'))
									: _scriptNameNico;

	args[0] = const_cast<char *>(interpreter.c_str());
	args[1] = const_cast<char *>(_script_name.c_str());
	args[2] = NULL;

	// Create pipes for input (to CGI) and output (from CGI)
	if (pipe(pipeInput) == -1 || pipe(pipeOutput) == -1)
	{
		std::cerr << "Pipe error: " << strerror(errno) << std::endl;
		_resStatus = 500;
		return;
	}

	std::stringstream ss;
	ss << "CGI execution:\n\t-interpreter: " << interpreter
	   << ";\n\t-script: " << _script_name;
	printLog(ss.str(), WHITE, std::cout);

	// Fork the process
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Fork error: " << strerror(errno) << std::endl;
		_resStatus = 500;
		close(pipeInput[0]); close(pipeInput[1]);
		close(pipeOutput[0]); close(pipeOutput[1]);
		return;
	}

	// ------------------- CHILD PROCESS -------------------
	if (pid == 0)
	{
		// Close unused ends
		close(pipeInput[1]);
		close(pipeOutput[0]);

		// Redirect stdin and stdout
		if (dup2(pipeInput[0], STDIN_FILENO) == -1 ||
			dup2(pipeOutput[1], STDOUT_FILENO) == -1)
		{
			std::cerr << "dup2 error: " << strerror(errno) << std::endl;
			exit(1);
		}

		// Close originals after dup2
		close(pipeInput[0]);
		close(pipeOutput[1]);

		// Redirect stderr to debug log
		int debugFd = open("./cgi_debug.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (debugFd == -1)
		{
			std::cerr << "Failed to open debug log file: " << strerror(errno) << std::endl;
			exit(1);
		}
		dup2(debugFd, STDERR_FILENO);
		close(debugFd);

		// Change directory to the CGI directory
		_fullPath += "/";
		size_t found = _fullPath.find_last_of("/");
		std::string path = _fullPath.substr(0, found);
		if (chdir(path.c_str()) == -1)
		{
			std::cerr << "CGI Chdir error: " << strerror(errno) << " " << path << std::endl;
			exit(1);
		}

		// Execute the CGI
		execve(interpreter.c_str(), const_cast<char **>(args), const_cast<char **>(_envp));

		std::cerr << "CGI Execution error: " << strerror(errno) << std::endl;
		exit(1);
	}
	// ------------------- PARENT PROCESS -------------------
	else
	{
		_cgiPid = pid;
		std::stringstream ss;
		ss << "CGI process started:\n\t-PID: " << _cgiPid;
		printLog(ss.str(), WHITE, std::cout);

		_client->setProcessingState(CGI_PROCESSING);

		// Close unused ends in the parent
		close(pipeInput[0]);
		close(pipeOutput[1]);

		// Store relevant ends
		_pipeIn = pipeInput[1];
		_pipeOut = pipeOutput[0];

		if (_req->getBody().size() > 0)
		{
			ssize_t bytesWritten = write(_pipeIn, _req->getBody().c_str(), _req->getBody().size());
			if (bytesWritten == -1 || static_cast<size_t>(bytesWritten) != _req->getBody().size())
			{
				std::cerr << "Error writing request body to CGI stdin: " << strerror(errno) << std::endl;
				close(_pipeIn);
				close(_pipeOut);
				_resStatus = 500;
				return;
			}
			// Important: close stdin after sending body, so CGI knows EOF
			close(_pipeIn);
			_pipeIn = -1;
		}

		// Parent returns immediately (non-blocking)
		return;
	}
}
// ### EXEC METHOD ###

void HttpResponse::startResponse(void)
{
	if (_req->hasParseError())
		return ;
	else if (_block->getRedirectStatusCode() != -1)
	{
		_resStatus = _block->getRedirectStatusCode();
		_useNewLocation = true;
		return;
	}
	execMethod();
}

std::size_t HttpResponse::getContentLength(void) const
{
	return _resContentLength;
}

std::ifstream &HttpResponse::getFileStream(void)
{
	return _file;
}

std::size_t HttpResponse::getFilePos(void) const
{
	return _filePos;
}

void	HttpResponse::setFilePos(std::size_t pos)
{
	_filePos = pos;
}

void HttpResponse::streamingFile(off_t fileSize, std::string contentType)
{
	_resContentLength = fileSize;
	_resContentType = contentType;
	_client->setProcessingState(STREAMING);
}

void HttpResponse::openReg(std::string path, int methodType, off_t fileSize)
{
	// std::ifstream file(path.c_str());
	_file.open(path.c_str());
	if (!_file.is_open())
	{
		_resStatus = 404;
		return;
	}
	_resStatus = 200;
	// streaming logic
	if (fileSize > MAX_MEMORY_FILE_SIZE)
		return streamingFile(fileSize, getContentType(path));
	if (methodType == DELETE)
		return;
	std::stringstream ss;

	ss << _file.rdbuf();
	_resBody = ss.str();
	_resContentLength = _resBody.size();
	// _file.close(); -> we will close after streaming file is completed
}

void HttpResponse::setMimeTypes()
{

	// Text
	_mimeTypes["html"] = "text/html";
	_mimeTypes["htm"] = "text/html";
	_mimeTypes["shtml"] = "text/html";
	_mimeTypes["css"] = "text/css";
	_mimeTypes["xml"] = "text/xml";
	_mimeTypes["txt"] = "text/plain";
	_mimeTypes["mml"] = "text/mathml";
	_mimeTypes["jad"] = "text/vnd.sun.j2me.app-descriptor";
	_mimeTypes["wml"] = "text/vnd.wap.wml";
	_mimeTypes["htc"] = "text/x-component";

	// Images
	_mimeTypes["gif"] = "image/gif";
	_mimeTypes["jpeg"] = "image/jpeg";
	_mimeTypes["jpg"] = "image/jpeg";
	_mimeTypes["avif"] = "image/avif";
	_mimeTypes["png"] = "image/png";
	_mimeTypes["svg"] = "image/svg+xml";
	_mimeTypes["svgz"] = "image/svg+xml";
	_mimeTypes["tif"] = "image/tiff";
	_mimeTypes["tiff"] = "image/tiff";
	_mimeTypes["wbmp"] = "image/vnd.wap.wbmp";
	_mimeTypes["webp"] = "image/webp";
	_mimeTypes["ico"] = "image/x-icon";
	_mimeTypes["jng"] = "image/x-jng";
	_mimeTypes["bmp"] = "image/x-ms-bmp";

	// Fonts
	_mimeTypes["woff"] = "font/woff";
	_mimeTypes["woff2"] = "font/woff2";

	// Applications
	_mimeTypes["js"] = "application/javascript";
	_mimeTypes["atom"] = "application/atom+xml";
	_mimeTypes["rss"] = "application/rss+xml";
	_mimeTypes["jar"] = "application/java-archive";
	_mimeTypes["war"] = "application/java-archive";
	_mimeTypes["ear"] = "application/java-archive";
	_mimeTypes["json"] = "application/json";
	_mimeTypes["hqx"] = "application/mac-binhex40";
	_mimeTypes["doc"] = "application/msword";
	_mimeTypes["pdf"] = "application/pdf";
	_mimeTypes["ps"] = "application/postscript";
	_mimeTypes["eps"] = "application/postscript";
	_mimeTypes["ai"] = "application/postscript";
	_mimeTypes["rtf"] = "application/rtf";
	_mimeTypes["m3u8"] = "application/vnd.apple.mpegurl";
	_mimeTypes["kml"] = "application/vnd.google-earth.kml+xml";
	_mimeTypes["kmz"] = "application/vnd.google-earth.kmz";
	_mimeTypes["xls"] = "application/vnd.ms-excel";
	_mimeTypes["eot"] = "application/vnd.ms-fontobject";
	_mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
	_mimeTypes["odg"] = "application/vnd.oasis.opendocument.graphics";
	_mimeTypes["odp"] = "application/vnd.oasis.opendocument.presentation";
	_mimeTypes["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	_mimeTypes["odt"] = "application/vnd.oasis.opendocument.text";
	_mimeTypes["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	_mimeTypes["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	_mimeTypes["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	_mimeTypes["wmlc"] = "application/vnd.wap.wmlc";
	_mimeTypes["wasm"] = "application/wasm";
	_mimeTypes["7z"] = "application/x-7z-compressed";
	_mimeTypes["cco"] = "application/x-cocoa";
	_mimeTypes["jardiff"] = "application/x-java-archive-diff";
	_mimeTypes["jnlp"] = "application/x-java-jnlp-file";
	_mimeTypes["run"] = "application/x-makeself";
	_mimeTypes["pl"] = "application/x-perl";
	_mimeTypes["pm"] = "application/x-perl";
	_mimeTypes["prc"] = "application/x-pilot";
	_mimeTypes["pdb"] = "application/x-pilot";
	_mimeTypes["rar"] = "application/x-rar-compressed";
	_mimeTypes["rpm"] = "application/x-redhat-package-manager";
	_mimeTypes["sea"] = "application/x-sea";
	_mimeTypes["swf"] = "application/x-shockwave-flash";
	_mimeTypes["sit"] = "application/x-stuffit";
	_mimeTypes["tcl"] = "application/x-tcl";
	_mimeTypes["tk"] = "application/x-tcl";
	_mimeTypes["der"] = "application/x-x509-ca-cert";
	_mimeTypes["pem"] = "application/x-x509-ca-cert";
	_mimeTypes["crt"] = "application/x-x509-ca-cert";
	_mimeTypes["xpi"] = "application/x-xpinstall";
	_mimeTypes["xhtml"] = "application/xhtml+xml";
	_mimeTypes["xspf"] = "application/xspf+xml";
	_mimeTypes["zip"] = "application/zip";

	// Application/octet-stream group
	_mimeTypes["bin"] = "application/octet-stream";
	_mimeTypes["exe"] = "application/octet-stream";
	_mimeTypes["dll"] = "application/octet-stream";
	_mimeTypes["deb"] = "application/octet-stream";
	_mimeTypes["dmg"] = "application/octet-stream";
	_mimeTypes["iso"] = "application/octet-stream";
	_mimeTypes["img"] = "application/octet-stream";
	_mimeTypes["msi"] = "application/octet-stream";
	_mimeTypes["msp"] = "application/octet-stream";
	_mimeTypes["msm"] = "application/octet-stream";

	// Audio
	_mimeTypes["mid"] = "audio/midi";
	_mimeTypes["midi"] = "audio/midi";
	_mimeTypes["kar"] = "audio/midi";
	_mimeTypes["mp3"] = "audio/mpeg";
	_mimeTypes["ogg"] = "audio/ogg";
	_mimeTypes["m4a"] = "audio/x-m4a";
	_mimeTypes["ra"] = "audio/x-realaudio";

	// Video
	_mimeTypes["3gpp"] = "video/3gpp";
	_mimeTypes["3gp"] = "video/3gpp";
	_mimeTypes["ts"] = "video/mp2t";
	_mimeTypes["mp4"] = "video/mp4";
	_mimeTypes["mpeg"] = "video/mpeg";
	_mimeTypes["mpg"] = "video/mpeg";
	_mimeTypes["ogv"] = "video/ogg";
	_mimeTypes["mov"] = "video/quicktime";
	_mimeTypes["webm"] = "video/webm";
	_mimeTypes["flv"] = "video/x-flv";
	_mimeTypes["m4v"] = "video/x-m4v";
	_mimeTypes["mkv"] = "video/x-matroska";
	_mimeTypes["mng"] = "video/x-mng";
	_mimeTypes["asx"] = "video/x-ms-asf";
	_mimeTypes["asf"] = "video/x-ms-asf";
	_mimeTypes["wmv"] = "video/x-ms-wmv";
	_mimeTypes["avi"] = "video/x-msvideo";
}

void HttpResponse::setStatusTexts()
{
	_statusTexts.insert(std::make_pair(200, "200 OK"));
	_statusTexts.insert(std::make_pair(201, "201 Created"));
	_statusTexts.insert(std::make_pair(202, "202 Accepted"));
	_statusTexts.insert(std::make_pair(204, "204 No Content"));
	_statusTexts.insert(std::make_pair(206, "206 Partial Content"));
	_statusTexts.insert(std::make_pair(301, "301 Moved Permanently"));
	_statusTexts.insert(std::make_pair(302, "302 Found"));
	_statusTexts.insert(std::make_pair(303, "303 See Other"));
	_statusTexts.insert(std::make_pair(304, "304 Not Modified"));
	_statusTexts.insert(std::make_pair(307, "307 Temporary Redirect"));
	_statusTexts.insert(std::make_pair(308, "308 Permanent Redirect"));
	_statusTexts.insert(std::make_pair(400, "400 Bad Request"));
	_statusTexts.insert(std::make_pair(401, "401 Unauthorized"));
	_statusTexts.insert(std::make_pair(402, "402 Payment Required"));
	_statusTexts.insert(std::make_pair(403, "403 Forbidden"));
	_statusTexts.insert(std::make_pair(404, "404 Not Found"));
	_statusTexts.insert(std::make_pair(405, "405 Method Not Allowed"));
	_statusTexts.insert(std::make_pair(406, "406 Not Acceptable"));
	_statusTexts.insert(std::make_pair(408, "408 Request Timeout"));
	_statusTexts.insert(std::make_pair(409, "409 Conflict"));
	_statusTexts.insert(std::make_pair(410, "410 Gone"));
	_statusTexts.insert(std::make_pair(411, "411 Length Required"));
	_statusTexts.insert(std::make_pair(412, "412 Precondition Failed"));
	_statusTexts.insert(std::make_pair(413, "413 Payload Too Large"));
	_statusTexts.insert(std::make_pair(414, "414 URI Too Long"));
	_statusTexts.insert(std::make_pair(415, "415 Unsupported Media Type"));
	_statusTexts.insert(std::make_pair(416, "416 Requested Range Not Satisfiable"));
	_statusTexts.insert(std::make_pair(421, "421 Misdirected Request"));
	_statusTexts.insert(std::make_pair(429, "429 Too Many Requests"));
	_statusTexts.insert(std::make_pair(500, "500 Internal Server Error"));
	_statusTexts.insert(std::make_pair(501, "501 Not Implemented"));
	_statusTexts.insert(std::make_pair(502, "502 Bad Gateway"));
	_statusTexts.insert(std::make_pair(503, "503 Service Unavailable"));
	_statusTexts.insert(std::make_pair(504, "504 Gateway Timeout"));
	_statusTexts.insert(std::make_pair(505, "505 HTTP Version Not Supported"));
	_statusTexts.insert(std::make_pair(507, "507 Insufficient Storage"));
}

static std::string createDirIndex(std::string path)
{
	std::string dirIndex = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Directory Index</title></head><body><h1>Directory Index for " + path + "</h1><ul>";

	DIR *dir = opendir(path.c_str());
	if (!dir)
	{
		std::cerr << "Failed to open directory: " << strerror(errno) << std::endl;
		return "<p>Error opening directory.</p>";
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_name[0] != '.')
		{ // Skip hidden files
			dirIndex += "<li>" + std::string(entry->d_name) + "</li>";
		}
	}

	dirIndex += "</ul></body></html>";
	closedir(dir);
	return dirIndex;
}

void HttpResponse::openDir(std::string path)
{
	std::string newPath = removeSlashes(path);
	for (std::vector<std::string>::const_iterator it = _block->getDefaultFiles().begin(); it != _block->getDefaultFiles().end(); ++it)
	{
		std::string newDefault = removeSlashes(*it);
		_fileName = newPath + "/" + newDefault;
		std::ifstream file(_fileName.c_str());
		if (file.is_open())
		{
			std::stringstream ss;
			ss << file.rdbuf();
			_resBody = ss.str();
			_resContentLength = _resBody.size();
			file.close();
			_resStatus = 200;
			return;
		}
	}
	if (_block->getAutoIndex())
	{

		_resBody = createDirIndex(path);
		_resContentLength = _resBody.size();
		_resStatus = 200;
	}
	else {
		_resStatus = 403;
		return;
	}
}

std::string HttpResponse::getFullPath()
{
	std::string locPath = _req->getPath();
	if (_block->getRoot().empty())
		return (locPath);
	if (_loc != NULL)
	{
		size_t locIndex = _req->getPath().find(_loc->getLocation());

		if (locIndex == 0)
		{
			locPath = _req->getPath().substr(_loc->getLocation().size());
			if (locPath[0] == '/')
				locPath.erase(0, 1);
		}
	}
	return (locPath);
}

void HttpResponse::checkFile(int methodType)
{
	struct stat st;
	std::stringstream ss;

	std::size_t pos = _fileName.find_last_of("?");
	if (pos != std::string::npos)
		_fileName = _fileName.substr(0, pos);

	if (stat(_fileName.c_str(), &st) == -1)
	{
		ss << "stat error: " << strerror(errno) << " '" << _fileName << "'";
		printLog(ss.str(), RED, std::cout);
		_resStatus = 404;
		return;
	}
	ss << "stat sucess - _fileName: " << " '" << _fileName << "'";
	printLog(ss.str(), GREEN, std::cout);
	if (_cgiRequest == true)
	{
		forkExecCgi(_cgiPath);
		return;
	}
	if (methodType == GET && S_ISREG(st.st_mode))
			openReg(_fileName, methodType, st.st_size);
	else if (S_ISDIR(st.st_mode))
	{
		if (methodType == GET)
			openDir(_fileName);
		else if (methodType == DELETE)
			_resStatus = 409;
	}
}

LocationBlock *HttpResponse::checkLocationBlock()
{
	std::vector<LocationBlock>::iterator it;
	size_t longestMatch;
	LocationBlock *tempLocation = NULL;

	it = _conf->_locations.begin();
	longestMatch = 0;
	while (it != _conf->_locations.end())
	{
		const std::string &locationPath = it->getLocation();
		const std::string &requestPath = _req->getPath();
		if (requestPath.compare(0, locationPath.size(), locationPath) == 0)
		{
			if (locationPath.size() > longestMatch)
			{
				longestMatch = locationPath.size();
				tempLocation = &(*it);
			}
		}
		it++;
	}
	return (tempLocation);
}

void HttpResponse::handleDELETE()
{
	checkFile(DELETE);
	if (_resStatus != 200)
		return;
	int removed = std::remove(_fileName.c_str());
	if (removed == 0)
		_resStatus = 204;
	else
		_resStatus = 403;
}

const std::string parseHostHeader(const std::string &host)
{
	std::string str;
	size_t pos = host.find(":");
	if (pos != std::string::npos)
		str = host.substr(0, pos);
	else
		str = host;
	return (str);
}

const std::string parseContentType(const std::map<std::string, std::string> &headers)
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Type");
	if (it == headers.end())
		return ("");
	return (it->second);
}

bool HttpResponse::checkValidCGI(const std::string &temp)
{
	std::map<std::string, std::string>::const_iterator it = _block->getCgiMap().begin();
	while (it != _block->getCgiMap().end())
	{
		if (temp.size() >= it->first.size() && temp.compare(temp.size() - it->first.size(), it->first.size(), it->first) == 0)
		{
			_scriptName = "SCRIPT_NAME=" + temp;
			_cgiRequest = true;
			_cgiPath = it->second;
			return (true);
		}
		it++;
	}
	_scriptName = "SCRIPT_NAME=";
	return (false);
}

void HttpResponse::parseScriptName(const std::string &str)
{
	std::string path;
	std::string temp;
	std::stringstream ss(str);

	while (std::getline(ss, path, '/'))
	{
		if (path.empty())
			continue;
		temp = temp + "/" + path;
		if (checkValidCGI(temp) == true)
			return;
	}
}

void HttpResponse::parsePath()
{
	std::string path = this->_req->getPath();
	std::string temp;
	size_t index = path.find("?");
	if (index != std::string::npos)
	{
		_queryString = "QUERY_STRING=" + path.substr(index + 1, (path.size() - index));
		temp = path.substr(0, index);
	}
	else
	{
		_queryString = "QUERY_STRING=";
		temp = path;
	}
	parseScriptName(temp);
	if (temp.size() > _scriptName.size())
		_pathInfo = "PATH_INFO=" + temp.substr(_scriptName.size());
	else
		_pathInfo = "PATH_INFO=";
}

const std::string HttpResponse::parseContentLength(const std::map<std::string, std::string> &headers)
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
	if (it != headers.end())
		return ("CONTENT_LENGTH=" + it->second);
	std::stringstream ss;
	ss << _req->getBody().size();
	return ("CONTENT_LENGTH=" + ss.str());
}

void HttpResponse::buildEnv()
{
	size_t size;

	size = _tempEnv.size();
	_envp = new char *[size + 1];
	for (size_t i = 0; i < size; i++)
	{
		_envp[i] = new char[_tempEnv[i].size() + 1];
		std::strcpy(_envp[i], _tempEnv[i].c_str());
	}
	_envp[size] = NULL;
}

void HttpResponse::setEnv()
{

	_serverSoftware = "SERVER_SOFTWARE=WebServer/1.0";
	_serverProtocol = "SERVER_PROTOCOL=" + this->_req->getVersion();
	_serverPort = "SERVER_PORT=" + _client->_server->getPort();
	_requestMethod = "REQUEST_METHOD=" + this->_req->getMethod();
	_remoteAddress = "REMOTE_ADDR=" + _client->_server->getIp();
	_gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";
	setTempEnv(_serverSoftware);
	setTempEnv(_serverProtocol);
	setTempEnv(_serverPort);
	setTempEnv(_requestMethod);
	setTempEnv(_remoteAddress);
	setTempEnv(_gatewayInterface);
	if (this->_req->getHeaders().find("Host") != this->_req->getHeaders().end())
	{
		_serverName = "SERVER_NAME=" + parseHostHeader(this->_req->getHeaders().find("Host")->second);
	}
	setTempEnv(_serverName);
	_cgiContentType = parseContentType(this->_req->getHeaders());
	if (_cgiContentType != "")
	{
		_cgiContentType = "CONTENT_TYPE=" + _cgiContentType;
		setTempEnv(_cgiContentType);
	}
	parsePath();
	setTempEnv(_pathInfo);
	setTempEnv(_queryString);
	setTempEnv(_scriptName);
	if (_req->getBody().size() > 0)
	{
		_contentLength = parseContentLength(this->_req->getHeaders());
		setTempEnv(_contentLength);
	}
	buildEnv();
}

void	HttpResponse::buildFullPath() {
	_newRoot = removeSlashes(_block->getRoot());
	std::string locPath = removeSlashes(this->getFullPath());
	_scriptNameNico = locPath.substr(locPath.find_last_of('/') + 1, locPath.size());
	std::size_t pos = locPath.find_last_of('/');
	if (pos != std::string::npos)
		_fullPath = _newRoot + "/" + locPath.substr(0, pos);
	else
		_fullPath = _newRoot + "/" + locPath;
	locPath = removeSlashes(this->getFullPath());
	_fileName = _newRoot + "/" + locPath;
}

void HttpResponse::execMethod()
{
	bool methodFound = false;
	std::string root;
	std::string method = _req->getMethod();
	std::vector<std::string>::const_iterator it;

	if (_resStatus == 400)
		return;
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		_resStatus = 501;
		return;
	}
	for (it = _block->getMethods().begin(); it != _block->getMethods().end(); ++it)
		if (*it == method)
			methodFound = true;
	if (methodFound == false)
	{
		_resStatus = 405; // Method Not Allowed
		return;
	}
	if (this->_req->getBody().length() > static_cast<size_t>(_block->getRequestSize()))
	{
		_resStatus = 413;
		return ;
	}
	root = _block->getRoot();
	buildFullPath();
	if (!(_block->getNewLocation().empty()))
	{
		_resStatus = _block->getRedirectStatusCode();
		return;
	}
	if (method == "GET")
		checkFile(GET);
	else if (method == "DELETE")
		handleDELETE();
	else if (method == "POST")
    	handlePOST();
}

int HttpResponse::getStatusCode(void) const
{
	return _resStatus;
}

const std::string HttpResponse::httpFileContent(int errorPage)
{

	char buffer[BUFFER_SIZE];
	std::string result;
	ssize_t bytesRead;
	while ((bytesRead = read(errorPage, buffer, BUFFER_SIZE)) > 0)
	{
		result.append(buffer, bytesRead);
	}
	if (bytesRead < 0)
	{
		_fileName = ".html";
		_resStatus = 404;
		return http_error_404_page;
	}
	return (result);
}


int HttpResponse::openFile()
{
	if (!_block || _block->getErrorPage() == NULL)
		return (0);
	std::set<ErrorPageRule>::const_iterator it = _block->getErrorPage()->begin();

	while (it != _block->getErrorPage()->end())
	{
		if ((*it).error == _resStatus)
		{
			if ((*it).newError != -1)
				_resStatus = (*it).newError;
			break;
		}

		it++;
	}
	if (it == _block->getErrorPage()->end())
	{
		_fileName = ".html";
		return (0);
	}
	int configFile = open((*it).errorPath.c_str(), O_RDONLY);
	if (configFile < 0)
		return (-1);
	_fileName = (*it).errorPath.c_str();
	return (configFile);
}

void	HttpResponse::parseCgiStatus(const std::string& segment) {
	std::stringstream	ss(segment);
	std::string			str;
	int					num;
	char				*endptr;
	
	ss >> str;
	if (!(ss >> str)) {
		_cgiStatusCode = 200;
		return ;
	}
	errno = 0;
	num = std::strtol(str.c_str(), &endptr, 10);
	if (str.size() != 3 || *endptr || num < 0) {
		_cgiStatusCode = 200;
		return ;
	}
	_cgiStatusCode = num;
}

void	HttpResponse::parseContentLength(const std::string& segment) {
	std::stringstream	ss(segment);
	std::string			str;
	long				num;
	char				*endptr;

	ss >> str;
	if (!(ss >> str))
		return ;
	num = std::strtol(str.c_str(), &endptr, 10);
	errno = 0;
	if (errno == ERANGE || *endptr || num < 0)
		return ;
	std::stringstream ss2;
	ss2 << num;
	_cgiContentLength = "Content-Length: " + ss2.str() + CRLF;
}

void HttpResponse::parseCgiHeaders() {
	std::string 		segment;
	std::string			header;
	std::stringstream	ss(_cgiHeaders);

	_cgiContentType.clear();
	while (std::getline(ss, segment)) {
		std::stringstream	ss2(segment);
		ss2 >> header;

		if (header == "Set-Cookie:")
			setCgiCookies(segment);
		else if (header == "Content-Type:")
			setCgiContentType(segment);
		else if (header == "Status:")
			parseCgiStatus(segment);
		else if (header == "Content-Length:")
			parseContentLength(segment);
		else if (header == "Location:")
			setCgiLocation(segment);
		_cgiHeadersFound++;
	}
	if (_cgiBody != "" && _cgiLocation == "" && _cgiContentType == "") {
		_resStatus = 500;
		return ;
	}
	else if (_cgiLocation != "" && _cgiHeadersFound == 1) {
		_resStatus = 302;
		return ;
	}
	if (_cgiContentLength == "") {
		std::stringstream ss2;
		ss2 << _cgiBody.size();
		_cgiContentLength = "Content-Length: " + ss2.str() + CRLF;
	}
	if (!_cgiCookies.empty())
		for (std::vector<std::string>::iterator it = _cgiCookies.begin(); it != _cgiCookies.end(); it++)
			_cgiParsedHeaders.push_back(*it + CRLF);
}

void HttpResponse::parseCgiScript() {
	size_t pos;

	pos  = _response.find("\r\n\r\n");
	if (pos != std::string::npos) {
		_cgiHeaders = _response.substr(0, pos);
		_cgiBody = _response.substr(pos + 4);
		std::cout << RED << "aqui: " << _cgiHeaders << std::endl << _cgiBody << RESET << std::endl;
	}
	else
		_cgiHeaders = _response;
	parseCgiHeaders();
}

std::string HttpResponse::cgiHeader()
{
	if (_resStatus == 200)
		parseCgiScript();
	if (_cgiStatusCode != 0) {
		_resStatus = 0;
		setHttpStatus(_cgiStatusCode);
	}
	else
		setHttpStatus(_resStatus);
	std::string fileType = getMimeType(_fileName);
	std::ostringstream header;
	header << "HTTP/1.1 " << _httpStatus << CRLF;
	header << "Server: WebServer/1.0" << CRLF;
	header << "Date: " << get_http_date() << CRLF;
	for (size_t i = 0; i < _cgiParsedHeaders.size(); i++)
		header << _cgiParsedHeaders[i];
	if (_resStatus == 500) {
		header << "Content-Length: " << http_error_500_page.size() << CRLF;
		return (header.str() + CRLF + http_error_500_page);
	}
	if (_req != NULL && _req->getParseStatus() == 200)
	{
		if (_req->session) {
			if (fileType == "text/html")
				checkCookies(_resBody);
			header << "Set-Cookie: " << "session_id=" + _req->session->getSessionId() << "; Path=/";
			if (!_req->session->getTheme().empty())
				header << "; Theme=" << _req->session->getTheme();
			header << CRLF;
		}	
	}
	if (_resStatus == 204 || _resStatus == 302 || _resStatus == 304) {
		header << "Content-Length: 0" << CRLF;
		return (header.str() + CRLF);
	}
	header << _cgiContentLength + CRLF;
	return (header.str() + CRLF + _cgiBody);
}


const std::string HttpResponse::getMimeType(const std::string &fileExtension)
{
	size_t pos = fileExtension.find_last_of('.');

	if (pos == std::string::npos)
		return "application/octet-stream"; // default

	std::string ext = fileExtension.substr(pos + 1);

	for (size_t i = 0; i < ext.size(); ++i)
		ext[i] = std::tolower(ext[i]); // lowercase because it could be insensitive case

	std::map<std::string, std::string>::const_iterator it = _mimeTypes.find(ext);
	if (it != _mimeTypes.end())
		return it->second;

	return ("application/octet-stream");
}

std::string HttpResponse::header(int requestType)
{
	std::ostringstream header;
	std::string fileType;

	fileType = getMimeType(_fileName);
	setHttpStatus(_resStatus);
	if (requestType == REDIRECT)
	{
		_resBody = "<html>";
		_resBody += "<head><title>";
		_resBody += _httpStatus;
		_resBody += "</title></head>";
		_resBody += "<body>";
		_resBody += "<center><h1>";
		_resBody += _httpStatus;
		_resBody += "</h1></center>";
		_resBody += "<hr><center>WebServer/1.0</center>";
		_resBody += "</body>";
		_resBody += "</html>";
		_resContentLength = _resBody.size();
	}
	header << "HTTP/1.1 " << _httpStatus << CRLF;
	header << "Server: WebServer/1.0" << CRLF;
	header << "Date: " << get_http_date() << CRLF;
	header << "Content-Type: " << fileType << CRLF;
	if (_req != NULL && _req->getParseStatus() == 200)
	{
		if (_req->session) {
			if (fileType == "text/html")
				checkCookies(_resBody);
			header << "Set-Cookie: " << "session_id=" + _req->session->getSessionId() << "; Path=/";
			if (!_req->session->getTheme().empty())
				header << "; Theme=" << _req->session->getTheme();
			header << CRLF;
		}	
	}
	if (_resStatus == 204 || _resStatus == 201 || _resStatus == 304)
		header << "Content-Length: 0" << CRLF;
	else
		header << "Content-Length: " << _resContentLength << CRLF;
	if (requestType == REDIRECT)
		header << "Location: " << _block->getNewLocation() << CRLF;
	header << CRLF;
	return (header.str());
}

void HttpResponse::checkCookies(std::string& body)
{
	std::string theme = _req->session->getTheme();

	if (theme.empty())
		return;

	if (theme == "dark")
	{
		std::string styleInjection;
		styleInjection =
			"<style>"
			"body { background-color: black; color: white; }"
			"h1, p { color: lightgray; }"
			"</style>";
		// Insert the style before </head>
		size_t headPos = body.find("</head>");
		if (headPos != std::string::npos)
		{
			body.insert(headPos, styleInjection);
			_resContentLength += styleInjection.size();
		}
	}
}

const std::string HttpResponse::checkErrorResponse(const std::string &page)
{
	int errorPage;

	errorPage = openFile();
	if (errorPage < 0)
	{
		_resBody = http_error_404_page;
		_resContentLength = _resBody.size();
		return (header(ERROR) + _resBody);
	}
	else if (errorPage == 0)
	{
		std::cout << RED << page << RESET << std::endl;
		_resBody = page;
		_resContentLength = _resBody.size();
		_fileName = ".html";
	}
	else
	{
		_resBody = httpFileContent(errorPage);
		_resContentLength = _resBody.size();
	}
	if (_resStatus == 204 || _resStatus == 304)
		return (header(OK));
	return (header(ERROR) + _resBody);
}

const std::string HttpResponse::checkStatusCode()
{
	std::string fileContent;


	switch (_resStatus)
	{
	case 200:
		if (_cgiRequest)
			return (cgiHeader());
		else if (_method == DELETE || _method == POST)
			return (header(OK));
		return (header(OK) + _resBody);
	case 206:
		if (_method == DELETE || _method == POST)
			return (header(OK));
		return (header(OK) + _resBody);
	case 201:
		return (header(OK));
	case 202:
		return (header(OK));
	case 204:
		return (header(OK));
	case 301:
		return (header(REDIRECT));
	case 302:
		return (header(REDIRECT));
	case 303:
		return (header(REDIRECT));
	case 304:
		return (header(REDIRECT));
	case 307:
		return (header(REDIRECT));
	case 308:
		return (header(REDIRECT));
	case 400:
		return (checkErrorResponse(http_error_400_page));
	case 403:
		return (checkErrorResponse(http_error_403_page));
	case 404:
		return (checkErrorResponse(http_error_404_page));
	case 405:
		return (checkErrorResponse(http_error_405_page));
	case 408:
		return (checkErrorResponse(http_error_408_page));
	case 409:
		return (checkErrorResponse(http_error_409_page));
	case 411:
		return (checkErrorResponse(http_error_411_page));
	case 413:
		return (checkErrorResponse(http_error_413_page));
	case 414:
		return (checkErrorResponse(http_error_414_page));
	case 415:
		return (checkErrorResponse(http_error_415_page));
	case 500:
		return (checkErrorResponse(http_error_500_page));
	case 501:
		return (checkErrorResponse(http_error_501_page));
	case 502:
		return (checkErrorResponse(http_error_502_page));
	case 503:
		return (checkErrorResponse(http_error_503_page));
	case 504:
		return (checkErrorResponse(http_error_504_page));
	case 505:
		return (checkErrorResponse(http_error_505_page));
	}
	return _resBody;
}

HttpResponse::HttpResponse(Client *client) : _resStatus(-1), _cgiPid(0),  _resContentLength(0), _method(-1), _cgiRequest(false), _cgiHeadersFound(0), _cgiStatusCode(0)
{
	_client = client;
	_conf = client->_request->_config;
	_req = client->_request;
	_loc = checkLocationBlock();
	setStatusTexts();
	if (_loc != NULL)
		_block = _loc;
	else
		_block = _conf;
	setMimeTypes();
	setEnv();
	_filePos = 0;
	if (_req->hasParseError()) {
		_resStatus = _req->getParseStatus();
		return ;
	}
	startResponse();
}

// ### SETTERS ###

void HttpResponse::setResStatus(int status)
{
	_resStatus = status;
}

void HttpResponse::setResponse(const std::string &response)
{
	_response = response;
}

const std::string &HttpResponse::getResponse(void) const
{
	return (_response);
}

const Configuration &HttpResponse::getConfig(void) const
{
	return (*_conf);
}

const std::string &HttpResponse::getResBody() const
{
	return (_resBody);
}

const std::string &HttpResponse::getResHeader() const
{
	return (_resHeader);
}

void HttpResponse::setTempEnv(const std::string &str)
{
	_tempEnv.push_back(str);
}

void	HttpResponse::extractFileName(const std::string& str) {
	size_t pos;

	pos = str.find("filename=\"");
	if (pos == std::string::npos) 
		return ;
	pos += 10; // skip 'filename="'
	size_t end = str.find("\"", pos);
	_fileName = str.substr(pos, end - pos);
}

void	HttpResponse::cleanUploadDir() {
	std::string	str = _block->getUploadDirectory();
	std::string	clean;

	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] != '.' && str[i] != '/')
			clean += str[i];
	}
	size_t	end = clean.find_last_not_of('/');
	if (end != std::string::npos)
		clean.erase(end + 1);
	_block->setUploadDirectory(clean);
}

void	HttpResponse::doRawUpload() {
        _fileName = _newUploadDir + _fileAfterRelativePath;

        // Save file content
        if (!saveBodyToFile(_fileName, _req->getBody()))
			_resStatus = 500;
		else 
			_resStatus = 201;
}

bool	HttpResponse::isRequestUpload() {
	_contentType = parseContentType(_req->getHeaders());
	size_t	pos = _contentType.find("multipart/form-data");
	std::string location = _block->getLocation();
	_relativePath = _req->getPath().substr(location.size());
	if (_relativePath[0] == '/')
		_relativePath.erase(0, 1);
	size_t pos2 = _relativePath.find("/");
	if (pos2 != std::string::npos) {
		_fileAfterRelativePath = _relativePath.substr(pos2 + 1);
		_relativePath = _relativePath.substr(0, pos2);
	}

	if (pos != std::string::npos) {
		size_t pos2 = _contentType.find("boundary=");
		if (pos2 == std::string::npos)
			return (false);
		_boundary = _contentType.substr(pos2 + 9, _contentType.size()); // +9 to skip 'boundary='
		_contentType = "multipart/form-data";
		return (true);
	}
	if (_fileAfterRelativePath != "") {
		_rawUpload = true;
		return (true);
	}
	return (false);
}

int HttpResponse::processMultipartFormData()
{
	if (_boundary.empty())
		return 400; // Bad Request
	std::string boundaryMarker = "--" + _boundary;
	std::string endBoundary = boundaryMarker + "--";
	size_t pos = 0;
	size_t next = 0;
	bool allSuccess = true;
	bool anySuccess = false;

	while (true)
	{
		// Find start of next part
		pos = _req->getBody().find(boundaryMarker, pos);
		if (pos == std::string::npos)
			break;

		pos += boundaryMarker.size();

		// End of body reached?
		if (_req->getBody().compare(pos, 2, "--") == 0)
			break; // reached final boundary

		// Skip CRLF
		if (_req->getBody().compare(pos, 2, "\r\n") == 0)
			pos += 2;

		// Find next boundary or end of body
		next = _req->getBody().find(boundaryMarker, pos);
		if (next == std::string::npos) {
			// This is the last part — take until endBoundary or end of body
			next = _req->getBody().find(endBoundary, pos);
			if (next == std::string::npos)
				next = _req->getBody().size();
		}

		std::string part = _req->getBody().substr(pos, next - pos);

		// Split headers and content
		size_t headerEnd = part.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
			return 400; // invalid part (missing header separator)

		std::string headers = part.substr(0, headerEnd);
		std::string content = part.substr(headerEnd + 4); // skip CRLFCRLF

		// Extract filename from this part's headers
		extractFileName(headers);
		if (_fileName.empty()) {
			allSuccess = false;
			continue; // skip this part (no filename)
		}

		_fileName = _newUploadDir + _fileName;

		// Save file content
		if (saveBodyToFile(_fileName, content))
			anySuccess = true;
		else
			allSuccess = false;

		// Advance to after next boundary marker
		pos = next;
    }

	if (!anySuccess)
		return 400; // all failed (Bad Request)
	if (!allSuccess)
		return 200; // some failed
	return 201;     // all succeeded (Created)
}

void	HttpResponse::setNewUploadDir() {
	if (_newUploadDir == "") {
		if (_block->getUploadDirectory() == "")
			_newUploadDir = _newRoot + "/" + _block->getUploadDirectory();
		else
			_newUploadDir = _newRoot + "/" + _block->getUploadDirectory() + "/";
		if (_relativePath != "")
			_newUploadDir += _relativePath + "/";
	}
}

void HttpResponse::handlePOST()
{
	checkFile(POST);
	if (_cgiRequest)
		return;
	// O header content-type é necessário para o POST
	std::string contentType = parseContentType(_req->getHeaders());
	if (!_req->getBody().empty() && contentType.empty())
	{
		_resStatus = 415; // Unsupported Media Type
		_resBody = http_error_415_page;
		_resContentLength = _resBody.size();
		return;
	}
	// Ver se upload dir existe no config.
	std::string uploadDir = _block->getUploadDirectory();
	if (uploadDir == "" || !isRequestUpload())
	{
		_resStatus = 405;
		_resBody = http_error_405_page;
		_resContentLength = _resBody.size();
		return ;
	}
	cleanUploadDir();
	setNewUploadDir(); // this sets the Upload Directory with the root
	if (access(uploadDir.c_str(), W_OK) != 0)
	{
		_resStatus = 500; // Internal Server Error
		_resBody = http_error_500_page;
		_resContentLength = _resBody.size();
		return ;
	}
	if (_rawUpload == true) {
		doRawUpload();
		return ;
	}
	_resStatus = processMultipartFormData();
}

bool HttpResponse::saveBodyToFile(const std::string &path, const std::string &content) const
{
	std::ofstream file(path.c_str(), std::ios::binary); // abordagem ideal para arquivos binarios, nao textuais
	if (!file.is_open())
	{
		return false;
	}

	file.write(content.c_str(), content.length());
	file.close();

	return !file.fail();
}

void HttpResponse::terminateCgiProcess(void)
{
	if (_cgiPid > 0)
		kill(_cgiPid, SIGKILL);
	_resStatus = 504; // Gateway Timeout
}

int HttpResponse::getCgiPid(void) const
{
	return (_cgiPid);
}