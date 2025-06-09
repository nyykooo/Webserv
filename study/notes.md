## WEBSERVER

Precisamos codar o nosso próprio HTTP server.  
*The primary function of a web server is to store, process, and deliver web pages to clients.*  
*HTTP also enables clients to send data. This feature is used for submitting web forms, including the uploading of files.*

### Primeiros conceitos:

#### **HTTP**
Então, o que é HTTP. É **H**ype**T**ext **T**ransfer **P**rotocol. Protocolo de transferência de Hipertexto. 

Tá. E o que é Hipertexto. É uma forma de organização de informações em que textos (ou outros recursos) estão interligados por links, permitindo navegação não linear. O HTML (HyperText Markup Language) é a principal linguagem usada para criar documentos hipertexto na Web.

Desse modo, HTTP é o protocolo responsável por transferir esses documentos hipertexto (e outros recursos) entre clientes e servidores.

Métodos HTTP principais  
	**GET**: solicita dados do servidor.  
	**POST**: envia dados para o servidor.  
	**PUT**: atualiza ou cria recursos.  
	**DELETE**: remove recursos. Outros métodos como **HEAD**, **OPTIONS**, **PATCH** e **TRACE** têm funções específicas.

Essa tranfererência acontece por meio de uma conexão TCP/IP.

	Primeiro o cliente abre a conexão TCP e Envia uma requisição GET/POST...
	Em seguida, o Servidor processa a requisição e Envia uma resposta (status, header, body)
	Por fim, a conexão será mantida, pois estamos trabalharemos com o protocolo HTTP/1.1

##### **TCP (Transmission Control Protocol)**
Em uma conexão TCP dois sockets se comunicarão (cliente e server) através de uma tupla única de dados: IP e Porta do Client e IP e Porta do Server.	

Em sistemas UNIX sockets (sockets descriptors) são tratados comos fds (file descriptors). Um socket aberto é um tipo especial de fd e ambos são recursos abertos pelo processo e representados por ints em uma tabela. Ou seja, podemos usar no socket read(), write(), close()...

Após o envio da resposta, a conexão pode ser fechada (HTTP/1.0) ou mantida para múltiplas requisições (HTTP/1.1 com *keep alive*)  

O cliente cria um **socket** para se conectar ao endereço IP e porta do servidor, enquanto o servidor mantém um socket "escutando" em uma porta específica e, ao aceitar a conexão, cria um **socket** para se comunicar com aquele cliente específico.

Estados de uma conexão entre sockets:

![alt text](image.png)

Lembrar de evocar a close() tanto no client quanto no server ao fim.

###### **Portas TCP**
são identificadores numéricos usados para distinguir diferentes serviços e aplicações que rodam em um mesmo computador conectado à rede. Elas funcionam como pontos de extremidade para a comunicação de dados, permitindo que múltiplos programas utilizem a rede simultaneamente sem que os dados se misturem. A porta 80 é usada para HTTP (navegação web), a porta 443 para HTTPS (navegação segura), a porta 21 para FTP (transferência de arquivos), entre outras. Existem 65.535 portas (o campo da porta tem 16 bits).

#### **RFC**
São documentos publicados pela IEFT (Internet Engineering Task Force) que especificam padrões, protocolos e boas práticas para a Internet. Estudá-los é bom, pois são a fonte detalhada sobre como os protocolos funcionam, o formato das mensagens, métodos, cabeçalhos, etc.

###### **Estrutura de uma requisição GET**
	GET <caminho> <versão do HTTP>  
	Host: <nome do host>  
	<linha em branco>  

O campo Host é obrigatório no HTTP/1.1.

Poderíamos acresecentar a esses campos alguns cabeçalhos opcionais como o User Agent:, ele identifica o software cliente que está fazendo a requisição, navegador, curl, Postman, etc. É útil por para adaptar a resposta, por exemplo, enviar uma página diferente para desktop ou mobile.

Há tb, por exemplo, o Accept:, Indica ao server quais tipos de conteúdo o cliente aceita receber como resposta. O server pode, nesse caso, escolher o formato de resposta mais adequado.

Aqui podemos mencioanr a importânia da **RFC 2616**! Ela define o protocolo HTTP/1.1

Ela detalha:

	Formato das mensagens HTTP (requisições e respostas)
	Métodos HTTP: GET, POST, PUT, DELETE, HEAD, OPTIONS, TRACE, CONNECT
	Códigos de status: 1xx (informativo), 2xx (sucesso), 3xx (redirecionamento), 4xx (erro do cliente), 5xx (erro do servidor)
	Cabeçalhos HTTP: como Host, Content-Type, Content-Length, User-Agent, etc.
	Conexões persistentes (keep-alive)
	Cache, cookies, autenticação e controle de conexões
	Regras de sintaxe e semântica para requisições e respostas


##### **Telnet**  
Protocolo de rede e também um programa de linha de comando usado para conectar a portas TCP de outros computadores (ou do próprio).  
É usado para acessar remotamente outro computador pela rede, utilizando o protocolo Telnet (porta padrão 23).  
Ele permite abrir uma sessão interativa de terminal em outro sistema.  

Para nós é muito interessante, pois através desse programa podemos nos conectar manuamente à porta 80 do nosso próprio computador e escrever requisições HTTP na mão e ver as respostas do servidor. Então, isso será importante para testar e debugar o nosso server, simulando um cliente básico.    

*Multiplexação não bloqueante  
	Multiplexação não bloqueante é uma técnica para lidar com múltiplas conexões de entrada/saída (I/O) ao mesmo tempo sem que o programa fique parado esperando (bloqueado) por dados em uma única conexão.







