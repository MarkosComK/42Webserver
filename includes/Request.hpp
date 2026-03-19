#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include <vector>

class Request {
	private:
		std::string _method;
		std::string _path;
		std::string _query;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::string _body;
		bool valid;
		int errorCode;
		bool parseRequestLine(const std::string &raw);

	public:
		Request(const std::string &raw);
		bool parseMethod();
		bool parsePath();
		bool parseVersion();
		bool parseHeaders(const std::string &raw);
		bool parseBody(const std::string &raw);

		std::string getMethod() const;
		std::string getPath() const;
		std::string getQuery() const;
		std::string getVersion() const;
		std::map<std::string, std::string> getHeaders() const;
		std::string getBody() const;
		bool isValid() const;
		int getErrorCode() const;
};

void testRequestParsing(); // delete later...

#endif
