#include "ConfigParser.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

// tester to read the config file and print the parsed server configs
// what is the expected output if there's no config file (argv[1] is empty or file doesn't exist)? error or load default values??
// delete later...
void testConfigParsing(const std::string &configPath) {
	std::cout << "Parsing config file: " << configPath << "\n" << std::endl;
	try {
		ConfigParser parser(configPath);
		const std::vector<ServerConfig> &servers = parser.getServers();

		std::cout << "Found " << servers.size() << " server block(s)\n" << std::endl;

		for (size_t s = 0; s < servers.size(); ++s) {
			const ServerConfig &srv = servers[s];
			std::cout << "=== Server " << s + 1 << " ==="Should it print an error message and exit, or should it use default values?  << std::endl;
			std::cout << "  host:               " << srv.host << std::endl;
			std::cout << "  port:               " << srv.port << std::endl;
			std::cout << "  server_name:        " << (srv.serverName.empty() ? "(none)" : srv.serverName) << std::endl;
			std::cout << "  client_max_body:    " << srv.clientMaxBodySize << " bytes" << std::endl;

			if (!srv.errorPages.empty()) {
				std::cout << "  error_pages:" << std::endl;
				for (std::map<int, std::string>::const_iterator it = srv.errorPages.begin(); it != srv.errorPages.end(); ++it)
					std::cout << "    " << it->first << " -> " << it->second << std::endl;
			}

			std::cout << "  locations (" << srv.locations.size() << "):" << std::endl;
			for (size_t l = 0; l < srv.locations.size(); ++l) {
				const Location &loc = srv.locations[l];
				std::cout << "    --- location " << loc.path << " ---" << std::endl;

				if (!loc.allowedMethods.empty()) {
					std::cout << "      methods:      ";
					for (size_t m = 0; m < loc.allowedMethods.size(); ++m)
						std::cout << loc.allowedMethods[m] << (m + 1 < loc.allowedMethods.size() ? " " : "");
					std::cout << std::endl;
				}
				if (!loc.root.empty())
					std::cout << "      root:         " << loc.root << std::endl;
				if (!loc.index.empty())
					std::cout << "      index:        " << loc.index << std::endl;
				std::cout << "      autoindex:     " << (loc.autoindex ? "on" : "off") << std::endl;
				if (!loc.uploadDir.empty())
					std::cout << "      upload_store: " << loc.uploadDir << std::endl;
				if (loc.redirectCode != 0)
					std::cout << "      return:       " << loc.redirectCode << " " << loc.redirect << std::endl;
				if (!loc.cgi.empty()) {
					std::cout << "      cgi:" << std::endl;
					for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it)
						std::cout << "        " << it->first << " -> " << it->second << std::endl;
				}
			}
			std::cout << std::endl;
		}
	} catch (const std::exception &e) {
		std::cerr << "[PARSE ERROR] " << e.what() << std::endl;
	}
}

ConfigParser::ConfigParser(const std::string &filePath) {
	std::string content = readFile(filePath);
	std::vector<std::string> tokens = tokenize(content);
	parse(tokens);
}

const std::vector<ServerConfig> &ConfigParser::getServers() const {
	return _servers;
}

ConfigParser::ParseException::ParseException(const std::string &msg) : _msg(msg) {
}

ConfigParser::ParseException::~ParseException() throw() {
}

const char *ConfigParser::ParseException::what() const throw() {
	return _msg.c_str();
}

std::string ConfigParser::readFile(const std::string &filePath) {
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		throw ParseException("Cannot open config file: " + filePath);
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

std::vector<std::string> ConfigParser::tokenize(const std::string &content) {
	std::vector<std::string> tokens;
	std::string current;

	for (size_t i = 0; i < content.size(); ++i) {
		char c = content[i];

		if (c == '#') { // skip comment until end of line
			while (i < content.size() && content[i] != '\n')
				++i;
			continue;
		}
		if (c == '{' || c == '}' || c == ';') {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
			tokens.push_back(std::string(1, c));
		} else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}
	if (!current.empty())
		tokens.push_back(current);
	return tokens;
}

void ConfigParser::parse(std::vector<std::string> &tokens) {
	size_t i = 0;
	while (i < tokens.size()) {
		if (tokens[i] == "server") {
			++i;
			if (i >= tokens.size() || tokens[i] != "{")
				throw ParseException("Expected '{' after 'server'");
			++i;
			_servers.push_back(parseServerBlock(tokens, i));
		} else {
			throw ParseException("Unexpected token outside server block: " + tokens[i]);
		}
	}
	if (_servers.empty())
		throw ParseException("Config file has no server blocks");
}

ServerConfig ConfigParser::parseServerBlock(std::vector<std::string> &tokens, size_t &i) {
	ServerConfig server;

	while (i < tokens.size() && tokens[i] != "}") {
		const std::string &directive = tokens[i];

		if (directive == "listen") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'listen' requires a value");
			parseHost(tokens[i], server);
			++i;
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'listen' value");
			++i;

		} else if (directive == "server_name") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'server_name' requires a value");
			server.serverName = tokens[i++];
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'server_name' value");
			++i;

		} else if (directive == "error_page") {
			++i;
			if (i + 1 >= tokens.size())
				throw ParseException("'error_page' requires a code and a path");
			int code = std::atoi(tokens[i].c_str());
			if (code <= 0)
				throw ParseException("Invalid error_page code: " + tokens[i]);
			++i;
			server.errorPages[code] = tokens[i++];
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'error_page' value");
			++i;

		} else if (directive == "client_max_body_size") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'client_max_body_size' requires a value");
			server.clientMaxBodySize = parseSize(tokens[i++]);
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'client_max_body_size' value");
			++i;

		} else if (directive == "location") {
			++i;
			if (i >= tokens.size())
				throw ParseException("'location' requires a path");
			std::string path = tokens[i++];
			if (i >= tokens.size() || tokens[i] != "{")
				throw ParseException("Expected '{' after location path '" + path + "'");
			++i;
			Location loc = parseLocationBlock(tokens, i);
			loc.path = path;
			server.locations.push_back(loc);

		} else {
			throw ParseException("Unknown server directive: " + directive);
		}
	}

	if (i >= tokens.size())
		throw ParseException("Unexpected end of file in server block");
	++i; // consume '}'

	if (server.port == 0)
		throw ParseException("Server block is missing 'listen' directive");
	return server;
}

Location ConfigParser::parseLocationBlock(std::vector<std::string> &tokens, size_t &i) {
	Location loc;

	while (i < tokens.size() && tokens[i] != "}") {
		const std::string &directive = tokens[i];

		if (directive == "methods") {
			++i;
			while (i < tokens.size() && tokens[i] != ";")
				loc.allowedMethods.push_back(tokens[i++]);
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'methods' values");
			++i;

		} else if (directive == "root") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'root' requires a value");
			loc.root = tokens[i++];
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'root' value");
			++i;

		} else if (directive == "index") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'index' requires a value");
			loc.index = tokens[i++];
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'index' value");
			++i;

		} else if (directive == "autoindex") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'autoindex' requires 'on' or 'off'");
			const std::string &val = tokens[i++];
			if (val == "on")
				loc.autoindex = true;
			else if (val == "off")
				loc.autoindex = false;
			else
				throw ParseException("'autoindex' value must be 'on' or 'off', got: " + val);
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'autoindex' value");
			++i;

		} else if (directive == "upload_store") {
			++i;
			if (i >= tokens.size() || tokens[i] == ";")
				throw ParseException("'upload_store' requires a value");
			loc.uploadDir = tokens[i++];
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'upload_store' value");
			++i;

		} else if (directive == "return") {
			++i;
			if (i + 1 >= tokens.size())
				throw ParseException("'return' requires a code and a URL");
			loc.redirectCode = std::atoi(tokens[i].c_str());
			if (loc.redirectCode <= 0)
				throw ParseException("Invalid redirect code: " + tokens[i]);
			++i;
			loc.redirect = tokens[i++];
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'return' value");
			++i;

		} else if (directive == "cgi") {
			++i;
			if (i + 1 >= tokens.size())
				throw ParseException("'cgi' requires an extension and an interpreter path");
			std::string ext = tokens[i++];
			std::string interpreter = tokens[i++];
			loc.cgi[ext] = interpreter;
			if (i >= tokens.size() || tokens[i] != ";")
				throw ParseException("Expected ';' after 'cgi' value");
			++i;

		} else {
			throw ParseException("Unknown location directive: " + directive);
		}
	}

	if (i >= tokens.size())
		throw ParseException("Unexpected end of file in location block");
	++i; // consume '}'
	return loc;
}

// Parses "10M", "1K", "512", etc. into bytes
size_t ConfigParser::parseSize(const std::string &value) {
	if (value.empty())
		throw ParseException("Empty size value");
	char suffix = value[value.size() - 1];
	std::string numeric = value;
	size_t multiplier = 1;
	if (suffix == 'M' || suffix == 'm') {
		multiplier = 1024 * 1024;
		numeric = value.substr(0, value.size() - 1);
	} else if (suffix == 'K' || suffix == 'k') {
		multiplier = 1024;
		numeric = value.substr(0, value.size() - 1);
	} else if (suffix == 'G' || suffix == 'g') {
		multiplier = 1024 * 1024 * 1024;
		numeric = value.substr(0, value.size() - 1);
	}
	for (size_t i = 0; i < numeric.size(); ++i) {
		if (numeric[i] < '0' || numeric[i] > '9')
			throw ParseException("Invalid size value: " + value);
	}
	long n = std::strtol(numeric.c_str(), NULL, 10);
	if (n <= 0)
		throw ParseException("Size must be positive: " + value);
	return static_cast<size_t>(n) * multiplier;
}

void ConfigParser::parseHost(const std::string &value, ServerConfig &server) {
	size_t colon = value.find(':');
	if (colon != std::string::npos) {
		server.host = value.substr(0, colon);
		server.port = std::atoi(value.substr(colon + 1).c_str());
	} else {
		server.host = "0.0.0.0";
		server.port = std::atoi(value.c_str());
	}
	if (server.port <= 0 || server.port > 65535)
		throw ParseException("Invalid port in 'listen': " + value);
}
