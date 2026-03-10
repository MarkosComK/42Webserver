/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/03/10 13:13:53 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Socket.hpp"
#include "includes/webserv.hpp"
#include "includes/ConfigParser.hpp"
#include <cstring>
#include <cstdlib>

int main(int argc, char **argv) {
	// Test mode: run parsing tests
	if (argc > 1 && strcmp(argv[1], "--test") == 0) {
		std::string configPath = "config/webserv.conf";
		if (argc > 2)
			configPath = argv[2];

		std::cout << "===================================== Running in TEST mode =====================================\n" << std::endl;
		testRequestParsing();
		testConfigParsing(configPath);
		return 0;
	}

	// Server mode: run the actual web server
	std::cout << "===================================== Running WEBSERV =====================================" << std::endl;

	int port = 8080; // default port, this will be overridden by the config file later...

	if (argc >= 2)
		port = atoi(argv[1]);
	// not sure why we're using this... isn't argv[1] for config path?? subject says: "Your executable should be executed as follows: ./webserv [configuration file]"

	Socket socket(port);
	socket.init_socket();
	socket.bind_socket();
	socket.listen_socket();
	socket.start_poll();
	socket.run();
	socket.close_socket();

	return 0;
}
