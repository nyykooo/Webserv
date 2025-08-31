#include "../includes/headers.hpp"

Configuration::Configuration(): Block(), _requestSize(1000000) {
	_allowedMethods.push_back("GET");
	_allowedMethods.push_back("POST");
	//_allowedMethods.push_back("DELETE");
} // we need to put here the default files for all the possible errors

Configuration::~Configuration() {}

Configuration::Configuration(const Configuration& other): Block(other), _host(other._host), _serverName(other._serverName), _errorPage(other._errorPage),
	_requestSize(other._requestSize), locations(other.locations) {
	// _root(other._root), _defaultFiles(other._defaultFiles),_redirectStatusCode(other._redirectStatusCode),  _autoIndex(other._autoIndex),_newLocation(other._newLocation), _allowedMethods(other._allowedMethods),
	_root = other.getRoot();
	_defaultFiles = other.getDefaultFiles();
	_autoIndex = other.getAutoIndex();
	_redirectStatusCode = other.getRedirectStatusCode();
	_allowedMethods = other.getMethods();
}

Configuration&	Configuration::operator=(const Configuration& other) {
	if (this != &other) {
		_host = other._host;
		_serverName = other._serverName;
		_errorPage = other._errorPage;
		_root = other._root;
		_defaultFiles = other._defaultFiles;
		_autoIndex = other._autoIndex;
		_requestSize = other._requestSize;
		_redirectStatusCode = other._redirectStatusCode;
		_newLocation = other._newLocation;
		_allowedMethods = other._allowedMethods;
		locations = other.locations;
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

void	Configuration::setHost(const std::string& host, const std::string& port)
{
	_allHosts.insert(std::pair<std::string, std::string>(host, port));
	this->_host.insert(std::pair<std::string, std::string>(host, port));
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

void	Configuration::setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus) {

	ErrorPageRule rule;
	rule.error = errorPage;
	rule.errorPath = errorPagePath;
	rule.newError = newStatus;

	// std::cout << "error: " << errorPage << " ;errorPath: " << errorPagePath << " ;newStatus: " << newStatus << std::endl;
	this->_errorPage.insert(rule);
}

const std::set<ErrorPageRule>& Configuration::getErrorPage(void) const {
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

void	Configuration::setUploadDirectory(const std::string& str) {
	_uploadDirectory = str;
}

const std::string&	Configuration::getUploadDirectory() const {
	return (_uploadDirectory);
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

static long checkNewStatus(std::string word, const std::string& lastWord, Configuration& confserv) {
	long status = -1;
	long value;
	errno = 0;
	char	*endptr;

	status = -1;
	if (word[0] == '=') {
		word = word.substr(1, word.size());
		status = std::strtol(word.c_str(), &endptr, 10);
		// std::cout << status << std::endl;
		if (errno == ERANGE || *endptr || status < 0 || word.empty())
			throw Configuration::WrongConfigFileException("value \"" + word + "\" is invalid");
		return (status);
	}
	value = std::strtol(word.c_str(), &endptr, 10);
	if (errno == ERANGE || *endptr || value > 599 || value < 300)
		throw Configuration::WrongConfigFileException("value \"" + word + "\" must be between 300 and 599");
	confserv.setErrorPage(static_cast<int>(value), lastWord, status);
	return (status);
}

static void	parseErrorPage(std::string& line, Configuration& confserv) {
	std::stringstream 			ss(line);
	std::string					word;
	std::vector<std::string>	words;
	long						newStatusCode;

	checkCurlyBrackets(line);
	ss >> word;
	newStatusCode = -1;
	while (ss >> word)
		words.push_back(word);
	if (words.size() < 2)
		throw Configuration::WrongConfigFileException("no error page defined");
	if (words.size() == 2) {
		errno = 0;
		char	*endptr;
		long value = std::strtol(words[0].c_str(), &endptr, 10);
		if (errno == ERANGE || *endptr || value > 599 || value < 300)
			throw Configuration::WrongConfigFileException("value \"" + words[0] + "\" must be between 300 and 599");
		confserv.setErrorPage(static_cast<int>(value), words.back(), -1);
		return ;
	}
	int index = words.size() - 2;
	newStatusCode = checkNewStatus(words[index], words.back(), confserv);
	for (size_t i = 0; i < words.size() - 2; i++) {
		errno = 0;
		char	*endptr;
		long value = std::strtol(words[i].c_str(), &endptr, 10);
		if (errno == ERANGE || *endptr || value > 599 || value < 300)
			throw Configuration::WrongConfigFileException("value \"" + words[i] + "\" must be between 300 and 599");
		confserv.setErrorPage(static_cast<int>(value), words.back(), newStatusCode);
	}
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

void	parseDefaultFiles(const std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("no default file defined.");
	else
		confserv.setDefaultFiles(word);
	while (ss >> word)
		confserv.setDefaultFiles(word);
}


void	parseAutoIndex(std::string& line, Configuration& config) {
	std::stringstream	ss(line);
	std::string			word;	

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		config.setAutoIndex(word);
	else
		throw Configuration::WrongConfigFileException("no value in autoindex defined.");
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining autoindex.");
}

void	parseRedirect(std::string& line, Configuration& config) {
	std::stringstream	ss(line);
	std::string			word;
	long				value;
	char				*endptr;

	errno = 0;
	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("wrong syntax in redirect line.");
	value = std::strtol(word.c_str(), &endptr, 10);
	if (errno == ERANGE || *endptr || value > 599 || value < 100)
		throw Configuration::WrongConfigFileException("value \"" + word + "\" must be between 300 and 599");
	config.setRedirectStatusCode(value);
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("no location defined in redirect line.");
	config.setNewLocation(word);
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining redirect.");
}

void	parseAllowedMethods(std::string& line, Configuration& config) {
	std::stringstream			ss(line);
	std::string					word;
	std::vector<std::string>	methods;

	checkCurlyBrackets(line);
	ss >> word;
	while (ss >> word) {
		methods.push_back(word);
	}
	if (methods.empty())
		throw Configuration::WrongConfigFileException("wrong syntax in allowed_methods.");
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		if (*it != "GET" && *it != "POST" && *it != "DELETE")
			throw Configuration::WrongConfigFileException(*it + "Invalid method");
	}
	config.removeAllowedMethods();
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		config.setAllowedMethods(*it);
	}

/* 	for (std::vector<std::string>::const_iterator it = location.getMethods().begin(); it != location.getMethods().end(); it++) {
		std::cout << *it << std::endl;
	} */
}

void	parseUploadDirectory(std::string& line, Configuration& config) {
	std::stringstream			ss(line);
	std::string					word;
	std::vector<std::string>	methods;

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("no upload directory defined.");
	config.setUploadDirectory(word);
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
			parseDefaultFiles(line, confserv);
		else if (word == "autoindex")
			parseAutoIndex(line, confserv);
		else if (word == "allowed_methods")
			parseAllowedMethods(line, confserv);
		else if (word == "return")
			parseRedirect(line, confserv);
		else if (word == "upload_dir")
			parseUploadDirectory(line, confserv);
		else if (word == "location") {
			confserv.locations.push_back(LocationBlock(confserv));
			parseLocationBlock(file, line, confserv.locations.back());
		}
		else
			throw Configuration::WrongConfigFileException(word + ": invalid keyword in server block.");
/* 		for (std::set<std::pair<int, std::string> >::iterator it = confserv.getErrorPage().begin(); it != confserv.getErrorPage().end(); it++) {
			std::cout << GRAY<< it->first << " " << it->second << RESET << std::endl;
		} */
	}
	//std::cout << Configuration::getCurlyBracketsCount() << std::endl;
	if (confserv.getCurlyBracketsCount() > 0)
		throw Configuration::WrongConfigFileException("server block not closed.");
	if (confserv.getHost().empty())
		confserv.setHost("0.0.0.0", "8080");
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
/* 	std::set<std::pair<std::string, std::string> >::const_iterator it;
    for (it = confserv[0].getAllHosts().begin(); it != confserv[0].getAllHosts().end(); ++it) {
		std::cout << CYAN << it->first << it->second << RESET << std::endl;
	} */
/* 	std::set<std::pair<std::string, std::string> >::iterator it;
	for (it = Configuration::getAllHosts().begin(); it != Configuration::getAllHosts().end(); it++) {
		std::cout << GREEN << "ip: " << it->first << " porta: " << it->second << std::endl;
	} */
/* 	std::vector<LocationBlock> loc = confserv[0].locations;
	//std::cout << loc[0].getLocation() << std::endl;
	for (std::vector<LocationBlock>::iterator it = loc.begin(); it != loc.end(); it++)   {
		std::cout << "location: " << it->getLocation() << std::endl;
	} */
}
