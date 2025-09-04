# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/10 19:46:12 by ncampbel          #+#    #+#              #
#    Updated: 2025/09/04 15:21:41 by ncampbel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME= Webserv

CXX= c++

CXXFLAGS= -Wall -Wextra -Werror -g -std=c++98

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
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rfd $(OBJ_SRC_DIR)
	
fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re