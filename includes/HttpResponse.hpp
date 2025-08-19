#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "headers.hpp"

class Block;

class HttpResponse {
	private:
		std::string							_response;
		std::string							_resBody;
		std::string							_fileName;
		int									_resStatus;
		Configuration						*_conf;
		HttpRequest							*_req;
		LocationBlock						*_loc;
		Block								*_block;
		int									_method;
		std::map<std::string, std::string>	_mimeTypes;

	public:

		//SETTERS
		void	setResponse(const std::string& response);
		void	setMimeTypes();

		//GETTERS
		const std::string&	getResponse(void) const;
		const std::string&	getResHeader(void) const;
		const std::string&	getResBody(void) const;
		std::string			getFullPath(void);
		const std::string	getMimeType(const std::string& fileExtension);

		// EXEC METHOD
		void	execMethod();
		void	handleGET();
		void	handleDELETE();
		void	openReg(std::string path, int methodType);
		void	openDir(std::string path);
		const std::string checkStatusCode();
		std::string	header(const std::string& status);
		int		openFile();
		const std::string httpFileContent(int errorPage);
		LocationBlock*	checkLocationBlock();
		void	checkFile(int methodType);
		const std::string	setRedirectHeader(const std::string& str);
		const std::string	checkErrorResponse(const std::string& httpStatus, const std::string& page);

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		HttpResponse(HttpRequest *request, Configuration *config);
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse operator=(const HttpResponse& other);
};

#endif