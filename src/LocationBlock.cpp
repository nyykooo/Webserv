#include "../includes/headers.hpp"

// ######### STATIC MEMBERS #########

int LocationBlock::_locationCurlyBracketsCount = 0;

// ######### LIFE CYCLE #########

LocationBlock::LocationBlock() : Block() {}


LocationBlock::LocationBlock(const Configuration& other): _exactMatchModifier(false)  {
	_root = other.getRoot(); 
	_allowedMethods = other.getMethods();
	_autoIndex = other.getAutoIndex();
	_redirectStatusCode = other.getRedirectStatusCode();
	_defaultFiles = other.getDefaultFiles();
	_newLocation = other.getNewLocation();
	_cgiMap = other.getCgiMap();
}

LocationBlock::~LocationBlock() {}

LocationBlock::LocationBlock(const LocationBlock& other): Block(other), _exactMatchModifier(other._exactMatchModifier) {
	_root = other.getRoot();
	_allowedMethods = other.getMethods();
	_autoIndex = other.getAutoIndex();
	_redirectStatusCode = other.getRedirectStatusCode();
	_defaultFiles = other.getDefaultFiles();
	_cgiMap = other.getCgiMap();
	_newLocation = other.getNewLocation();
}

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
		_cgiMap = other._cgiMap;
	}
	return (*this);
}

// ######### CURLY BRACKETS #########

void	LocationBlock::incrementLocationCurlyBracketsCount(void) {
	_locationCurlyBracketsCount++;
}

void	LocationBlock::decrementLocationCurlyBracketsCount(void) {
	_locationCurlyBracketsCount--;
}

// ######### VALIDATORS #########

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

// ######### PARSERS #########

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
	location.setRootInsideLocation(true);
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
	location.removeAllowedMethods();
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		location.setAllowedMethods(*it);
	}
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

/*This function checks multiple sizes because only the last argument is considered the error_page*/
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
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("no default file defined.");
	else
		location.setDefaultFiles(word);
	while (ss >> word)
		location.setDefaultFiles(word);	
}

void	parseCgiPath(const std::string& line, LocationBlock& location) {
	std::stringstream ss(line);
	std::string word, word2;

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word) || !(ss >> word2))
		throw Configuration::WrongConfigFileException("no cgi_path defined.");
	location.setCgiMap(word, word2);
}

void	parseUploadDirectory(std::string& line, LocationBlock& location) {
	std::stringstream			ss(line);
	std::string					word;
	std::vector<std::string>	methods;

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word))
		throw Configuration::WrongConfigFileException("no upload directory defined.");
	location.setUploadDirectory(word);
	if (ss >> word)
		throw Configuration::WrongConfigFileException("too many arguments in upload_dir.");
}

void	parseCgi(const std::string& line, LocationBlock& location) {
	std::stringstream ss(line);
	std::string word, word2, word3;

	checkCurlyBrackets(line);
	ss >> word;
	if (!(ss >> word) || !(ss >> word2))
		throw Configuration::WrongConfigFileException("no cgi_path defined.");
	if (ss >> word3)
		throw Configuration::WrongConfigFileException("too many arguments in CGI.");
	location.setCgiMap(word, word2);
}

void	parseRequestSize(const std::string& line, LocationBlock& location) {
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
		location.setRequestSize(size);
	}
	else
		throw Configuration::WrongConfigFileException("no request body size defined.");
	if (ss >> word) // check if there's anything after the first word
		throw Configuration::WrongConfigFileException("invalid body size defined.");
	
}

void	parseLocationBlock(std::ifstream& file, std::string& line,  LocationBlock& location) {
	std::stringstream	ss(line);
	std::string			word;

	line.erase(line.find_last_not_of(" \t\r\n\f\v") + 1);
 	if (line.find('{') == std::string::npos || line.find('}') != std::string::npos || line[line.size() - 1] != '{' 
		|| line.find('{') != line.rfind('{')) // the last condition checks if there's at least one and only one {
			throw Configuration::WrongConfigFileException("invalid sintax in location line.");
	line.erase(line.size() - 1);
	ss >> word;
	if (ss >> word && word != "}" && word != "{") {
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
		else if (word == "cgi_path")
			parseCgiPath(line, location);
		else if (word == "cgi_allowed")
			parseCgi(line, location);
		else if (word == "upload_dir")
			parseUploadDirectory(line, location);
		else if (word == "client_max_body_size")
			parseRequestSize(line, location);
		else
			throw Configuration::WrongConfigFileException(word + ": invalid keyword in conf file.");
	}
}

// ######### GETTERS #########

bool	LocationBlock::getExactMatchModifier(void) const {
	return (_exactMatchModifier);
}

int	LocationBlock::getLocationCurlyBracketsCount() {
	return (_locationCurlyBracketsCount);
}

// ######### SETTERS #########

void	LocationBlock::setExactMatchModifier(bool value) {
	_exactMatchModifier = value;
}


