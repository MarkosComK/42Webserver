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

std::string ftTrim(const std::string &s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

// Longest prefix match — same rules as nginx.
// Returns the Location whose path is the longest prefix of the request path.
// Returns NULL if no location matches (shouldn't happen if config has a "/" catch-all).
const Location *matchLocation(const std::string &path, const ServerConfig &server) {
	const Location *best = NULL;
	size_t bestLen = 0;

	for (size_t i = 0; i < server.locations.size(); ++i) {
		const Location &loc = server.locations[i];
		const std::string &lpath = loc.path;

		if (path.size() < lpath.size())
			continue;
		if (path.substr(0, lpath.size()) != lpath)
			continue;
		// after the prefix, must be end-of-string or '/' to avoid /foobaz matching /foo
		if (path.size() > lpath.size() && lpath[lpath.size() - 1] != '/' && path[lpath.size()] != '/')
			continue;

		if (lpath.size() > bestLen) {
			bestLen = lpath.size();
			best = &loc;
		}
	}
	return best;
}
