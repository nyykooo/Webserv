#include "../includes/headers.hpp"

LocationBlock::LocationBlock(): _autoIndex(false), _exactMatchModifier(false) {
	_allowedMethods.push_back("GET");
	_allowedMethods.push_back("POST");
	_allowedMethods.push_back("DELETE");
}

LocationBlock::~LocationBlock() {}

LocationBlock::LocationBlock(const LocationBlock& other): _root(other._root), _allowedMethods(other._allowedMethods), 
													_autoIndex(other._autoIndex), _exactMatchModifier(other._exactMatchModifier),
													_location(other._location), 
													_cgiExtension(other._cgiExtension), _cgiPath(other._cgiPath) {}

LocationBlock& LocationBlock::operator=(const LocationBlock& other) {
	if (this != &other) {
		_root = other._root; 
		_allowedMethods = other._allowedMethods;
		_autoIndex = other._autoIndex;
		_exactMatchModifier = other._exactMatchModifier;
		_location = other._location;
		_cgiExtension = other._cgiExtension;
		_cgiPath = other._cgiPath;
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

void	LocationBlock::setRedirectStatusCode(const std::string& statusCode) {
	_redirectStatusCode = statusCode;
}

const std::string&		LocationBlock::getStatusCode(void) const {
	return (_redirectStatusCode);
}

void	LocationBlock::setNewLocation(const std::string& newLocation) {
	_newLocation = newLocation;
}

const std::string& LocationBlock::getNewLocation(void) const {
	return (this->_newLocation);
}

void	LocationBlock::setErrorPage(const std::string& errorPage, const std::string& errorPagePath) {
	this->_errorPage.insert(std::pair<std::string, std::string>(errorPage, errorPagePath));
}

const std::set<std::pair<std::string, std::string> >&	LocationBlock::getErrorPage(void) const {
	return (this->_errorPage);
}

void	LocationBlock::setDefaultFile(const std::string& index) {
	this->_defaultFile = index;
}

const std::string&	LocationBlock::getDefaultFile(void) const {
	return (this->_defaultFile);
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

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("wrong syntax in redirect line.");
	location.setRedirectStatusCode(word);
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

void	parseErrorPage(std::string& line, LocationBlock& location) {
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
		location.setErrorPage(words[i], words.back());
	/* 	for (std::set<std::pair<std::string, std::string> >::iterator it = getHost().begin(); it != this->getHost().end(); it++) {
		std::cout << GRAY<< it->first << it->second << std::endl;
	} */
}

void	parseDefaultFile(const std::string& line, LocationBlock& location) {
	std::stringstream ss(line);
	std::string word;

	checkCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		location.setDefaultFile(word);
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