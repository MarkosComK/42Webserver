/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 12:33:10 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/03 19:46:48 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"

Socket::Socket(){
	port_ = 80;
}

Socket::Socket(int port){
	port_ = port;
}

Socket::Socket(const Socket &other){
	port_ = other.port_;
	listen_fd_ = other.listen_fd_;
	socket_addr_ = other.socket_addr_;
	poll_fds_ = other.poll_fds_;
	clients_ = other.clients_;
}

Socket &Socket::operator=(const Socket &other){
	if (this != &other)
	{
		port_ = other.port_;
		listen_fd_ = other.listen_fd_;
		socket_addr_ = other.socket_addr_;
		poll_fds_ = other.poll_fds_;
		clients_ = other.clients_;
	}
	return *this;
}

Socket::~Socket(){}

void Socket::init_socket(){
	listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd_ < 0)
		die("socket");

	int opt = 1;
	if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		die("setsockopt");
}

void Socket::bind_socket(){
	std::memset(&socket_addr_, 0, sizeof(socket_addr_));
	socket_addr_.sin_family = AF_INET;
	socket_addr_.sin_addr.s_addr = INADDR_ANY;
	socket_addr_.sin_port = htons(port_);

	if (bind(listen_fd_, (sockaddr*)&socket_addr_, sizeof(socket_addr_)) < 0)
		die("bind");
}

void Socket::listen_socket(){
	if (listen(listen_fd_, 128) < 0)
		die("listen");

	//Non blocking
	int flags = fcntl(listen_fd_, F_GETFL, 0);
	if (flags < 0 )
		die("fcntl(fd, F_GETFL, 0)");
	if (fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK) < 0)
		die("fcntl(fd, F_SETFL, flags | O_NONBLOCK)");

	std::cout << "Listening on port " << port_ << "..." << std::endl;
}

void Socket::start_poll(){
	poll_fds_.push_back(pollfd());
	poll_fds_[0].fd = listen_fd_;
	poll_fds_[0].events = POLLIN;
	poll_fds_[0].revents = 0;
}

void die(const char *msg){
	std::perror(msg);
	std::exit(1);
}
