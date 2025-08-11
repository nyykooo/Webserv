#include "../includes/headers.hpp"

HttpResponse::HttpResponse() : _resStatus(-1), _useNewLocation(false) {}

HttpResponse::HttpResponse(const HttpResponse& other) {
	*this = other;
}

HttpResponse HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		std::cout << std::endl;
	}
	return (*this);
}

HttpResponse::~HttpResponse() {}

// ### EXEC METHOD ###

void HttpResponse::openReg(std::string path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cerr << "ERRO AO ABRIR FICHEIRO: " << strerror(errno) << std::endl;
		_resStatus = 404;
		return ;
	}

	std::stringstream ss;

	ss << file.rdbuf();
	_resBody = ss.str();
	std::cout << "resBody: " << _resBody << std::endl;
	file.close();
	_resStatus = 200;
}

static std::string createDirIndex(std::string path)
{
	std::string dirIndex = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Directory Index</title></head><body><h1>Directory Index for " + path + "</h1><ul>";

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		std::cerr << "Failed to open directory: " << strerror(errno) << std::endl;
		return "<p>Error opening directory.</p>";
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') { // Skip hidden files
			dirIndex += "<li>" + std::string(entry->d_name) + "</li>";
		}
	}

	dirIndex += "</ul></body></html>";
	closedir(dir);
	return dirIndex;
}

void	HttpResponse::openDir(std::string path)
{
	for (std::vector<std::string>::const_iterator it = _block->getDefaultFiles().begin(); it != _block->getDefaultFiles().end(); ++it)
	{
		std::string filePath = path + "/" + *it;
		std::ifstream file(filePath.c_str());
		if (file.is_open())
		{
			std::stringstream ss;
			ss << file.rdbuf();
			_resBody = ss.str();
			file.close();
			_resStatus = 200;
			return ;
		}
	}

	switch (_block->getAutoIndex())
	{
		case false:
			_resStatus = 404; // mudar para 403
			return ;
		case true:
			_resBody = createDirIndex(path);
			_resStatus = 200;
			break;
	}
}

void	HttpResponse::handleGET(const std::string path, const std::string root)
{
	std::string locPath = path;
	if (_loc != NULL){
		size_t locIndex = path.find(_loc->getLocation());
		if (locIndex == 0)
		{
			locPath = path.substr(_loc->getLocation().size());
			if (locPath[0] == '/')
				locPath.erase(0, 1);
		}
	}

	std::string fileName = "./" + root + locPath;
	std::cout << "GET file: " << fileName << std::endl;

	struct stat st;
	if (stat(fileName.c_str(), &st) == -1)
	{
		std::cerr << "stat error:" << strerror(errno) << std::endl;
		_resStatus = 404;
		return ;
	}

	if (S_ISREG(st.st_mode))
		openReg(fileName);
	else if (S_ISDIR(st.st_mode))
	{
		std::cout << "Diretório encontrado: " << fileName << std::endl;
		openDir(fileName);
	}
	else if (S_ISLNK(st.st_mode))
		std::cout << "Link simbólico\n";
	else
		std::cout << "Outro tipo de arquivo\n";
}

LocationBlock* HttpResponse::checkLocationBlock() {
	std::vector<LocationBlock>::iterator it; 
	size_t			longestMatch;
	LocationBlock	*tempLocation = NULL;

	it = _conf->locations.begin();
	longestMatch = 0;
	while (it != _conf->locations.end())
	{
		const std::string&	locationPath = it->getLocation();
		const std::string&	requestPath = _req->getPath();
		if (requestPath.compare(0, locationPath.size(), locationPath) == 0) {
			if (locationPath.size() > longestMatch) {
				longestMatch = locationPath.size();
				tempLocation = &(*it);
			}
		}
		it++;
	}
	return (tempLocation);
}

void HttpResponse::handleDELETE() {
	std::vector<std::string>::const_iterator it;
	for (it = _loc->getMethods().begin(); it != _loc->getMethods().end(); it++) {
		if (*it == "DELETE")
			break;
	}
	if (it == _loc->getMethods().end()) {
		_resStatus = 405;
		return ;
	}

	if (!(_loc->getNewLocation().empty())) {

		_resStatus = _loc->getRedirectStatusCode();
		//std::cout << _resStatus << std::endl;
	}
}

void	HttpResponse::execMethod()
{
	bool methodFound = false;
	std::string root;
	std::string	method = _req->getMethod();
	std::vector<std::string>::const_iterator it;

	for (it = _block->getMethods().begin(); it != _block->getMethods().end(); ++it) {
		if (*it == method)
			methodFound = true;
	}
	if (methodFound == false)
	{
		_resStatus = 405; // Method Not Allowed
		return ;
	}
	root = _block->getRoot();

	if (_block->getRedirectStatusCode() != -1)
	{
		_resStatus = _block->getRedirectStatusCode();
		_useNewLocation = true;
		return;
	}

	if (method == "GET")
		handleGET(_req->getPath(), _conf->getRoot());
	else if (method == "DELETE")
		handleDELETE();
	else
		_resStatus = 405; // Not Found, as we only handle GET for now
}

static const std::string& http_error_404_page =
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
"<h1>Page not found.</h1>"
"</body>"
"</html>";

static const std::string& http_error_405_page =
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
"<h1>Method Not Allowed.</h1>"
"</body>"
"</html>";

static const std::string& http_error_413_page =
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
"<h1>Payload Too Large.</h1>"
"</body>"
"</html>";

const std::string HttpResponse::httpFileContent(int errorPage) {
	
	char buffer[BUFFER_SIZE];
	std::string result;
	ssize_t bytesRead;
	while ((bytesRead = read(errorPage, buffer, BUFFER_SIZE)) > 0) {
		result.append(buffer, bytesRead);
	}
	if (bytesRead < 0) {
		_resStatus = 404;
        return http_error_404_page;
	}
	return (result);
}

int HttpResponse::openFile() {

	std::set<ErrorPageRule>::const_iterator it = _conf->getErrorPage().begin();

	while (it != _conf->getErrorPage().end()) {
		if ((*it).error == _resStatus)
			break ;
		it++;
	}
	if (it == _conf->getErrorPage().end())
		return (-1);
		
	int	configFile = open((*it).errorPath.c_str(), O_RDONLY);
	if (configFile < 0)
		return (-1);
	return (configFile);
}

std::string	HttpResponse::header(const std::string& status) {

    std::ostringstream header;
    header << "HTTP/1.1 " << status << CRLF;
	if (_useNewLocation)
		header << "Location: " << _block->getNewLocation() << CRLF;
    header << "Content-Type: text/html" CRLF;
    header << "Content-Length: " << _resBody.size() << CRLF;
    header << CRLF;
	_resHeader = header.str();
	_response = _resHeader + _resBody;

	return (header.str());
}

const std::string HttpResponse::checkStatusCode() {
	int	errorPage;
	std::string fileContent;
	
	// _resStatus = 413;

	// esta função so valida caso o error_page esteja definido no .config file. tem de ser ajustada para caso não exista.
	switch (_resStatus) {
		case 413:
			errorPage = openFile();
			if (errorPage < 0) {
				_resBody = http_error_404_page;
				return (header(HTTP_404) + _resBody);
			}
			_resBody = httpFileContent(errorPage);
			return (header(HTTP_413) + _resBody);
		case 404:
			errorPage = openFile();
			if (errorPage < 0) {
				_resBody = http_error_404_page;
				return (header(HTTP_404) + _resBody);
			}
			_resBody = httpFileContent(errorPage);
			return (header(HTTP_404) + _resBody);
		case 405:
			errorPage = openFile();
			if (errorPage < 0) {
				_resBody = http_error_404_page;
				return (header(HTTP_404) + _resBody);
			}
			_resBody = httpFileContent(errorPage);
			return (header(HTTP_405) + _resBody);
		case 200:
			return (header(HTTP_200) + _resBody);
		case 301:
			return (header(HTTP_301) + _resBody);
		case 302:
			return (header(HTTP_302) + _resBody);
		default:
			break ;
	}
	std::cout << "Status code not handled: " << _resStatus << std::endl;
	return _resBody;
}

HttpResponse::HttpResponse(HttpRequest *request, Configuration *config) {
	_conf = config;
	_req = request;
	_loc = checkLocationBlock();
	if (_loc != NULL)
		_block = _loc;
	else
		_block = _conf;
	std::string	pageContent;



	execMethod();
	pageContent = checkStatusCode();
	setResponse(pageContent);
}

// ### SETTERS ###

void HttpResponse::setResponse(const std::string& response) {
	_response = response;
}

const std::string&	HttpResponse::getResponse(void) const {
	return (_response);
}

