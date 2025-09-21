// exemplo_simple_concorrente_test.cpp
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <gmock/gmock.h>

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
        // redirecionar stdout e stderr para o pipe
        // dup2(pipefd[1], STDOUT_FILENO); // redireciona stdout para o pipe
        dup2(pipefd[1], STDERR_FILENO); // redireciona stdcerr para o pipe
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

    // --- Processo pai ---
    close(pipefd[1]); // fecha escrita

    // ler stderr do filho
    std::string output;
    char buffer[256];
    ssize_t n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, n);
    }
    close(pipefd[0]);

    // Timeout para o processo filho
    int status;
    // std::this_thread::sleep_for(std::chrono::seconds(5)); // Aguarde 5 segundos (ou ajuste conforme necessário)

    // Verifique se o processo filho ainda está rodando
    int done = waitpid(pid, &status, WNOHANG);
    if (done == 0) {
        std::cout << "Processo filho ainda está rodando. Tentando encerrar manualmente..." << std::endl;

        // Envie um sinal para encerrar o processo filho
        kill(pid, SIGTERM); // Tente encerrar o processo filho graciosamente
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Aguarde um pouco para o processo encerrar

        // Verifique novamente se o processo ainda está rodando
        done = waitpid(pid, &status, WNOHANG);
        if (done == 0) {
            std::cout << "Processo filho não respondeu ao SIGTERM. Forçando encerramento..." << std::endl;
            kill(pid, SIGKILL); // Força o término do processo filho
        }
    }

    // Garanta que o processo filho seja coletado
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return output; // Processo filho terminou normalmente
    } else if (WIFSIGNALED(status)) {
        std::cerr << "Processo filho foi terminado por um sinal: " << WTERMSIG(status) << std::endl;
        return "";
    } else {
        std::cerr << "Processo filho não terminou corretamente." << std::endl;
        return "";
    }
}

// first test case will try to execute the server with invalid config files
// the expect result are fails and aligned to the error handling of the server
// not need of a fixture yet because is a single test case and later when the server will be active we can use the fixture class to create clients
// the test will try to execute the server in threads with bad config files and check the return value or the error message
// as we will check the return message we will use EXPECT_THAT and ASSERT_THAT macros
TEST(ExecuteServer, BadConfigFiles)
{
    // INVALID CONFIG FILE
    EXPECT_THAT(execute_server({"nonexistent.config"}), testing::HasSubstr("Error opening file: "));
    EXPECT_THAT(execute_server({"default.config", "pipipi"}), testing::HasSubstr("Wrong input. Usage: ./webserv [configuration_file]"));
    EXPECT_THAT(execute_server({"confs/tester/bad_curly.config"}), testing::HasSubstr("Invalid configuration file: invalid sintax."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_keyword_conf.config"}), testing::HasSubstr("invalid keyword in conf file."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_keyword_server.config"}), testing::HasSubstr("invalid keyword in server block."));
    EXPECT_THAT(execute_server({"confs/tester/wrong_curly_pos_open.config"}), testing::HasSubstr("line shouldn't have \'{\'"));
    EXPECT_THAT(execute_server({"confs/tester/wrong_curly_pos_close.config"}), testing::HasSubstr("} should be at the end of the line."));
    // HOST TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_host.config"}), testing::HasSubstr(" no host mentioned."));
    EXPECT_THAT(execute_server({"confs/tester/invalid_host.config"}), testing::HasSubstr("invalid host/port"));
    EXPECT_THAT(execute_server({"confs/tester/inv_port.config"}), testing::HasSubstr("invalid port number: "));
    EXPECT_THAT(execute_server({"confs/tester/inv_ip.config"}), testing::HasSubstr("invalid host: "));
    // SERVER NAME TESTS
    EXPECT_THAT(execute_server({"confs/tester/no_servername.config"}), testing::HasSubstr(" no server_name defined"));
}

int main(int ac, char **av)
{
    ::testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}