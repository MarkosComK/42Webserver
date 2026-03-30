/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 23:31:25 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/21 00:00:00 by GitHub Copilot    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <string>
# include <utility>
# include <vector>
# include <poll.h>

# include "Config.hpp"
# include "Client.hpp"
# include "Socket.hpp"

class Server {
	private:
		std::vector<pollfd>                 poll_fds_; // pollfd struct contains: int fd; short events; short revents;
		std::map<int, Client>               clients_; // int = client socket fd; Client = socket and buffers
		std::map<std::string, Socket>       sockets_; // string = "host:port"; Socket = listening socket for that host:port
		std::map<int, Socket*>              listen_fd_to_socket_; // int = listen socket fd; Socket* = pointer to the Socket object in sockets_ (for easy lookup when accepting new clients)
		std::map<Socket*, const ServerConfig*> socket_to_config_; // Socket* = pointer to the Socket object in sockets_; ServerConfig* = pointer to the ServerConfig that should be used for clients accepted on that socket

		std::string listenKey_(const ListenPair &listen) const;
		void addPollFd_(int fd, short events);
		void acceptNewClients_(int listen_fd);
		void closeClient_(size_t poll_index);
		bool clientRead_(size_t poll_index);
		bool clientWrite_(size_t poll_index);

	public:
		Server();
		~Server();

		void addListen(const ListenPair &listen, const ServerConfig *config);
		void run();
		void closeAll();
};

#endif
