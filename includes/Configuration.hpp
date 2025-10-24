#include "headers.hpp"

class LocationBlock;

class Configuration : public Block
{
	private:
		static std::set<std::pair<std::string, std::string> >	_allHosts;
		std::set<std::pair<std::string, std::string> >			_host;
		std::vector<std::string>								_serverName;
		static int												_curlyBracketsCount;
		

	public:
		std::vector<LocationBlock>			_locations;

		// SETTERS
		void								setHost(const std::string& host, const std::string& port);
		void								setServerName(const std::string& serverName);
		static void							incrementCurlyBracketsCount(void);
		static void							decrementCurlyBracketsCount(void);

		// GETTERS
		static std::set<std::pair<std::string, std::string> >&	getAllHosts(void);
		const std::set<std::pair<std::string, std::string> >&	getHost(void) const;
		const std::vector<std::string>&							getServerName(void) const;
		static int												getCurlyBracketsCount(void);

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