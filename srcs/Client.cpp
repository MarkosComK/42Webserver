/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:10:11 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/04 14:39:57 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::Client(){
	fd_ = -1;
	headers_done_ = false;
	out_bytes_sent_ = 0;
}

Client::Client(int fd){
	fd_ = fd;
	headers_done_ = false;
	out_bytes_sent_ = 0;
}

Client::Client(const Client &other){
	fd_ = other.fd_;
	headers_done_ = other.headers_done_;
	out_bytes_sent_ = other.out_bytes_sent_;
}

Client &Client::operator=(const Client &other){
	if (this != &other)
	{
		fd_ = other.fd_;
		headers_done_ = other.headers_done_;
		out_bytes_sent_ = other.out_bytes_sent_;
	}
	return *this;
}

Client::~Client(){}

bool Client::getHeaders_done() const{
	return headers_done_;
}

void Client::setHeaders_done(bool value){
	headers_done_ = value;
}

const std::string& Client::getBf_in() const{
	return bf_in_;
}

void Client::appendBf_in(const char *data, size_t n){
	bf_in_.append(data, n);
}

const std::string& Client::getBf_out() const{
	return bf_out_;
}

void Client::appendBf_out(const std::string &data){
	bf_out_.append(data);
}

size_t Client::getOut_bytes_sent() const{
	return out_bytes_sent_;
}

void Client::setOut_bytes_sent(size_t n){
	out_bytes_sent_ = n;
}

void Client::addOut_bytes_sent(size_t n){
	out_bytes_sent_ += n;
}