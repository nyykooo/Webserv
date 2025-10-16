/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageRule.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 12:14:57 by ncampbel          #+#    #+#             */
/*   Updated: 2025/08/02 12:19:30 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

ErrorPageRule::ErrorPageRule() : error(0), errorPath(""), newError(-1) {}

ErrorPageRule::ErrorPageRule(int errorCode, const std::string& path, int newError)
	: error(errorCode), errorPath(path), newError(newError) {}

ErrorPageRule::~ErrorPageRule() {}

ErrorPageRule::ErrorPageRule(const ErrorPageRule& other)
	: error(other.error), errorPath(other.errorPath), newError(other.newError) {}

ErrorPageRule& ErrorPageRule::operator=(const ErrorPageRule& other) {
	if (this != &other) {
		error = other.error;
		errorPath = other.errorPath;
		newError = other.newError;
	}
	return *this;
}

bool ErrorPageRule::operator<(const ErrorPageRule& other) const {
	if (error != other.error)
		return error < other.error;
	if (errorPath != other.errorPath)
		return errorPath < other.errorPath;
	return newError < other.newError;
}