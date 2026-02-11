# Summary

- [Overview](#overview)
- [Members](#members)
- [Folder Structure](#folder-structure)
- [Why this structure?](#why-this-structure)
- [Makefile](#makefile)

## Overview

This project implements a **non-blocking HTTP/1.0 web server** from scratch in **C++98**, following the requirements of the 42 school Webserv project.

Main feature:
- Multiple listening ports / server blocks
- Static file serving (GET / HEAD)
- Basic configuration parsing (port, root, index, allowed methods)
- Proper HTTP/1.0 response formatting (status line + headers + body)
- Error handling (404, 403, 400, 500…)
- Non-blocking I/O using select() / poll()
- Clean modular structure with separate responsibilities per team member

The server aims to be RFC 1945 compliant where required, while remaining simple enough for educational purposes.

## Members

| Intra         | GitHub username   | Main responsibility                  | Contact / Discord |
|-------------|-------------------|--------------------------------------|-------------------|
| carlos-j      | @podefteza| Sockets, event loop, Client handling | todo#1234      |
| msantos     | @MarkosComK       | Request parsing, Config parsing      | todo#5678       |
| pemirand      | @pedroraposoalves| Response building, file & error serving | todo#9012   |

## Folder Structure

```
webserv/
├── includes/                   # All header files (.hpp)
│   ├── webserv.hpp
│   ├── Server.hpp
│   ├── ServerManager.hpp
│   ├── Client.hpp
│   ├── Request.hpp
│   ├── Response.hpp
│   ├── Config.hpp
│   ├── ConfigParser.hpp
│   ├── Location.hpp
│   ├── HttpStatus.hpp
│   ├── Utils.hpp
│   ├── Logger.hpp
│   ├── MimeTypes.hpp
│   ├── constants.hpp           ← #defines / consts (BUFFER_SIZE, TIMEOUT…)
│   └── ...
├── srcs/
│   ├── main.cpp
│   ├── server/                 # Sockets & I/O logic
│   ├── http/                   # Request & config parsing
│   ├── response/               # Response building & serving
│   └── utils/                  # Shared helpers
├── config/
│   └── webserv.conf
├── www/                        # Default web root
│   ├── index.html
│   └── ...
├── logs/
├── Makefile
└── README.md
```

### Why this structure?

- **includes/** — central place for all headers → easy to `#include "Request.hpp"` from anywhere
- **srcs/** subfolders — clear separation of concerns:
  - `server/` → connection & event loop (Person A)
  - `http/`   → parsing & configuration (Person B)
  - `response/` → response generation & file/CGI serving (Person C)
- Shared utilities in `utils/` → no code duplication
- **config/** and **www/** — mimic real server layout (easy browser testing)
- Scalable and git-friendly

### Makefile

```makefile
NAME = webserv

SRCS_DIR = srcs
OBJS_DIR = objs

INCLUDES = -I includes

SRCS = main.cpp \
       $(wildcard $(SRCS_DIR)/server/*.cpp) \
       $(wildcard $(SRCS_DIR)/http/*.cpp) \
       $(wildcard $(SRCS_DIR)/response/*.cpp) \
       $(wildcard $(SRCS_DIR)/utils/*.cpp)

OBJS = $$   (SRCS:   $$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
