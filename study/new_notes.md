O conceito de normalização é importante para o parsing das requisições.

O que fazer, 
	Remover ou adicionar o prefixo www (ex: http://www.example.com/ → http://example.com/)

	Ordenar parâmetros de consulta (ex: ?lang=en&article=fred → ?article=fred&lang=en)

	Decodificar caracteres percentuais para caracteres reservados e não reservados conforme RFC 3986

	Uniformizar o uso de letras maiúsculas e minúsculas onde não há distinção semântica (ex: hostnames)

	Remover parâmetros desnecessários ou duplicados

Por exemplo, http://example.com/page e HTTP://EXAMPLE.COM:80/page podem ser considerados equivalentes após normalização, pois o HTTP padrão usa porta 80 e o hostname não diferencia maiúsculas de minúsculas.
