NAME= Webserv

TESTERNAME= runTests

CXX= c++

CXXFLAGS= -Wall -Wextra -Werror -g -std=c++98 -O1 -fsanitize=address -fno-omit-frame-pointer

CXXTESTERFLAGS = -Wall -Wextra -Werror -g -std=c++17 -pthread

TESTERLINKS = -lgtest -lgmock -lcurl

PURPLE = \033[1;35m
CYAN = \033[1;36m
UNDERLINED_PURPLE = \033[4;35m
RESET = \033[0m

SRC = main.cpp src/Socket.cpp src/HttpRequest.cpp \
	   src/WebServer.cpp src/Client.cpp src/Server.cpp \
	   src/LocationBlock.cpp src/Configuration.cpp \
	   src/utils.cpp  src/HttpResponse.cpp \
	   src/ErrorPageRule.cpp src/Block.cpp \
	   src/SessionData.cpp

OBJ_DIR = obj

OBJ = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
		@echo "$(PURPLE)$(NAME) compiled successfully.$(RESET)"

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
		@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: src/%.cpp
		@mkdir -p $(@D)
		@echo "$(PURPLE)Compiling $(UNDERLINED_PURPLE)$<$(RESET)"
		@$(CXX) $(CXXFLAGS) -c $< -o $@

#$(OBJ_SRC_DIR)/%.o: %.cpp
#	mkdir -p $(dir $@)
#	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
		@echo "$(PURPLE)Cleaning up...$(RESET)"
		@rm -f $(OBJ)
		@if [ -d "$(OBJ_DIR)" ]; then rm -rf $(OBJ_DIR); fi
		@echo "$(PURPLE)Clean complete!$(RESET)"

fclean: clean
		@echo "$(PURPLE)Removing  \033[9m$(NAME)$(RESET)"
		@rm -f $(NAME)
		@echo "$(PURPLE)Removing  \033[9m$(TESTERNAME)$(RESET)"
		@rm -f $(TESTERNAME) server_output.log cgi_debug.log
		@echo "$(PURPLE)Removing complete!$(RESET)"

re: fclean all

tester: re
	cd tester
	$(CXX) $(CXXTESTERFLAGS) tester/googleTester.cpp $(TESTERLINKS) -o $(TESTERNAME)
	./$(TESTERNAME)

.PHONY: all clean fclean re