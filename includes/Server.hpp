/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 23:31:25 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/18 23:37:43 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CPP
# define SERVER_CPP

# include <map>
# include <Socket.hpp>

class Server{
	private:
		std::map<int, Socket> sockets_;
		std::vector<pollfd>	poll_fds_;
	public:
		Server(/* args */);
		~Server();
};

#endif