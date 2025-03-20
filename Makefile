.SILENT:

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -g -std=c++98

SRC = src/ircserv.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Channel.cpp \
		src/Command.cpp \
		src/Tools.cpp \
		src/Bot.cpp \

OBJ = $(SRC:.cpp=.o)

NAME = ircserv

all: $(NAME)

$(NAME): $(OBJ)
		$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
		echo "\033[32m$(NAME) compiled\033[0m"

%.o: %.cpp
		@echo "\033[34mCompiling $<...\033[0m"
		$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
		rm -f $(OBJ)
		echo "\033[31m$(NAME) object files removed\033[0m"

fclean: clean
		rm -f $(NAME)
		echo "\033[31m$(NAME) removed\033[0m"

re: fclean all

.PHONY: all clean fclean re