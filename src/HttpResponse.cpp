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

/* const std::string& httpFileContent(int errorPage) {
	
	const std::string content = "";
	errorPage++;
	return (content);
} */

static int openFile(const Configuration& config, int statusCode) {

	std::set<std::pair<int, std::string> >::iterator it = config.getErrorPage().begin();

	for (; it != config.getErrorPage().end(); ++it) {
		if (it->first == statusCode)
			break ;
	}
	if (it == config.getErrorPage().end())
		return (-1);
	int	configFile = open(it->second.c_str(), O_RDONLY);
	if (configFile < 0)
		return (-1);
	return (configFile);
}

static const std::string& checkStatusCode(const Configuration& config, int statusCode) {
	int	errorPage;

	switch (statusCode) {
		case 413:
			errorPage = openFile(config, statusCode);
			if (errorPage < 0)
				return (http_error_413_page);
			//return (httpFileContent(errorPage));
			break ;
		default:
			return http_error_404_page;
	}
	return http_error_404_page;
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
	setResponse(header.str() + http_error_404_page);
}


void HttpResponse::setResponse(const std::string& response) {
	_response = response;
}

const std::string&	HttpResponse::getResponse(void) const {
	return (_response);
}


