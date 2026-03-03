/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pemirand <pemirand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 13:52:08 by pemirand          #+#    #+#             */
/*   Updated: 2026/03/03 19:21:56 by pemirand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <cstddef>

class Client{
	private:
		int			fd_;
		std::string	bf_in_;
		std::string	bf_out_;
		bool		headers_done_;
		size_t		out_bytes_sent_;
	public:
		Client();
		Client(int fd);
		Client(const Client &other);
		Client &operator=(const Client &other);
		~Client();
};

#endif