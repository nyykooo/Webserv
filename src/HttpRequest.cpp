/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 14:40:07 by brunhenr          #+#    #+#             */
/*   Updated: 2025/06/20 18:26:18 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/headers.hpp"

HttpRequest::HttpRequest() : method(""), path(""), version(""), headers(), body("")
{}

static bool ft_isspace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}


void HttpRequest::parse_headers(std::istringstream &stream)
{
	std::string header_line;
	
	while (std::getline(stream, header_line) && !header_line.empty())
	{
		// remove os /r do final de cada linha
		if (header_line[header_line.size() - 1] == '\r')
			header_line.erase(header_line.size() - 1);

		if (ft_isspace(header_line[0]))
		{
			throw std::invalid_argument("Sintax error. Line folding is forbidden in HTTP headers.");
		}	
		size_t colon_pos = header_line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string header_name = header_line.substr(0, colon_pos);
			if (header_name.length() > 1024)
			{
				throw std::invalid_argument("Header name too long: " + header_name);
			}
			std::string header_value = header_line.substr(colon_pos + 1);
			if (header_value.length() > 8192)
			{
				throw std::invalid_argument("Header value too long: " + header_value);
			}
			
			header_name.erase(header_name.find_last_not_of(" \t") + 1);
			header_value.erase(0, header_value.find_first_not_of(" \t"));
			//verificar a questão das listas nos headers e da consolidação de valores em listas
			
			this->headers[header_name] = header_value;
			if (headers.size() > 64) // 64 é o tamanho máximo de um header HTTP
			{
				throw std::invalid_argument("Too many headers");
			}
		}
	}
}


void HttpRequest::parse_requestline(const std::string& request_line)
{
	std::istringstream stream(request_line);
	std::string method, path, version, further;

	stream >> method >> path >> version >> further;
	if (method.empty() || path.empty() || version.empty() || !further.empty())
	{
		throw std::invalid_argument("Invalid HTTP request line");
	}
	else if (method != "GET" && method != "POST" && method != "DELETE") //Avaliar a inclusão do PUT?
	{
		throw std::invalid_argument("Unsupported or non-existent HTTP method: " + method);
	}
	else if (version != "HTTP/1.1" && version != "HTTP/1.0")// se a version for mais moderna, rodar com o HTTP/1.1? E HTTP/1.0 é possível?
	{
		throw std::invalid_argument("Unsupported or non-existent HTTP version: " + version);
	}
	this->method = method;
	this->path = path;
	this->version = version;
}

void HttpRequest::parseBody(std::istringstream &stream)
{
	std::string body_str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	body = body_str;
	if (body.length() > 131072)
		throw std::invalid_argument("Body too long");
}

void HttpRequest::parse(const std::string &request_text)
{
	std::istringstream	stream(request_text);
	std::string			request_line;

	if (std::getline(stream, request_line) && !request_line.empty())
	{
		parse_requestline(request_line);
		std::cout << "Request line parsed: " << method << " " << path << " " << version << std::endl;
	}
	
	parse_headers(stream);
	if (stream.peek() != EOF) // a peek retorna o próximo caractere sem removê-lo do stream
		parseBody(stream);
}

HttpRequest::HttpRequest(const std::string& request_text)
{
	parse(request_text);
}

HttpRequest::HttpRequest(const HttpRequest& other) : method(other.method), path(other.path), version(other.version),
	  headers(other.headers), body(other.body)
{}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		method = other.method;
		path = other.path;
		version = other.version;
		headers = other.headers;
		body = other.body;
	}
	return *this;
}

HttpRequest::~HttpRequest()
{}

// GETTERS
const std::string& HttpRequest::getMethod() const
{
	return (method);
}

const std::string& HttpRequest::getPath() const
{
	return (path);
}

const std::string& HttpRequest::getVersion() const
{
	return (version);
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
	return (headers);
}

const std::string& HttpRequest::getBody() const
{
	return (body);
}