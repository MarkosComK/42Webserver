/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:10:11 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/03 19:46:59 by pemirand         ###   ########.fr       */
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