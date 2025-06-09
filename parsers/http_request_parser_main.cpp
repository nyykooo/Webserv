#include "HttpRequest.hpp"

int	main(void)
{
	// Requisição advinda do buffer do nico, que é o responsável por receber as requisições HTTP do cliente.
	std::string request_text = "GET /index.html	HTTP/1.1\r\nHost: example.com\nUser-Agent:MeuClienteCPP\n\nAqui é o Body da requisição do Bruno, Nico e Diogo\n";

	try
	{
		HttpRequest request(request_text);
		std::cout << "Method: " << request.getMethod() << std::endl;
		std::cout << "Path: " << request.getPath() << std::endl;
		std::cout << "Version: " << request.getVersion() << std::endl;
		std::cout << "Headers:" << std::endl;
		std::map<std::string, std::string> headers = request.getHeaders();
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		{
			std::cout << "  " << it->first << ": " << it->second << std::endl;
		}
		std::cout << "Body: " << request.getBody() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error parsing HTTP request: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}	