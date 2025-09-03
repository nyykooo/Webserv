#include "headers.hpp"

class SessionData {
	private:
		std::string							_theme;
		std::string							_sessionId;

	public:
		SessionData() {};
		~SessionData() {};

		void	setSessionId(const std::string& newSession);
		void	setTheme(const std::string& newTheme);
		

		const std::string& 							getSessionId() const;
		const std::string& 							getTheme() const;

};