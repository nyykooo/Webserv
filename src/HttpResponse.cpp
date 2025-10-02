#include "../includes/headers.hpp"

HttpResponse::HttpResponse() : _resStatus(-1), _useNewLocation(false), _pipeIn(-1), _pipeOut(-1), _cgiPid(-1) {}

HttpResponse::HttpResponse(const HttpResponse &other)
{
	*this = other;
}

HttpResponse HttpResponse::operator=(const HttpResponse &other)
{
	if (this != &other)
	{
		std::cout << std::endl;
	}
	return (*this);
}

HttpResponse::~HttpResponse() {}

static std::string readFd(int fd)
{
	const size_t bufSize = 4096; // usa-se 4KB pois eh um meio termo entre chamadas pequenas e muito longas
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
			ss << "Erro ao ler ficheiro de saida do CGI fd (" << fd << ")";
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
		if (exitStatus == 1)
			_resStatus = 500; // Internal Server Error
		else if (exitStatus == 0)
			_resStatus = 200; // OK

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

// ### CGI ###
void HttpResponse::forkExecCgi(std::string interpreter)
{
	_fileName = removeSlashes(_fileName);
	char *args[2];
	int pipeInput[2];
	int pipeOutput[2];
	std::string _script_name = _scriptNameNico.find_last_of('?') != std::string::npos ? _scriptNameNico.substr(0, _scriptNameNico.find_last_of('?')).c_str() : _scriptNameNico.c_str();
	args[0] = const_cast<char *>(interpreter.c_str());
	args[1] = const_cast<char *>(_script_name.c_str());
	args[2] = NULL; // Terminate the args array with NULL

	pipeInput[0] = _pipeIn;
	pipeInput[1] = STDIN_FILENO;

	pipeOutput[0] = STDOUT_FILENO;
	pipeOutput[1] = _pipeOut;

	if (_cgiPid > 0)
		checkCgiProcess();

	// Create pipes for input and output
	if (pipe(pipeInput) == -1 || pipe(pipeOutput) == -1)
	{
		std::cerr << "Pipe error: " << strerror(errno) << std::endl;
		_resStatus = 500; // Internal Server Error
		return;
	}

	std::stringstream ss;
	ss << "CGI execution: \n\t-interpreter: " << interpreter << ";\n\t-args: " << _fileName; // adicionar variaveis de ambiente depois
	printLog(ss.str(), WHITE, std::cout);

	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Fork error: " << strerror(errno) << std::endl;
		_resStatus = 500; // Internal Server Error
		return;
	}
	if (pid == 0)
	{
		std::stringstream ss;
		// processo filho
		dup2(pipeInput[0], STDIN_FILENO);	// Redirect stdin to pipe input
		dup2(pipeOutput[1], STDOUT_FILENO); // Redirect stdout to pipe output

		// Abrir um arquivo para redirecionar o stderr
		int debugFd = open("./cgi_debug.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (debugFd == -1)
		{
			std::cerr << "Failed to open debug log file: " << strerror(errno) << std::endl;
			exit(1); // Internal Server Error
		}
		dup2(debugFd, STDERR_FILENO); // Redirect stderr to the debug file
		close(debugFd);				  // Fechar o descritor original

		// mudar de diretorio para o diretorio do CGI
		if (chdir(_fullPath.c_str()) == -1)
		{
			ss << "CGI Chdir error: " << strerror(errno) << std::endl;
			printLog(ss.str(), RED, std::cerr);
			exit(1); // Internal Server Error
		}
		// Preparar variaveis de ambiente futuramente (pesquisar melhor sobre elas e sobre como o CGI as usa)

		// Execve interpretador do cgi, passando como arg o path para cgi e as envps
		execve(interpreter.c_str(), const_cast<char **>(args), const_cast<char **>(_envp));
		ss << "CGI Execution error: " << strerror(errno);
		printLog(ss.str(), RED, std::cerr);
		exit(1); // Internal Server Error
	}
	else
	{
		// processo pai
		_cgiPid = pid;
		std::stringstream ss;
		ss << "CGI process started:\n\t-PID: " << _cgiPid;
		printLog(ss.str(), WHITE, std::cout);
		_client->setProcessingState(CGI_PROCESSING);
		close(pipeInput[0]); // entender melhor a ordem de fechamento dos fds aqui
		close(pipeOutput[1]);
		_pipeIn = pipeInput[1];	  // lado de escrita (pai -> CGI stdin)
		_pipeOut = pipeOutput[0]; // lado de leitura (pai <- CGI stdout)

		return; // Retorna para não bloquear o servidor
	}
}

/* bool HttpResponse::lookForCgi(void)
{
	std::string fileExtension = removeSlashes(_fileName);
	size_t pos = fileExtension.find_last_of('.');
	if (pos != std::string::npos) {
		fileExtension = fileExtension.substr(pos);
		std::map<std::string, std::string>::const_iterator it = _block->getCgiMap().find(fileExtension);
		if (it != _block->getCgiMap().end()) {
			// avaliar se o ficheiro existe, se nao existir retornar 404(?)
			forkExecCgi(it->second);
			return true;
		}
	}
	return false;
} */

// ### EXEC METHOD ###

void HttpResponse::startResponse(void)
{
	if (_block->getRedirectStatusCode() != -1)
	{
		_resStatus = _block->getRedirectStatusCode();
		_useNewLocation = true;
		return;
	}
	execMethod();
}

void HttpResponse::openReg(std::string path, int methodType)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cerr << "ERRO AO ABRIR FICHEIRO: " << strerror(errno) << std::endl;
		_resStatus = 404;
		return;
	}
	_resStatus = 200;
	if (methodType == DELETE)
		return;
	std::stringstream ss;

	ss << file.rdbuf();
	_resBody = ss.str();
	file.close();
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
			file.close();
			_resStatus = 200;
			return;
		}
	}
	if (_block->getAutoIndex())
	{

		_resBody = createDirIndex(path);
		_resStatus = 200;
	}
	else {
		_resStatus = 403; // mudar para 403
		return;
	}
}

// pensar se uma exact match interferiria definicao dessa fullpath (redirecao)
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

	// procurar '?' e apagar para fazer o stat REMOVER ESSA SEQUENCIA DE CODIGO POS IMPLEMENTACAO DA QUERY STRING DO DIOGO
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
		return; // mudar status do HttpResponse/Client Bruno feature e retorna para nao bloquear o server
	}

	if (S_ISREG(st.st_mode))
	{
		openReg(_fileName, methodType);
	}
	else if (S_ISDIR(st.st_mode))
	{
		std::cout << "Diretório encontrado: " << _fileName << std::endl;
		if (methodType == GET)
			openDir(_fileName);
		else if (methodType == DELETE)
			_resStatus = 409;
	}
	else if (S_ISLNK(st.st_mode))
		std::cout << "Link simbólico\n";
	else
		std::cout << "Outro tipo de arquivo\n";
}

void HttpResponse::handleGET()
{
	if (!(_block->getNewLocation().empty()))
	{
		_resStatus = _block->getRedirectStatusCode();
		return;
	}
	std::string newRoot = removeSlashes(_conf->getRoot());
	std::string locPath = removeSlashes(this->getFullPath());

	// TODO: ESSAS INFORMACOES DEVEM SER EXTRAIDAS ANTES EM OUTRO LOCAL
	_scriptNameNico = locPath.substr(locPath.find_last_of('/') + 1, locPath.size());
	// _fullPath = newRoot + "/" + locPath.erase(locPath.find_last_of('/'));
	std::size_t pos = locPath.find_last_of('/');
	if (pos != std::string::npos)
	{
		_fullPath = newRoot + "/" + locPath.substr(0, pos);
	}
	else
	{
		_fullPath = newRoot + "/" + locPath; // se não tem '/', usa a string inteira
	}
	
	locPath = removeSlashes(this->getFullPath());
	_fileName = newRoot + "/" + locPath;
	checkFile(GET);
}

LocationBlock *HttpResponse::checkLocationBlock()
{
	std::vector<LocationBlock>::iterator it;
	size_t longestMatch;
	LocationBlock *tempLocation = NULL;

	it = _conf->locations.begin();
	longestMatch = 0;
	while (it != _conf->locations.end())
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
	if (!(_block->getNewLocation().empty()))
	{
		_resStatus = _block->getRedirectStatusCode();
		return;
	}
	std::string newRoot = removeSlashes(_conf->getRoot());
	std::string locPath = removeSlashes(this->getFullPath());
	if (!newRoot.empty())
		newRoot = "/" + newRoot;
	_fileName = newRoot + "/" + locPath;
	checkFile(DELETE);
	if (_resStatus != 200)
		return;
	// std::cout << _fileName << std::endl;
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
		_pathInfo = "PATH_INFO=" + temp;
	}
	else
	{
		_queryString = "QUERY_STRING=";
		temp = path.substr(0, path.size());
		_pathInfo = "PATH_INFO=" + temp;
	}
	parseScriptName(temp);
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
	// CONTENT_LENGTH - já está definido, mas antes de colocar nas envs é necessario verificar se é igual a "", porque se for não se coloca nas ENV
	// CONTENT_TYPE - já está definido, mas antes de colocar nas envs é necessario verificar se é igual a "", porque se for não se coloca nas ENV
	// SCRIPT_NAME

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
	/* 	if (this->_req->getBody().size() > 0)
			_contentLength = "CONTENT_LENGTH=" + this->_req->g */
	if (this->_req->getHeaders().find("Host") != this->_req->getHeaders().end())
	{
		_serverName = "SERVER_NAME=" + parseHostHeader(this->_req->getHeaders().find("Host")->second);
	}
	setTempEnv(_serverName);
	_contentType = parseContentType(this->_req->getHeaders());
	if (_contentType != "")
	{
		_contentType = "CONTENT_TYPE=" + _contentType;
		setTempEnv(_contentType);
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
	std::stringstream ss;
	ss << "CGI-URI parsed meta-variables:";
	ss << "\n\t" << _serverSoftware;
	ss << "\n\t" << _serverProtocol;
	ss << "\n\t" << _serverPort;
	ss << "\n\t" << _requestMethod;
	ss << "\n\t" << _remoteAddress;
	ss << "\n\t" << _gatewayInterface;
	ss << "\n\t" << _contentType;
	ss << "\n\t" << _scriptName;
	ss << "\n\t" << _serverName;
	ss << "\n\t" << _queryString;
	ss << "\n\t" << _pathInfo;
	printLog(ss.str(), GREEN, std::cout);

	buildEnv();
}

void HttpResponse::execMethod()
{
	bool methodFound = false;
	std::string root;
	std::string method = _req->getMethod();
	std::vector<std::string>::const_iterator it;

	for (it = _block->getMethods().begin(); it != _block->getMethods().end(); ++it)
	{
		if (*it != "GET" && *it != "POST" && *it != "DELETE")
		{
			_resStatus = 501;
			return;
		}
		if (*it == method)
			methodFound = true;
	}
	if (methodFound == false)
	{
		if (_resStatus == 400)
			return;
		_resStatus = 405; // Method Not Allowed
		return;
	}
	root = _block->getRoot();
	if (method == "GET")
		handleGET();
	else if (method == "DELETE")
		handleDELETE();
	else if (method == "POST")
    	handlePost();
}

static const std::string &http_error_400_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>400</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Bad Request</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_403_page =
	"<!DOCTYPE html>"
	"<html>"
	"<head>"
	"<title>403 Forbidden</title>"
	"</head>"
	"<body>"
	"<h1>403 Forbidden</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_404_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>404</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Page not found</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_405_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>405</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Method Not Allowed</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_408_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>408</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Request Timeout</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_409_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>409</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Conflict</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_411_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>411</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Length Required</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_413_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>413</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Payload Too Large</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_414_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>414</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>URI Too Long</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_500_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>500</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Internal Server Error</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_501_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>501</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Not implemented</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_502_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>502</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Bad Gateway</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_503_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>503</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Service Unavailable</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_504_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>504</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Gateway Timeout</h1>"
	"</body>"
	"</html>";

static const std::string &http_error_505_page =
	"<!DOCTYPE html>"
	"<html lang=\"en\">"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"<title>505</title>"
	"<style>"
	"body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
	"h1 { color: #333; }"
	"</style>"
	"</head>"
	"<body>"
	"<h1>Version Not Supported</h1>"
	"</body>"
	"</html>";

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

	std::set<ErrorPageRule>::const_iterator it = _conf->getErrorPage().begin();

	while (it != _conf->getErrorPage().end())
	{
		if ((*it).error == _resStatus)
		{
			if ((*it).newError != -1)
				_resStatus = (*it).newError;
			break;
		}

		it++;
	}
	std::map<int, std::string>::iterator itt = _statusTexts.find(_resStatus);
	if (itt != _statusTexts.end())
		_httpStatus = itt->second;
	else
	{
		std::ostringstream oss;
		oss << _resStatus;
		_httpStatus = oss.str();
	}
	if (it == _conf->getErrorPage().end())
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

/* static std::string cgiHeader(const std::string& status)
{
	std::ostringstream header;

	header << "HTTP/1.1 " << status << CRLF;
	header << "Server: WebServer/1.0" << CRLF;
	return (header.str());
} */
std::string HttpResponse::cgiHeader(const std::string &status)
{
	std::ostringstream header;
	header << "HTTP/1.1 " << status << CRLF;
	header << "Server: WebServer/1.0" << CRLF;
	return (header.str());
}

std::string HttpResponse::header(const std::string &status, int requestType)
{

	std::ostringstream header;
	std::string fileType;

	// std::cout << RED << "file: " << _fileName << RESET << std::endl;
	fileType = getMimeType(_fileName);
	if (requestType == REDIRECT)
	{
		_resBody = "<html>";
		_resBody += "<head><title>";
		_resBody += status;
		_resBody += "</title></head>";
		_resBody += "<body>";
		_resBody += "<center><h1>";
		_resBody += status;
		_resBody += "</h1></center>";
		_resBody += "<hr><center>WebServer/1.0</center>";
		_resBody += "</body>";
		_resBody += "</html>";
	}
	if (fileType == "text/html")
		checkCookies();
	header << "HTTP/1.1 " << status << CRLF;
	header << "Server: WebServer/1.0" << CRLF;
	header << "Date: " << get_http_date() << CRLF;
	header << "Content-Type: " << fileType << CRLF;
	header << "Content-Length: " << _resBody.size() << CRLF;
	if (requestType == REDIRECT)
		header << "Location: " << _block->getNewLocation() << CRLF;
	if (_req->session)
		header << "Set-Cookie: " << "session_id=" + _req->session->getSessionId() << "; Path=/" << CRLF;
	header << CRLF;
	return (header.str());
}

void HttpResponse::checkCookies()
{
	std::string theme = _req->session->getTheme();
	if (!_req->session)
		return;
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
		size_t headPos = _resBody.find("</head>");
		if (headPos != std::string::npos)
		{
			_resBody.insert(headPos, styleInjection);
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
		return (header(HTTP_404, ERROR) + _resBody);
	}
	else if (errorPage == 0)
		_resBody = page;
	else
		_resBody = httpFileContent(errorPage);
	if (_resStatus == 204 || _resStatus == 304)
		return (header(_httpStatus, OK));
	return (header(_httpStatus, ERROR) + _resBody);
}

const std::string HttpResponse::checkStatusCode()
{
	std::string fileContent;

	// esta função so valida caso o error_page esteja definido no .config file. tem de ser ajustada para caso não exista.
	switch (_resStatus)
	{
	case 200:
		if (_client->getProcessingState() == CGI_COMPLETED)
			return (cgiHeader("200 OK") + _response); // nao usa header pois a _response eh toda montada pelo cgi
		return (header("200 OK", OK) + _resBody);
	case 206:
		if (_method == DELETE || _method == POST)
			return (header("200 OK", OK));
		return (header("200 OK", OK) + _resBody);
	case 201:				// [NCC] pelo que entendi o 201 so sera gerado pelo POST, por isso podemos garantir que o CGI atuara sempre
		return (_response); // nao usa header pois a _response eh toda montada pelo cgi
	case 202:
		return ("HTTP/1.1 202 Accepted");
	case 204:
		return (header("204 No Content", OK));
	case 301:
		return (header("301 Moved Permanently", REDIRECT));
	case 302:
		return (header("302 Found", REDIRECT));
	case 303:
		return (header("303 See Other", REDIRECT));
	case 304:
		return (header("304 Not Modified", REDIRECT));
	case 307:
		return (header("307 Temporary Redirect", REDIRECT));
	case 308:
		return (header("308 Permanent Redirect", REDIRECT));
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

HttpResponse::HttpResponse(Client *client) : _resStatus(-1), _method(-1), _cgiRequest(false)
{
	_client = client;
	_conf = client->_request->_config;
	_req = client->_request;
	if (_req->hasParseError())
	{
		_resStatus = _req->getParseStatus();
		_loc = NULL;
		_block = _conf;
		setStatusTexts();
		setMimeTypes();
		setEnv(); // teste
		return;
	}
	_loc = checkLocationBlock();
	setStatusTexts();
	if (_loc != NULL)
		_block = _loc;
	else
		_block = _conf;
	setMimeTypes();
	setEnv(); // teste
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

void HttpResponse::handlePost()
{
	// O header content-type é necessário para o POST
	std::string contentType = parseContentType(_req->getHeaders());
	if (contentType.empty())
	{
		_resStatus = 415; // Unsupported Media Type
		_resBody = "Content-Type header is required for POST";
		return;
	}

	// Ver se upload dir existe no config.
	std::string uploadDir = _conf->getUploadDirectory();
	if (uploadDir.empty())
	{
		uploadDir = "/tmp/webserv_uploads";
		printLog("Upload directory not configured, using default: " + uploadDir, YELLOW, std::cout);
	}

	if (!createUploadDirectory(uploadDir) || access(uploadDir.c_str(), W_OK) != 0)
	{
		_resStatus = 500; // Internal Server Error
		_resBody = "Upload directory not accessible or not writable";
		return;
	}

	if (!_req->getUploadPath().empty())
	{
		// Upload grande - o arquivo já está no disco (temporário)
		// Arrumar o nome considerando content/type.
		std::string tempPath = _req->getUploadPath();
		std::string tempFilename = tempPath.substr(tempPath.find_last_of("/") + 1);
		std::string finalPath = uploadDir + "/" + tempFilename;
		if (rename(tempPath.c_str(), finalPath.c_str()) != 0)
		{
			_resStatus = 500;
			_resBody = "Failed to move uploaded file";
			return;
		}
		_resStatus = 201; // Created
		_resBody = "File uploaded successfully: " + tempFilename;
		return;
	}
	// O mesmo aqui para pequenos files, Arrumar o nome considerando content/type.
	time_t now = time(NULL);
	std::stringstream ss;
	ss << "upload_" << now << ".bin"; // Extensão genérica temporária
	std::string filename = ss.str();
	std::string fullPath = uploadDir + "/" + filename;
	// Salvar o body no arquivo
	std::string body = _req->getBody();
	if (body.empty())
	{
		_resStatus = 400; // Bad Request
		_resBody = "Empty body in POST request";
		return;
	}
	if (!saveBodyToFile(fullPath, body))
	{
		_resStatus = 500; // Internal Server Error
		_resBody = "Failed to save uploaded file";
		return;
	}
	// Sucesso - 201 Created
	_resStatus = 201;
	_resBody = "File uploaded successfully: " + filename;
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

bool HttpResponse::createUploadDirectory(const std::string& path) const
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
	{
		return S_ISDIR(st.st_mode);
	}
	std::string currentPath;
	std::stringstream pathStream(path);
	std::string segment;

	if (path[0] == '/')
	{
		currentPath = "/";
	}
	// Dividir o caminho por segmentos e criar cada diretório
	while (std::getline(pathStream, segment, '/'))
	{
		if (segment.empty())
			continue; // Ignorar segmentos vazios (como em /home//user)
		currentPath += segment;
		if (stat(currentPath.c_str(), &st) == 0)
		{
			if (!S_ISDIR(st.st_mode))
				return false; // Existe mas não é dir
		}
		else
		{
			if (mkdir(currentPath.c_str(), 0755) != 0)
				return false;
		}
		currentPath += "/";
	}

	return true;
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