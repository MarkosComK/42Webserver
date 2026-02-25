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

int main(int argc, char *argv[]) {
	std::string configPath = "config/webserv.conf"; // default path, if no argument is provided

	if (argc > 1) {
		configPath = argv[1];
	}
	// std::cout << "HTTP Status 99: " << HttpStatus::getPhrase(99) << std::endl;

	//testRequestParsing();
	testConfigParsing(configPath);

	return 0;
}
