#include "headers.hpp"

class SessionData {
	private:
		std::time_t				_time;
		std::string							_theme;
		std::string							_sessionId;

	public:
		SessionData();
		~SessionData();

		void	setTime(std::time_t time);
		void	setSessionId(const std::string& newSession);
		void	setTheme(const std::string& newTheme);
		

		std::time_t 		getTime() const;
		const std::string& 							getSessionId() const;
		const std::string& 							getTheme() const;

		bool	checkTimeout() const;
};