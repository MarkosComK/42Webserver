/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 23:31:08 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/19 16:01:12 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server(){}
Server::~Server(){}

void Server::addSocket(const ListenPair &listen, const Socket &socket){
	sockets_[listen] = socket;
	sockets_[listen].init_socket();
	sockets_[listen].bind_socket();
	sockets_[listen].listen_socket();
	pollfd p;
	p.fd = sockets_[listen].getListen_fd();
	p.events = POLLIN;
	p.revents = 0;
	poll_fds_.push_back(p);
	listen_fd_to_socket_[sockets_[listen].getListen_fd()] = &sockets_[listen];
}

void Server::acceptNewClients(int listen_fd){
	while (true) {
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
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
		clients_[client_fd] = Client(client_fd, listen_fd_to_socket_[listen_fd]);
		std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << " | fd = " << client_fd << std::endl;
	}
}

void Server::closeClient(size_t id){
	int fd = poll_fds_[id].fd;
	close(fd);
	clients_.erase(fd);
	poll_fds_.erase(poll_fds_.begin() + id);
}

bool Server::clientRead(size_t id){
	int client_fd = poll_fds_[id].fd;
	Client &client = clients_[client_fd];
	char buf[2048];
	while (true){
		ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
		if (n > 0){
			client.appendBf_in(buf, (size_t)n);
			if (!client.getHeaders_done() && client.getBf_in().find("\r\n\r\n") != std::string::npos){
				client.setHeaders_done(true);

				//Ainda sem parsing, fixo
				const std::string body = "Hello! poll + non-blocking OK\n";
				client.appendBf_out(build_response_200_text(body));
				client.setOut_bytes_sent(0);

				//muda interesse para escrita
				poll_fds_[id].events = POLLOUT;
				break; //já tem resposta pronta
			}
			//limitar headers para evitar infinito
			if (client.getBf_in().size() > 8192){
				//fecha por simplicidade e envia 431/413
				closeClient(id);
				return true;
			}
		} else if (n == 0)
			return (closeClient(id), true); //cliente fechou
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; //sem mais dados agora
			return (closeClient(id), true);
		}
	}
	return false;
}

bool Server::clientWrite(size_t id){
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
			return (closeClient(id), true); //cliente fechou
		}
		else if (n < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; //tenta depois
			return (closeClient(id), true);
		}
	}
	//terminou de enviar -> fechar http 1.0
	if (client.getOut_bytes_sent() >= client.getBf_out().size())
		return (closeClient(id), true);
	return false;
}

void Server::run(){
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
				if (listen_fd_to_socket_[poll_fds_[i].fd])
					die("Listen socket error!");
				closeClient(i);
				--i;
				continue;
			}

			//Accept New Clients
			if (listen_fd_to_socket_[poll_fds_[i].fd] && (poll_fds_[i].revents & POLLIN)){
				acceptNewClients(poll_fds_[i].fd);
				continue;
			}

			//Client Read
			if (poll_fds_[i].revents & POLLIN){
				if (clientRead(i))
					--i; //client_read pode fechar o cliente
				continue;
			}
			// client -> write
			if (poll_fds_[i].revents & POLLOUT){
				if (clientWrite(i))
					--i; //client_write pode fechar o cliente
				continue;
			}

			//Cliente fechou o write-side, mas ainda há algo para escrever, não fecha logo a conexão
			if (poll_fds_[i].revents & POLLHUP){
				if (listen_fd_to_socket_[poll_fds_[i].fd])
					die("Listen socket hangup!");
				Client &client = clients_[poll_fds_[i].fd];
				if (!client.getBf_out().empty() && client.getOut_bytes_sent() < client.getBf_out().size()){
					//continua a tentar enviar o que falta
					poll_fds_[i].events = POLLOUT;
					continue;
				}
				closeClient(i);
				--i;
				continue;
			}
		}
	}
}

void Server::closeAll(){
	for (size_t i = 0; i < poll_fds_.size(); ++i)
		close(poll_fds_[i].fd);
	clients_.clear();
	poll_fds_.clear();
	sockets_.clear();
}