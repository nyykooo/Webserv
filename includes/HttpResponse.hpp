#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "headers.hpp"

class Client;

class HttpResponse {
	private:
		std::string		_response;
		std::string		_resHeader;
		std::string		_resBody;
		int				_resStatus;

	public:

		//SETTERS
		void	setResponse(const std::string& response);

		//GETTERS
		const std::string&	getResponse(void) const;
		const std::string&	getResHeader(void) const;
		const std::string&	getResBody(void) const;

		// EXEC METHOD
		void	execMethod(Client *client);
		void	handleGET(const std::string path, const std::string root);
		void	openFile(std::string path);

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		HttpResponse(const HttpRequest& request, const Configuration& config);
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse operator=(const HttpResponse& other);
};

#endif