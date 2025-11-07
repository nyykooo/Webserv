#!/bin/bash

# Disable any unintended stderr output
exec 2>/dev/null

# Read the HTTP method
method="${REQUEST_METHOD:-GET}"

# Parse the query string (extract key=value)
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

# Extract ?key=... from QUERY_STRING
string=$(parse_query "$QUERY_STRING")

# Build HTML table rows (skip PATH_INFO, real newlines, and HTML-escape values)
env_table_rows=""
while IFS='=' read -r name value; do
    [[ "$name" == "PATH_INFO" ]] && continue
    # Escape special HTML chars
    value=$(printf '%s' "$value" | sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g')
    env_table_rows+="<tr><td><b>${name}</b></td><td>${value}</td></tr>
"
done < <(env | sort)

# Build HTML document
html="$(cat <<EOF
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Bash CGI Environment Dump</title>
    <style>
        body { font-family: monospace; background: #fafafa; color: #222; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ccc; padding: 4px 8px; vertical-align: top; }
        th { background: #eee; }
        h1 { color: #0055aa; }
    </style>
</head>
<body>
    <h1>CGI Environment Variables</h1>
    <hr>
    <table>
        <tr><th>Variable</th><th>Value</th></tr>
        ${env_table_rows}
    </table>
</body>
</html>
EOF
)"

# Compute content length (byte-accurate)
length=$(printf "%s" "$html" | wc -c)

# Send CGI headers
printf "Content-Type: text/html; charset=utf-8\r\n"
printf "Content-Length: %d\r\n" "$length"
printf "\r\n"

# Send body
printf "%s" "$html"
