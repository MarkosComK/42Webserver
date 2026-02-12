NAME		= webserv

# Recursive wildcard function for flexibility. LLM MADE
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Automatically find ALL .cpp files anywhere under srcs/
SRCS		= $(call rwildcard,srcs,*.cpp)

OBJS		= $(patsubst srcs/%.cpp,objs/%.o,$(SRCS))

CXX		= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -I./includes

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# objs/ folder for my friends that uses vscode. [neo]Vim is better btw.
objs/%.o: srcs/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
