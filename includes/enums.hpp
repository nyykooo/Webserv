
#ifndef ENUMS_HPP
# define ENUMS_HPP

enum	RequestProcessingState
{
	RECEIVING, // server está recebendo dados do cliente
	RECEIVING_LARGE,
	PROCESSING, // server está processando a requisição do cliente
	PROCESSING_LARGE, // server está processando a requisição do cliente, mas a PIZZA a ser preparada eh GRANDE
	STREAMING, // server está servindo pedaço a pedaço da PIZZA GRANDE
	COMPLETED, // server finalizou o processamento da requisição do cliente
	CGI_PROCESSING, // server está processando uma requisição CGI
	CGI_COMPLETED // server finalizou o processamento da requisição CGI
};

typedef enum 	e_method_type
{
	GET,
	POST,
	DELETE,
}				t_method_type;

typedef enum	e_requestType {
	OK,
	REDIRECT,
	ERROR,
}				t_requestType;

#endif