#include "Utils.hpp"
#include <sstream>
#include <vector>
#include <cstdio>
#include <cctype>
#include <cstdlib>

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

bool isMethodAllowedForLocation(const std::string &method, const std::string &requestPath, const ServerConfig &config) {
	const Location *loc = matchLocation(requestPath, config);
	if (!loc || loc->allowedMethods.empty())
		return true;
	for (size_t i = 0; i < loc->allowedMethods.size(); ++i) {
		if (loc->allowedMethods[i] == method)
			return true;
	}
	return false;
}

long getContentLengthHeader(const std::string &rawRequest) {
	size_t firstLineEnd = rawRequest.find("\r\n");
	size_t headersEnd = rawRequest.find("\r\n\r\n");
	if (firstLineEnd == std::string::npos || headersEnd == std::string::npos || headersEnd <= firstLineEnd)
		return 0;

	std::string headersBlock = rawRequest.substr(firstLineEnd + 2, headersEnd - (firstLineEnd + 2));
	std::vector<std::string> lines = ftSplit(headersBlock, "\r\n");

	for (size_t i = 0; i < lines.size(); ++i) {
		size_t colon = lines[i].find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = ftTrim(lines[i].substr(0, colon));
		for (size_t j = 0; j < key.size(); ++j)
			key[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[j])));
		if (key != "content-length")
			continue;

		std::string value = ftTrim(lines[i].substr(colon + 1));
		if (value.empty())
			return 0;
		for (size_t j = 0; j < value.size(); ++j) {
			if (value[j] < '0' || value[j] > '9')
				return 0;
		}
		return std::strtol(value.c_str(), NULL, 10);
	}
	return 0;
}
