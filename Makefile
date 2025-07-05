# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/10 19:46:12 by ncampbel          #+#    #+#              #
#    Updated: 2025/07/05 16:32:25 by ncampbel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME= Webserv

CXX= c++

CXXFLAGS= -Wall -Wextra -Werror -g -std=c++98

SRC = main.cpp src/HttpServer.cpp src/Socket.cpp src/HttpRequest.cpp \
	   src/WebServer.cpp src/Client.cpp src/Server.cpp

TIMENAME= timeexec

TIME = time/main.cpp

TIMEOBJ = $(TIME:.cpp=.o)

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ) $(TIMEOBJ)
	
fclean: clean
	rm -f $(NAME) $(TIMENAME)

re: fclean all

time: $(TIMENAME)

$(TIMENAME): $(TIMEOBJ)
	$(CXX) $(CXXFLAGS) $(TIMEOBJ) -o $(TIMENAME)

.PHONY: all clean fclean re