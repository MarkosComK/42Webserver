NAME = webserv

# Recursive wildcard function for flexibility
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Automatically find ALL .cpp files in main directory and srcs/
SRCS = main.cpp $(call rwildcard,srcs,*.cpp)

HEADERS	= includes/Socket.hpp includes/Client.hpp includes/Request.hpp includes/Config.hpp includes/ConfigParser.hpp includes/Utils.hpp includes/webserv.hpp

OBJS = $(SRCS:.cpp=.o)

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes
CXXFLAGS += -g3

RM = rm -rf

RESET = "\033[0m"
BLACK = "\033[1m\033[37m"

all:
	@$(MAKE) $(NAME) -j5
$(NAME) : $(OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo $(BLACK)-webserv compiled 🌐 $(RESET)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: 	fclean all

.PHONY: all clean fclean re
