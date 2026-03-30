# WebServ Project TODO

This document tracks the progress of the 42 WebServ project, outlining completed features and remaining tasks.

---

## ✅ What is Already Implemented

### 1. Sockets & Event Loop (`Server.cpp`, `Socket.cpp`)
*   **Non-blocking I/O:** Sockets are correctly set to non-blocking using `fcntl` and `O_NONBLOCK`.
*   **Multiplexing:** The server successfully uses `poll()` to handle multiple client connections and listening sockets without blocking.
*   **Multiple Ports:** The server can listen on multiple `host:port` combinations as defined in the configuration file.

### 2. Configuration Parsing (`ConfigParser.cpp`)
*   **Custom Config Format:** A robust parser is in place that tokenizes and extracts `server` and `location` blocks from the configuration file.
*   **Directives:** The parser successfully handles `listen`, `root`, `error_page`, `client_max_body_size`, `methods`, `index`, `autoindex`, `upload_store`, `return` (for redirects), and `cgi`.
*   **Size Parsing:** A utility function correctly parses size values with units (e.g., `10M`, `512K`) for directives like `client_max_body_size`.
*   **Root Inheritance:** The `root` directive set in a `server` block is correctly inherited by `location` blocks that do not define their own `root`.

### 3. Request Handling (`Request.cpp`, `Server.cpp`)
*   **Method Parsing:** Validates `GET`, `HEAD`, `POST`, and `DELETE` methods.
*   **Header Parsing:** Extracts headers into a `std::map` and correctly converts keys to lowercase for case-insensitive lookups.
*   **Body Processing:** Reads the request body based on the `Content-Length` header.
*   **Early Request Rejection:** The server efficiently checks `client_max_body_size` and `methods` allowed for a location *before* reading the entire request body, allowing it to quickly reject invalid requests with `413 (Payload Too Large)` or `405 (Method Not Allowed)`.

### 4. Response & Static Serving
*   **Static Files:** Serves static files with basic MIME type handling.
*   **Directory Listing:** Generates an HTML directory listing when `autoindex` is enabled for a location.
*   **Error Pages:** Supports custom error pages defined in the configuration, with a fallback to default error messages.
*   **Redirections:** Implements HTTP 301/302 redirects based on the `return` directive.
*   **File Operations:** The `POST` method can create files in the specified `upload_store`, and `DELETE` can remove files.

---

## 🚀 What Needs to be Done (Missing Requirements)

### 1. CGI Execution (High Priority)
While the `ConfigParser` reads `cgi` directives, there is no logic to execute them.
*   **Task:** Implement CGI script execution.
*   **Requirements:**
    *   Use `fork()` to create a child process.
    *   Use `execve()` to run the CGI script (e.g., a PHP or Python interpreter).
    *   Use `pipe()` to pass the request body to the script's `stdin` and read the generated response from its `stdout`.
    *   **Crucially, this must not block the main server loop.** The pipes for CGI I/O must be added to the `poll()` set to be handled asynchronously.
    *   Set all required CGI meta-variables (e.g., `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`, `PATH_INFO`) in the environment for the child process.

### 2. Chunked Transfer Encoding
*   **Issue:** The request parser does not support `Transfer-Encoding: chunked`, as noted by the `TODO` in `Request.cpp`. This is essential for handling requests where the total size is not known in advance.
*   **Fix:** Implement a parser state machine in `clientRead_` or `Request.cpp` to handle chunked-encoded bodies. This involves reading a hex-formatted size, followed by the data chunk, and repeating until a zero-size chunk (`0\r\n\r\n`) is received.

### 3. URI Decoding ✅ DONE
*   **Issue:** The server does not decode percent-encoded characters in the request URI (e.g., `%20` for a space), as noted in the `TODO` in `Request.cpp`.
*   **Fix:** Create and apply a utility function to decode the `%XX` hex sequences in the request path and query string before using them for file lookups or CGI execution.

### 4. HTTP/1.1 Connection Management
*   **Issue:** The server currently closes the connection after every response (`Connection: close`). While simple, the subject may require supporting persistent connections.
*   **Fix:** Investigate and implement support for `Connection: keep-alive`. This would involve not closing the client socket after a response is sent, but rather resetting its state and waiting for a new request on the same `fd`. This also requires correctly handling timeouts for idle keep-alive connections.

### 5. Implement client_max_body_size inside server location in the config file ✅ DONE --> test it!

### 6. Add a "visitors" board that lets users submit their name via a POST and show a wall of visitors, with the results of a POST saved in a CVS (is that even possible?)

---

## ✨ Bonus / Optional Features

### 1. Virtual Hosting (`server_name`)
The subject explicitly mentions that virtual hosting is out of scope but allowed. If implemented, the server must be able to host multiple "websites" on the same `host:port` and route requests to the correct one based on the `Host` header.
*   **Issue:** The current data structure `std::map<Socket*, const ServerConfig*>` only allows one server configuration per listening socket.
*   **Fix:**
    1.  **Update Config Parser:** Add support for parsing the `server_name` directive in `ConfigParser.cpp`. A server block can have multiple server names.
    2.  **Modify Data Structures:** In `Server.hpp`, change the mapping to associate a listening socket `fd` with a **vector** of server configurations. For example: `std::map<int, std::vector<const ServerConfig*> > listen_fd_to_configs_;`.
    3.  **Implement Routing Logic:** In `Server::clientRead_`, after receiving the request headers, parse the `Host` header. Use this hostname to find the correct `ServerConfig` from the vector associated with that client's listening socket. If no `server_name` matches, the first server block for that `host:port` should be used as the default.