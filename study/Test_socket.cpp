#include <iostream>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

#include <arpa/inet.h>

int main()
{
	struct addrinfo *res = nullptr;

	getaddrinfo("localhost", "80", nullptr, &res); //8080 é a do meu http.server do python. 80 é a do meu NGINX server.

	if (res == nullptr) {
		std::cerr << "getaddrinfo failed" << std::endl;
		return 1;
	}
	
    // for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
	// 	std::cout << "Flags: " << p->ai_flags << std::endl;
    //     std::cout << "Address family: " << p->ai_family << std::endl;
    //     std::cout << "Socket type: " << p->ai_socktype << std::endl;
    //     std::cout << "Protocol: " << p->ai_protocol << std::endl;
    //     std::cout << "Address length: " << p->ai_addrlen << std::endl;
	// 	std::cout << "Canonical name: " << (p->ai_canonname ? p->ai_canonname : "N/A") << std::endl;
	// 	std::cout << std::endl;
    // }

	int socket_fd = socket(res->ai_family, 1, 6); // 1 é SOCK_STREAM, 6 é IPPROTO_TCP
	if (socket_fd < 0)
	{
		std::cerr << "Failed to create socket" << std::endl;
		freeaddrinfo(res);
		return 1;
	}

	struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN);
	std::cout << "IP: " << ip_str << std::endl;

	connect(socket_fd, res->ai_addr, res->ai_addrlen);

	const char *http_get = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
	ssize_t sent = send(socket_fd, http_get, strlen(http_get), 0);
	if (sent < 0)
	{
		std::cerr << "Failed to send HTTP request" << std::endl;
		close(socket_fd);
		freeaddrinfo(res);
		return 1;
	}

	char buffer[4096];
	ssize_t received;
	while ((received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		buffer[received] = '\0';
		std::cout << buffer;
	}

	freeaddrinfo(res);
	close(socket_fd);

	return 0;
}