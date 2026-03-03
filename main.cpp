/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/03/03 19:46:28 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Socket.hpp"

int main(int argc, char **argv) {
	Socket socket;
	if (argc >= 2)
		Socket socket(atoi(argv[1]));
	socket.init_socket();
	socket.bind_socket();
	socket.start_poll();
	return 0;
}
