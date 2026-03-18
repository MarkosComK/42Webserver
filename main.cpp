/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/03/18 17:23:26 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Socket.hpp"
#include "includes/webserv.hpp"
#include "includes/ConfigParser.hpp"
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

		//int port = servers[0].listens.empty() ? 8080 : servers[0].listens[0].port;

		//Socket socket(port, servers[0]);
		Socket socket(servers[0]);

		// TODO: support multiple servers and virtual hosting based on Host header
		// Socket socket(parser.getServers()); // something like this...

		// TODO: make init, bind and listen run in a loop for multiple ports, creating one fd per host:port and pushing them to poll_fds
		socket.init_socket();
		socket.bind_socket();
		socket.listen_socket();
		socket.start_poll();
		socket.run();
		socket.close_socket();

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
