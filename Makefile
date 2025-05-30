# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ncampbel <ncampbel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/10 19:46:12 by ncampbel          #+#    #+#              #
#    Updated: 2025/05/28 17:24:12 by ncampbel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME= Webserv

CXX= c++

CXXFLAGS= -Wall -Wextra -Werror -g -std=c++98

SRC = main.cpp src/HTTPServer.cpp src/Socket.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)
	
fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re