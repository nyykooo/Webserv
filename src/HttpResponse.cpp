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
		return ;
	}

	std::stringstream ss;

	ss << file.rdbuf();
	_resBody = ss.str();
	std::cout << "resBody: " << _resBody << std::endl;
	file.close();
}

void	HttpResponse::handleGET(const std::string path, const std::string root)
{
	std::string fileName = "./" + root + path;
	std::cout << "GET file: " << fileName << std::endl;

	struct stat st;
	if (stat(fileName.c_str(), &st) == -1)
	{
		std::cerr << "stat error:" << strerror(errno) << std::endl;
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
	_resStatus = 200;
}

void	HttpResponse::execMethod(Client *client)
{
	HttpRequest	*req = client->_request;
	std::string	method = req->getMethod();

	if (method == "GET")
		handleGET(req->getPath(), req->_config->getRoot());
	else
		_resStatus = 400;
	
    std::ostringstream header;
    header << "HTTP/1.1 " << _resStatus << " OK\r\n";
    header << "Content-Type: text/html\r\n";
    header << "Content-Length: " << _resBody.size() << "\r\n";
    header << "\r\n";
	_resHeader = header.str();
	_response = _resHeader + _resBody;
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

const std::string httpFileContent(int errorPage) {
	
	char buffer[BUFFER_SIZE];
	std::string result;

	ssize_t bytesRead;
	while ((bytesRead = read(errorPage, buffer, BUFFER_SIZE)) > 0) {
		result.append(buffer, bytesRead);
	}
	if (bytesRead < 0) {
        return http_error_404_page;
	}
	return (result);
}

static int openFile(const Configuration& config, int statusCode) {

	std::set<std::pair<int, std::string> >::const_iterator it = config.getErrorPage().begin();

	while (it != config.getErrorPage().end()) {
		std::cout << it->first << it->second << std::endl;
		if (it->first == statusCode)
			break ;
		it++;
	}
	if (it == config.getErrorPage().end())
		return (-1);
		
	int	configFile = open(it->second.c_str(), O_RDONLY);
	if (configFile < 0)
		return (-1);
	return (configFile);
}

static const std::string checkStatusCode(const Configuration& config, int statusCode) {
	int	errorPage;

	switch (statusCode) {
		case 413:
			errorPage = openFile(config, statusCode);
			if (errorPage < 0)
				return (http_error_404_page);
			return (httpFileContent(errorPage));
		default:
			break ;
	}
	return http_error_413_page;
}

HttpResponse::HttpResponse(const HttpRequest& request, const Configuration& config) {
	std::string test = request.getBody();
	long test2 = config.getRequestSize();
	test2++;
	std::string	pageContent;
	//std::cout << "aqui" << std::endl;

    std::ostringstream header;
    header << "HTTP/1.1 400 Bad Request" << CRLF;
    header << "Content-Type: text/html" << CRLF;
    header << "Content-Length: " << http_error_404_page.size() << CRLF;
    header << CRLF;

	pageContent = checkStatusCode(config, 413);
	setResponse(header.str() + pageContent);
}

// ### SETTERS ###

void HttpResponse::setResponse(const std::string& response) {
	_response = response;
}

const std::string&	HttpResponse::getResponse(void) const {
	return (_response);
}

