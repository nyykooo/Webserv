#include "../includes/headers.hpp"

LocationBlock::LocationBlock() {}


LocationBlock::LocationBlock(const Configuration& other): _exactMatchModifier(false)  {
	_root = other.getRoot(); 
	_allowedMethods = other.getMethods();
	_autoIndex = other.getAutoIndex();
	_redirectStatusCode = other.getStatusCode();
	_defaultFiles = other.getDefaultFiles();
	_newLocation = other.getNewLocation();
	/* _cgiExtension = other.get;
	_cgiPath = other._cgiPath; */
	_errorPage = other.getErrorPage();
}

LocationBlock::~LocationBlock() {}

LocationBlock::LocationBlock(const LocationBlock& other): _root(other._root), _allowedMethods(other._allowedMethods), 
													_autoIndex(other._autoIndex), _exactMatchModifier(other._exactMatchModifier),
													_location(other._location), _redirectStatusCode(other._redirectStatusCode),
													_defaultFiles(other._defaultFiles), _newLocation(other._newLocation),
													_cgiExtension(other._cgiExtension), _cgiPath(other._cgiPath),
													_errorPage(other._errorPage) {}

LocationBlock& LocationBlock::operator=(const LocationBlock& other) {
	if (this != &other) {
		_root = other._root; 
		_allowedMethods = other._allowedMethods;
		_autoIndex = other._autoIndex;
		_exactMatchModifier = other._exactMatchModifier;
		_location = other._location;
		_redirectStatusCode = other._redirectStatusCode;
		_defaultFiles = other._defaultFiles;
		_newLocation = other._newLocation;
		_cgiExtension = other._cgiExtension;
		_cgiPath = other._cgiPath;
		_errorPage = other._errorPage;
	}
	return (*this);
}

int LocationBlock::_locationCurlyBracketsCount = 0;


void	LocationBlock::setExactMatchModifier(bool value) {
	_exactMatchModifier = value;
}

bool	LocationBlock::getExactMatchModifier(void) const {
	return (_exactMatchModifier);
}

void	LocationBlock::setLocation(const std::string& location) {
	_location = location;
}

const std::string&	LocationBlock::getLocation(void) const {
	return (_location);
}

int	LocationBlock::getLocationCurlyBracketsCount() {
	return (_locationCurlyBracketsCount);
}

void	LocationBlock::incrementLocationCurlyBracketsCount(void) {
	_locationCurlyBracketsCount++;
}

void	LocationBlock::decrementLocationCurlyBracketsCount(void) {
	_locationCurlyBracketsCount--;
}

const std::string&	LocationBlock::getRoot(void) const {
	return (this->_root);
}

void	LocationBlock::setRoot(const std::string& root) {
	this->_root = root;
}

void	LocationBlock::setAllowedMethods(const std::string& location) {
	_allowedMethods.push_back(location);
}

const std::vector<std::string>&	LocationBlock::getMethods(void) const {
	return (this->_allowedMethods);
}

void	LocationBlock::removeAllowedMethods(void) {
	_allowedMethods.clear();
}

bool	LocationBlock::getAutoIndex(void) const {
	return (this->_autoIndex);
}

void	LocationBlock::setAutoIndex(const std::string& value) {
	if (value == "on")
		this->_autoIndex = true;
	else if (value == "off")
		this->_autoIndex = false;
	else
		throw Configuration::WrongConfigFileException("Invalid autoindex value");
}

void	LocationBlock::setRedirectStatusCode(int statusCode) {
	_redirectStatusCode = statusCode;
}

int		LocationBlock::getStatusCode(void) const {
	return (_redirectStatusCode);
}

void	LocationBlock::setNewLocation(const std::string& newLocation) {
	_newLocation = newLocation;
}

const std::string& LocationBlock::getNewLocation(void) const {
	return (this->_newLocation);
}

void	LocationBlock::setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus) {

	ErrorPageRule rule;
	rule.error = errorPage;
	rule.errorPath = errorPagePath;
	rule.newError = newStatus;

	//std::cout << "error: " << errorPage << " ;errorPath: " << errorPagePath << " ;newStatus: " << newStatus << std::endl;
	this->_errorPage.insert(rule);
}

const std::set<ErrorPageRule>& LocationBlock::getErrorPage(void) const {
	return (this->_errorPage);
}

void	LocationBlock::setDefaultFiles(const std::string& index) {
	this->_defaultFiles.push_back(index);
}

const std::vector<std::string>&	LocationBlock::getDefaultFiles(void) const {
	return (this->_defaultFiles);
}

void	parseRoot(std::string& line, LocationBlock& location) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		location.setRoot(word);
	else
		throw Configuration::WrongConfigFileException("no root defined.");
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining root.");
	//std::cout << GRAY << webserv.getRoot() << RESET << std::endl;
}

void	parseAllowedMethods(std::string& line, LocationBlock& location) {
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
	location.removeAllowedMethods();
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		location.setAllowedMethods(*it);
	}

/* 	for (std::vector<std::string>::const_iterator it = location.getMethods().begin(); it != location.getMethods().end(); it++) {
		std::cout << *it << std::endl;
	} */
}

void	parseRedirect(std::string& line, LocationBlock& location) {
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
	location.setRedirectStatusCode(value);
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("no location defined in redirect line.");
	location.setNewLocation(word);
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining redirect.");
}

void	parseAutoIndex(std::string& line, LocationBlock& location) {
	std::stringstream	ss(line);
	std::string			word;

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		location.setAutoIndex(word);
	else
		throw Configuration::WrongConfigFileException("no value in autoindex defined.");
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining autoindex.");
}

static long checkNewStatus(std::string word, const std::string& lastWord, LocationBlock& location) {
	long status;
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
	location.setErrorPage(static_cast<int>(value), lastWord, status);
	return (status);
}

static void	parseErrorPage(std::string& line, LocationBlock& location) {
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
		location.setErrorPage(static_cast<int>(value), words.back(), -1);
		return ;
	}
	int index = words.size() - 2;
	newStatusCode = checkNewStatus(words[index], words.back(), location);
	for (size_t i = 0; i < words.size() - 2; i++) {
		errno = 0;
		char	*endptr;
		long value = std::strtol(words[i].c_str(), &endptr, 10);
		if (errno == ERANGE || *endptr || value > 599 || value < 300)
			throw Configuration::WrongConfigFileException("value \"" + words[i] + "\" must be between 300 and 599");
		location.setErrorPage(static_cast<int>(value), words.back(), newStatusCode);
	}
}

void	parseDefaultFile(const std::string& line, LocationBlock& location) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		location.setDefaultFiles(word);
	else
		throw Configuration::WrongConfigFileException("no index defined.");
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments when defining index.");	
}

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location) {
	std::stringstream	ss(line);
	std::string			word;

	line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
 	if (line.find('{') == std::string::npos || line.find('}') != std::string::npos || (line[line.size() - 1] != '{' 
		&& line.find('{') != line.rfind('{'))) // the last condition checks if there's at least one and only one {
			throw Configuration::WrongConfigFileException("invalid sintax in location line.");
	line.erase(line.size() - 1);
	ss >> word;
	if (ss >> word) {
		if (word == "=") {
			if (!(ss >> word) || word == "{") // check if there's a word after the =
				throw Configuration::WrongConfigFileException("no literal location defined.");
			location.setExactMatchModifier(true);
		}
		location.setLocation(word);
	}
	else	
		throw Configuration::WrongConfigFileException("no location defined.");
	LocationBlock::incrementLocationCurlyBracketsCount();
	while (std::getline(file, line)) {
		std::stringstream ss(line);

		ss >> word;
		if (word.at(0) == '#')
			continue ;
		if (word == "}" || LocationBlock::getLocationCurlyBracketsCount() == 0) {
			if (ss >> word)
				throw Configuration::WrongConfigFileException(word + " invalid keyword after \'}\'.");
			break ;
		}
		else if (word == "root")
			parseRoot(line, location);
		else if (word == "allowed_methods")
			parseAllowedMethods(line, location);
		else if (word == "autoindex")
			parseAutoIndex(line, location);
		else if (word == "return")
			parseRedirect(line, location);
		else if (word == "error_page")
			parseErrorPage(line, location);
		else if (word == "index")
			parseDefaultFile(line, location);
		else
			throw Configuration::WrongConfigFileException(word + ": invalid keyword in conf file.");
	}
}