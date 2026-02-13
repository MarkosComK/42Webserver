#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include <vector>

/*Define HTTP Request Class
Description: Create the class that will hold the parsed request data.
Attributes:
_method (GET, POST, DELETE).
_path (URI).
_version (HTTP/1.1 or 1.0).
_headers (std::map<string, string>).
_body (std::vector or string).*/

class Request {
  private:
	std::string _method;
	std::string _path;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;
	bool valid;
	int errorCode;

  public:
	Request(const std::string &raw);
};

#endif
