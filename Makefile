NAME= Webserv

TESTERNAME= runTests

CXX= c++

CXXFLAGS= -Wall -Wextra -Werror -g -std=c++98 #-O1 -fsanitize=address -fno-omit-frame-pointer

CXXTESTERFLAGS = -Wall -Wextra -Werror -g -std=c++17 -pthread

TESTERLINKS = -lgtest -lgmock -lcurl

SRC_DIR = src
INC_DIR = includes
OBJ_DIR = obj

PURPLE = \033[1;35m
CYAN = \033[1;36m
UNDERLINED_PURPLE = \033[4;35m
RESET = \033[0m

SRC_FILES = main.cpp \
           core/WebServer.cpp core/Server.cpp core/Socket.cpp \
           http/HttpRequest.cpp http/HttpResponse.cpp http/ErrorPages.cpp \
           config/Configuration.cpp config/LocationBlock.cpp config/Block.cpp config/ErrorPageRule.cpp \
           client/Client.cpp client/SessionData.cpp \
           utils/utils.cpp

SRC = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

INC = -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJ)
		@$(CXX) $(CXXFLAGS) $^ -o $@
		@echo "$(PURPLE)$(NAME) compiled successfully.$(RESET)"

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@echo "$(PURPLE)Compiling $(UNDERLINED_PURPLE)$<$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	@echo "$(PURPLE)Cleaning up...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@echo "$(PURPLE)Clean complete!$(RESET)"

fclean: clean
	@echo "$(PURPLE)Removing  \033[9m$(NAME)$(RESET)"
	@rm -f $(NAME)
	@echo "$(PURPLE)Removing  \033[9m$(TESTERNAME)$(RESET)"
	@rm -f $(TESTERNAME) server_output.log cgi_debug.log
	@echo "$(PURPLE)Removing complete!$(RESET)"

re: fclean all

tester: re
	$(CXX) $(CXXTESTERFLAGS) tester/googleTester.cpp $(TESTERLINKS) -o $(TESTERNAME)
	./$(TESTERNAME)

.PHONY: all clean fclean re