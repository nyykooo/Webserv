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
		std::size_t							_resContentLength;
		std::string							_resContentType;
		std::string							_fileName;
		Configuration						*_conf;
		HttpRequest							*_req;
		LocationBlock						*_loc;
		Client 								*_client;
		Block								*_block;
		int									_method;
		std::map<std::string, std::string>	_mimeTypes;
		std::string							_httpStatus;
		std::map<int, std::string>			_statusTexts;
		std::string							_fullPath;
		std::string							_scriptNameNico;
		std::string							_newRoot;

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
		std::string							_cgiPath;
		bool								_cgiRequest;
		char**								_envp;
		std::vector<std::string>			_tempEnv;

		std::vector<std::string>			_cgiParsedHeaders;
		std::string							_cgiHeaders;
		std::string							_cgiBody;
		std::vector<std::string>			_cgiCookies;
		std::string							_cgiContentType;
		std::string							_cgiLocation;
		std::string							_cgiContentLength;
		int									_cgiHeadersFound;
		int									_cgiStatusCode;

		int									_file;
		std::size_t							_filePos;
		bool								_rawUpload;

		std::string							_newUploadDir;
		std::string							_boundary;
		std::string							_relativePath;
		std::string							_fileAfterRelativePath;
		size_t								_bytesSent;

		bool								_headersSent;
		std::string							_resBuffer;

		public:
		
		//SETTERS
		void	setResponse(const std::string& response);
		void	setMimeTypes();
		void	setStatusTexts();
		void	setTempEnv(const std::string& str);
		void 	setResStatus(int status);
		void	setHttpStatus(int status);
		void	setCgiCookies(const std::string& cookie);
		void	setCgiContentType(std::string& type);
		void	setCgiLocation(const std::string& location);
		void	setFilePos(std::size_t pos);
		void	setNewUploadDir();
		void	setCgiInput(int i);
		void	setCgiOutput(int i);
		void	setBodySent(int n);
		void	setResponseCgi(char *buffer, int bytesRead);
		void	setFileStream(int i);
		void	setHasData(bool hasData);
		void	setHeadersSent(bool headersSent);

		//GETTERS
		const std::string&		getResponse(void) const;
		const std::string&		getResHeader(void) const;
		const std::string&		getResBody(void) const;
		std::string				getFullPath(void);
		const std::string		getMimeType(const std::string& fileExtension);
		const Configuration&	getConfig(void) const;
		int						getCgiPid(void) const;
		int						getFileStream(void) const;
		std::size_t				getFilePos(void) const;
		std::size_t				getContentLength(void) const;
		int						getStatusCode(void) const;
		int						getCgiInput(void) const;
		int						getCgiOutput(void) const;
		std::size_t				getBodySent(void) const;
		bool					getHasData() const;
		bool					getHeadersSent() const;
		
		// EXEC METHOD
		void				startResponse();
		void				execMethod();
		void				handlePOST();
		bool				saveBodyToFile(const std::string& path, const std::string& content) const;
		void				handleDELETE();
		void				openReg(std::string path, int methodType, off_t fileSize);
		void				openDir(std::string path);
		const std::string	checkStatusCode();
		std::string			cgiHeader();
		std::string			header(int requestType);
		int					openFile();
		const std::string	httpFileContent(int errorPage);
		LocationBlock*		checkLocationBlock();
		void				checkFile(int methodType);
		const std::string	checkErrorResponse(const std::string& page);
		void				checkCookies(std::string& body);
		void				setEnv();
		void				parsePath();
		void				parseScriptName(const std::string& str);
		const std::string	parseContentLength(const std::map<std::string, std::string>& headers);
		bool				checkValidCGI(const std::string& temp);
		void				buildEnv();
		void				parseCgiScript();
		void				parseCgiHeaders();
		void				parseCgiStatus(const std::string& segment);
		void				parseContentLength(const std::string& segment);
		void				buildFullPath();
		void				extractFileName(const std::string& str);
		void				cleanUploadDir();
		bool				isRequestUpload();
		int					processMultipartFormData();
		void				doRawUpload();

		//ORTHODOX CANONICAL FORM
		HttpResponse();
		// HttpResponse(HttpRequest *request, Configuration *config);
		HttpResponse(Client *client);
		~HttpResponse();

		// CGI
		void	forkExecCgi(std::string interpreter);
		void	checkCgiProcess();
		void 	terminateCgiProcess(void);
		void	streamingFile(off_t fileSize, std::string contentType);

		// file
		bool isFileOpen();
		std::string &getResponseBuffer();
};

#endif