/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/19 12:00:56 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/19 12:38:52 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

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
	std::map<int, std::string> errorPages;
	size_t clientMaxBodySize;
	std::vector<Location> locations;

	ServerConfig() : clientMaxBodySize(1024 * 1024) {}
};

class ConfigParser
{
private:
	std::vector<ServerConfig> _servers;
public:
	ConfigParser(std::vector<ListenPair> listens);
	~ConfigParser();
	void addServer(const ServerConfig &server);
	const std::vector<ServerConfig> &getServers() const;
	void printConfig() const; // For debugging
};
#endif
