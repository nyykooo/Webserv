#!/usr/bin/env python3
import sys
import os

# Get HTTP method safely
method = os.environ.get("REQUEST_METHOD", "GET")

# Parse the query string
def parse_query(query: str) -> dict:
    result = {}
    for pair in query.split("&"):
        if "=" in pair:
            key, value = pair.split("=", 1)
            result[key] = value
    return result

qs = parse_query(os.environ.get("QUERY_STRING", ""))
string = qs.get("key", "")

# Build environment table excluding PATH_INFO
env_table_rows = ""
for key in sorted(os.environ.keys()):
    if key == "PATH_INFO":
        continue  # skip PATH_INFO
    env_table_rows += f"<tr><td><b>{key}</b></td><td>{os.environ[key]}</td></tr>\n"

# HTML body
html = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Python CGI Environment Dump</title>
    <style>
        body {{
            font-family: monospace;
            background: #fafafa;
            color: #222;
        }}
        table {{
            border-collapse: collapse;
            width: 100%;
        }}
        th, td {{
            border: 1px solid #ccc;
            padding: 4px 8px;
        }}
        th {{
            background: #eee;
        }}
        h1 {{
            color: #0055aa;
        }}
    </style>
</head>
<body>
    <h1>CGI Environment Variables</h1>
    <hr>
    <table>
        <tr><th>Variable</th><th>Value</th></tr>
        {env_table_rows}
    </table>
</body>
</html>"""

# Calculate content length
length = len(html.encode("utf-8"))

# CGI headers
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.write(f"Content-Length: {length}\r\n")
sys.stdout.write("\r\n")

# Output body
sys.stdout.write(html)
