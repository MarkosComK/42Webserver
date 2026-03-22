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

# include <string>
# include <unistd.h>
# include <iostream>
# include <fcntl.h>
# include <poll.h>
# include <errno.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>

# include "Config.hpp"

class Socket{
	private:
		int						port_;
		std::string				host_;
		int							listen_fd_;
		sockaddr_in					socket_addr_;

	public:
		Socket();
		Socket(int port, const std::string &host);
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
		~Socket();

		void init_socket();
		void bind_socket();
		void listen_socket();
		void close_socket();
		int getPort() const;
		std::string getHost() const;
		int getListen_fd() const;
		sockaddr_in getSocket_addr() const;
};

void die(const char *msg);

#endif
