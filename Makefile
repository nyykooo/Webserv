NAME= Webserv

TESTERNAME= runTests

CXX= c++

CXXFLAGS= -Wall -Wextra -Werror -g -std=c++98 -O1 -fsanitize=address -fno-omit-frame-pointer

CXXTESTERFLAGS = -Wall -Wextra -Werror -g -std=c++17 -pthread

TESTERLINKS = -lgtest -lgmock -lcurl

SRC = main.cpp src/Socket.cpp src/HttpRequest.cpp \
	   src/WebServer.cpp src/Client.cpp src/Server.cpp \
	   src/LocationBlock.cpp src/Configuration.cpp \
	   src/utils.cpp  src/HttpResponse.cpp \
	   src/ErrorPageRule.cpp src/Block.cpp \
	   src/SessionData.cpp

OBJ_SRC_DIR = ./obj

OBJ = $(SRC:%.cpp=$(OBJ_SRC_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_SRC_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rfd $(OBJ_SRC_DIR)
	
fclean: clean
	rm -f $(NAME) $(TESTERNAME) server_output.log cgi_output.log

re: fclean all

tester: re
	cd tester
	$(CXX) $(CXXTESTERFLAGS) tester/googleTester.cpp $(TESTERLINKS) -o $(TESTERNAME)
	./$(TESTERNAME)

.PHONY: all clean fclean re