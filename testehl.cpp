#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
    char hostname[256];
	std::string link;
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent* host = gethostbyname(hostname);
        if (host != NULL) {
            struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
			std::string addr(inet_ntoa(*addr_list[0]));
			link = "http://" + addr + ":8080";
        }
    }

    std::cout << "\033]8;;" << link << "\033\\"
              << link
              << "\033]8;;\033\\" << std::endl;
    return 0;
}
