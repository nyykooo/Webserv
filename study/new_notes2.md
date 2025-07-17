a variável **uint32_t events** na struct [epoll_event](epoll.h) indica **quais eventos você quer monitorar** (quando registra o fd com **epoll_ctl**) ou **quais eventos realmente ocorreram** (quando recebe o evento em **epoll_wait**)

- EPOLLIN — Pronto para leitura (há dados para ler)
- EPOLLOUT — Pronto para escrita (pode escrever sem bloquear)
- EPOLLERR — Ocorreu um erro
- EPOLLHUP — O peer fechou a conexão
- EPOLLET — Edge Triggered (modo de notificação)
- EPOLLONESHOT — Notifica apenas uma vez

struct epoll_event
{
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;

a epoll_data é uma union. Ela é usada para economia de espaço. Uma union em C/C++ é um tipo especial de struct onde todos os membros compartilham o mesmo espaço de memória. Ou seja, uma union pode armazenar diferentes tipos de dados, mas **apenas um valor de cada vez**. 
- O tamanho da union é igual ao tamanho do seu maior membro.
- Ao atribuir um valor a um membro, os outros membros passam a ter valores indefinidos.
- Útil para representar dados que podem assumir diferentes formatos, mas nunca ao mesmo tempo.