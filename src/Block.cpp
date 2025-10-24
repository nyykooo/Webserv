/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Block.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 12:08:01 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/24 13:04:07 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ######### LIFE CYCLE #########

Block::Block(): _root(""), _autoIndex(false), _newLocation(""), _defaultFiles(), _redirectStatusCode(-1), _requestSize(1000000) {}

Block::Block(const Block& other)
{
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
}

Block::~Block() {}

Block &Block::operator=(const Block &other)
{
	if (this != &other)
	{
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
	}
	return *this;
}

// ######### SETTERS #########

void Block::setRoot(const std::string& root)
{
	_root = root;
}

void Block::setAutoIndex(const std::string& value)
{
	if (value == "on")
		_autoIndex = true;
	else if (value == "off")
		_autoIndex = false;
	else
		throw std::invalid_argument("Invalid value for autoindex: " + value);
}

void Block::setNewLocation(const std::string& newLocation)
{
	_newLocation = newLocation;
}

void Block::setDefaultFiles(const std::string& index)
{
	std::istringstream iss(index);
	std::string file;
	while (std::getline(iss, file, ' '))
	{
		if (!file.empty())
			_defaultFiles.push_back(file);
	}
}

void Block::setAllowedMethods(const std::string& method)
{
	std::istringstream iss(method);
	std::string m;
	while (std::getline(iss, m, ' '))
	{
		if (!m.empty())
			_allowedMethods.push_back(m);
	}
}

void	Block::setUploadDirectory(const std::string& str) {
	_uploadDirectory = str;
}

const std::string&	Block::getUploadDirectory() const {
	return (_uploadDirectory);
}

void	Block::setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus) {

	ErrorPageRule rule;
	rule.error = errorPage;
	rule.errorPath = errorPagePath;
	rule.newError = newStatus;
	this->_errorPage.insert(rule);
}

void	Block::setRedirectStatusCode(int statusCode) {
	_redirectStatusCode = statusCode;
}

void Block::setCgiMap(const std::string& extension, const std::string& path) {
	_cgiMap[extension] = path;
}

void	Block::setRequestSize(long reqSize) {
	this->_requestSize = reqSize;
}

// ######### GETTERS #########

const std::string &Block::getRoot(void) const
{
	return _root;
}

bool Block::getAutoIndex(void) const
{
	return _autoIndex;
}

const std::string &Block::getNewLocation(void) const
{
	return _newLocation;
}

const std::vector<std::string>& Block::getDefaultFiles(void) const
{
	return _defaultFiles;
}

const std::vector<std::string>& Block::getMethods(void) const
{
	return _allowedMethods;
}

int Block::getRedirectStatusCode(void) const {
	return (_redirectStatusCode);
}

const std::map<std::string, std::string>& Block::getCgiMap(void) const {
	return (_cgiMap);
}

const std::set<ErrorPageRule>& Block::getErrorPage(void) const {
	return (this->_errorPage);
}

long	Block::getRequestSize(void) const {
	return (this->_requestSize);
}

// ######### REMOVE #########

void Block::removeAllowedMethods(void)
{
	_allowedMethods.clear();
}