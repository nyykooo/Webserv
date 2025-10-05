#!/usr/bin/env python3

import os
import cgi
import json

form = cgi.FieldStorage()

request_method = os.environ.get('REQUEST_METHOD', '')
print("Content-Type: application/json")
response = {}

#??
if (request_method != 'POST'):
    print("Status: 405 Method not allowed")
    print()
    response = {"status": "failure", "message": "Method not allowed"}
    print(json.dumps(response))
    exit(0)

upload_path = os.environ.get('UPLOAD_PATH', '')
os.makedirs(upload_path, exist_ok=True)

if 'file' not in form:
    print()
    response = {"status": "failure", "message": "No file uploaded"}
    print(json.dumps(response))
    exit(0)
    
file_item = form['file']
if file_item.filename:
    # Sanitize filename to avoid security issues
    filename = os.path.basename(file_item.filename)

    # Define full file path
    file_path = os.path.join(upload_path, filename)

    # Save file
    try:
        with open(file_path, 'wb') as f:
            f.write(file_item.file.read())

        response = {
            "status": "success",
            "message": f"File '{filename}' uploaded successfully!",
            "file_path": file_path
        }
    except Exception as e:
        response = {"status": "failure", "message": f"File upload failed: {str(e)}"}
else:
    response = {"status": "failure", "message": "Invalid file"}

print()
print(json.dumps(response))
print()
