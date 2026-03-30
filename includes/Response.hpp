#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include "Config.hpp"

std::string serve_file(const std::string& method, const std::string& requestPath, const std::string& requestBody, const ServerConfig& config);
std::string buildErrorResponse(int errorCode, const std::string& body);
std::string buildResponseFromRequest(const std::string &rawRequest, const ServerConfig &config);


#endif
