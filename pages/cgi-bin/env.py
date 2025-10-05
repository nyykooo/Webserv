#!/usr/bin/python3
import os
print ("Content-Type: text/html; charset=UTF-8\r\n\r\n")
print("<!DOCTYPE html>")
print("<html lang=\"en\">")
print("<head>")
print("    <meta charset=\"UTF-8\">")
print("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">")
print("    <link rel=\"shortcut icon\" href=\"favicon.ico\" type=\"image/x-icon\">")
print("</head>")
print("<body>")
print("<ul>")
for name, value in os.environ.items():
    print("<li>")
    print("{0}: {1}".format(name, value))
    print("</li>")
print("</ul>")
print("</body>")
print("</html>")
