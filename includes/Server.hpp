/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 23:31:25 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/19 15:44:22 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CPP
# define SERVER_CPP

# include <map>
#include "Client.hpp"
#include "ConfigParser.hpp"
# include "Socket.hpp"

class Server{
	private:
		std::map<ListenPair, Socket> sockets_;
		std::vector<pollfd> poll_fds_;
		std::map<int, Client> clients_;
		std::map<int, Socket*> listen_fd_to_socket_;
	public:
		Server();
		~Server();
		void addSocket(const ListenPair &listen, const Socket &socket);
		void acceptNewClients(int listen_fd);
		void closeClient(size_t id);
		bool clientRead(size_t id);
		bool clientWrite(size_t id);
		void run();
		void closeAll();
};

#endif