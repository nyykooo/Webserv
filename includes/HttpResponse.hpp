#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP
	// CONTENT_LENGTH
	// CONTENT_TYPE
	// GATEWAY_INTERFACE
	// PATH_INFO
	// QUERY_STRING
	// REMOTE_ADDR
	// REQUEST_METHOD
	// SCRIPT_NAME
	// SERVER_NAME
	// SERVER_PORT
	// SERVER_PROTOCOL
	// SERVER_SOFTWARE
#include "headers.hpp"

class Block;
class Client;

class HttpResponse {
	private:
		int									_resStatus;
		std::string							_resHeader;
		bool								_useNewLocation;
		int									_pipeIn;
		int									_pipeOut;
		int									_cgiPid;
		std::string							_response;
		std::string							_resBody;
		std::string							_fileName;
		Configuration						*_conf; // pensar sobre manter ou colocar dentro do client
		HttpRequest							*_req; // pensar sobre manter ou colocar dentro do client
		LocationBlock						*_loc; // pensar sobre manter ou colocar dentro do client
		Client 								*_client; // se passar na hora de construir o HttpResponse podemos manipular o _status diretamente sem precisar retornar nada especifico
		Block								*_block;
		int									_method;
		std::map<std::string, std::string>	_mimeTypes;
		std::string							_httpStatus;
		std::map<int, std::string>			_statusTexts;
		std::string							_clientSession;
		std::string							_contentLength;
		std::string							_contentType;
		std::string							_gatewayInterface;
		std::string							_pathInfo;
		std::string							_queryString;
		std::string							_remoteAddress;
		std::string							_requestMethod;
		std::string							_scriptName;
		std::string							_serverName;
		std::string							_serverPort;
		std::string							_serverProtocol;
		std::string							_serverSoftware;
		
	public:

		//SETTERS
		void	setResponse(const std::string& response);
		void	setMimeTypes();
		void	setStatusTexts();

		//GETTERS
		const std::string&		getResponse(void) const;
		const std::string&		getResHeader(void) const;
		const std::string&		getResBody(void) const;
		std::string				getFullPath(void);
		const std::string		getMimeType(const std::string& fileExtension);
		const Configuration&	getConfig(void) const;

		// EXEC METHOD
		void	startResponse();
		void	execMethod();
		void	handleGET();
		void	handleDELETE();
		void	openReg(std::string path, int methodType);
		void	openDir(std::string path);
		const std::string checkStatusCode();
		std::string	header(const std::string& status, int requestType);
		int		openFile();
		const std::string httpFileContent(int errorPage);
		LocationBlock*	checkLocationBlock();
		void	checkFile(int methodType);
		const std::string	checkErrorResponse(const std::string& page);
		void	checkCookies();
		void	setEnv();

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		// HttpResponse(HttpRequest *request, Configuration *config);
		HttpResponse(Client *client);
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse operator=(const HttpResponse& other);

		// CGI
		bool	lookForCgi();
		void	forkExecCgi(std::string interpreter);
};

#endif