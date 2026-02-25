/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlos-j <carlos-j@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/02/25 10:36:07 by carlos-j         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"
#include "../includes/ConfigParser.hpp"

int main() {
	// std::cout << "HTTP Status 99: " << HttpStatus::getPhrase(99) << std::endl;

	//testRequestParsing();
	testConfigParsing("config/webserv.conf");

	return 0;
}
