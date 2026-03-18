#include "../includes/Response.hpp"
#include "../includes/webserv.hpp"
#include "../includes/Utils.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cerrno>

static std::string itoa_int(int n){
	char buf[32];
	std::snprintf(buf, sizeof(buf), "%d", n);
	return std::string(buf);
}

static std::string build_no_content_response() {
	return std::string("HTTP/1.1 ") + STATUS204 + "\r\n"
		+ "Content-Length: 0\r\n"
		+ "Connection: close\r\n"
		+ "\r\n";
}

static std::string build_created_response(const std::string &body) {
	return std::string("HTTP/1.1 ") + STATUS201 + "\r\n"
		+ "Content-Type: text/plain\r\n"
		+ "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
		+ "Connection: close\r\n"
		+ "\r\n"
		+ body;
}

static std::string mime_type(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos) return "application/octet-stream";
	std::string ext = path.substr(dot);
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css")  return "text/css";
	if (ext == ".js")   return "application/javascript";
	if (ext == ".png")  return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif")  return "image/gif";
	if (ext == ".ico")  return "image/x-icon";
	if (ext == ".txt")  return "text/plain";
	return "application/octet-stream";
}

static bool hasUnsafePathSegments(const std::string &relativePath) {
	std::vector<std::string> parts = ftSplit(relativePath, "/");
	for (size_t i = 0; i < parts.size(); ++i) {
		if (parts[i].empty())
			continue;
		if (parts[i] == "." || parts[i] == "..")
			return true;
	}
	return false;
}

std::string serve_file(const std::string& method, const std::string& requestPath, const std::string& requestBody, const ServerConfig& config) {
	const Location *loc = matchLocation(requestPath, config);

	if (requestPath.size() > 1) {
		std::string withSlash = requestPath + "/";
		const Location *slashLoc = matchLocation(withSlash, config);
		if (slashLoc
			&& !slashLoc->path.empty()
			&& slashLoc->path[slashLoc->path.size() - 1] == '/'
			&& (!loc || slashLoc->path.size() > loc->path.size())) {
			if (!isMethodAllowedForLocation(method, withSlash, config))
				return build_error_response(405, "Error 405\n");
			return std::string("HTTP/1.1 ") + STATUS301 + "\r\n"
				+ "Location: " + withSlash + "\r\n"
				+ "Content-Type: text/plain\r\n"
				+ "Content-Length: 0\r\n"
				+ "Connection: close\r\n"
				+ "\r\n";
		}
	}

	if (!isMethodAllowedForLocation(method, requestPath, config))
		return build_error_response(405, "Error 405\n");

	if (loc && loc->redirectCode != 0) {
		std::string body = "<html><body>Redirecting to <a href=\"" + loc->redirect + "\">" + loc->redirect + "</a></body></html>\n";
		std::string statusLine = "HTTP/1.1 " + itoa_int(loc->redirectCode) + " Redirect";
		if (loc->redirectCode == 301) statusLine = "HTTP/1.1 " STATUS301;
		if (loc->redirectCode == 302) statusLine = "HTTP/1.1 " STATUS302;
		return statusLine + "\r\n"
			+ "Location: " + loc->redirect + "\r\n"
			+ "Content-Type: text/html\r\n"
			+ "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
			+ "Connection: close\r\n"
			+ "\r\n" + body;
	}

	std::string root  = (loc && !loc->root.empty())  ? loc->root  : "www";
	std::string index = (loc && !loc->index.empty()) ? loc->index : "index.html";

	std::string relativePath = requestPath;
	if (loc && loc->path != "/") {
		const std::string &locPath = loc->path;
		if (requestPath.size() >= locPath.size() && requestPath.substr(0, locPath.size()) == locPath) {
			relativePath = requestPath.substr(locPath.size());
			if (relativePath.empty())
				relativePath = "/";
			else if (relativePath[0] != '/')
				relativePath = "/" + relativePath;
		}
	}

	std::string filePath = root + (relativePath == "/" ? "/" + index : relativePath);

	if (method == "POST") {
		if (relativePath == "/")
			return build_error_response(400, "Error 400\n");
		if (hasUnsafePathSegments(relativePath))
			return build_error_response(400, "Error 400\n");

		std::string uploadBase = root;
		if (loc && !loc->uploadDir.empty())
			uploadBase = loc->uploadDir;

		std::string targetPath = uploadBase + relativePath;
		struct stat targetSt;
		if (stat(targetPath.c_str(), &targetSt) == 0 && S_ISDIR(targetSt.st_mode))
			return build_error_response(403, "Error 403\n");

		std::ofstream out(targetPath.c_str(), std::ios::binary | std::ios::trunc);
		if (!out.is_open()) {
			if (errno == EACCES || errno == EPERM)
				return build_error_response(403, "Error 403\n");
			return build_error_response(500, "Error 500\n");
		}
		if (!requestBody.empty())
			out.write(requestBody.data(), static_cast<std::streamsize>(requestBody.size()));
		if (!out.good()) {
			out.close();
			return build_error_response(500, "Error 500\n");
		}
		out.close();

		return build_created_response("Created\n");
	}

	if (method == "DELETE") {
		if (relativePath == "/")
			return build_error_response(403, "Error 403\n");
		if (hasUnsafePathSegments(relativePath))
			return build_error_response(400, "Error 400\n");

		std::string deleteBase = root;
		if (loc && !loc->uploadDir.empty())
			deleteBase = loc->uploadDir;

		std::string targetPath = deleteBase + relativePath;
		struct stat targetSt;
		if (stat(targetPath.c_str(), &targetSt) != 0)
			return build_error_response(404, "Error 404\n");

		if (S_ISDIR(targetSt.st_mode))
			return build_error_response(403, "Error 403\n");

		if (std::remove(targetPath.c_str()) != 0) {
			if (errno == ENOENT)
				return build_error_response(404, "Error 404\n");
			if (errno == EACCES || errno == EPERM)
				return build_error_response(403, "Error 403\n");
			return build_error_response(500, "Error 500\n");
		}

		return build_no_content_response();
	}

	struct stat st;
	if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		if (filePath.empty() || filePath[filePath.size() - 1] != '/')
			filePath += "/";
		filePath += index;
	}

	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::string body404;
		std::map<int, std::string>::const_iterator ep = config.errorPages.find(404);
		if (ep != config.errorPages.end()) {
			std::ifstream ef((root + ep->second).c_str(), std::ios::binary);
			if (ef.is_open()) { std::ostringstream ss; ss << ef.rdbuf(); body404 = ss.str(); }
		}
		if (body404.empty()) {
			std::ifstream page404("www/404.html", std::ios::binary);
			if (page404.is_open()) { std::ostringstream ss; ss << page404.rdbuf(); body404 = ss.str(); }
			else body404 = "<h1>404 Not Found</h1>\n";
		}
		return std::string("HTTP/1.1 ") + STATUS404 + "\r\n"
			+ "Content-Type: text/html\r\n"
			+ "Content-Length: " + itoa_int((int)body404.size()) + "\r\n"
			+ "Connection: close\r\n"
			+ "\r\n" + body404;
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	std::string body = ss.str();
	return std::string("HTTP/1.1 ") + STATUS200 + "\r\n"
		+ "Content-Type: " + mime_type(filePath) + "\r\n"
		+ "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
		+ "Connection: close\r\n"
		+ "\r\n" + body;
}

std::string build_error_response(int errorCode, const std::string& body) {
	std::string statusLine;
	switch (errorCode) {
		case 400: statusLine = "HTTP/1.1 " STATUS400 "\r\n"; break;
		case 403: statusLine = "HTTP/1.1 " STATUS403 "\r\n"; break;
		case 404: statusLine = "HTTP/1.1 " STATUS404 "\r\n"; break;
		case 405: statusLine = "HTTP/1.1 " STATUS405 "\r\n"; break;
		case 411: statusLine = "HTTP/1.1 " STATUS411 "\r\n"; break;
		case 413: statusLine = "HTTP/1.1 " STATUS413 "\r\n"; break;
		case 505: statusLine = "HTTP/1.1 " STATUS505 "\r\n"; break;
		default: statusLine = "HTTP/1.1 " STATUS500 "\r\n"; break;
	}
	return statusLine
		+ "Content-Type: text/plain\r\n"
		+ "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
		+ "Connection: close\r\n"
		+ "\r\n"
		+ body;
}

static std::string build_ok_response(const std::string &body) {
	return std::string("HTTP/1.1 ") + STATUS200 + "\r\n"
		+ "Content-Type: text/plain\r\n"
		+ "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
		+ "Connection: close\r\n"
		+ "\r\n"
		+ body;
}

std::string build_response_from_request(const std::string &rawRequest, const ServerConfig &config) {
	Request request(rawRequest);
	if (!request.isValid()) {
		int errorCode = request.getErrorCode();
		std::string errorBody = "Error " + itoa_int(errorCode) + "\n";
		return build_error_response(errorCode, errorBody);
	}

	if (request.getMethod() == "POST" && request.getPath() == "/post_body") {
		if (request.getBody().size() > 100)
			return build_error_response(413, "Error 413\n");
		return build_ok_response("POST OK\n");
	}

	return serve_file(request.getMethod(), request.getPath(), request.getBody(), config);
}
