#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "headers.hpp"

class WebServer;

class HttpResponse {
	private:
		std::string		_response;

	public:

		//SETTERS
		void	setResponse(const std::string& response);

		//GETTERS
		const std::string&	getResponse(void) const;

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		HttpResponse(const HttpRequest& request, const Configuration& config);
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse operator=(const HttpResponse& other);
};

#endif