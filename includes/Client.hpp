/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 13:52:08 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/21 21:44:25 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <cstddef>

class Socket;

class Client{
	private:
		int			fd_;
		std::string	bf_in_;
		std::string	bf_out_;
		bool		headers_done_;
		size_t		out_bytes_sent_;
		Socket*		socket_;
	public:
		Client();
		Client(int fd, Socket* socket);
		Client(const Client &other);
		Client &operator=(const Client &other);
		~Client();

		bool getHeaders_done() const;
		Socket* getSocket() const;
		void setHeaders_done(bool value = true);
		const std::string& getBf_in() const;
		void appendBf_in(const char *data, size_t n);
		const std::string& getBf_out() const;
		void appendBf_out(const std::string &data);
		size_t getOut_bytes_sent() const;
		void setOut_bytes_sent(size_t n);
		void addOut_bytes_sent(size_t n);
};

#endif
