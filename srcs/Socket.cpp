/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 12:33:10 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/19 15:31:49 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"

Socket::Socket(){
	port_ = 80;
	host_ = "0.0.0.0";
}

Socket::Socket(int port, std::string host){
	port_ = port;
	host_ = host;
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

int Socket::getPort() const{
	return port_;
}

std::string Socket::getHost() const{
	return host_;
}

int Socket::getListen_fd() const{
	return listen_fd_;
}

sockaddr_in Socket::getSocket_addr() const{
	return socket_addr_;
}

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
	socket_addr_.sin_addr.s_addr = inet_addr(host_.c_str());
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

void Socket::close_socket(){
	close(listen_fd_);
}

void die(const char *msg){
	std::perror(msg);
	std::exit(1);
}

/*Só para teste esta funções, depois de ter response, apagar*/
static std::string itoa_int(int n){
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", n);
    return std::string(buf);
}

std::string build_response_200_text(const std::string& body)
{
    return
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;
}