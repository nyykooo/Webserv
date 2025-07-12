#include "../includes/headers.hpp"

LocationBlock::LocationBlock(): _exactMatchModifier(false) {
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

const char* LocationBlock::WrongLocationBlock::what() const throw() {
	return (_message.c_str());
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

/* void	LocationBlock::setRedirectStatusCode(const std::string& statusCode) {
	errno = 0;
	char* endptr = NULL;

	_redirectStatusCode = std::strtol(statusCode.c_str(), &endptr, 10);
	if (endptr)
		throw WrongLocationBlock("invalid status code.");
} */

int		LocationBlock::getStatusCode(void) const {
	return (_redirectStatusCode);
}

void	LocationBlock::setNewLocation(const std::string& newLocation) {
	_newLocation = newLocation;
}

const std::string& LocationBlock::getNewLocation(void) const {
	return (this->_newLocation);
}

void	checkLocationCurlyBrackets(std::string& line) {
	line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
	if (line.find('{') != std::string::npos)
		throw LocationBlock::WrongLocationBlock("line shouldn't have \'{\'");
	if (line.find('}') != std::string::npos) {
		if (line[line.size() - 1] != '}' || line.find('}') != line.rfind('}'))	
			throw LocationBlock::WrongLocationBlock("} should be at the end of the line.");
		line.erase(line.size() - 1);
		LocationBlock::decrementLocationCurlyBracketsCount();
	}
}

void	parseRoot(std::string& line, LocationBlock& location) {
	std::stringstream ss(line);
	std::string word;

	checkLocationCurlyBrackets(line);
	ss >> word;
	if (ss >> word)
		location.setRoot(word);
	else
		throw LocationBlock::WrongLocationBlock("no root defined.");
	if (ss >> word)
		throw LocationBlock::WrongLocationBlock("too many arguments when defining root.");
	//std::cout << GRAY << webserv.getRoot() << RESET << std::endl;
}

void	parseAllowedMethods(std::string& line, LocationBlock& location) {
	std::stringstream			ss(line);
	std::string					word;
	std::vector<std::string>	methods;

	checkLocationCurlyBrackets(line);
	ss >> word;
	while (ss >> word) {
		methods.push_back(word);
	}
	if (methods.empty())
		throw LocationBlock::WrongLocationBlock("wrong syntax in allowed_methods.");
	
	location.removeAllowedMethods();
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		location.setAllowedMethods(*it);
	}
/* 	for (std::vector<std::string>::const_iterator it = location.getMethods().begin(); it != location.getMethods().end(); it++) {
		std::cout << *it << std::endl;
	} */
}

/* void	parseRedirect(std::string& line, LocationBlock& location) {
	std::stringstream	ss(line);
	std::string			word;

	ss >> word;
	if (!(ss >> word))
		throw LocationBlock::WrongLocationBlock("wrong syntax in redirect block.");
	location.setRedirectStatusCode(word);
} */

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location) {
	std::stringstream	ss(line);
	std::string			word;

	line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
 	if (line.find('{') == std::string::npos || line.find('}') != std::string::npos || (line[line.size() - 1] != '{' 
		&& line.find('{') != line.rfind('{'))) // the last condition checks if there's only one {
			throw LocationBlock::WrongLocationBlock("invalid sintax in server line.");
	line.erase(line.size() - 1);
	ss >> word;
	if (ss >> word) {
		if (word == "=") {
			if (!(ss >> word) || word == "{") // check if there's a word after the =
				throw LocationBlock::WrongLocationBlock("no literal location defined.");
			location.setExactMatchModifier(true);
		}
		location.setLocation(word);
	}
	else	
		throw LocationBlock::WrongLocationBlock("no location defined.");
	LocationBlock::incrementLocationCurlyBracketsCount();
	while (std::getline(file, line)) {
		std::stringstream ss(line);

		ss >> word;
		if (word.at(0) == '#')
			continue ;
		if (LocationBlock::getLocationCurlyBracketsCount() == 0)
			break ;
		if (word == "}") {
			LocationBlock::decrementLocationCurlyBracketsCount();
			break ;
		}
		else if (word == "root")
			parseRoot(line, location);
		else if (word == "allowed_methods")
			parseAllowedMethods(line, location);
	/* 	else if (word == "return")
			parseRedirect(line, location); */
		std::cout << line << std::endl;
	}
}