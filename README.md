_This project has been created as part of the 42 curriculum by dioalexa._

# Webserver
## Table of Contents
- [Description](#Description)
- [Instructions](#Instructions)
- [Resources](#resources)
- [Project Description](#project-description)

## Description

Webserv is a systems and network programming project whose objective is to implement a fully functional HTTP/1.1 web server in C++ from scratch, without relying on external web server libraries.

The project aims to replicate the core behavior of production-grade web servers (such as NGINX) while respecting strict constraints on performance, non-blocking I/O, and protocol compliance.
It focuses on low-level networking concepts, event-driven architecture, and correct handling of HTTP requests and responses.

The server is configurable via a custom .conf file and supports hosting multiple virtual servers with different routes, methods, and behaviors.

This implementation includes all mandatory features plus bonuses, notably:

- Support for multiple CGI interpreters
- Cookie parsing and generation
- Session management across HTTP requests

### Features

Implemented features include:

- Fully compliant HTTP/1.1 request parsing
- Non-blocking I/O using poll() (or equivalent)
- Multiple virtual servers (host/port based)
- Static file serving
- Custom error pages

HTTP methods:

- GET
- POST
- DELETE

- File upload handling
- Directory listing (autoindex)
- Redirections
- Request size limits
- Custom configuration file parser

### Bonus Features

- Multiple CGI handlers (e.g. PHP, Python)
- Cookie creation and parsing
- Session management using cookies
- Persistent client sessions across requests

## Instructions
**Requirements:**

### Build the project
```bash
make
```

This will compile the web server executable.

### Run the server:
```bash
./webserv config/webserv.conf
```

If no configuration file is provided, a default one is used.

### Test the server

Open a browser or use curl:
```bash
curl http://localhost:8080
```
Or test specific features:
```bash
curl -X POST http://localhost:8080/upload
curl -I http://localhost:8080
```
### Stop the server

Simply terminate the process:
```bash
Ctrl + C
```
## Resources
### Documentation

    - [RFC 7230 – HTTP/1.1 Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
    - [RFC 7231 – HTTP/1.1 Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
    - [Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/)
    - [GNU C Library – Sockets](https://www.gnu.org/software/libc/manual/html_node/Sockets.html)

### CGI

    - [Common Gateway Interface (CGI) Specification](https://www.rfc-editor.org/rfc/rfc3875)
    - [PHP CGI documentation](https://www.php.net/manual/en/install.unix.commandline.php)
    - [Python CGI Programming](https://docs.python.org/3/library/cgi.html)

### Cookies & Sessions

    - [RFC 6265 – HTTP State Management Mechanism](https://datatracker.ietf.org/doc/html/rfc6265)
	- [MDN – HTTP Cookies](https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies)
	- [MDN – Sessions](https://developer.mozilla.org/en-US/docs/Web/HTTP/Session)

### Use of AI

AI (ChatGPT) was used for:

- Reviewing HTTP parsing logic and edge cases
- Verifying CGI execution flow and environment variables
- Clarifying RFC requirements and protocol behavior
- Improving documentation clarity and structure

All architectural decisions, implementation, debugging, and validation were performed manually.
## Project Description
### Server Architecture

The web server follows an event-driven architecture using non-blocking sockets:

    A single event loop manages all client connections

    poll() monitors:

        Listening sockets

        Client sockets

        CGI pipes

    No blocking calls are used in request handling

This ensures scalability and compliance with the project requirements.

### Configuration File

The server is configured via a custom .conf file inspired by NGINX syntax.

Supported directives include:
```bash
    server

    listen

    server_name

    location

    root

    index

    methods

    error_page

    client_max_body_size

    autoindex

    return (redirection)
```

Each server block is isolated and parsed into internal configuration structures at startup.

### CGI Handling

    Multiple CGI interpreters are supported (e.g. .php, .py)

    Each CGI request:

        Forks a child process

        Sets up required CGI environment variables

        Uses pipes to capture script output

    CGI execution is fully non-blocking from the server perspective

## Cookies and Session Management (Bonus)

    The server parses incoming Cookie headers

    Session IDs are generated and stored server-side

    Sessions persist across multiple HTTP requests

    Cookies are attached to responses via Set-Cookie

    Session expiration and validation are handled internally

This allows stateful behavior on top of the stateless HTTP protocol.

### Error Handling

    All errors return valid HTTP responses

    Custom error pages are configurable

    Malformed requests never crash the server

    Invalid configurations are detected at startup

### Sources Included

    src/ — core server implementation

    includes/ — headers and data structures

    config/ — example configuration files

    cgi-bin/ — CGI scripts for testing

    www/ — static files and uploads

    Makefile


### This project demonstrates a deep understanding of:

- Network programming
- HTTP protocol internals
- Event-driven server design
- Process management
- Configuration parsing
- Stateful behavior over HTTP

**Webserv serves as a solid foundation for understanding how real-world web servers operate internally.**