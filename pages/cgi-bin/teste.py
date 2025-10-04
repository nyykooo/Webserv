#!/usr/bin/env python3
import sys
import os

# Corpo HTML
rows = ""
for key, value in sorted(os.environ.items()):
	rows +=f"<tr><td>{key}</td><td>{value}</td></tr>\n"

html = f"""<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<title>Python CGI Test</title>
	</head>
	<body>
		<h1>CGI em Python funcionando!</h1>
		<table>
			<tr><th>Variables</th><th>Value</th></tr>
			{rows}
		</table>
	</body>
</html>"""

# Calcula o tamanho em bytes
length = len(html.encode("utf-8"))

# Cabeçalhos CGI (sem status line!)
sys.stdout.write("Status: 400\r\n")
sys.stdout.write("Set-Cookie: test2\r\n")
sys.stdout.write("Set-Cookie: test3\r\n")
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write(f"Content-Length: {length}\r\n")
sys.stdout.write("\r\n")

# Corpo
sys.stdout.write(html)
