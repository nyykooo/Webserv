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
;

HttpResponse::HttpResponse(const HttpRequest& request, const Configuration& config) {
	std::string test = request.getBody();
	long test2 = config.getRequestSize();
	test2++;
	//std::cout << "aqui" << std::endl;
	setResponse(http_error_404_page);
}


void HttpResponse::setResponse(const std::string& response) {
	_response = response;
}

const std::string&	HttpResponse::getResponse(void) const {
	return (_response);
}


