#!/usr/bin/python3
import os

#!/usr/bin/env python3

import os
import sys
import cgi
import http.cookies
import datetime
import json

SESSION_COOKIE_NAME = "SESSION_ID"
SESSION_STORAGE = os.path.join(sys.path[0]) + "/sessions"  # Change this path based on your setup

# Ensure session storage exists
if not os.path.exists(SESSION_STORAGE):
    os.makedirs(SESSION_STORAGE)

def set_cookie(name, value, expires=None):
    cookie = http.cookies.SimpleCookie()
    cookie[name] = value
    cookie[name]["Path"] = "/"
    if expires:
        cookie[name]["Expires"] = expires
    print(cookie)

def get_cookie():
    cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
    return cookie.get(SESSION_COOKIE_NAME)

def generate_session_id():
    return str(int(datetime.datetime.now().timestamp()))

def save_session(session_id, username):
    with open(f"{SESSION_STORAGE}/{session_id}", "w") as f:
        f.write(username)

def get_session(session_id):
    session_file = f"{SESSION_STORAGE}/{session_id}"
    if os.path.exists(session_file):
        with open(session_file, "r") as f:
            return f.read().strip()
    return None

def delete_session(session_id):
    session_file = f"{SESSION_STORAGE}/{session_id}"
    if os.path.exists(session_file):
        os.remove(session_file)

print("Content-Type: application/json")

# Parse request
form = cgi.FieldStorage()
action = form.getvalue("action")
response = {}

if action == "login":
    username = form.getvalue("username")
    if username:
        session_id = generate_session_id()
        save_session(session_id, username)
        set_cookie(SESSION_COOKIE_NAME, session_id, expires="Fri, 31 Dec 9999 23:59:59 GMT")
        response = {"status": "success", "message": f"Login successful! Welcome, {username}."}
    else:
        response = {"status": "error", "message": "Missing username!"}

elif action == "status":
    session = get_cookie()
    if session:
        username = get_session(session.value)
        if username:
            response = {"status": "active", "message": f"Session Active! Welcome, {username}."}
        else:
            response = {"status": "expired", "message": "Session Expired."}
    else:
        response = {"status": "inactive", "message": "No active session."}

elif action == "logout":
    session = get_cookie()
    if session:
        delete_session(session.value)
        set_cookie(SESSION_COOKIE_NAME, "", expires="Thu, 01 Jan 1970 00:00:00 GMT")
        response = {"status": "success", "message": "Logged out successfully."}
    else:
        response = {"status": "error", "message": "No session to log out from."}
print()
print(json.dumps(response, indent=4))
print()
