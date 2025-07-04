/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 17:45:50 by ncampbel          #+#    #+#             */
/*   Updated: 2025/07/04 16:07:54 by ncampbel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/headers.hpp"

static void printHL(void)
{
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

    std::cout << "Machine IP URL: " << "\033]8;;" << link << "\033\\"
              << link
              << "\033]8;;\033\\" << std::endl;
    std::cout << "localhost URL:" << "\033]8;;http://127.0.0.1:8080\033\\"
              << "http://127.0.0.1:8080"
              << "\033]8;;\033\\" << std::endl;
    return;
}

// static bool tryConnection(WebServer &web, int i)
// {
//     std::map<std::string, HttpServer *>::iterator it;
//     std::map<std::string, HttpServer *> _servers_map = web.getServersMap();
//     for (it = _servers_map.begin(); it != _servers_map.end(); ++it)
//     {
//         if(web.getEvents()[i].data.fd == it->second->getServerSocket()->getSocketFd())
//         {
//             int new_client = it->second->handleNewClient();
//             if (new_client == -1)
//             {
//                 std::cerr << "Erro ao aceitar nova conexão" << std::endl;
//                 continue; // Continua para o próximo evento
//             }
//             std::cout << "✅ Novo cliente conectado - new_client_fd: " << new_client << " ✅" << std::endl;

//             // Adiciona o novo socket no vector e no epoll
//             struct epoll_event client_event;
//             client_event.data.fd = new_client;
//             client_event.events = EPOLLIN; // Habilita leitura e modo edge-triggered
//             if (epoll_ctl(web.getEpollFd(), EPOLL_CTL_ADD, new_client, &client_event) == -1) {
//                 std::cerr << "Erro ao adicionar o socket do cliente ao epoll" << std::endl;
//                 close(new_client);
//                 return false;
//             }
//             return true; // Conexão aceita com sucesso
//         }
//     }
//     return false;
// }

// static int receiveData(WebServer &web, int client_fd)
// {
//     ssize_t bytes = recv(client_fd, web.getBuffer(), BUFFER_SIZE - 1, 0);
//     // se bytes for -1 significa que houve um erro
//     if (bytes == -1)
//     {
//         std::cerr << "Erro ao receber dados do cliente: " << strerror(errno) << std::endl;
//         return -1;
//     }
//     // se bytes for 0 significa que houve desconexao
//     else if (bytes == 0)
//     {
//         return 0;
//     }
//     else
//     {
//         // lidar com a leitura e envio de resposta
//         std::string requestBuffer(web.getBuffer());
//         std::cout << "Recebido: " << requestBuffer << std::endl;

//         try
//         {
//             HttpRequest request(requestBuffer);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error parsing HTTP request: " << e.what() << std::endl;
//             return 1; 
//         }
//     }
//     return 1;
// }

// static void sendData(WebServer &web, int client_fd)
// {			
//     // resposta padrao
//     std::string responseBody = 
//     "<!DOCTYPE html>"
//     "<html lang=\"en\">"
//     "<head>"
//     "<meta charset=\"UTF-8\">"
//     "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
//     "<title>Bem-vindo</title>"
//     "<style>"
//     "body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 50px; }"
//     "h1 { color: #333; }"
//     "</style>"
//     "</head>"
//     "<body>"
//     "<h1>Bem-vindo ao WebServr de ncampbel, bruhenr dioalexa</h1>"
//     "</body>"
//     "</html>";
    
//     std::ostringstream header;
//     header << "HTTP/1.1 200 OK\r\n";
//     header << "Content-Type: text/html\r\n";
//     header << "Content-Length: " << responseBody.size() << "\r\n";
//     header << "\r\n";

//     std::string response = header.str() + responseBody;

//     // envia a resposta ao cliente
//     int sent = send(client_fd, response.c_str(), response.size(), 0);
//     if (sent == -1)
//     {
//         std::cerr << "Erro ao enviar dados para o cliente: " << strerror(errno) << std::endl;
//         return; // Retorna se houver erro ao enviar
//     }
//     else
//         std::cout << "✅ Dados enviados para o cliente - client_fd: " << client_fd << " ✅" << std::endl;
    
//     static_cast<void>(web); // Para evitar warning de variável não utilizada
// }

// static void treatExistingClient(WebServer &web, int i)
// {
//     if (web.getEvents()[i].events == EPOLLIN)
//     {
//         int data = receiveData(web, web.getEvents()[i].data.fd);
//         if (data == -1)
//         {
//             // criar metodo para desconectar o cliente
//             std::cout << "❌ Cliente desconectado - _client_fd: " << web.getEvents()[i].data.fd << " ❌" << std::endl;
//             close(web.getEvents()[i].data.fd);
//             // Remove o cliente do vetor e do epoll
//             epoll_ctl(web.getEpollFd(), EPOLL_CTL_DEL, web.getEvents()[i].data.fd, NULL);
//             return;
//         }
//         web.getEvents()[i].events = EPOLLOUT; // Muda o evento para EPOLLOUT após receber dados
//         epoll_ctl(web.getEpollFd(), EPOLL_CTL_MOD, web.getEvents()[i].data.fd, &web.getEvents()[i]);
//     }
//     else if (web.getEvents()[i].events == EPOLLOUT)
//     {
//         sendData(web, web.getEvents()[i].data.fd);
//         web.getEvents()[i].events = EPOLLIN; // Muda o evento de volta para EPOLLIN após enviar dados
//         epoll_ctl(web.getEpollFd(), EPOLL_CTL_MOD, web.getEvents()[i].data.fd, &web.getEvents()[i]);
//     }
// }

// static void handleEvents(WebServer &web, int event_count)
// {
//     for (int i = 0; i <= event_count; ++i)
//     {
//         std::cout << web.getEvents()[i].data.fd << std::endl;
//         // verifica se o evento corresponde a um server (conexao nova)
//         bool server_found = tryConnection(web, i);
//         if (!server_found)
//         {
//             treatExistingClient(web, i);
//         }
//     }
// }

// static void startServer(WebServer &web)
// {
//     while (true)
//     {
//         // Espera por novos eventos
//         int event_count = epoll_wait(web.getEpollFd(), web.getEvents().data(), MAX_EVENTS, 100);
// 		if (event_count == -1) {
// 			std::cerr << "Erro no epoll_wait" << std::endl;
// 			return;
// 		}

//         try
//         {
//             handleEvents(web, event_count);
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//         }

//         // verifica possiveis timeouts
//     }
// }

static void printAllServersInfo(WebServer &webServer)
{
    std::map<std::string, HttpServer *> serversMap = webServer.getServersMap();
    if (serversMap.empty())
    {
        std::cout << "Nenhum servidor configurado." << std::endl;
        return;
    }
    std::cout << "Servidores configurados:" << std::endl;
    std::map<std::string, HttpServer *>::iterator it;
    for (it = serversMap.begin(); it != serversMap.end(); ++it)
    {
        std::cout << "Servidor: " << it->first << std::endl;
        std::cout << "  Endereço: " << it->second->getServerSocket()->getEvent().data.fd << std::endl;
        std::cout << "  Socket FD: " << it->second->getServerSocket()->getEvent().events << std::endl;
        std::cout << "  Porta: " << it->second->getServerSocket()->getSocketFd() << std::endl;
    }
}

int main(void)
{
	printHL();
	WebServer webServer;
    printAllServersInfo(webServer);
    // startServer(webServer);
	// webServer.startServer();
}