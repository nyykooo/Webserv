/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Block.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 12:08:26 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/25 16:33:01 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCK_HPP
# define BLOCK_HPP

# include "headers.hpp"

class  ErrorPageRule;

class Block
{
	protected:
		std::string							_root; // block
		bool								_autoIndex; // block
		std::string							_newLocation; // block
		std::vector<std::string>			_defaultFiles; // block
		std::vector<std::string>			_allowedMethods; // block
		int									_redirectStatusCode; // block
		std::map<std::string, std::string>	_cgiMap; // block
		std::string							_uploadDirectory;
		std::set<ErrorPageRule>				*_errorPage;
		long								_requestSize;
		std::string							_location;
		
	public:
		// ORTHODOX CANONICAL FORM
		Block();
		Block(const Block& other);
		~Block();
		Block &operator=(const Block &other);

		// SETTERS
		void	setRoot(const std::string& root);
		void	setAutoIndex(const std::string& value);
		void	setNewLocation(const std::string& newLocation);
		void	setDefaultFiles(const std::string& index);
		void	setAllowedMethods(const std::string& method);
		void	removeAllowedMethods(void);
		void	setRedirectStatusCode(const int statusCode);
		void	setCgiMap(const std::string& extension, const std::string& path);
		void	setUploadDirectory(const std::string& str);
		void	setErrorPage(int errorPage, const std::string& errorPagePath, int newStatus);
		void	setRequestSize(long reqSize);
		void	setLocation(const std::string& location);
		
		// GETTERS
		const std::string&							getRoot(void) const;
		bool										getAutoIndex(void) const;
		const std::string&							getNewLocation(void) const;
		const std::vector<std::string>&				getDefaultFiles(void) const;
		const std::vector<std::string>&				getMethods(void) const;
		int											getRedirectStatusCode(void) const;
		const std::map<std::string, std::string>&	getCgiMap(void) const;
		const std::string&							getUploadDirectory(void) const;
		const std::set<ErrorPageRule>*				getErrorPage(void) const;
		long										getRequestSize(void) const;
		const std::string&							getLocation(void) const;
};


#endif