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
# include "Client.hpp"

class Socket{
	private:
		int							port_;
		int							listen_fd_;
		sockaddr_in					socket_addr_;
		std::vector<pollfd>			poll_fds_;
		std::map<int, Client>		clients_;
	public:
		Socket();
		Socket(int port);
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

std::string build_error_response(int errorCode, const std::string& body);
std::string build_response_200_text(const std::string& body);

#endif
