// exemplo_simple_concorrente_test.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

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
    EXPECT_THAT(execute_server({"confs/tester/unclosed_server.config"}), testing::HasSubstr("server block not closed."));
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

int main(int ac, char **av)
{
    ::testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}
