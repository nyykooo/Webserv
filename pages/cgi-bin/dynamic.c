#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_query_param(const char *query, const char *key) {
    if (!query || !key) return "";

    static char value[1024];
    value[0] = '\0';

    // Find key in query string
    char *pos = strstr(query, key);
    if (!pos)
        return "";

    pos += strlen(key);

    if (*pos == '=') {
        pos++;
        size_t i = 0;
        while (*pos && *pos != '&' && i < sizeof(value) - 1) {
            value[i++] = *pos++;
        }
        value[i] = '\0';
    }
    return value;
}

int main(void) {
    const char *method = getenv("REQUEST_METHOD");
    const char *query = getenv("QUERY_STRING");

    if (!method) method = "UNKNOWN";
    if (!query) query = "";

    const char *string = get_query_param(query, "string");

    // Build HTML
    char html[4096];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html>\n"
        "<html>\n"
        "\t<head>\n"
        "\t\t<meta charset=\"UTF-8\">\n"
        "\t\t<title>C CGI Test</title>\n"
        "\t</head>\n"
        "\t<body>\n"
        "\t\t<textarea id=\"query_string\" name=\"query_string\"></textarea>\n"
        "\t\t<button onClick=\"updateCgiPage()\">Update</button>\n"
        "\t\t<h1>QUERY STRING! %s</h1>\n"
        "\t\t<p>REQUEST_METHOD: %s</p>\n"
        "\t\t<script>\n"
        "\t\t\tfunction updateCgiPage() {\n"
        "\t\t\t\tlet query_string = document.getElementById(\"query_string\").value;\n"
        "\t\t\t\twindow.location.href = \"/cgi-bin/dynamic.cgi?string=\" + query_string;\n"
        "\t\t\t}\n"
        "\t\t</script>\n"
        "\t</body>\n"
        "</html>\n",
        string, method
    );

    int length = strlen(html);

    // CGI headers (no status line)
    printf("Content-Type: text/html; charset=utf-8\r\n");
    printf("Content-Length: %d\r\n", length);
    printf("\r\n");

    // Body
    printf("%s", html);

    return 0;
}

