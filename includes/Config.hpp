/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 17:03:30 by carlos-j          #+#    #+#             */
/*   Updated: 2026/03/30 17:03:41 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	size_t clientMaxBodySize;
	std::map<std::string, std::string> cgi; // e.g. ".php" -> "/usr/bin/php-cgi"

	// initialize members of the struct with default values
	Location() : redirectCode(0), autoindex(false), clientMaxBodySize(1024 * 1024) {} // default max body size is 1MB
};

// could we make ListenPair a map instead of a struct??
struct ListenPair {
	std::string host;
	int port;

	// initialize members of the struct with default values
	ListenPair(const std::string &h, int p) : host(h), port(p) {}
};

struct ServerConfig {
	std::vector<ListenPair> listens;
	std::string root;
	std::map<int, std::string> errorPages;
	size_t clientMaxBodySize;
	std::vector<Location> locations;

	// initialize members of the struct with default values
	ServerConfig() : clientMaxBodySize(1024 * 1024) {} // default max body size is 1MB
};

#endif
