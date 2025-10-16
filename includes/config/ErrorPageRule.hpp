#ifndef ERROR_PAGE_RULE_HPP
# define ERROR_PAGE_RULE_HPP

# include "headers.hpp"

class ErrorPageRule
{
	public:
		// DATA PROPERTIES
		int			error;
		std::string	errorPath;
		int			newError;

		// ORTHODOX CANONICAL FORM
		ErrorPageRule();
		~ErrorPageRule();
		ErrorPageRule(const ErrorPageRule& other);
		ErrorPageRule& operator=(const ErrorPageRule& other);

		// CONSTRUCTORS
		ErrorPageRule(int errorCode, const std::string& path, int newError);

		// OPERATOR OVERLOAD
		bool operator<(const ErrorPageRule& other) const;
};


#endif