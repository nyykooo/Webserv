
#ifndef ENUMS_HPP
# define ENUMS_HPP

enum	RequestProcessingState
{
	RECEIVING, // server está recebendo dados do cliente
	PROCESSING, // server está processando a requisição do cliente
	PROCESSING_LARGE, // server está processando a requisição do cliente, mas a PIZZA a ser preparada eh GRANDE
	STREAMING, // server está servindo pedaço a pedaço da PIZZA GRANDE
	COMPLETED // server finalizou o processamento da requisição do cliente
};

#endif