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

struct ServerConfig {
	std::string host;        // IP to bind, default "0.0.0.0"
	int port;
	std::string serverName;
	std::map<int, std::string> errorPages; // e.g. 404 -> "/errors/404.html"
	size_t clientMaxBodySize;
	std::vector<Location> locations;

	ServerConfig() : host("0.0.0.0"), port(0), clientMaxBodySize(1024 * 1024) {}
};

#endif
