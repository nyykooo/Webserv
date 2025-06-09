Quanto aos headers das requisições:
	Os nomes de campos são case-insensitive. ex: content-type ≡ Content-Type, mas valores podem ser sensíveis. 

	Nomes de campo (header field names)
		Permitidos: Apenas caracteres ASCII, sem espaços ou caracteres de controle.
		Sintaxe: O nome deve ser composto por caracteres chamados de "token", que incluem:
		Letras maiúsculas e minúsculas (A-Z, a-z)
		Dígitos (0-9)
		Símbolos: ! # $ % & ' * + - . ^ _ | ~`
		Proibidos: Espaços, tabulações, dois pontos (:), aspas, barras invertidas, e qualquer caractere de controle (como \r, \n).

	Valores de campo (header field values)
		Permitidos: Quase todos os caracteres ASCII, exceto caracteres de controle (como \r, \n), mas espaços são permitidos.
		Estrutura: O valor pode ser um "token" simples ou uma quoted-string (string entre aspas duplas).

	Valores entre aspas (quoted-string)
		Permitidos: Quase todos os caracteres ASCII podem aparecer dentro de uma quoted-string, inclusive espaços e alguns caracteres especiais.
		Regras especiais:
		Aspas duplas (") e barras invertidas (\) dentro da quoted-string devem ser precedidas por uma barra invertida (ex: \", \\).
		Caracteres de controle (como \r, \n) não são permitidos nem mesmo dentro de quoted-string.

	Os campos podem ser **estáticos** (Não mudam entre requisições, ex: Host.) ou **dinâmicos** (Cookie, authorization...). Tbm podem ser campos **de controle**: Diretivas como Cache-Control: no-store ou Content-Length

	é sugerido pela RFC que campos de controle venham primeiro. ex, Host.

	Limites de campos. É recomendado Nome do campo: ≤ 1024 caracteres e Valor do campo: ≤ 8192 caracteres. Motivação: Prevenção de ataques DoS (ex: campos excessivamente longos). Tratamento de exceções: Campos inválidos devem gerar resposta 400 Bad Request

	O ; separa parâmetros.

	No parsing devo remover os espaços redundantes. Rejeitar caracteres não permitidos. Limitar o número de campos para menor ou igual a 64. Ignorar campos duplicados conflitantes. Mas campos duplicados que permitam múltiplos valores devem ser unificados e todos os valores devem ficar separados por vírgulas.

	Ao interpretar um valor entre aspas de um header, seu parser deve: Remover as aspas externas e substituir cada sequência \" por ", e cada \\ por \.

	recomenda-se rejeitar ou ignorar campos de cabeçalho com line folding.

	Listas: Valores separados por vírgula em headers.
	Parâmetros: Pares nome=valor após o valor principal, separados por ;.
	Comentários: Entre parênteses, só onde permitido.
	Data/hora: RFC 1123 é o padrão, mas clientes devem aceitar três formatos; sempre em GMT.
		RFC 1123: Sun, 06 Nov 1994 08:49:37 GMT
		RFC 850 (obsoleto): Sunday, 06-Nov-94 08:49:37 GMT
		ANSI C asctime(): Sun Nov 6 08:49:37 1994

		O formato a ser gerado deve ser sempre o RFC 1123

##BODY
	Exemplos de cabeçalhos que afetm a semântico do body,

		Content-Type: Indica o tipo de mídia do corpo (ex: application/json, text/html).
		Content-Encoding: Indica se o corpo está comprimido ou codificado (ex: gzip).
		Content-Language: Indica o idioma do conteúdo.
		?Content-Location: Pode indicar um local alternativo para a representação do recurso.

	Métodos como HEAD e respostas com status 204 (No Content), 304 (Not Modified) e 1xx (Informational) não têm corpo.

	Mesmo quando o corpo está ausente, os cabeçalhos ainda podem fornecer informações semânticas relevantes.

	A semântica é independente da forma de transmissão:
	A interpretação do conteúdo é separada da forma como ele é transferido (chunked, codificado, etc.).

	Definição de Content Coding
	Um content coding é uma transformação aplicada ao corpo da mensagem, indicada pelo campo de cabeçalho Content-Encoding.
		Content-Encoding: gzip
	O cliente pode indicar quais codificações aceita usando o campo Accept-Encoding na requisição. O servidor pode escolher uma codificação suportada pelo cliente.

	Content Coding vs. Transfer Coding

	Content coding afeta a representação do recurso (armazenada ou processada).
	Transfer coding (ex: chunked) afeta apenas a transferência na rede, não a representação armazenada.

	A seção 6.6 define "representação" como o corpo da mensagem mais os metadados que o descrevem. tipo, idioma, codificação, etc

	A selected representation é a versão do recurso que o servidor escolheu para enviar ao cliente, levando em conta as preferências indicadas pelo cliente nos cabeçalhos de requisição (como Accept, Accept-Language, Accept-Encoding) e as capacidades do servidor.
	Exemplo: Um recurso pode estar disponível em JSON e XML; se o cliente pedir Accept: application/json, a representação selecionada será JSON.

	Negociação de conteúdo

	O processo de seleção pode envolver:

	Tipo de mídia (Accept)
	Idioma (Accept-Language)
	Codificação (Accept-Encoding)
	Outros critérios definidos pelo servidor ou pela aplicação

	O servidor deve indicar claramente, nos cabeçalhos da resposta, as características da representação selecionada (ex: Content-Type, Content-Language).

	Relevância para cache
	Caches HTTP armazenam a representação selecionada.
	Os cabeçalhos de negociação (Vary, etc.) informam ao cache sob quais condições diferentes representações devem ser armazenadas ou servidas.

	Vamos rejeitar DELETE com corpo? Costuma ser uma boa prática de segurança e interoperabilidade.  A resposta a um DELETE pode incluir um corpo (por exemplo, uma mensagem de confirmação ou o estado do recurso excluído), mas também pode ser vazia. Os códigos de status mais comuns são 200 OK (remoção bem-sucedida com resposta), 202 Accepted (remoção aceita para processamento assíncrono) ou 204 No Content (remoção bem-sucedida sem corpo de resposta). 409 Conflict (restrição de negócio impede a exclusão). ou o 404 Not Found (recurso já não existe).

	Nosso website estático terá conteúdo fixo: Todos os visitantes recebem exatamente o mesmo conteúdo para cada URL, sem personalização baseada em usuário, localização ou outras variáveis de sessão. Ou seja, os headers do client — como Cookie, Authorization, Accept-Language, entre outros — não influenciam o conteúdo retornado por um site totalmente estático. O servidor simplesmente serve o mesmo arquivo HTML (e recursos associados) para qualquer usuário que acesse uma determinada URL, independentemente dos headers enviados na requisição.