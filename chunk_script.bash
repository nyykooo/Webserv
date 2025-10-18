#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Uso: $0 <filename>"
  exit 1
fi

filename="$1"
length=$((11 + ${#filename}))  # calcula o tamanho dinamicamente

(
  printf "POST /upload HTTP/1.1\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n"
  sleep 2
  # imprime o tamanho em hexadecimal, como exige o formato chunked
  printf "%X\r\nfilename=\"%s\"\r\n" "$length" "$filename"
  sleep 2
  printf "4\r\nWiki\r\n"
  sleep 2
  printf "5\r\npedia\r\n"
  sleep 2
  printf "C\r\n in\r\nchunks.\r\n"
  sleep 2
  printf "0\r\n\r\n"
) | nc 127.0.0.1 8080
