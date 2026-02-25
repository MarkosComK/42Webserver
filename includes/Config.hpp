#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

// struct of locations defined in the server block of the config file
struct Location {
	std::string path;
	std::string root;
	std::string index;
	std::vector<std::string> allowedMethods;
	std::string uploadDir;
	std::string redirect;    // redirect target URL, empty if none
	int redirectCode;        // e.g. 301, 302; 0 if not a redirect
	bool autoindex;
	std::map<std::string, std::string> cgi; // e.g. ".php" -> "/usr/bin/php-cgi"

	Location() : redirectCode(0), autoindex(false) {}
};

struct ListenPair {
	std::string host;
	int port;

	ListenPair(const std::string &h, int p) : host(h), port(p) {}
};

struct ServerConfig {
	std::vector<ListenPair> listens;
	std::string root; // TODO: create a default root
	std::string serverName;
	std::map<int, std::string> errorPages; // e.g. 404 -> "/errors/404.html"
	size_t clientMaxBodySize;
	std::vector<Location> locations;

	ServerConfig() : clientMaxBodySize(1024 * 1024) {}
};

#endif
