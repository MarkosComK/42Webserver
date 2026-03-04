/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/03/04 15:06:32 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Socket.hpp"

int main(int argc, char **argv) {
	int port = 80;
	if (argc >= 2)
		port = atoi(argv[1]);
	Socket socket(port);
	socket.init_socket();
	socket.bind_socket();
	socket.listen_socket();
	socket.start_poll();
	socket.run();
	socket.close_socket();
	return 0;
}
