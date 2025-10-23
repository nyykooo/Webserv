/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: discallow <discallow@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 21:13:55 by ncampbel          #+#    #+#             */
/*   Updated: 2025/10/23 18:02:40 by discallow        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

// ### LOCATION HANDLERS ###

int handleLocation(Client *client, LocationBlock loc)
{
	std::vector<std::string> methods = loc.getMethods();
	HttpRequest *req = client->_request;
	std::string reqMethod = req->getMethod();

	std::vector<std::string>::iterator it;
	for (it = methods.begin(); it != methods.end(); ++it)
	{
		if ((*it).compare(reqMethod) == 0)
			return 1;
	}
	return 0;
}

int findLocation(Client *client)
{
	std::string path = client->_request->getPath();
	std::vector<LocationBlock> loc = client->_request->_config->_locations;

	std::vector<LocationBlock>::iterator it;
	for (it = loc.begin(); it != loc.end(); ++it)
	{
		if (path.compare((*it).getLocation()) == 0)
			return handleLocation(client, (*it));
	}
	return 200; // handleSBRoot
}

// ### TIMESTAMP LOGS ###
std::string setTimeStamp()
{
	std::time_t timestamp = std::time(&timestamp);
	std::tm *timeinfo = std::localtime(&timestamp);

	std::ostringstream oss;
	oss << (1900 + timeinfo->tm_year)
		<< "-" << std::setfill('0') << std::setw(2) << (1 + timeinfo->tm_mon)
		<< "-" << std::setfill('0') << std::setw(2) << (timeinfo->tm_mday)
		<< "T" << std::setfill('0') << std::setw(2) << (timeinfo->tm_hour)
		<< ":" << std::setfill('0') << std::setw(2) << (timeinfo->tm_min)
		<< ":" << std::setfill('0') << std::setw(2) << (timeinfo->tm_sec);

	return oss.str();
}

std::string get_http_date()
{
	char buf[100];
	std::time_t now = std::time(0);

	std::tm *gmt = std::gmtime(&now);
	if (gmt == NULL)
		return "";

    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    return std::string(buf);
}


void	printLogNew(std::stringstream &ss, const char	*color, std::ostream &stream, bool clean)
{
	std::string message = ss.str();
	std::string time = setTimeStamp();
	stream << "[" + time + "]: " << color << message << RESET << std::endl;
	if (clean)
	{
		ss.str("");
		ss.clear();
	}
}

void	printLog(std::string message, const char	*color, std::ostream &stream)
{
	std::string time = setTimeStamp();
	stream << "[" + time + "]: " << color << message << RESET << std::endl;
}

std::string toLower(const std::string &str)
{
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i)
	{
		result[i] = std::tolower(result[i]);
	}
	return result;
}

// ### CLEANING STRINGS ###

std::string removeSlashes(std::string path) {
	std::string newPath;
	size_t		index;
	
	newPath = path;
	index = 0;
	index = newPath.find_first_not_of('/');
	if (index != newPath.npos)
		newPath.erase(0, index);
	size_t end = newPath.find_last_not_of('/');
	if (end != std::string::npos)
		newPath.erase(end + 1);
	return (newPath);
}

// ### CONTENT TYPE ###
std::string getContentType(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos)
	{
		return "application/octet-stream";
	}

	std::string extension = filePath.substr(dotPos + 1);

	if (extension == "html" || extension == "htm")
		return "text/html";
	if (extension == "css")
		return "text/css";
	if (extension == "js")
		return "application/javascript";
	if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	if (extension == "png")
		return "image/png";
	if (extension == "gif")
		return "image/gif";
	if (extension == "pdf")
		return "application/pdf";
	if (extension == "mp4")
		return "video/mp4";
	if (extension == "mp3")
		return "audio/mpeg";
	if (extension == "txt")
		return "text/plain";

	return "application/octet-stream";
}

unsigned long long validateRequestSize(std::string word, const char *tmpWord, char *endptr)
{
	long				size = 0;
	unsigned long long temp;

	size = std::strtol(tmpWord, &endptr, 10);
	temp = size;
	if (!*endptr)
		return (temp);
	else if (*endptr == 'b' || *endptr == 'B')
		;
	else if (*endptr == 'k' || *endptr == 'K')
		temp = temp * 1024;
	else if (*endptr == 'm' || *endptr == 'M')
		temp = temp * 1024 * 1024;
	else if (*endptr == 'g' || *endptr == 'G')
		temp = temp * 1024 * 1024 * 1024;
	else
		throw Configuration::WrongConfigFileException(word + " invalid body size number");
	return (temp);
}
