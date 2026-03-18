#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// maybe we should find a better way to do this, but for now we can just include all the headers here
#define STATUS200 "200 OK"
#define STATUS201 "201 Created"
#define STATUS202 "202 Accepted"
#define STATUS204 "204 No Content"
#define STATUS301 "301 Moved Permanently"
#define STATUS302 "302 Moved Temporarily" // or "Found" ?
#define STATUS304 "304 Not Modified"
#define STATUS400 "400 Bad Request"
#define STATUS401 "401 Unauthorized"
#define STATUS403 "403 Forbidden"
#define STATUS404 "404 Not Found"
#define STATUS405 "405 Method Not Allowed"
#define STATUS411 "411 Length Required"
#define STATUS413 "413 Payload Too Large"
#define STATUS500 "500 Internal Server Error"
#define STATUS501 "501 Not Implemented"
#define STATUS505 "505 HTTP Version Not Supported"
#define STATUS502 "502 Bad Gateway"
#define STATUS503 "503 Service Unavailable"

#include "Config.hpp"
#include "Request.hpp"
#include "Utils.hpp"
#include <iostream>

#endif
