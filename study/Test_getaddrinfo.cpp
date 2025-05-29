#include <iostream>
#include <netdb.h>

int main()
{
	struct addrinfo *res = nullptr;

	getaddrinfo("localhost", "80", nullptr, &res);

	if (res == nullptr) {
		std::cerr << "getaddrinfo failed" << std::endl;
		return 1;
	}
	
    for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
		std::cout << "Flags: " << p->ai_flags << std::endl;
        std::cout << "Address family: " << p->ai_family << std::endl;
        std::cout << "Socket type: " << p->ai_socktype << std::endl;
        std::cout << "Protocol: " << p->ai_protocol << std::endl;
        std::cout << "Address length: " << p->ai_addrlen << std::endl;
		std::cout << "Canonical name: " << (p->ai_canonname ? p->ai_canonname : "N/A") << std::endl;
		std::cout << std::endl;
    }

	freeaddrinfo(res);

	return 0;
}

//O meu retorno aqui é:
/*
Address family: 2  "esse 2 significa que é AF_INET, ou seja, IPv4"
Socket type: 1 "1 é SOCK_STREAM, ou seja, TCP."
Protocol: 6 "6 é IPPROTO_TCP, ou seja, TCP"
Address length: 16

Address family: 2 "esse 2 significa que é AF_INET, ou seja, IPv4"
Socket type: 2 "2 é SOCK_DGRAM, ou seja, UDP."
Protocol: 17 "17 é IPPROTO_UDP, ou seja, UDP"
Address length: 16 

Address family: 2 
Socket type: 3 "3 é SOCK_RAW, ou seja, RAW."
Protocol: 0 "0 é IPPROTO_RAW, ou seja, RAW"
Address length: 16 
*/

// isso não siginifica que o meu servidor Python (htttp.server) está aceitando conexões TCP, UDP e RAW.
// A função getaddrinfo mostra todoas as formas possíveis de acordo com os protocolos suportados pelo OS. Não significa que haverá um serviço escutando por UDP ou RAW nessa porta.
// Esse server só aceita conexões TCP, ou seja, o socket type 1 (SOCK_STREAM) e o protocolo 6 (IPPROTO_TCP).