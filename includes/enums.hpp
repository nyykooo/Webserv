
#ifndef ENUMS_HPP
# define ENUMS_HPP

enum	RequestProcessingState
{
	RECEIVING, // server is receiving data from the client
	RECEIVING_LARGE, // server is receiving a large amount of data from the client
	PROCESSING, // server is processing the client's request
	PROCESSING_LARGE, // server is processing the client's request, but the PIZZA being prepared is LARGE
	STREAMING, // server is serving the LARGE PIZZA piece by piece
	COMPLETED, // server has finished processing the client's request
	CGI_PROCESSING, // server is processing a CGI request
	CGI_COMPLETED, // server has finished processing the CGI request
	SEND_DATA // data is ready to be sent to the client -> processing state to send "chunks"
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