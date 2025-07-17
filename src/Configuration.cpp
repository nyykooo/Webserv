#include "../includes/headers.hpp"

Configuration::Configuration(): _requestSize(0) {} // we need to put here the default files for all the possible errors

Configuration::~Configuration() {}

Configuration::Configuration(const Configuration& other): _host(other._host), _serverName(other._serverName), _errorPage(other._errorPage),
											_root(other._root), _requestSize(other._requestSize){
}

Configuration&	Configuration::operator=(const Configuration& other) {
	if (this != &other) {
		_host = other._host;
		_serverName = other._serverName;
		_errorPage = other._errorPage;
		_root = other._root;
		_requestSize = other._requestSize;
	}
	return (*this);
}

int Configuration::_curlyBracketsCount = 0;
std::set<std::pair<std::string, std::string> >	Configuration::_allHosts;

int	Configuration::getCurlyBracketsCount(void) {
	return (_curlyBracketsCount);
}

void	Configuration::incrementCurlyBracketsCount(void) {
	_curlyBracketsCount++;
}

void	Configuration::decrementCurlyBracketsCount(void) {
	_curlyBracketsCount--;
}

const std::set<std::pair<std::string, std::string> >&	Configuration::getHost(void) const {
	return (this->_host);
}

// void	Configuration::setHost(const std::string& host, const std::string& port) {
// 	//std::cout << GREEN << host << CYAN << port << std::endl;
// 	_allHosts.insert(std::pair<std::string, std::string>(host, port));
// 	this->_host.insert(std::pair<std::string, std::string>(host, port));
// }
void	Configuration::setHost(const std::string& host, const std::string& port)
{
	std::pair<std::string, std::string> hostPair(host, port);

	// Verificação dos duplicados
	if (_allHosts.find(hostPair) != _allHosts.end()) {
		throw WrongConfigFileException("Duplicate host: " + host + ":" + port);
	}

	_allHosts.insert(hostPair);
	this->_host.insert(hostPair);
}

std::set<std::pair<std::string, std::string> >&	Configuration::getAllHosts(void) {
	return (_allHosts);
}

const std::vector<std::string>&	Configuration::getServerName(void) const {
	return (this->_serverName);
}

void	Configuration::setServerName(const std::string& serverName) {
	this->_serverName.push_back(serverName);
}

void	Configuration::setErrorPage(const std::string& errorPage, const std::string& errorPagePath) {
	this->_errorPage[errorPage] = errorPagePath;
}

const std::map<std::string, std::string>& Configuration::getErrorPage(void) const {
	return (this->_errorPage);
}

const char* Configuration::WrongConfigFileException::what() const throw() {
	return (_message.c_str());
}

void	Configuration::setRequestSize(long reqSize) {
	this->_requestSize = reqSize;
}

long	Configuration::getRequestSize(void) const {
	return (this->_requestSize);
}

const std::string&	Configuration::getRoot(void) const {
	return (this->_root);
}

void	Configuration::setRoot(const std::string& root) {
	this->_root = root;
}

void	Configuration::setDefaultFile(const std::string& index) {
	this->_defaultFile = index;
}

const std::string&	Configuration::getDefaultFile(void) const {
	return (this->_defaultFile);
}

void	checkCurlyBrackets(std::string line) {
	line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
	if (line.find('{') != std::string::npos)
		throw Configuration::WrongConfigFileException("line shouldn't have \'{\'");
	if (line.find('}') != std::string::npos) {
		if (line[line.size() - 1] != '}' && line.find('}') != line.rfind('}'))	
			throw Configuration::WrongConfigFileException("} should be at the end of the line.");
		line.erase(line.size() - 1);
		Configuration::decrementCurlyBracketsCount();
	}
		
}

bool	isValidIP(const std::string& host) {
	int					dotNum = 0;
	std::stringstream 	ss(host);
	std::string 		segment;

	while (std::getline(ss, segment, '.')) {
		dotNum++;
		if (dotNum > 4)
			return (false);
		for (std::string::iterator it = segment.begin(); it != segment.end(); it++) {
			if (*it == '{' || *it == '}')
				continue ;
			if (!std::isdigit(*it))
				return (false);
		}
		if (segment.size() > 4 || std::strtol(segment.c_str(), NULL, 10) > 255) {
			throw Configuration::WrongConfigFileException("invalid host: " + host);
		}
	}
	return (dotNum == 4);
}

static bool isWhiteSpaceOnly(const std::string& line) {
	for (std::string::const_iterator it = line.begin(); it != line.end(); it++) {
		if (!std::isspace(*it))
			return (false);
	}
	return (true);
}

bool	isOnlyDigit(const std::string& word) {
	for (std::string::const_iterator it = word.begin(); it != word.end(); it++) {
		if (*it == '{' || *it == '}')
			continue ;
		else if (!std::isdigit(*it))
			return (false);	
	}
	return (true);
}

bool	isValidPort(const std::string& word) {
	if (!isOnlyDigit(word))
		return (false);
	// check if it's bigger than 65535 which is the max number for the port
	if (word.size() > 6 || std::strtol(word.c_str(), NULL, 10) > 65535) 
		throw Configuration::WrongConfigFileException("invalid port number: " + word);
	return (true);
}

void	parseServerName(std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	while (ss >> word) {
		confserv.setServerName(word);
	}
/* 	for (std::set<std::pair<std::string, std::string> >::iterator it = getHost().begin(); it != this->getHost().end(); it++) {
		std::cout << GRAY<< it->first << it->second << std::endl;
	} */
	if (confserv.getServerName().empty())
		throw Configuration::WrongConfigFileException("no server_name defined");
}

void	parseHost(std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	//std::cout << GRAY << line << RESET << std::endl;
	ss >> word;
	while (ss >> word) {
		size_t pos = word.find(':');
		if (pos == 0 || pos == word.length() - 1) { // if a : is in the string without a host and a port
			throw Configuration::WrongConfigFileException("invalid host/port " + word);
		}
		else if (pos != std::string::npos && isValidIP(word.substr(0, pos)) 
				&& isValidPort(word.substr(pos + 1, word.size() - pos)))
			confserv.setHost(word.substr(0, pos), word.substr(pos + 1, word.size() - pos));
		else {
			if (isValidPort(word))
				confserv.setHost("0.0.0.0", word); // 0.0.0.0 or NULL is the default host
			else if (isValidIP(word))
				confserv.setHost(word, "8080"); // 8080 is the default port
		}
		//std::cout << BLUE << word << RESET << std::endl;
	}
	if (confserv.getHost().empty())
		throw Configuration::WrongConfigFileException("no host mentioned.");
}

static void	parseErrorPage(std::string& line, Configuration& confserv) {
	std::stringstream 			ss(line);
	std::string					word;
	std::vector<std::string>	words;

	checkCurlyBrackets(line);
	ss >> word;
	while (ss >> word)
		words.push_back(word);
	if (words.size() < 2)
		throw Configuration::WrongConfigFileException("no error page defined");
	for (size_t i = 0; i < words.size() - 1; i++)
		confserv.setErrorPage(words[i], words.back());
	/* 	for (std::set<std::pair<std::string, std::string> >::iterator it = getHost().begin(); it != this->getHost().end(); it++) {
		std::cout << GRAY<< it->first << it->second << std::endl;
	} */
}

void	parseRequestSize(const std::string& line, Configuration& confserv) {
	std::string			word;
	const char*			tmpWord;
	std::stringstream	ss(line);
	unsigned long long	temp;
	long				size = 0;
	char				*endptr = NULL;
	
	checkCurlyBrackets(line);
	errno = 0;
	ss >> word;
	if (ss >> word) {
		tmpWord = word.c_str();
		size = std::strtol(tmpWord, &endptr, 10);
		if (!std::isdigit(*tmpWord) || tmpWord == endptr || errno == ERANGE)
			throw Configuration::WrongConfigFileException("invalid body size number");
		temp = size;
		if (*endptr == 'b' || *endptr == 'B')
			;
		else if (*endptr == 'k' || *endptr == 'K')
			temp = temp * 1024;
		else if (*endptr == 'm' || *endptr == 'M')
			temp = temp * 1024 * 1024;
		else if (*endptr == 'g' || *endptr == 'G')
			temp = temp * 1024 * 1024 * 1024;
		else
			throw Configuration::WrongConfigFileException(word + " invalid body size number");
		if ((*endptr && *(endptr + 1)) || temp > LONG_MAX)
			throw Configuration::WrongConfigFileException(word + " invalid body size number");
		size = static_cast<long>(temp);
		confserv.setRequestSize(size);
	}
	else
		throw Configuration::WrongConfigFileException("no request body size defined.");
	if (ss >> word) // if there's anything after the first word
		throw Configuration::WrongConfigFileException("invalid body size defined.");
	
}

void	parseRoot(const std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		confserv.setRoot(word);
	else
		throw Configuration::WrongConfigFileException("no root defined.");
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining root.");
	//std::cout << GRAY << confserv.getRoot() << RESET << std::endl;
}

void	parseDefaultFile(const std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		confserv.setDefaultFile(word);
	else
		throw Configuration::WrongConfigFileException("no index defined.");
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining index.");	
}

void parseServer(std::ifstream& file, Configuration& confserv) {
	std::string	line;

	Configuration::incrementCurlyBracketsCount();
	while (std::getline(file, line)) {
		if (isWhiteSpaceOnly(line))
			continue ;
		std::stringstream ss(line);
		std::string	word;
		ss >> word;
		// std::cout << YELLOW << word << RESET << std::endl;
		if (word.at(0) == '#')
			continue ;
		if (Configuration::getCurlyBracketsCount() == 0)
			break ;
		if (word == "}") {
			Configuration::decrementCurlyBracketsCount();
			break ; // end of a Server block
		}
		else if (word == "listen")
			parseHost(line, confserv);
		else if (word == "server_name")
			parseServerName(line, confserv);
		else if (word == "error_page")
			parseErrorPage(line, confserv);
		else if (word == "client_max_body_size")
			parseRequestSize(line, confserv);
		else if (word == "root")
			parseRoot(line, confserv);
		else if (word == "index")
			parseDefaultFile(line, confserv);
		else if (word == "location") {
			confserv.locations.push_back(LocationBlock());
			parseLocationBlock(file, line, confserv.locations.back());
			if (confserv.locations.back().getRoot().empty() && confserv.getRoot().empty())
				throw Configuration::WrongConfigFileException("root needs to be defined.");
		}
		else
			throw Configuration::WrongConfigFileException(word + ": invalid keyword in server block.");
	}
	//std::cout << Configuration::getCurlyBracketsCount() << std::endl;
	if (confserv.getCurlyBracketsCount() > 0 || confserv.getHost().empty())
		throw Configuration::WrongConfigFileException("server block incomplete / not closed.");
}

void	setup(const char* file, std::vector<Configuration>& confserv) {

	std::ifstream	configFile(file);
	std::string		line;
	if (!configFile.is_open()) {
		std::cerr << RED << "Error opening file: " << strerror(errno) << RESET << std::endl;
		return ;
	}
	while (std::getline(configFile, line)) {
		if (isWhiteSpaceOnly(line))
			continue ;
		std::stringstream	ss(line);
		std::string			word;
		ss >> word;
		// std::cout << YELLOW << line << RESET << std::endl;
		if (word.at(0) == '#')
			continue ;
		line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
		if (line.find('{') == std::string::npos || line.find('}') != std::string::npos || (line[line.size() - 1] != '{' 
			&& line.find('{') != line.rfind('{'))) // the last condition checks if there's only one {
			throw Configuration::WrongConfigFileException("invalid sintax.");
		line.erase(line.size() - 1);
		if (word == "server") {
			confserv.push_back(Configuration());
			parseServer(configFile, confserv.back());
		}
		else
			throw Configuration::WrongConfigFileException(word + ": invalid keyword in conf file.");
	}
/* 	std::set<std::pair<std::string, std::string> >::iterator it;
	for (it = Configuration::getAllHosts().begin(); it != Configuration::getAllHosts().end(); it++) {
		std::cout << GREEN << "ip: " << it->first << " porta: " << it->second << std::endl;
	} */
}
