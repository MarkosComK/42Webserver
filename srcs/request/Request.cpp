#include <Request.hpp>
#include <Utils.hpp> // For split()
#include <iostream>

Request::Request(const std::string &raw) : valid(true), errorCode(200) {
	parseFunction(raw); // call functions to parse the raw request and populate the attributes
}
