/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marsoare <marsoare@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:21:48 by marsoare          #+#    #+#             */
/*   Updated: 2026/02/12 20:22:21 by marsoare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <webserv.hpp>

int main() {
	std::cout << "HTTP Status 99: " << HttpStatus::getPhrase(99) << std::endl;

	return 0;
}
