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
		std::vector<pollfd>                 poll_fds_;
		std::map<int, Client>               clients_;
		std::map<std::string, Socket>       sockets_;
		std::map<int, Socket*>              listen_fd_to_socket_;
		std::map<Socket*, const ServerConfig*> socket_to_config_;

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
