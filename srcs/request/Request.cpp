#include "Request.hpp"
#include <Utils.hpp>
#include <cstdlib>
#include <iostream>

// delete later...
void testRequestParsing() {
	std::cout << "===================================== Parsing HTTP Request tests... =====================================" << std::endl;

	struct TestCase {
		const char *name;
		const char *raw;
		bool expectedValid;
		int expectedCode;
	};

	TestCase tests[] = {
		{"Valid POST (200)", "POST /submit-form HTTP/1.0\r\nHost: example.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 20\r\n\r\nname=John+Doe&age=30", true, 200},
		{"POST missing Content-Length (411)", "POST /submit-form HTTP/1.0\r\nHost: example.com\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nname=John+Doe&age=30", false, 411},
		{"Invalid version (505)", "GET /index.html?debug=true HTTP/1.1\r\nHost: example.com\r\n\r\n", false, 505},
		{"Valid GET (200)", "GET /index.html?debug=true HTTP/1.0\r\nHost: example.com\r\n\r\n", true, 200},
		{"Invalid method (405)", "PUT /index.html HTTP/1.0\r\nHost: example.com\r\n\r\n", false, 405},
		{"Missing path (400)", "GET  HTTP/1.0\r\nHost: example.com\r\n\r\n", false, 400},
		{"Invalid version (505)", "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n", false, 505},
		{"Headers with extra whitespace (200)", "GET /index.html HTTP/1.0\r\nHost: example.com\r\nContent-Type: text/html \r\nContent-Length: 5 \r\n\r\nHello", true, 200},
		{"Invalid header format (400)", "GET /index.html HTTP/1.0\r\nHost example.com\r\nContent-Type text/html\r\n\r\n", false, 400},
		{"Empty body with Content-Length (400)", "POST /submit HTTP/1.0\r\nHost: example.com\r\nContent-Length: 5\r\n\r\n", false, 400},
		{"Body length mismatch (400)", "POST /submit HTTP/1.0\r\nHost: example.com\r\nContent-Length: 5\r\n\r\nHi", false, 400},
		{"Valid POST with empty body (200)", "POST /submit HTTP/1.0\r\nHost: example.com\r\nContent-Length: 0\r\n\r\n", true, 200},
		{"Invalid request line (400)", "GET/index.htmlHTTP/1.0\r\n\n\r\n", false, 400},
		{"Query string parsing (200)", "GET /search?q=webserver&lang=en HTTP/1.0\r\nHost: example.com\r\n\r\n", true, 200},
		{"GET with a full page (200)", "GET /index.html HTTP/1.0\r\nHost: example.com\r\n\r\n<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, World!</h1></body></html>", true, 200}
	};

	for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		Request request(tests[i].raw);

		std::cout << "--- " << tests[i].name << " ---" << std::endl;
		std::cout << "Method: '" << request.getMethod() << "'" << std::endl;
		std::cout << "Path: '" << request.getPath() << "'" << std::endl;
		std::cout << "Query: '" << request.getQuery() << "'" << std::endl;
		std::cout << "Version: '" << request.getVersion() << "'" << std::endl;

		std::cout << "Headers:" << std::endl;
		const std::map<std::string, std::string> &headers = request.getHeaders();
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
			std::cout << "  '" << it->first << "': '" << it->second << "'" << std::endl;
		}
		std::cout << "Body: '" << request.getBody() << "'" << std::endl;
		std::cout << "Valid: " << (request.isValid() ? "true" : "false") << " (expected "
		          << (tests[i].expectedValid ? "true" : "false") << ")" << std::endl;
		std::cout << "Error code: " << request.getErrorCode() << " (expected " << tests[i].expectedCode << ")"
		          << std::endl;
		std::cout << std::endl;
	}
}

Request::Request(const std::string &raw) : valid(true), errorCode(200) {
	if (!parseRequestLine(raw) || !parseMethod() || !parsePath() || !parseVersion() || !parseHeaders(raw) ||
	    !parseBody(raw))
		return;
}

// splits the first line into method, path, and version. Returns false if the format is invalid.
bool Request::parseRequestLine(const std::string &raw) {
	size_t lineEnd = raw.find("\r\n");
	if (lineEnd == std::string::npos) {
		errorCode = 400;
		return valid = false;
	}

	std::string requestLine = raw.substr(0, lineEnd);
	std::vector<std::string> parts = splitByWhitespace(requestLine);
	if (parts.size() != 3) {
		errorCode = 400;
		return valid = false;
	}

	_method = parts[0];
	_path = parts[1];
	_version = parts[2];
	return true;
}

// You need at least the GET, POST, and DELETE methods.
bool Request::parseMethod() {
	if (_method != "GET" && _method != "POST" && _method != "DELETE") {
		errorCode = 405; // Method Not Allowed
		return valid = false;
	}
	return true;
}

bool Request::parsePath() {
	if (_path.empty() || _path[0] != '/') {
		errorCode = 400;
		return valid = false;
	}

	_query.clear();
	size_t queryPos = _path.find('?');
	if (queryPos != std::string::npos) {
		_query = _path.substr(queryPos + 1);
		_path = _path.substr(0, queryPos);
		if (_path.empty())
			_path = "/";
	}
	return true;
}

bool Request::parseVersion() {
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
		errorCode = 505;
		return valid = false;
	}
	return true;
}
// TODO: HTTP/1.1 support
// Currently only accepting HTTP/1.0 to keep things simple.
// To support 1.1, we would need to handle:
//	- Mandatory "Host" header validation (400 if missing)
//	- "Connection: keep-alive" / "Connection: close" header (1.0 closes by default)
//	- "Transfer-Encoding: chunked" body decoding (required for CGI POST input)
//	- "Expect: 100-continue" header (client waits for OK before sending body)
// For now, accepting both 1.0 and 1.1 but treating them the same would be the minimal fix to avoid rejecting modern browser requests.

bool Request::parseHeaders(const std::string &raw) {
	size_t firstLineEnd = raw.find("\r\n");
	if (firstLineEnd == std::string::npos) {
		errorCode = 400;
		return valid = false;
	}
	size_t headersStart = firstLineEnd + 2; // skip the first line and the \r\n
	size_t headersEnd = raw.find("\r\n\r\n");
	if (headersEnd == std::string::npos || headersEnd < headersStart) {
		errorCode = 400;
		return valid = false;
	}
	std::string headersStr = raw.substr(headersStart, headersEnd - headersStart); // no header is still valid
	if (headersStr.empty())
		return true;

	std::vector<std::string> lines = ftSplit(headersStr, "\r\n"); // split headers into lines
	for (size_t i = 0; i < lines.size(); ++i) {
		if (lines[i].empty())
			continue;
		size_t colonPos = lines[i].find(':');
		if (colonPos == std::string::npos) {
			errorCode = 400;
			return valid = false;
		}
		std::string key = lines[i].substr(0, colonPos);
		std::string value = lines[i].substr(colonPos + 1);
		key = ftTrim(key);
		value = ftTrim(value);
		if (key.empty()) {
			errorCode = 400;
			return valid = false;
		}
		for (size_t i = 0; i < key.size(); ++i) {
			key[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[i])));
		}
		_headers[key] = value;
	}
	return true;
}

bool Request::parseBody(const std::string &raw) {
	size_t bodyStart = raw.find("\r\n\r\n");
	if (bodyStart == std::string::npos) { // no body is still valid
		errorCode = 400;
		return valid = false;
	}

	std::string body = raw.substr(bodyStart + 4);
	std::map<std::string, std::string>::const_iterator it = _headers.find("content-length");
	if (it == _headers.end()) {
		if (_method == "POST") { // POST requests must have a body, so Content-Length is required
			errorCode = 411;
			return valid = false;
		}
		_body = body;
		return true;
	}

	const std::string &lengthValue = it->second;
	if (lengthValue.empty()) { // Content-Length header is present but empty
		errorCode = 400;
		return valid = false;
	}
	for (size_t i = 0; i < lengthValue.size(); ++i) {
		if (lengthValue[i] < '0' || lengthValue[i] > '9') {
			errorCode = 400;
			return valid = false;
		}
	}

	long contentLength =
	    std::strtol(lengthValue.c_str(), NULL, 10); // strtol returns 0 on failure, but we already checked for non-digit
	                                                // characters, so 0 is valid if the header is "Content-Length: 0"
	if (contentLength < 0) {
		errorCode = 400;
		return valid = false;
	}
	if (static_cast<size_t>(contentLength) != body.size()) {
		errorCode = 400;
		return valid = false;
	}
	_body = body.substr(0, static_cast<size_t>(contentLength));
	return true;
}

// TODO:
// * URI decoding for path and query string (e.g. %20 -> space, %2F -> /)
// ...

std::string Request::getMethod() const {
	return _method;
}
std::string Request::getPath() const {
	return _path;
}
std::string Request::getQuery() const {
	return _query;
}
std::string Request::getVersion() const {
	return _version;
}
std::map<std::string, std::string> Request::getHeaders() const {
	return _headers;
}
std::string Request::getBody() const {
	return _body;
}
bool Request::isValid() const {
	return valid;
}
int Request::getErrorCode() const {
	return errorCode;
}
