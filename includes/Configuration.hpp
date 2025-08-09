#include "headers.hpp"

class LocationBlock;
class  ErrorPageRule;

// struct errorPageRule {
// 	int			error;
// 	std::string	errorPath;
// 	int			newError;
		
// 	// Comparador necessário para uso com std::set
// 	bool operator<(const errorPageRule& other) const {
// 		// Defina a regra de ordenação aqui
// 		// Exemplo: ordenar primeiro por código de erro, depois por caminho
// 		if (error != other.error)
// 			return error < other.error;
// 		if (errorPath != other.errorPath)
// 			return errorPath < other.errorPath;
// 		return newError < other.newError;
// 	}
// };


class Configuration {
	private:
		static std::set<std::pair<std::string, std::string> >	_allHosts;
		std::set<std::pair<std::string, std::string> >			_host;
		std::vector<std::string>								_serverName;
		std::set<ErrorPageRule>									_errorPage;
		std::string												_root;
		std::vector<std::string>								_defaultFiles; // diretiva index
		bool													_autoIndex;
		long													_requestSize;
		std::string												_redirectStatusCode;
		std::string												_newLocation;
		std::vector<std::string>								_allowedMethods;
		static int												_curlyBracketsCount;

	public:
		std::vector<LocationBlock>			locations;

		// SETTERS
		void								setHost(const std::string& host, const std::string& port);
		void								setServerName(const std::string& serverName);
		void								setAutoIndex(const std::string& value);
		void								setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus);
		void								setRoot(const std::string& root);
		void								setRequestSize(long reqSize);
		static void							incrementCurlyBracketsCount(void);
		static void							decrementCurlyBracketsCount(void);
		void								setDefaultFiles(const std::string& index);
		void								setRedirectStatusCode(const std::string& statusCode);
		void								setNewLocation(const std::string& newLocation);
		void								setAllowedMethods(const std::string& method);
		const std::vector<std::string>&		getMethods(void) const;
		void								removeAllowedMethods(void);

		// GETTERS
		static std::set<std::pair<std::string, std::string> >&	getAllHosts(void);
		const std::set<std::pair<std::string, std::string> >&	getHost(void) const;
		bool													getAutoIndex(void) const;
		const std::vector<std::string>&							getServerName(void) const;
		const std::set<ErrorPageRule>&							getErrorPage(void) const;
		const std::string&										getRoot(void) const;
		const std::vector<std::string>&							getDefaultFiles(void) const;
		long													getRequestSize(void) const;
		static int												getCurlyBracketsCount(void);
		const std::string&										getStatusCode(void) const;
		const std::string&										getNewLocation(void) const;

		// ORTHODOX CANONICAL FORM 
		
		Configuration();
		~Configuration();
		Configuration(const Configuration& other);
		Configuration& operator=(const Configuration& other);

		// PARSER

		class WrongConfigFileException: public std::exception {
			private:
				std::string	_message;
			public:
				WrongConfigFileException(const std::string& message): _message("Invalid configuration file: " + message) {}
				virtual ~WrongConfigFileException() throw() {};
				const char* what() const throw();
		};
};

void	setup(const char* file, std::vector<Configuration>& webserv);
void	checkCurlyBrackets(std::string line);