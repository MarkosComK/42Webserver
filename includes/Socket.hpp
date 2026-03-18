 /* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 12:32:24 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/03 18:08:16 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <unistd.h>
# include <iostream>
# include <fcntl.h>
# include <poll.h>
# include <errno.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <vector>
# include <map>
# include <netinet/in.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>
# include "Client.hpp"
# include "Config.hpp"

class Socket{
	private:
		int							port_; // change with a list for multiple ports (like std::vector<ListenPair> )
		int							listen_fd_;
		sockaddr_in					socket_addr_;
		std::vector<pollfd>			poll_fds_;
		std::map<int, Client>		clients_;
		ServerConfig				server_config_;
	public:
		//Socket();
		//Socket(int port); // remove this later and keep jsut the one that takes the ServerConfig ?
		//Socket(int port, const ServerConfig &config);
		Socket(const ServerConfig &config);
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
		~Socket();

		void init_socket();
		void bind_socket();
		void listen_socket();
		void start_poll();
		void close_socket();
		void close_client(size_t id);
		void run();
		void accept_new_clients();
		bool client_read(size_t id);
		bool client_write(size_t id);
};

void die(const char *msg);

#endif
