/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/03/19 16:20:52 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//#include "includes/Socket.hpp"
#include "includes/ConfigParser.hpp"
#include "includes/Server.hpp"

int main() {

	ListenPair pair1("0.0.0.0", 8080);
	ListenPair pair2("127.0.0.1", 8081);
	ListenPair pair3("0.0.0.0", 8082);
	ListenPair pair4("127.0.0.1", 8083);
	ListenPair pair5("127.0.0.1", 8080);

	std::vector<ListenPair> listens1;
	listens1.push_back(pair1);

	ConfigParser parser(listens1);

	std::vector<ListenPair> listens2;
	listens2.push_back(pair2);
	ServerConfig server2;
	server2.listens = listens2;
	parser.addServer(server2);

	std::vector<ListenPair> listens3;
	listens3.push_back(pair3);
	listens3.push_back(pair4);
	ServerConfig server3;
	server3.listens = listens3;
	parser.addServer(server3);

	std::vector<ListenPair> listens4;
	listens4.push_back(pair5);
	ServerConfig server4;
	server4.listens = listens4;
	parser.addServer(server4);

	parser.printConfig();

	Server server;
	for (size_t i = 0; i < parser.getServers().size(); ++i)
	{
		const ServerConfig &conf = parser.getServers()[i];
		for (size_t j = 0; j < conf.listens.size(); ++j)
		{
			server.addSocket(conf.listens[j], Socket(conf.listens[j].port, conf.listens[j].host));
		}
	}
	server.run();
	server.closeAll();
	return 0;
}

// server {
//     listen 8080;
//     root /var/www/default_8080;
// }

// server {
//     listen 127.0.0.1:8081;
//     root /var/www/default_8081;
// }

// server {
//     listen 8082;
//     listen 127.0.0.1:8083;
//     server_name multi.com;
//     root /var/www/multi_main;
// }

// server {
//     listen 127.0.0.2:8083;
//     server_name other8083.com;
//     root /var/www/other_8083;
// }
