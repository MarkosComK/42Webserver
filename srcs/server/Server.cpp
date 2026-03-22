/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: GitHub Copilot                                 +#+  +:+       +#+    */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 00:00:00 by GitHub Copilot     #+#    #+#             */
/*   Updated: 2026/03/21 00:00:00 by GitHub Copilot    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Utils.hpp"
#include "../includes/webserv.hpp"


Server::Server() {
}

Server::~Server() {
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


// ...
std::string Server::listenKey_(const ListenPair &listen) const {
	return listen.host + ":" + itoa_int(listen.port);
}

// added to save a new fd (listen/client) in the shared poll() list, using the struct pollfd (list of file descriptors to be monitored).
void Server::addPollFd_(int fd, short events) {
	pollfd pollFd;
	pollFd.fd = fd;
	pollFd.events = events;
	pollFd.revents = 0; // revents is set by the kernel to indicate which events occurred, we initialize it to 0 before polling
	poll_fds_.push_back(pollFd);
}

// accepts new clients on the given listen socket fd, and adds them to the poll list and clients map
// removed the previous addSocket() entirely
void Server::addListen(const ListenPair &listen, const ServerConfig *config) {
	const std::string key = listenKey_(listen);
	std::map<std::string, Socket>::iterator it = sockets_.find(key);
	if (it == sockets_.end()) {
		sockets_.insert(std::make_pair(key, Socket(listen.port, listen.host)));
		Socket *sockPtr = &sockets_.find(key)->second;
		try {
			sockPtr->init_socket();
			sockPtr->bind_socket();
			sockPtr->listen_socket();
		} catch (const std::exception &e) {
			std::cerr << "[listen setup failed] " << listen.host << ":" << listen.port << " -> " << e.what() << std::endl;
			if (sockPtr->getListen_fd() >= 0)
				close(sockPtr->getListen_fd());
			sockets_.erase(key);
			return;
		}

		listen_fd_to_socket_[sockPtr->getListen_fd()] = sockPtr;
		if (config)
			socket_to_config_[sockPtr] = config;
		addPollFd_(sockPtr->getListen_fd(), POLLIN);
		return;
	}

	Socket *existingSock = &it->second;
	// For now, a listen socket maps to the first server block we see.
	if (config && socket_to_config_.find(existingSock) == socket_to_config_.end())
		socket_to_config_[existingSock] = config;
}

void Server::acceptNewClients_(int listen_fd) {
	Socket *listenSock = listen_fd_to_socket_[listen_fd];
	while (true) {
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			die("accept");
		}
		int flags = fcntl(client_fd, F_GETFL, 0);
		if (flags == -1)
			die("fcntl(cfd, F_GETFL, 0)");
		if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
			die("fcntl(cfd, F_SETFL, flags | O_NONBLOCK)");

		addPollFd_(client_fd, POLLIN);
		clients_[client_fd] = Client(client_fd, listenSock);
		std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr)
				  << ":" << ntohs(client_addr.sin_port)
				  << " | fd = " << client_fd
				  << " | listen = " << listenSock->getHost() << ":" << listenSock->getPort()
				  << std::endl;
	}
}

void Server::closeClient_(size_t poll_index) {
	int fd = poll_fds_[poll_index].fd;
	close(fd);
	clients_.erase(fd);
	poll_fds_.erase(poll_fds_.begin() + poll_index);
}

bool Server::clientRead_(size_t poll_index) {
	int client_fd = poll_fds_[poll_index].fd;
	Client &client = clients_[client_fd];
	Socket *sock = client.getSocket();
	const ServerConfig *cfgPtr = NULL;
	if (sock) {
		std::map<Socket*, const ServerConfig*>::const_iterator it = socket_to_config_.find(sock);
		if (it != socket_to_config_.end())
			cfgPtr = it->second;
	}
	const ServerConfig *server_config = cfgPtr;

	char buf[2048];
	while (true) {
		ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
		if (n > 0) {
			client.appendBf_in(buf, (size_t)n);
			if (!client.getHeaders_done()) {
				if (client.getBf_in().find("\r\n\r\n") != std::string::npos)
					client.setHeaders_done(true);
				else if (client.getBf_in().size() > 8192) {
					closeClient_(poll_index);
					return true;
				}
			}

			if (client.getHeaders_done()) {
				const std::string &buffer = client.getBf_in();
				size_t headersEnd = buffer.find("\r\n\r\n");
				if (headersEnd == std::string::npos)
					continue;

				// If we can't resolve a config for this client, treat it as a server misconfiguration.
				if (!server_config) {
					std::string response = build_error_response(500, "Error 500\n");
					client.appendBf_out(response);
					client.setOut_bytes_sent(0);
					poll_fds_[poll_index].events = POLLOUT;
					break;
				}

				std::string earlyMethod;
				std::string earlyPath;
				parseRequestLineMethodPath(buffer, earlyMethod, earlyPath);

				if (!earlyPath.empty() && earlyPath[0] == '/' && !earlyMethod.empty()
					&& !isMethodAllowedForLocation(earlyMethod, earlyPath, *server_config)) {
					std::string response = build_error_response(405, "Error 405\n");
					client.appendBf_out(response);
					client.setOut_bytes_sent(0);
					poll_fds_[poll_index].events = POLLOUT;
					break;
				}

				long contentLength = getContentLengthHeader(buffer);
				if (contentLength < 0)
					contentLength = 0;

				if (contentLength > 0 && static_cast<size_t>(contentLength) > server_config->clientMaxBodySize) {
					std::string response = build_error_response(413, "Error 413\n");
					client.appendBf_out(response);
					client.setOut_bytes_sent(0);
					poll_fds_[poll_index].events = POLLOUT;
					break;
				}

				size_t totalNeeded = headersEnd + 4;
				if (contentLength > 0)
					totalNeeded += static_cast<size_t>(contentLength);
				if (buffer.size() < totalNeeded)
					continue;

				std::string rawRequest = buffer.substr(0, totalNeeded);
				std::string response = build_response_from_request(rawRequest, *server_config);

				client.appendBf_out(response);
				client.setOut_bytes_sent(0);
				poll_fds_[poll_index].events = POLLOUT;
				break;
			}
		} else if (n == 0) {
			closeClient_(poll_index);
			return true;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			closeClient_(poll_index);
			return true;
		}
	}
	return false;
}

bool Server::clientWrite_(size_t poll_index) {
	int client_fd = poll_fds_[poll_index].fd;
	Client &client = clients_[client_fd];
	while (client.getOut_bytes_sent() < client.getBf_out().size())
	{
		const char *data = client.getBf_out().c_str() + client.getOut_bytes_sent();
		size_t left = client.getBf_out().size() - client.getOut_bytes_sent();

		ssize_t n = send(client_fd, data, left, MSG_NOSIGNAL);
		if (n > 0)
			client.addOut_bytes_sent((size_t)n);
		else if (n == 0) {
			//Se já tem resposta terminada, envia
			if (!client.getBf_out().empty()) {
				poll_fds_[poll_index].events = POLLOUT; //continua tentando enviar
				break;
			}
			closeClient_(poll_index);  //cliente fechou
			return true;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; //tenta depois
			closeClient_(poll_index);
			return true;
		}
	}
	//terminou de enviar -> fechar http 1.0
	if (client.getOut_bytes_sent() >= client.getBf_out().size()) {
		closeClient_(poll_index);
		return true;
	}
	return false;
}

void Server::run() {
	if (poll_fds_.empty())
		throw std::runtime_error("No listen sockets configured");

	while (true) {
		int ret = poll(&poll_fds_[0], poll_fds_.size(), -1);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			die("poll");
		}

		//events
		for (size_t i = 0; i < poll_fds_.size(); ++i) {
			if (poll_fds_[i].revents == 0)
				continue;

			//errors / hangups
			if (poll_fds_[i].revents & (POLLERR | POLLNVAL)) {
				if (listen_fd_to_socket_.count(poll_fds_[i].fd))
					die("Listen socket error!");
				closeClient_(i);
				--i;
				continue;
			}

			//Accept New Clients
			if ((poll_fds_[i].revents & POLLIN) && listen_fd_to_socket_.count(poll_fds_[i].fd)) {
				acceptNewClients_(poll_fds_[i].fd);
				continue;
			}

			//Client Read
			if (poll_fds_[i].revents & POLLIN) {
				if (clientRead_(i))
					--i;  //client_read pode fechar o cliente
				continue;
			}

			if (poll_fds_[i].revents & POLLOUT) {
				if (clientWrite_(i))
					--i; ; //client_write pode fechar o cliente
				continue;
			}

			//Cliente fechou o write-side, mas ainda há algo para escrever, não fecha logo a conexão
			if (poll_fds_[i].revents & POLLHUP) {
				if (listen_fd_to_socket_.count(poll_fds_[i].fd))
					die("Listen socket hangup!");
				Client &client = clients_[poll_fds_[i].fd];
				if (!client.getBf_out().empty() && client.getOut_bytes_sent() < client.getBf_out().size()) {
					//continua a tentar enviar o que falta
					poll_fds_[i].events = POLLOUT;
					continue;
				}
				closeClient_(i);
				--i;
				continue;
			}
		}
	}
}

void Server::closeAll() {
	for (std::vector<pollfd>::iterator it = poll_fds_.begin(); it != poll_fds_.end(); ++it)
		close(it->fd);
	poll_fds_.clear();
	clients_.clear();
	listen_fd_to_socket_.clear();
	socket_to_config_.clear();
	sockets_.clear();
}
