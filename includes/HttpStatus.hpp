#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <string>
#include <map> //map function for dependecy teste

class HttpStatus {
public:
	static std::string getPhrase(int code);

private:
	static std::map<int, std::string> statusMap;
};

#endif
