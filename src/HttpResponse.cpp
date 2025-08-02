#include "../includes/headers.hpp"

HttpResponse::HttpResponse() {}

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

void HttpResponse::openFileNico(std::string path)
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

void	HttpResponse::handleGET(const std::string path, const std::string root)
{
	std::string fileName = "./" + root + path;
	std::cout << "GET file: " << fileName << std::endl;

	struct stat st;
	if (stat(fileName.c_str(), &st) == -1)
	{
		std::cerr << "stat error:" << strerror(errno) << std::endl;
		_resStatus = 404;
		return ;
	}

	if (S_ISREG(st.st_mode))
		openFileNico(fileName);
	else if (S_ISDIR(st.st_mode))
		std::cout << "Diretório\n";
	else if (S_ISLNK(st.st_mode))
		std::cout << "Link simbólico\n";
	else
		std::cout << "Outro tipo de arquivo\n";
}

void	HttpResponse::execMethod(const HttpRequest& req)
{
	std::string	method = req.getMethod();

	if (method == "GET")
		handleGET(req.getPath(), req._config->getRoot());
	else
		_resStatus = 400;
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

int HttpResponse::openFile(const Configuration& config) {

	std::set<ErrorPageRule>::const_iterator it = config.getErrorPage().begin();

	while (it != config.getErrorPage().end()) {
		//std::cout << it->first << it->second << std::endl;
		if ((*it).error == _resStatus)
			break ;
		it++;
	}
	if (it == config.getErrorPage().end())
		return (-1);
		
	int	configFile = open((*it).errorPath.c_str(), O_RDONLY);
	if (configFile < 0)
		return (-1);
	return (configFile);
}

std::string	HttpResponse::header(const std::string& status) {

    std::ostringstream header;
    header << "HTTP/1.1 " << status << CRLF;
    header << "Content-Type: text/html" CRLF;
    header << "Content-Length: " << _resBody.size() << CRLF;
    header << CRLF;
	_resHeader = header.str();
	_response = _resHeader + _resBody;

	return (header.str());
}

const std::string HttpResponse::checkStatusCode(const Configuration& config) {
	int	errorPage;
	std::string fileContent;
	
	// _resStatus = 413;
	switch (_resStatus) {
		case 413:
			errorPage = openFile(config);
			if (errorPage < 0) {
				_resBody = http_error_404_page;
				return (header(ERROR_404) + _resBody);
			}
			_resBody = httpFileContent(errorPage);
			return (header(ERROR_413) + _resBody);
		case 404:
			errorPage = openFile(config);
			if (errorPage < 0) {
				_resBody = http_error_404_page;
				return (header(ERROR_404) + _resBody);
			}
			_resBody = httpFileContent(errorPage);
			return (header(ERROR_404) + _resBody);
		case 200:
			return (header(HTTP_200) + _resBody);
		default:
			break ;
	}
	std::cout << "Status code not handled: " << _resStatus << std::endl;
	return _resBody;
}

HttpResponse::HttpResponse(const HttpRequest& request, const Configuration& config) {
	std::string test = request.getBody();
	long test2 = config.getRequestSize();
	test2++;
	std::string	pageContent;

	execMethod(request);
	pageContent = checkStatusCode(config);
	setResponse(pageContent);
}

// ### SETTERS ###

void HttpResponse::setResponse(const std::string& response) {
	_response = response;
}

const std::string&	HttpResponse::getResponse(void) const {
	return (_response);
}

