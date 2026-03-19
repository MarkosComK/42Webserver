#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include "Config.hpp"

std::vector<std::string> ftSplit(const std::string &s, const std::string &delimiter);
std::string ftTrim(const std::string &s);
std::vector<std::string> splitByWhitespace(const std::string &s);
std::string itoa_int(int n);
const Location *matchLocation(const std::string &path, const ServerConfig &server);
bool isMethodAllowedForLocation(const std::string &method, const std::string &requestPath, const ServerConfig &config);
long getContentLengthHeader(const std::string &rawRequest);



#endif
