#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> ftSplit(const std::string &s, const std::string &delimiter);
std::string ftTrim(const std::string &s);
std::vector<std::string> splitByWhitespace(const std::string &s);

#endif
