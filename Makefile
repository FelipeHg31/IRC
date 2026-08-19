# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lde-medi <lde-medi@student.42madrid.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/08/15 15:48:49 by juan-her          #+#    #+#              #
#    Updated: 2026/08/19 21:35:58 by lde-medi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_irc
CXX = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes -g3
SRCS =	src/Server.cpp src/Client.cpp src/Channel.cpp src/Command.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)

GREEN = \033[0;32m
DEFAULT = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created!$(DEFAULT)"

%.o: %.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
	@echo "$(GREEN)$(NAME) obj files deleted!$(DEFAULT)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(GREEN)$(NAME) all files deleted!$(DEFAULT)"

re: fclean all

run: all
	@./$(NAME)

.PHONY: all clean fclean re
