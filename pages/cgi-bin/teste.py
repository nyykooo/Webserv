#!/usr/bin/env python3
import sys

# Corpo HTML
html = """<!DOCTYPE html>
<html><head><title>Python CGI Test</title></head>
<body><h1>CGI em Python funcionando!</h1></body></html>"""

# Calcula o tamanho em bytes
length = len(html.encode("utf-8"))

# Cabeçalhos CGI (sem status line!)
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write(f"Content-Length: {length}\r\n")
sys.stdout.write("\r\n")

# Corpo
sys.stdout.write(html)
