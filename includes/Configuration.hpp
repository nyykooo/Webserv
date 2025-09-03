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


class Configuration : public Block
{
	private:
		static std::set<std::pair<std::string, std::string> >	_allHosts;
		std::set<std::pair<std::string, std::string> >			_host;
		std::vector<std::string>								_serverName;
		std::set<ErrorPageRule>									_errorPage;
		long													_requestSize;
		static int												_curlyBracketsCount;
		std::string												_uploadDirectory;

	public:
		std::vector<LocationBlock>			locations;

		// SETTERS
		void								setHost(const std::string& host, const std::string& port);
		void								setServerName(const std::string& serverName);
		void								setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus);
		void								setRequestSize(long reqSize);
		static void							incrementCurlyBracketsCount(void);
		static void							decrementCurlyBracketsCount(void);
		void								setUploadDirectory(const std::string& str);

		// GETTERS
		static std::set<std::pair<std::string, std::string> >&	getAllHosts(void);
		const std::set<std::pair<std::string, std::string> >&	getHost(void) const;
		const std::vector<std::string>&							getServerName(void) const;
		const std::set<ErrorPageRule>&							getErrorPage(void) const;
		long													getRequestSize(void) const;
		static int												getCurlyBracketsCount(void);
		const std::string&										getUploadDirectory(void) const;

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