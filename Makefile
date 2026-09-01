
NAME = ft_irc
CXX = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes -g3 -MMD -MP
SRCS =	src/Server.cpp \
		src/Server_network.cpp \
		src/Server_messages.cpp \
		src/Server_state.cpp \
		src/Client.cpp \
		src/Channel.cpp \
		src/Message.cpp \
		src/CommandHandler.cpp \
		src/CommandHandler_auth.cpp \
		src/CommandHandler_chan.cpp \
		src/CommandHandler_msg.cpp \
		src/CommandHandler_info.cpp \
		main.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

GREEN = \033[0;32m
DEFAULT = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created!$(DEFAULT)"

%.o: %.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(DEPS)
	@echo "$(GREEN)$(NAME) obj files deleted!$(DEFAULT)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(GREEN)$(NAME) all files deleted!$(DEFAULT)"

re: fclean all

run: all
	@./$(NAME)

-include $(DEPS)

.PHONY: all clean fclean re
