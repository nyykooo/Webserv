// exemplo_simple_concorrente_test.cpp
#include <gtest/gtest.h> // sudo apt install libgtest-dev
#include <gmock/gmock.h> // sudo apt install libgmock-dev
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
// include curl to make client requests to the server
#include <curl/curl.h> // sudo apt install libcurl4-openssl-dev
// NO INSTALL: git submodule add https://github.com/curl/curl.git external/curl ESTUDAR MAIS ESSA ABORDAGEM
#include <string>
#include <cstdarg>  // Para manipulação de argumentos variádicos


// ### OUTPUT COLORS ###
# define RED	"\033[31m"
# define GREEN	"\033[32m"
# define BLUE	"\033[34m"
# define WHITE	"\033[37m"
# define BLACK	"\033[30m"
# define YELLOW	"\033[33m"
# define CYAN	"\033[36m"
# define GRAY	"\033[90m"
# define RESET	"\033[0m"

extern char **environ;
int server_pid = -1;

std::string execute_server(const std::vector<std::string> &config_file)
{    
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return "";
    }

    if (pid == 0) {
        close(pipefd[0]); // fecha leitura
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        std::vector<const char*> args;
        args.push_back("./Webserv");
        for (size_t i = 0; i < config_file.size(); ++i) {
            args.push_back(config_file[i].c_str());
        }
        args.push_back(nullptr);
        execve(args[0], const_cast<char* const*>(args.data()), environ);
        _exit(1);
    }
    else if (pid > 0) {
        int status;
        int childStatus = 0;
        while (childStatus == 0)
        {
            childStatus = waitpid(pid, &status, WNOHANG);
            usleep(50000);
            if (childStatus == 0)
                kill(pid, SIGKILL);
        }
        close(pipefd[1]);

        std::string output;
        char buffer[256];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, n);
        }
        close(pipefd[0]);

        if (WIFEXITED(status)) {
            return output;
        } else if (WIFSIGNALED(status)) {
            return "Child process terminated by signal: " + std::to_string(WTERMSIG(status));
        } else {
            return "Child process terminated abnormally.";
        }
    }
    return "";
}

// // 🔹 Matcher customizado: combina um matcher real + label para feedback
// MATCHER_P2(PrintOkIf, matcher, label, "") {
//     std::cout << RESET;
//     if (ExplainMatchResult(matcher, arg, result_listener)) {
//         std::cout << GREEN << "[  OK      ] " << label << RESET << std::endl;
//         return true;
//     }
//     else
//     {
//         std::cout << RED;
//         return false; // falha → gtest imprime normalmente
//     }
// }

// first test case will try to execute the server with invalid config files 
// the expect result are fails and aligned to the error handling of the server 
// not need of a fixture yet because is a single test case and later when the server will be active we can use the fixture class to create clients 
// the test will try to execute the server in threads with bad config files and check the return value or the error message 
// as we will check the return message we will use
TEST(BadConfigFiles, InvalidConfigTests)
{
    // INVALID CONFIG FILE
    EXPECT_THAT(execute_server({"nonexistent.config"}), testing::HasSubstr("Error opening file: "));
    EXPECT_THAT(execute_server({"default.config", "pipipi"}), testing::HasSubstr("Wrong input. Usage: ./webserv [configuration_file]"));
    EXPECT_THAT(execute_server({"confs/tester/bad_curly.config"}), testing::HasSubstr("Invalid configuration file: invalid sintax."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_keyword_conf.config"}), testing::HasSubstr("invalid keyword in conf file."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_keyword_server.config"}), testing::HasSubstr("invalid keyword in server block."));
    EXPECT_THAT(execute_server({"confs/tester/wrong_curly_pos_open.config"}), testing::HasSubstr("line shouldn't have '{'"));
    EXPECT_THAT(execute_server({"confs/tester/wrong_curly_pos_close.config"}), testing::HasSubstr("} should be at the end of the line."));
    EXPECT_THAT(execute_server({"confs/tester/inv_keyword.config"}), testing::HasSubstr(" invalid keyword in server block."));
    EXPECT_THAT(execute_server({"confs/tester/unclosed_server.config"}), testing::HasSubstr("block brackets \"{}\" are misplaced."));
}

TEST(BadConfigFiles, HostTests)
{
    // HOST TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_host.config"}), testing::HasSubstr(" no host mentioned."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_host.config"}), testing::HasSubstr("invalid host/port"));
    EXPECT_THAT(execute_server({"confs/tester/inv_port.config"}), testing::HasSubstr("invalid port number: "));
    EXPECT_THAT(execute_server({"confs/tester/inv_ip.config"}), testing::HasSubstr("invalid host: "));
}

TEST(BadConfigFiles, ServerNameTests)
{
     // SERVER NAME TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_servername.config"}), testing::HasSubstr(" no server_name defined"));
}

TEST(BadConfigFiles, ErrorPageTests)
{
    // ERROR PAGE TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_error_page.config"}), testing::HasSubstr("no error page defined"));
    EXPECT_THAT(execute_server({"confs/tester/error_page_range.config"}), testing::HasSubstr("must be between 300 and 599"));
    EXPECT_THAT(execute_server({"confs/tester/invalid_error_page.config"}), testing::HasSubstr("is invalid"));
}
TEST(BadConfigFiles, ClientMaxBodySizeTests)
{
    // CLIENT MAX BODY SIZE TESTS
    EXPECT_THAT(execute_server({"confs/tester/invalid_body_size.config"}), testing::HasSubstr("invalid body size number"));
    EXPECT_THAT(execute_server({"confs/tester/no_body_size.config"}), testing::HasSubstr("no request body size defined."));
    EXPECT_THAT(execute_server({"confs/tester/extra_body_size.config"}), testing::HasSubstr("invalid body size defined."));
 
}

TEST(BadConfigFiles, RootTests)
{
   // ROOT TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_root.config"}), testing::HasSubstr("no root defined."));
    EXPECT_THAT(execute_server({"confs/tester/extra_root.config"}), testing::HasSubstr("too many arguments when defining root."));
}

TEST(BadConfigFiles, IndexTests)
{
    // INDEX TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_index.config"}), testing::HasSubstr("no default file defined."));
}

TEST(BadConfigFiles, AutoIndexTests)
{
    // AUTOINDEX TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_autoindex.config"}), testing::HasSubstr("no value in autoindex defined."));
    EXPECT_THAT(execute_server({"confs/tester/extra_autoindex.config"}), testing::HasSubstr("too many arguments when defining autoindex."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_autoindex.config"}), testing::HasSubstr("Invalid value for autoindex: "));
}

TEST(BadConfigFiles, AllowedMethodsTests)
{
    // ALLOWED METHODS TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_method.config"}), testing::HasSubstr(" wrong syntax in allowed_methods."));
    EXPECT_THAT(execute_server({"confs/tester/inv_method.config"}), testing::HasSubstr("Invalid method"));
}

TEST(BadConfigFiles, RedirectTests)
{
    // REDIRECT TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_redirect.config"}), testing::HasSubstr("wrong syntax in redirect line."));
    EXPECT_THAT(execute_server({"confs/tester/range_return.config"}), testing::HasSubstr("must be between 300 and 599"));
    EXPECT_THAT(execute_server({"confs/tester/no_loc_return.config"}), testing::HasSubstr("no location defined in redirect line."));
    EXPECT_THAT(execute_server({"confs/tester/extra_return.config"}), testing::HasSubstr("too many arguments when defining redirect."));
}

TEST(BadConfigFile, CgiTest)
{
    // CGI TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_cgi.config"}), testing::HasSubstr("no cgi_path defined."));
    EXPECT_THAT(execute_server({"confs/tester/extra_cgi.config"}), testing::HasSubstr("too many arguments in CGI."));
}

TEST(BadConfigFile, UploadDirTest)
{
    // UPLOAD DIRECTORY TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_upload.config"}), testing::HasSubstr("no upload directory defined."));
}

TEST(BadConfigFile, LocationTests)
{
    // LOCATION BLOCK TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/inv_location_closing.config"}), testing::HasSubstr(" invalid sintax in location line."));
    EXPECT_THAT(execute_server({"confs/tester/location/inv_location_curly_not_last_char.config"}), testing::HasSubstr(" invalid sintax in location line."));
    EXPECT_THAT(execute_server({"confs/tester/location/inv_location_multiple_opening.config"}), testing::HasSubstr(" invalid sintax in location line."));
    EXPECT_THAT(execute_server({"confs/tester/location/inv_location_no_opening.config"}), testing::HasSubstr(" invalid sintax in location line."));
    EXPECT_THAT(execute_server({"confs/tester/location/inv_keyword.config"}), testing::HasSubstr("invalid keyword in conf file."));
    EXPECT_THAT(execute_server({"confs/tester/location/inv_keyword_closing.config"}), testing::HasSubstr(" invalid keyword after '}'."));
}

TEST(BadConfigFiles, LocationRootTests)
{
   // LOCATION ROOT TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_root.config"}), testing::HasSubstr("no root defined."));
    EXPECT_THAT(execute_server({"confs/tester/location/extra_root.config"}), testing::HasSubstr("too many arguments when defining root."));
}

TEST(BadConfigFiles, LocationAllowedMethodsTests)
{
    // LOCATION ALLOWED METHODS TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_method.config"}), testing::HasSubstr(" wrong syntax in allowed_methods."));
    EXPECT_THAT(execute_server({"confs/tester/location/inv_method.config"}), testing::HasSubstr("Invalid method"));
}

TEST(BadConfigFiles, LocationAutoIndexTests)
{
    // LOCATION AUTOINDEX TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_autoindex.config"}), testing::HasSubstr("no value in autoindex defined."));
    EXPECT_THAT(execute_server({"confs/tester/location/extra_autoindex.config"}), testing::HasSubstr("too many arguments when defining autoindex."));
    EXPECT_THAT(execute_server({"confs/tester/location/invalid_autoindex.config"}), testing::HasSubstr("Invalid value for autoindex: "));
}

TEST(BadConfigFiles, LocationRedirectTests)
{
    // LOCATION REDIRECT TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_redirect.config"}), testing::HasSubstr("wrong syntax in redirect line."));
    EXPECT_THAT(execute_server({"confs/tester/location/range_return.config"}), testing::HasSubstr("must be between 300 and 599"));
    EXPECT_THAT(execute_server({"confs/tester/location/no_loc_return.config"}), testing::HasSubstr("no location defined in redirect line."));
    EXPECT_THAT(execute_server({"confs/tester/location/extra_return.config"}), testing::HasSubstr("too many arguments when defining redirect."));
}

TEST(BadConfigFiles, LocationErrorPageTests)
{
    // LOCATION ERROR PAGE TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_error_page.config"}), testing::HasSubstr("no error page defined"));
    EXPECT_THAT(execute_server({"confs/tester/location/error_page_range.config"}), testing::HasSubstr("must be between 300 and 599"));
    EXPECT_THAT(execute_server({"confs/tester/location/invalid_error_page.config"}), testing::HasSubstr("is invalid"));
}

TEST(BadConfigFiles, LocationIndexTests)
{
    // LOCATION INDEX TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_index.config"}), testing::HasSubstr(" no index defined."));
}

TEST(BadConfigFile, LocationCgiTest)
{
    // LOCATION CGI TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_cgi.config"}), testing::HasSubstr("no cgi_path defined."));
    EXPECT_THAT(execute_server({"confs/tester/location/extra_cgi.config"}), testing::HasSubstr("too many arguments in CGI."));
}

TEST(BadConfigFile, LocationUploadDirTest)
{
    // LOCATION UPLOAD DIRECTORY TESTS
    EXPECT_THAT(execute_server({"confs/tester/location/no_upload.config"}), testing::HasSubstr("no upload directory defined."));
}

// ESCREVER TESTE DE PARSING QUE DETECTA ERROS DE BIDING

class ExecuteServerFixture : public ::testing::Test {
protected:
    static int   _pipeFd[2];

    static void execute_server_fixture(const std::vector<std::string> &config_file)
    {
        pipe(_pipeFd);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return ;
        }

        if (pid == 0) {
            close(_pipeFd[0]); // fecha leitura do pipe
            dup2(_pipeFd[1], STDOUT_FILENO); // redireciona stdout para o pipe
            dup2(_pipeFd[1], STDERR_FILENO); // redireciona stderr para o pipe
            close(_pipeFd[1]);

            // Criar arquivos de log para capturar stdout e stderr
            FILE *logFile = fopen("server_output.log", "w");
            if (logFile == nullptr) {
                perror("Erro ao abrir o arquivo de log");
                _exit(1);
            }
            
            // Redirecionar stdout e stderr para o arquivo de log
            dup2(fileno(logFile), STDOUT_FILENO); // stdout para o arquivo de log
            dup2(fileno(logFile), STDERR_FILENO); // stderr para o arquivo de log
            
            std::vector<const char*> args;
            args.push_back("./Webserv");
            for (size_t i = 0; i < config_file.size(); ++i) {
                args.push_back(config_file[i].c_str());
            }
            args.push_back(nullptr);
            
            execve(args[0], const_cast<char* const*>(args.data()), environ);
            
            // Caso o execve falhe
            perror("execve falhou");
            _exit(1);
        }
        else if (pid > 0) {
            server_pid = pid;
        }

        return;
    }


    bool isServerRunning() {
       if (server_pid <= 0) {
            return false;
        }

        if (kill(server_pid, 0) == 0) {
            return true;
        } else {
            server_pid = -1;
            return false;
        }

    }

    // inicializa a fixture quando o teste suite começar
    static void SetUpTestSuite() {
        execute_server_fixture({"default.config"});
    }

    // limpa a fixture quando o teste suite terminar
    static void TearDownTestSuite() {
        if (server_pid > 0) {
            std::cout << WHITE << "[  STATUS  ] Terminating server process " << server_pid << "..." << RESET << std::endl;
            int status;

            pid_t result = waitpid(server_pid, &status, WNOHANG);
            if (result == 0){
                kill(server_pid, SIGTERM);
                waitpid(server_pid, &status, 0);
            }
            close(_pipeFd[1]);
            close(_pipeFd[0]);
        }
    }
};

int   ExecuteServerFixture::_pipeFd[2] = {-1, -1};

// Função variádica para adicionar múltiplos cabeçalhos
struct curl_slist* create_headers(int n_headers, ...) {
    struct curl_slist *headers = NULL;
    
    va_list args;
    va_start(args, n_headers);
    
    for (int i = 0; i < n_headers; ++i) {
        const char *full_header = va_arg(args, const char*);
        
        // Adiciona o cabeçalho à lista
        headers = curl_slist_append(headers, full_header);
    }
    
    va_end(args);
    return headers;
}

// // CALLBACK FUNCTION TO CAPTURE CURL RESPONSE HEADERS
// static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
//     std::string* headers = static_cast<std::string*>(userp);
//     size_t totalSize = size * nmemb;
//     headers->append(static_cast<char*>(contents), totalSize);
//     return totalSize;
// }

// CALLBACK FUNCTION TO CAPTURE CURL RESPONSE BODY
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* output = static_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

void check_resStatus(CURL* curl, long expectedStatus) {
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    if (response_code == expectedStatus) {
        std::cout << GREEN << "[       OK ] Response status: " << response_code << RESET << std::endl;
    } else {
        std::cout << RED << "[  FAILED  ] Expected status: " << expectedStatus << ", but got: " << response_code << RESET << std::endl;
    }
}

TEST_F(ExecuteServerFixture, ServerRunningTest)
{
    EXPECT_TRUE(isServerRunning());
}

TEST_F(ExecuteServerFixture, ClientSimpleRequestTest)
{
    if (isServerRunning())
        std::cout << GREEN << "[       OK ] Server is running." << RESET << std::endl;
    else{
        std::cout << RED << "[  FAILED  ] Server is not running." << RESET << std::endl;
        return;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);

    sleep(1); // Aguarde 1 segundo antes de conectar
    CURL* curl = curl_easy_init();
    if (curl) {

        struct curl_slist* headers = NULL;
        headers = create_headers(1, "Host: localhost");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/pages/index.html");
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            check_resStatus(curl, 200);
        } else {
            std::cerr << RED << "Erro: " << curl_easy_strerror(res) << RESET << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main(int ac, char **av)
{
    ::testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}
