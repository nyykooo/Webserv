#!/usr/bin/env python3
import os
import http.cookies
import cgi

form = cgi.FieldStorage()
action = form.getvalue("action", "")
cookie_name = form.getvalue("name", "")
cookie_value = form.getvalue("value", "")

# Get existing cookies
cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))

print("Content-Type: text/plain")
if action == "set":
    if not cookie_name or not cookie_value:
        print("\nCookie name and value are required!\n")
        exit()
    # Set a new cookie
    cookie[cookie_name] = cookie_value
    cookie[cookie_name]["path"] = "/"  # Make it available site-wide
    print(f"{cookie.output()}")
    print(f"\nCookie '{cookie_name}' set successfully.")

elif action == "get":
    # Display all stored cookies
    if cookie:
        print("\nStored Cookies:\n")
        for key, morsel in cookie.items():
            print(f"{key}: {morsel.value}")
    else:
        print("\nNo cookies found.")

elif action == "delete":
    if not cookie_name:
        print("\nCookie name is required!\n")
        exit()
    # Delete a cookie
    if cookie_name in cookie:
        cookie[cookie_name] = ""
        cookie[cookie_name]["Expires"] = "Fri, 31 Dec 9999 23:59:59 GMT"
        cookie[cookie_name]["Max-Age"] = 0
        print(f"{cookie.output()}")
        print(f"\nCookie '{cookie_name}' deleted successfully.")
    else:
        print("\nCookie not found.")

