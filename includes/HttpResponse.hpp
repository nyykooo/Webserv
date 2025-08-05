#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "headers.hpp"

class Client;
class HttpRequest;

class HttpResponse {
	private:
		std::string		_response;
		std::string		_resHeader;
		std::string		_resBody;
		int				_resStatus;
		Configuration	*_conf;
		HttpRequest		*_req;

	public:

		//SETTERS
		void	setResponse(const std::string& response);

		//GETTERS
		const std::string&	getResponse(void) const;
		const std::string&	getResHeader(void) const;
		const std::string&	getResBody(void) const;

		// EXEC METHOD
		void	execMethod();
		void	handleGET(const std::string path, const std::string root);
		void	openReg(std::string path);
		void	openDir(std::string path);
		const std::string checkStatusCode();
		std::string	header(const std::string& status);
		int		openFile();
		const std::string httpFileContent(int errorPage);

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		HttpResponse(HttpRequest *request, Configuration *config);
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse operator=(const HttpResponse& other);
};

#endif