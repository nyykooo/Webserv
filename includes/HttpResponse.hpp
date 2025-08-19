#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "headers.hpp"

class Block;

class HttpResponse {
	private:
		std::string		_response;
		std::string		_resHeader;
		std::string		_resBody;
		int				_resStatus;
		HttpRequest		*_req;
		LocationBlock	*_loc;
		Block			*_block;
		bool			_useNewLocation;
		int				_pipeIn;
		int				_pipeOut;
		int				_cgiPid;

		Configuration	*_conf;
		
	public:

		//SETTERS
		void	setResponse(const std::string& response);

		//GETTERS
		const std::string&		getResponse(void) const;
		const std::string&		getResHeader(void) const;
		const std::string&		getResBody(void) const;
		std::string				getFullPath(void);
		const Configuration&	getConfig(void) const;
		// EXEC METHOD
		void	execMethod();
		void	handleGET();
		void	handleDELETE();
		void	openReg(std::string path);
		void	openDir(std::string path);
		const std::string checkStatusCode();
		std::string	header(const std::string& status);
		int		openFile();
		const std::string httpFileContent(int errorPage);
		LocationBlock*	checkLocationBlock();
		void	checkFile(std::string fileName);

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		HttpResponse(HttpRequest *request, Configuration *config);
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse operator=(const HttpResponse& other);

		// CGI
		bool	lookForCgi();
		void	forkExecCgi(std::string interpreter);
};

#endif