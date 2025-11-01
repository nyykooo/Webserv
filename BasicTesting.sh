#!/usr/bin/env bash
# webserv basic tests (default: http://127.0.0.1:8080)

set -u
BASE="${BASE:-http://127.0.0.1:8080}"
GET_PATH="${GET_PATH:-/index.html}"
UPLOAD_DIR=${UPLOAD_DIR:-/uploads}
FILENAME="${FILENAME:-/42LisbonBornToCode}"
TIMEOUT="${TIMEOUT:-5}"
TMPDIR="$(mktemp -d)"
FAILS=0
trap 'rm -rf "$TMPDIR"' EXIT

if [[ -t 1 && -z "${NO_COLOR:-}" ]]; then
  if command -v tput >/dev/null 2>&1 && [[ $(tput colors 2>/dev/null || echo 0) -ge 8 ]]; then
    RED="$(tput setaf 1)"; GREEN="$(tput setaf 2)"; YELLOW="$(tput setaf 3)"; RESET="$(tput sgr0)"
  else
    RED=$'\e[31m'; GREEN=$'\e[32m'; YELLOW=$'\e[33m'; RESET=$'\e[0m'
  fi
else
  RED=""; GREEN=""; YELLOW=""; RESET=""
fi

say()  { printf "%s\n" "$*"; }
pass() { printf "%b\n" "${GREEN}[PASS]${RESET} $*"; }
fail() { printf "%b\n" "${RED}[FAIL]${RESET} $*"; FAILS=$((FAILS+1)); }
info() { printf "%b\n" "${YELLOW}[INFO]${RESET} $*"; }

need() { command -v "$1" >/dev/null 2>&1 || { echo "Falta o comando: $1"; exit 127; }; }
need curl

# Sanity: servidor no ar?
if ! curl -sS --max-time "$TIMEOUT" -o /dev/null "$BASE/"; then
  echo "Servidor indisponível em $BASE. Verifique host/porta ou exporte BASE=..."; exit 2
fi

request() { # method path [data_file] [content_type]
  local method="$1" path="$2" dataf="${3:-}" ctype="${4:-text/plain}"
  local body="$TMPDIR/body_$$.bin"
  local url="${BASE}${path}"
  local code
  if [[ -n "$dataf" ]]; then
    code="$(curl -sS --max-time "$TIMEOUT" -o "$body" -w '%{http_code}' -X "$method" -H "Content-Type: $ctype" --data-binary "@${dataf}" "$url" || echo 000)"
  else
    code="$(curl -sS --max-time "$TIMEOUT" -o "$body" -w '%{http_code}' -X "$method" "$url" || echo 000)"
  fi
  # saída parseável (dois campos separados por espaço, sem '|')
  printf "%s %s\n" "$code" "$body"
}

status_in() { local code="$1"; shift; for a in "$@"; do [[ "$code" == "$a" ]] && return 0; done; return 1; }
status_2xx_3xx() { local c="$1"; [[ "$c" =~ ^(2..|3..)$ ]]; }

# 1) GET básico
read -r code body <<<"$(request GET "$GET_PATH")"
if status_2xx_3xx "$code"; then pass "GET $GET_PATH (status $code)"; else fail "GET $GET_PATH (status $code)"; fi

# 2) Método desconhecido não derruba o servidor
# HTTP 501 (Not Implemented) 
read -r code body <<<"$(request FOO "/")"
if status_in "$code" 400 405 501; then pass "UNKNOWN method FOO / (status $code)"; else fail "UNKNOWN method FOO / (status $code)"; fi

read -r code body <<<"$(request GET "$GET_PATH")"
if status_2xx_3xx "$code"; then pass "Server vivo após UNKNOWN (GET ok, $code)"; else fail "Server pode ter caído (GET $code)"; fi

# 3) POST -> GET -> DELETE de um arquivo
path="${FILENAME%}"
payload="$TMPDIR/payload.txt"
echo -n "MakeCodeNotWar" > "$payload"

# POST
read -r code body <<<"$(request POST "$path" "$payload")"
if status_in "$code" 200 201 202 204 303; then pass "POST $path (status $code)"; else fail "POST $path (status $code)"; fi

# GET e compara conteúdo
read -r code got <<<"$(request GET "$UPLOAD_DIR$path")"
if [[ "$code" =~ ^2..$ ]] && cmp -s "$got" "$payload"; then
  pass "GET $path confere conteúdo (status $code)"
else
  fail "GET $path conteúdo divergente ou status $code"
fi

# DELETE
# 204 No Content.
# 200 OK: Indica sucesso e a resposta pode incluir uma representação confirmando a exclusão.
# 202 Accepted: Indica que a solicitação foi aceita para processamento, mas a exclusão pode não ter sido concluída ainda.

202 Accepted: Indica que a solicitação foi aceita para processamento, mas a exclusão pode não ter sido concluída ainda.
read -r code body <<<"$(request DELETE "$UPLOAD_DIR$path")"
if status_in "$code" 200 202 204; then pass "DELETE $path (status $code)"; else fail "DELETE $path (status $code)"; fi

# GET após DELETE deve ser 404/410
# 410 é usado quando o recurso foi apagado de forma definitiva e não há redirecionamento ou substituição programada para ele
read -r code body <<<"$(request GET "$UPLOAD_DIR$path")"
if status_in "$code" 404 410; then pass "GET após DELETE $path (status $code)"; else fail "GET após DELETE $path (status $code)"; fi

# POST FINAL
read -r code body <<<"$(request POST "$path" "$payload")"
if status_in "$code" 200 201 202 204 303; then pass "POST $path (status $code)"; else fail "POST $path (status $code)"; fi
# Resumo
if [[ $FAILS -eq 0 ]]; then
  say "All good!"; exit 0
else
  say "$FAILS fail"; exit 1
fi