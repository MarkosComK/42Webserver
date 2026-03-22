/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/03/21 21:59:46 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/webserv.hpp"
#include "includes/ConfigParser.hpp"
#include "includes/Server.hpp"
#include <cstring>
#include <cstdlib>

int main(int argc, char **argv) {
	if (argc > 1 && strcmp(argv[1], "--test") == 0) {
		std::string configPath = "config/webserv.conf";
		if (argc > 2)
			configPath = argv[2];

		std::cout << "===================================== Running in TEST mode =====================================\n" << std::endl;
		testRequestParsing();
		testConfigParsing(configPath);
		return 0;
	}

	std::cout << "===================================== Running WEBSERV =====================================" << std::endl;

	std::string configPath = "config/webserv.conf";
	if (argc >= 2)
		configPath = argv[1];

	try {
		ConfigParser parser(configPath);
		const std::vector<ServerConfig> &servers = parser.getServers();

		Server server;
		for (size_t server_index = 0; server_index < servers.size(); ++server_index) {
			for (size_t listen_index = 0; listen_index < servers[server_index].listens.size(); ++listen_index) {
				server.addListen(servers[server_index].listens[listen_index], &servers[server_index]);
			}
		}
		server.run();
		server.closeAll();

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
