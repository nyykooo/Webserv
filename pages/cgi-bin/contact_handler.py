#!/usr/bin/env python3
import os
import sys
import cgi
import datetime

form = cgi.FieldStorage()

request_method = os.environ['REQUEST_METHOD']

if (request_method == 'POST'):
    name = form.getvalue("name", "").strip()
    email = form.getvalue("email", "").strip()
    message = form.getvalue("message", "").strip()
    timestamp = datetime.datetime.now()
else:
    # Unsupported request method
    print('Content-Type: text/plain')
    print('Status: 405 Method Not Allowed')
    print()
    print('Method Not Allowed')
    exit()

if not name or not email or not message:
    print('Content-Type: text/plain')
    print('Status: 400 Bad Request')
    print()
    print('Invalid input')
    exit()

email_content = "New Contact Form Submission:\n\n"
email_content += f"Name: {name}\n"
email_content += f"Email: {email}\n"
email_content += f"Message: {message}\n"
email_content += f"Submitted on: {timestamp}\n\n"

#save to a file
file_path = os.path.join(sys.path[0]) + '/logs/messages.log'
with open(file_path, "a") as f:
    f.write(email_content)

# respond with success
response =  f"<!DOCTYPE html>\
<html lang='en'>\
<head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>Message Received</title>\
    <style>\
    h1, p {{ width: 50%; margin: 20px auto;}}\
    </style>\
</head>\
<body>\
    <div class='home_bnt'>\
    <a href='/index.html'>H O M E</a><br />\
    <h1>Hello, {name}!</h1>\
    <p>Thank you for your message</p>\
</body>\
</html>"

# return a HTML page response
print ('Content-Type: text/html')
print ('Status: 200 OK')
print()
print (response)