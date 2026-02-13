#include "Utils.hpp"
#include <sstream>
#include <vector>

std::vector<std::string> ftSplit(const std::string &s, const std::string &delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	size_t start = 0;
	size_t end = 0;

	while ((end = s.find(delimiter, start)) != std::string::npos) {
		token = s.substr(start, end - start);
		tokens.push_back(token); // push_back adds the token to the end of the vector
		start = end + delimiter.length();
	}
	tokens.push_back(s.substr(start));

	return tokens;
}

// for config files (cases like "listen    8080;")
std::vector<std::string> splitByWhitespace(const std::string &s) {
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream ss(s);

	// stream extraction operator >> skips whitespace
	while (ss >> token) {
		tokens.push_back(token);
	}

	return tokens;
}
