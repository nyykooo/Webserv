/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Block.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 12:08:26 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/10 16:07:21 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCK_HPP
# define BLOCK_HPP

# include "headers.hpp"

class Block
{
	protected:
		std::string					_root; // block
		bool						_autoIndex; // block
		std::string					_newLocation; // block
		std::vector<std::string>	_defaultFiles; // block
		std::vector<std::string>	_allowedMethods; // block
		int							_redirectStatusCode; // block
		
	public:
		// ORTHODOX CANONICAL FORM
		Block();
		Block(const Block& other);
		~Block();
		Block &operator=(const Block &other);

		// SETTERS
		void setRoot(const std::string& root);
		void setAutoIndex(const std::string& value);
		void setNewLocation(const std::string& newLocation);
		void setDefaultFiles(const std::string& index);
		void setAllowedMethods(const std::string& method);
		void removeAllowedMethods(void);
		void setRedirectStatusCode(const int statusCode);
		
		// GETTERS
		const std::string& getRoot(void) const;
		bool getAutoIndex(void) const;
		const std::string& getNewLocation(void) const;
		const std::vector<std::string>& getDefaultFiles(void) const;
		const std::vector<std::string>& getMethods(void) const;
		const std::string& getStatusCode(void) const;
		int getRedirectStatusCode(void) const;
};


#endif