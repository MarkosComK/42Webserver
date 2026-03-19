/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/19 12:03:43 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/19 12:35:55 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigParser.hpp"

ConfigParser::ConfigParser(std::vector<ListenPair> listens) {
	ServerConfig server;
	server.listens = listens;
	_servers.push_back(server);
}

ConfigParser::~ConfigParser(){}

void ConfigParser::addServer(const ServerConfig &server) {
	_servers.push_back(server);
}

const std::vector<ServerConfig> &ConfigParser::getServers() const{
	return _servers;
}

void ConfigParser::printConfig() const {
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig &server = _servers[i];
		std::cout << "Server " << i + 1 << ":\n";
		for (size_t j = 0; j < server.listens.size(); ++j) {
			std::cout << "  Listen: " << server.listens[j].host << ":" << server.listens[j].port << "\n";
		}
		std::cout << "  Root: " << server.root << "\n";
		std::cout << "  Server Name: " << server.serverName << "\n";
		std::cout << "  Client Max Body Size: " << server.clientMaxBodySize << "\n";
		std::cout << "  Locations:\n";
		for (size_t k = 0; k < server.locations.size(); ++k) {
			const Location &loc = server.locations[k];
			std::cout << "    Path: " << loc.path << "\n";
			std::cout << "    Root: " << loc.root << "\n";
			std::cout << "    Index: " << loc.index << "\n";
			std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
			if (!loc.redirect.empty()) {
				std::cout << "    Redirect: " << loc.redirect << " (code: " << loc.redirectCode << ")\n";
			}
			if (!loc.cgi.empty()) {
				std::cout << "    CGI:\n";
				for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
					std::cout << "      Extension: " << it->first << ", Executable: " << it->second << "\n";
				}
			}
		}
	}
}