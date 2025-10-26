# read the REQUEST_METHOD

# message header:
#	- The message-header contains one or more header fields.
# 	- Document Response:
#		- The script MUST return a Content-Type header field.
#	- Local Redirect Response:
#		- The CGI script can return a URI path and query-string ('local-pathquery') for a local resource in a Location header field.
#		- The script MUST NOT return any other header fields or a message-body
# 	- Client Redirect Response:
#		- The CGI script can return an absolute URI path in a Location header field, to indicate to the client that it should reprocess the request using the URI specified.
# 		- The script MUST not provide any other header fields, except for server-defined CGI extension fields
#		- For an HTTP client request, the server MUST generate a 302 'Found' HTTP response message.
#	- Client Redirect Response with Document:
#		- The Status header field MUST be supplied and MUST contain a status value of 302 'Found'
#   - Pelo menos um dos campo-CGI DEVE ser fornecido
#		CGI-field       = Content-Type | Location | Status
#		- Content-Type: 
#			- Se um corpo da entidade for retornado, o script DEVE fornecer um campo Tipo de conteúdo na resposta
#			- Portanto, o script DEVE incluir um parâmetro de charset
#		- Location:
#			- O campo de cabeçalho Localização é usado para especificar ao servidor que o script está a devolver uma referência a um documento em vez de um documento real
#		- Status:
#			- O campo do cabeçalho Status contém um código de resultado inteiro de 3 dígitos que indica o nível de sucesso da tentativa do script de processar a solicitação.
#			- O script DEVE verificar o valor de SERVER_PROTOCOL antes de usar códigos de status HTTP/1.1
# blank line
# message body -> The body may be NULL.

#!/usr/bin/env python3
import sys
import os


# Le o metodo HTTP e guarda para usar posteriormente
method = os.environ.get("REQUEST_METHOD")

# string dinamica -> QUERY_STRING#!/usr/bin/env python3
def parse_query(query: str) -> dict:
    result = {}
    pairs = query.split("&")  # separa cada "key=value"
    for pair in pairs:
        if "=" in pair:
            key, value = pair.split("=", 1)  # separa apenas na primeira igualdade
            result[key] = value
    return result

qs = parse_query(os.environ.get("QUERY_STRING"))
string = qs["key"]


# Corpo HTML
html = f"""<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<title>Python CGI Test</title>
	</head>
	<body>
		<h1>QUERY STRING! {string}</h1>
		<p>REQUEST_METHOD: {method}</p>
	</body>
</html>"""

# Calcula o tamanho em bytes
length = len(html.encode("utf-8"))

# Cabeçalhos CGI (sem status line!)
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.write(f"Content-Length: {length}\r\n")
sys.stdout.write("\r\n")

# Corpo
sys.stdout.write(html)
