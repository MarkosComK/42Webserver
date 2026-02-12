/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marsoare <marsoare@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 20:13:27 by marsoare          #+#    #+#             */
/*   Updated: 2026/02/12 20:15:59 by marsoare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <HttpStatus.hpp>

std::string HttpStatus::getPhrase(int code) {
	if (code == 99)
		return ("code was 99 bruh");
	return "SomeStringHere";
}
