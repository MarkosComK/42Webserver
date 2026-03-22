/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 12:33:10 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/21 22:25:09 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdexcept>


Socket::Socket() : port_(80), host_("0.0.0.0"), listen_fd_(-1) {
}

Socket::Socket(int port, const std::string &host) : port_(port), host_(host), listen_fd_(-1) {
}

Socket::Socket(const Socket &other){
	port_ = other.port_;
	host_ = other.host_;
	listen_fd_ = other.listen_fd_;
	socket_addr_ = other.socket_addr_;
}

Socket &Socket::operator=(const Socket &other){
	if (this != &other)
	{
		port_ = other.port_;
		host_ = other.host_;
		listen_fd_ = other.listen_fd_;
		socket_addr_ = other.socket_addr_;
	}
	return *this;
}

Socket::~Socket(){}

// changed the previous behavior where it called "die()", so now it throws exceptions instead
void Socket::init_socket(){
	listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd_ < 0)
		throw std::runtime_error(std::string("socket: ") + std::strerror(errno));

	int opt = 1;
	if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error(std::string("setsockopt: ") + std::strerror(errno));
}

// Uses INADDR_ANY for 0.0.0.0/empty host and inet_pton() for robust IPv4 parsing;
// throws on errors so the caller can skip a single failed listen (multi-host:port startup).
void Socket::bind_socket(){
	std::memset(&socket_addr_, 0, sizeof(socket_addr_));
	socket_addr_.sin_family = AF_INET;
	if (host_ == "0.0.0.0" || host_.empty())
		socket_addr_.sin_addr.s_addr = INADDR_ANY;
	else {
		if (inet_pton(AF_INET, host_.c_str(), &socket_addr_.sin_addr) != 1)
			throw std::runtime_error(std::string("inet_pton: ") + std::strerror(errno));
	}
	socket_addr_.sin_port = htons(port_);

	if (bind(listen_fd_, (sockaddr*)&socket_addr_, sizeof(socket_addr_)) < 0)
		throw std::runtime_error(std::string("bind: ") + std::strerror(errno));
}

void Socket::listen_socket(){
	if (listen(listen_fd_, 128) < 0)
		throw std::runtime_error(std::string("listen: ") + std::strerror(errno));

	//Non blocking
	int flags = fcntl(listen_fd_, F_GETFL, 0);
	if (flags < 0 )
		throw std::runtime_error(std::string("fcntl(F_GETFL): ") + std::strerror(errno));
	if (fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error(std::string("fcntl(F_SETFL): ") + std::strerror(errno));

	std::cout << "Listening on " << host_ << ":" << port_ << "..." << std::endl;
}

void Socket::close_socket(){
	if (listen_fd_ >= 0)
		close(listen_fd_);
}

void die(const char *msg){
	std::perror(msg);
	std::exit(1);
}

int Socket::getPort() const {
	return port_;
}

std::string Socket::getHost() const {
	return host_;
}

int Socket::getListen_fd() const {
	return listen_fd_;
}

sockaddr_in Socket::getSocket_addr() const {
	return socket_addr_;
}
