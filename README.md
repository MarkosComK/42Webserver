
## Folder Structure

```
webserv/                        ← root of the repo
├── includes/                   ← ALL .h files go here (like in minishell)
│   ├── webserv.hpp             ← main header (includes most other headers)
│   ├──
│   ├── Server.hpp              ← Server class (listen sockets, config per server block)
│   ├── ServerManager.hpp       ← Manages multiple Server instances
│   ├──
│   ├── Client.hpp              ← Client connection state
│   ├── Request.hpp             ← Parsed HTTP request
│   ├── Response.hpp            ← Built HTTP response
│   ├── HttpStatus.hpp          ← enum / map for status codes + reason phrases
│   ├──
│   ├── Config.hpp              ← Parsed configuration (Server blocks, locations…)
│   ├── ConfigParser.hpp        ← Config file reader / parser
│   ├── Location.hpp            ← Single location block (root, index, methods, cgi…)
│   │
│   ├── Socket.hpp              ← low-level socket wrapper (optional)
│   ├── PollHandler.hpp         ← select()/poll()/kqueue() wrapper
│   ├──
│   ├── Utils.hpp               ← string split, trim, tolower, time, etc.
│   ├── Logger.hpp              ← simple logging to file/stdout with levels
│   ├── MimeTypes.hpp           ← extension → Content-Type map
│   ├──
│   ├── CgiHandler.hpp          ← CGI execution logic (fork, env, pipes)
│   └── constants.hpp           ← #defines / consts (BUFFER_SIZE, TIMEOUT…)
│
├── srcs/                       ← implementation (.cpp files) — can be split further
│   ├── main.cpp                ← entry point, creates ServerManager
│   │
│   ├── server/                 ← Person A – sockets & event loop
│   │   ├── Server.cpp
│   │   ├── ServerManager.cpp
│   │   ├── Client.cpp
│   │   ├── PollHandler.cpp     (or SelectHandler.cpp / KqueueHandler.cpp)
│   │   └── Socket.cpp
│   │
│   ├── http/                   ← Person B – parsing & config
│   │   ├── Request.cpp
│   │   ├── Config.cpp
│   │   ├── ConfigParser.cpp
│   │   ├── Location.cpp
│   │   └── HttpStatus.cpp
│   │
│   ├── response/               ← Person C – response building & serving
│   │   ├── Response.cpp
│   │   ├── CgiHandler.cpp
│   │   └── static_file_serving.cpp  (or FileHandler.cpp)
│   │
│   └── utils/                  ← shared helpers
│       ├── Utils.cpp
│       ├── Logger.cpp
│       ├── MimeTypes.cpp
│       └── string_utils.cpp
│
├── config/                     ← configuration files (nginx-like)
│   ├── webserv.conf            ← default / main config
│   ├── server1.conf            ← example with multiple servers
│   └── error_pages.conf        ← (optional – separate error pages config)
│
├── www/                        ← default website root (can be overridden in config)
│   ├── index.html
│   ├── 404.html
│   ├── styles/
│   │   └── main.css
│   ├── images/
│   └── uploads/                ← POST upload destination (if implemented)
│
├── cgi-bin/                    ← example CGI scripts (perl/python/php/…)
│   ├── hello.py
│   └── upload.php              (optional – for testing POST)
│
├── logs/                       ← runtime logs (generated)
│   └── webserv.log             (gitignore this folder or the files inside)
│
├── Makefile
├── .gitignore
├── README.md
└── subject/                    (optional – put pdf or notes here)
```

### Why this structure?

- **includes/** — central place for all headers → easy to `#include "Request.hpp"` from anywhere
- **srcs/** subfolders — lets each team member mostly work in their own area:
  - `server/` → Person A (sockets, loop, clients)
  - `http/`   → Person B (config + request parsing)
  - `response/` → Person C (response + file/CGI)
- Shared utilities in `utils/` → avoids duplication
- **config/** and **www/** — mimic real server layout (easy to test with browser)
- Scalable — later you can add more subfolders like `tests/`, `docs/`, `bonus/`

### Makefile - Base

```makefile
NAME = webserv

SRCS_DIR = srcs
OBJS_DIR = objs

INCLUDES = -I includes

# List all .cpp files (you can use wildcards or list them)
SRCS = main.cpp \
       $(wildcard $(SRCS_DIR)/server/*.cpp) \
       $(wildcard $(SRCS_DIR)/http/*.cpp) \
       $(wildcard $(SRCS_DIR)/response/*.cpp) \
       $(wildcard $(SRCS_DIR)/utils/*.cpp)

OBJS = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

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
```
