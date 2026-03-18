/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 12:33:10 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/18 17:25:40 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/webserv.hpp"
#include "../includes/Utils.hpp"

// Forward declarations
static bool parseRequestLineMethodPath(const std::string &rawRequest, std::string &outMethod, std::string &outPath);

/* // not needed, the server will always start with a config file
Socket::Socket(){
	port_ = 80;
	// TODO: default config with a "/" location that serves files from current directory
}

 // remove this later and keep jsut the one that takes the ServerConfig ?
Socket::Socket(int port){
	port_ = port;
}*/

//Socket::Socket(int port, const ServerConfig &config){
Socket::Socket(const ServerConfig &config){
	//port_ = port;
	port_ = config.listens.empty() ? 8080 : config.listens[0].port;
	server_config_ = config;
}

Socket::Socket(const Socket &other){
	port_ = other.port_;
	listen_fd_ = other.listen_fd_;
	socket_addr_ = other.socket_addr_;
	poll_fds_ = other.poll_fds_;
	clients_ = other.clients_;
	server_config_ = other.server_config_;
}

Socket &Socket::operator=(const Socket &other){
	if (this != &other)
	{
		port_ = other.port_;
		listen_fd_ = other.listen_fd_;
		socket_addr_ = other.socket_addr_;
		poll_fds_ = other.poll_fds_;
		clients_ = other.clients_;
		server_config_ = other.server_config_;
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

void Socket::close_socket(){
	close(listen_fd_);
}

void Socket::close_client(size_t id){
	int fd = poll_fds_[id].fd;
	close(fd);
	clients_.erase(fd);
	poll_fds_.erase(poll_fds_.begin() + id);
}

void Socket::accept_new_clients(){
	while (true){
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(listen_fd_, (sockaddr*)&client_addr, &client_len);
		if (client_fd < 0){
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			die("accept");
		}
		int flags = fcntl(client_fd, F_GETFL, 0);
		if (flags == -1 ) die("fcntl(cfd, F_GETFL, 0)");
		if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) die("fcntl(cfd, F_SETFL, flags | O_NONBLOCK)");
		pollfd p ;
		p.fd = client_fd;
		p.events = POLLIN;
		p.revents = 0;
		poll_fds_.push_back(p);
		clients_[client_fd] = Client(client_fd);
		std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << " | fd = " << client_fd << std::endl;
	}
}

bool Socket::client_read(size_t id){
	int client_fd = poll_fds_[id].fd;
	Client &client = clients_[client_fd];
	char buf[2048];
	while (true){
		ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
		if (n > 0){
			client.appendBf_in(buf, (size_t)n);
			// detect end of headers and enforce a limit size of 8KB
			if (!client.getHeaders_done()) {
				if (client.getBf_in().find("\r\n\r\n") != std::string::npos)
					client.setHeaders_done(true);
				else if (client.getBf_in().size() > 8192) {
					close_client(id);
					return true;
				}
			}

			if (client.getHeaders_done()) {
				const std::string &buffer = client.getBf_in();
				size_t headersEnd = buffer.find("\r\n\r\n");
				if (headersEnd == std::string::npos)
					continue;


				// parser made by carlos-j
				// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				std::string earlyMethod;
				std::string earlyPath;
				parseRequestLineMethodPath(buffer, earlyMethod, earlyPath);

				if (!earlyPath.empty() && earlyPath[0] == '/' && !earlyMethod.empty()
					&& !isMethodAllowedForLocation(earlyMethod, earlyPath, server_config_)) {
					std::string response = build_error_response(405, "Error 405\n");
					client.appendBf_out(response);
					client.setOut_bytes_sent(0);
					poll_fds_[id].events = POLLOUT;
					break;
				}

				long contentLength = getContentLengthHeader(buffer);
				if (contentLength < 0)
					contentLength = 0;

				// enforce server max body size based on Content-Length
				if (contentLength > 0 && static_cast<size_t>(contentLength) > server_config_.clientMaxBodySize) {
					std::string response = build_error_response(413, "Error 413\n");
					client.appendBf_out(response);
					client.setOut_bytes_sent(0);
					poll_fds_[id].events = POLLOUT;
					break;
				}

				// wait for full body based on Content-Length, then parse as a complete request
				size_t totalNeeded = headersEnd + 4;
				if (contentLength > 0)
					totalNeeded += static_cast<size_t>(contentLength);
				if (buffer.size() < totalNeeded)
					continue;

				std::string rawRequest = buffer.substr(0, totalNeeded);
				std::string response = build_response_from_request(rawRequest, server_config_);

				// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

				client.appendBf_out(response);
				client.setOut_bytes_sent(0);
				poll_fds_[id].events = POLLOUT;
				break;
			}
		} else if (n == 0)
			return (close_client(id), true); //cliente fechou
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; //sem mais dados agora
			return (close_client(id), true);
		}
	}
	return false;
}

bool Socket::client_write(size_t id){
	int client_fd = poll_fds_[id].fd;
	Client &client = clients_[client_fd];
	while(client.getOut_bytes_sent() < client.getBf_out().size())
	{
		const char *data = client.getBf_out().c_str() + client.getOut_bytes_sent();
		size_t left = client.getBf_out().size() - client.getOut_bytes_sent();

		ssize_t n = send(client_fd, data, left, MSG_NOSIGNAL);
		if (n > 0)
			client.addOut_bytes_sent((size_t)n);
		else if (n == 0){
			//Se já tem resposta terminada, envia
			if (!client.getBf_out().empty()){
				poll_fds_[id].events = POLLOUT; //continua tentando enviar
				break;
			}
			return (close_client(id), true); //cliente fechou
		}
		else if (n < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; //tenta depois
			return (close_client(id), true);
		}
	}
	//terminou de enviar -> fechar http 1.0
	if (client.getOut_bytes_sent() >= client.getBf_out().size())
		return (close_client(id), true);
	return false;
}

void Socket::run(){
	while (true)
	{
		int ret = poll(&poll_fds_[0], poll_fds_.size(), -1);
		if (ret < 0){
			if (errno == EINTR)
				continue;
			die("poll");
		}

		//events
		for (size_t i = 0; i < poll_fds_.size(); ++i){
			if (poll_fds_[i].revents == 0)
				continue;

			//errors / hangups
			if (poll_fds_[i].revents & (POLLERR | POLLNVAL)){
				if (poll_fds_[i].fd == listen_fd_)
					die("Listen socket error!");
				close_client(i);
				--i;
				continue;
			}

			//Accept New Clients
			if (poll_fds_[i].fd == listen_fd_ && (poll_fds_[i].revents & POLLIN)){
				accept_new_clients();
				continue;
			}

			//Client Read
			if (poll_fds_[i].revents & POLLIN){
				if (client_read(i))
					--i; //client_read pode fechar o cliente
				continue;
			}
			// client -> write
			if (poll_fds_[i].revents & POLLOUT){
				if (client_write(i))
					--i; //client_write pode fechar o cliente
				continue;
			}

			//Cliente fechou o write-side, mas ainda há algo para escrever, não fecha logo a conexão
			if (poll_fds_[i].revents & POLLHUP){
				if (poll_fds_[i].fd == listen_fd_)
					die("Listen socket hangup!");
				Client &client = clients_[poll_fds_[i].fd];
				if (!client.getBf_out().empty() && client.getOut_bytes_sent() < client.getBf_out().size()){
					//continua a tentar enviar o que falta
					poll_fds_[i].events = POLLOUT;
					continue;
				}
				close_client(i);
				--i;
				continue;
			}
		}
	}
}

void die(const char *msg){
	std::perror(msg);
	std::exit(1);
}

static bool parseRequestLineMethodPath(const std::string &rawRequest, std::string &outMethod, std::string &outPath) {
	outMethod.clear();
	outPath.clear();
	size_t lineEnd = rawRequest.find("\r\n");
	if (lineEnd == std::string::npos)
		return false;
	std::string requestLine = rawRequest.substr(0, lineEnd);
	std::vector<std::string> parts = splitByWhitespace(requestLine);
	if (parts.size() < 2)
		return false;
	outMethod = parts[0];
	outPath = parts[1];
	size_t qPos = outPath.find('?');
	if (qPos != std::string::npos)
		outPath = outPath.substr(0, qPos);
	return true;
}

