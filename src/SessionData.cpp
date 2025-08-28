#include "../includes/headers.hpp"

void	SessionData::setSessionId(const std::string& newSession) {
	_sessionId = newSession;
}

void	SessionData::setTheme(const std::string& newTheme) {
	_theme = newTheme;
}

const std::string&	SessionData::getSessionId() const {
	return (_sessionId);
}

const std::string& SessionData::getTheme() const {
	return (_theme);
}

