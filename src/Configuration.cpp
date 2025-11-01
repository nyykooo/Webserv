#include "../includes/headers.hpp"

// ######### STATIC MEMBERS #########

int Configuration::_curlyBracketsCount = 0;
std::set<std::pair<std::string, std::string> >	Configuration::_allHosts;

// ######### LIFE CYCLE #########

Configuration::Configuration(): Block() {
	_allowedMethods.push_back("GET");
	_allowedMethods.push_back("POST");
	_allowedMethods.push_back("DELETE");
}

Configuration::~Configuration() {
}

Configuration::Configuration(const Configuration& other): Block(other), _host(other._host), _serverName(other._serverName),
	_locations(other._locations) {
	_root = other._root;
	_autoIndex = other._autoIndex;
	_newLocation = other._newLocation;
	_defaultFiles = other._defaultFiles;
	_allowedMethods = other._allowedMethods;
	_redirectStatusCode = other._redirectStatusCode;
	_cgiMap = other._cgiMap;
	_uploadDirectory = other._uploadDirectory;
	//_errorPage = other._errorPage;
	_requestSize = other._requestSize;
	_location = other._location;
	_rootInsideLocation = other._rootInsideLocation;
}

Configuration&	Configuration::operator=(const Configuration& other) {
	if (this != &other) {
		_root = other._root;
		_autoIndex = other._autoIndex;
		_newLocation = other._newLocation;
		_defaultFiles = other._defaultFiles;
		_allowedMethods = other._allowedMethods;
		_redirectStatusCode = other._redirectStatusCode;
		_cgiMap = other._cgiMap;
		_uploadDirectory = other._uploadDirectory;
		_errorPage = other._errorPage;
		_requestSize = other._requestSize;
		_location = other._location;
		_rootInsideLocation = other._rootInsideLocation;
	}
	return (*this);
}

// ######### VALIDATORS #########

/*This function is used to check if there are {} between words and misplaced {} as well*/
void	checkCurlyBrackets(std::string line) {
	line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
	if (line.find('{') != std::string::npos)
		throw Configuration::WrongConfigFileException("line shouldn't have \'{\'"); // check if there's any { that's not in the end
	if (line.find('}') != std::string::npos) { // check if there are multiple } and if it's not in the end of the line
		if (line[line.size() - 1] != '}' || line.find('}') != line.rfind('}'))	
			throw Configuration::WrongConfigFileException("} should be at the end of the line.");
		line.erase(line.size() - 1);
		Configuration::decrementCurlyBracketsCount();
	}
}

/*This function checks if there's only 3 dots between each segment and if between each one it ranges from 0 to 255*/
bool	isValidIP(const std::string& host) {
	int					dotNum = 0;
	errno = 0;
	std::stringstream 	ss(host);
	std::string 		segment;

	while (std::getline(ss, segment, '.')) {
		dotNum++;
		if (dotNum > 4 || segment == "")
			throw Configuration::WrongConfigFileException("invalid host: " + host);
		for (std::string::iterator it = segment.begin(); it != segment.end(); it++) {
			if (!std::isdigit(*it))
				throw Configuration::WrongConfigFileException("invalid host: " + host);
		}
		if (std::strtol(segment.c_str(), NULL, 10) > 255 || errno == ERANGE) {
			throw Configuration::WrongConfigFileException("invalid host: " + host);
		}
	}
	if (!host.empty() && host[host.size() - 1] == '.')
		throw Configuration::WrongConfigFileException("invalid host: " + host);
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
	errno = 0;
	// check if it's bigger than 65535 which is the max number for the port
	if (std::strtol(word.c_str(), NULL, 10) > 65535 || errno == ERANGE) 
		throw Configuration::WrongConfigFileException("invalid port number: " + word);
	return (true);
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

// ######### PARSERS #########

void	parseServerName(std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	while (ss >> word) {
		confserv.setServerName(word);
	}
	if (confserv.getServerName().empty())
		throw Configuration::WrongConfigFileException("no server_name defined");
}

void	parseHost(std::string& line, Configuration& confserv) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
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
	}
	if (confserv.getHost().empty())
		throw Configuration::WrongConfigFileException("no host mentioned.");
}

/*This function checks multiple sizes because only the last argument is considered the error_page*/
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
		if (!std::isdigit(*tmpWord) || tmpWord == endptr || errno == ERANGE)
			throw Configuration::WrongConfigFileException("invalid body size number");
		size = std::strtol(tmpWord, &endptr, 10);
		temp = validateRequestSize(word, tmpWord, endptr);
		if ((*endptr && *(endptr + 1)) || temp > LONG_MAX)
			throw Configuration::WrongConfigFileException(word + " invalid body size number");
		size = static_cast<long>(temp);
		confserv.setRequestSize(size);
	}
	else
		throw Configuration::WrongConfigFileException("no request body size defined.");
	if (ss >> word) // check if there's anything after the first word
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
	if (ss >> word) // check if there's anything after the first word
		throw Configuration::WrongConfigFileException("too many arguments when defining root.");
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

/*If there any allowed_methods defined, this function first remove all the methods and then add the methods that were set in the .conf file*/
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
	config.removeAllowedMethods();
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		config.setAllowedMethods(*it);
	}
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
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments in upload_dir.");
}

void	parseCgi(const std::string& line, Configuration& config) {
	std::stringstream ss(line);
	std::string word, word2, word3;

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word) || !(ss >> word2))
		throw Configuration::WrongConfigFileException("no cgi_path defined.");
	if (ss >> word3)
		throw Configuration::WrongConfigFileException("too many arguments in CGI.");
	config.setCgiMap(word, word2);
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
		if (word.at(0) == '#')
			continue ;
		if (word == "}") {
			Configuration::decrementCurlyBracketsCount();
			break ; // end of a Server block
		}
		if (Configuration::getCurlyBracketsCount() <= 0)
			break ;
		if (word == "listen")
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
		else if (word == "cgi_allowed")
			parseCgi(line, confserv);
		else if (word == "upload_dir")
			parseUploadDirectory(line, confserv);
		else if (word == "location") {
			confserv._locations.push_back(LocationBlock(confserv));
			parseLocationBlock(file, line, confserv._locations.back());
		}
		else
			throw Configuration::WrongConfigFileException(word + ": invalid keyword in server block.");
	}
	if (Configuration::getCurlyBracketsCount() != 0)
		throw Configuration::WrongConfigFileException("block brackets \"{}\" are misplaced.");
	if (confserv.getHost().empty())
		confserv.setHost("0.0.0.0", "8080");
}

// ######### SETUP #########

void	setup(const char* file, std::vector<Configuration> &confserv) {
	std::string file1 = file;
	std::ifstream	configFile(file);
	std::string		line;
	if (!configFile.is_open()) {
		throw Configuration::WrongConfigFileException("Error opening file: " + file1 + " " + std::string(strerror(errno)));
		return ;
	}
	while (std::getline(configFile, line)) {
		if (isWhiteSpaceOnly(line))
			continue ;
		std::stringstream	ss(line);
		std::string			word;
		ss >> word;
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
}

// ######### GETTERS #########

int	Configuration::getCurlyBracketsCount(void) {
	return (_curlyBracketsCount);
}

const std::set<std::pair<std::string, std::string> >&	Configuration::getHost(void) const {
	return (this->_host);
}

std::set<std::pair<std::string, std::string> >&	Configuration::getAllHosts(void) {
	return (_allHosts);
}

const std::vector<std::string>&	Configuration::getServerName(void) const {
	return (this->_serverName);
}

// ######### SETTERS #########

void	Configuration::setHost(const std::string& host, const std::string& port)
{
	_allHosts.insert(std::pair<std::string, std::string>(host, port));
	this->_host.insert(std::pair<std::string, std::string>(host, port));
}

void	Configuration::setServerName(const std::string& serverName) {
	this->_serverName.push_back(serverName);
}

// ######### CURLY BRACKETS METHODS

void	Configuration::incrementCurlyBracketsCount(void) {
	_curlyBracketsCount++;
}

void	Configuration::decrementCurlyBracketsCount(void) {
	_curlyBracketsCount--;
}

// ######### EXCEPTIONS #########

const char* Configuration::WrongConfigFileException::what() const throw() {
	return (_message.c_str());
}
