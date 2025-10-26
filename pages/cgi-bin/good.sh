#!/bin/bash

# Le o metodo HTTP
method="${REQUEST_METHOD:-GET}"

# Função para parsear QUERY_STRING
parse_query() {
    local query="$1"
    local key value
    declare -A result

    IFS='&' read -ra pairs <<< "$query"
    for pair in "${pairs[@]}"; do
        if [[ "$pair" == *"="* ]]; then
            key="${pair%%=*}"
            value="${pair#*=}"
            result["$key"]="$value"
        fi
    done

    echo "${result[key]}"
}

# Pega o valor da chave 'key'
string=$(parse_query "$QUERY_STRING")

# Corpo HTML
html="<!DOCTYPE html>
<html>
<head>
    <meta charset=\"UTF-8\">
    <title>Bash CGI Test</title>
</head>
<body>
    <h1>QUERY STRING! $string</h1>
    <p>REQUEST_METHOD: $method</p>
</body>
</html>"

# Calcula tamanho em bytes
length=$(echo -n "$html" | wc -c)

# Cabeçalhos CGI
printf "Content-Type: text/html; charset=utf-8\r\n"
printf "Content-Length: %d\r\n" "$length"
printf "\r\n" 

# Corpo
printf "%s" "$html"

