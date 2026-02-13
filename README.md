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
| carlos-j      | @podefteza| Request parsing, Config parsing | todo#1234      |
| msantos     | @MarkosComK       | Response building, file & error serving | todo#5678       |
| pemirand      | @pedroraposoalves| Sockets, event loop, Client handling | todo#9012   |

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

Sup fellas, msantos here. I've made a structure in which we can use `#include <webserv.hpp>`. So, instead of this spaguetti that people do having a lot of includes split amongs .cpp files we can use a single `#include <webserv.hpp>` over the whole project and whenever a dependency (new .hpp) file is missing, we just need to add to `webserv.hpp` and this dependency will be available over the whole project. Let's go!

In sum:
- **includes/** — central place for all headers → easy to `#include "Request.hpp"` from anywhere
- **srcs/** subfolders — clear separation of concerns:
  - `server/` → connection & event loop (Person A)
  - `http/`   → parsing & configuration (Person B)
  - `response/` → response generation & file/CGI serving (Person C)
- Shared utilities in `utils/` → no code duplication
- **config/** and **www/** — mimic real server layout (easy browser testing)
- Scalable and git-friendly

### Makefile

The makefile is based on the rule above. At the beginning we can include wildcards to avoid manually typing every new .cpp/.hpp file to it. We worry about coding which is what matters more now.
