#include "../includes/headers.hpp"

SessionData::SessionData(): _time(std::time(NULL)), _theme(""), _sessionId("") {}

SessionData::~SessionData() {}

void SessionData::setTime(std::time_t time)
{
	_time = time;
}

void	SessionData::setSessionId(const std::string& newSession) {
	_sessionId = newSession;
}

void	SessionData::setTheme(const std::string& newTheme) {
	_theme = newTheme;
}

std::time_t SessionData::getTime() const
{
	return _time;
}

const std::string&	SessionData::getSessionId() const {
	return (_sessionId);
}

const std::string& SessionData::getTheme() const {
	return (_theme);
}


bool SessionData::checkTimeout() const
{
	std::time_t curr_time = std::time(NULL);
	// Verifica se o tempo decorrido desde a última atividade é maior que o timeout
	if (curr_time - _time > SESSION_TIMEOUT)
		return true;
	else
		return false;
}
