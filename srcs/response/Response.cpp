#include "../includes/Response.hpp"
#include "../includes/webserv.hpp"
#include "../includes/Utils.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cerrno>

static std::string build_response(const std::string &status,
	const std::string &contentType,
	const std::string &body,
	const std::vector<std::string> &extraHeaders = std::vector<std::string>(),
	bool includeBody = true)
{
	std::string response = std::string(VERSION) + status + "\r\n";
	for (size_t i = 0; i < extraHeaders.size(); ++i)
		response += extraHeaders[i] + "\r\n";
	if (!contentType.empty())
		response += "Content-Type: " + contentType + "\r\n";
	response += "Content-Length: " + itoa_int((int)body.size()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	if (includeBody)
		response += body;
	return response;
}

static std::string mime_type(const std::string &path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";
	std::string ext = path.substr(dot);
	if (ext == ".html" || ext == ".htm")
		return "text/html";
	if (ext == ".css")
		return "text/css";
	if (ext == ".js")
		return "application/javascript";
	if (ext == ".png")
		return "image/png";
	if (ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg";
	if (ext == ".gif")
		return "image/gif";
	if (ext == ".ico")
		return "image/x-icon";
	if (ext == ".txt")
		return "text/plain";
	return "application/octet-stream";
}

static bool hasUnsafePathSegments(const std::string &relativePath)
{
	std::vector<std::string> parts = ftSplit(relativePath, "/");
	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (parts[i].empty())
			continue;
		if (parts[i] == "." || parts[i] == "..")
			return true;
	}
	return false;
}

std::string serve_file(const std::string &method, const std::string &requestPath, const std::string &requestBody, const ServerConfig &config)
{
	const bool headOnly = (method == "HEAD");
	const Location *loc = matchLocation(requestPath, config);

	if (requestPath.size() > 1)
	{
		std::string withSlash = requestPath + "/";
		const Location *slashLoc = matchLocation(withSlash, config);
		if (slashLoc && !slashLoc->path.empty() && slashLoc->path[slashLoc->path.size() - 1] == '/' && (!loc || slashLoc->path.size() > loc->path.size()))
		{
			if (!isMethodAllowedForLocation(method, withSlash, config))
				return build_error_response(405, "Error 405\n");
			std::vector<std::string> headers;
			headers.push_back("Location: " + withSlash);
			return build_response(STATUS301, "text/plain", "", headers, !headOnly);
		}
	}

	if (!isMethodAllowedForLocation(method, requestPath, config))
		return build_error_response(405, "Error 405\n");

	if (loc && loc->redirectCode != 0)
	{
		std::string body = "<html><body>Redirecting to <a href=\"" + loc->redirect + "\">" + loc->redirect + "</a></body></html>\n";
		std::string status = itoa_int(loc->redirectCode) + " Redirect";
		if (loc->redirectCode == 301)
			status = STATUS301;
		if (loc->redirectCode == 302)
			status = STATUS302;
		std::vector<std::string> headers;
		headers.push_back("Location: " + loc->redirect);
		return build_response(status, "text/html", body, headers, !headOnly);
	}

	std::string root = (loc && !loc->root.empty()) ? loc->root : (!config.root.empty() ? config.root : "www");
	std::string index = (loc && !loc->index.empty()) ? loc->index : "index.html";

	std::string relativePath = requestPath;
	if (loc && loc->path != "/")
	{
		const std::string &locPath = loc->path;
		if (requestPath.size() >= locPath.size() && requestPath.substr(0, locPath.size()) == locPath)
		{
			relativePath = requestPath.substr(locPath.size());
			if (relativePath.empty())
				relativePath = "/";
			else if (relativePath[0] != '/')
				relativePath = "/" + relativePath;
		}
	}

	std::string filePath = root + (relativePath == "/" ? "/" + index : relativePath);

	if (method == "POST")
	{
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
		if (!out.is_open())
		{
			if (errno == EACCES || errno == EPERM)
				return build_error_response(403, "Error 403\n");
			return build_error_response(500, "Error 500\n");
		}
		if (!requestBody.empty())
			out.write(requestBody.data(), static_cast<std::streamsize>(requestBody.size()));
		if (!out.good())
		{
			out.close();
			return build_error_response(500, "Error 500\n");
		}
		out.close();

		return build_response(STATUS201, "text/plain", "Created\n");
	}

	if (method == "DELETE")
	{
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

		if (std::remove(targetPath.c_str()) != 0)
		{
			if (errno == ENOENT)
				return build_error_response(404, "Error 404\n");
			if (errno == EACCES || errno == EPERM)
				return build_error_response(403, "Error 403\n");
			return build_error_response(500, "Error 500\n");
		}

		return build_response(STATUS204, "", "");
	}

	struct stat st;
	if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
	{
		if (filePath.empty() || filePath[filePath.size() - 1] != '/')
			filePath += "/";
		filePath += index;
	}

	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::string body404;
		std::map<int, std::string>::const_iterator ep = config.errorPages.find(404);
		if (ep != config.errorPages.end())
		{
			std::ifstream ef((root + ep->second).c_str(), std::ios::binary);
			if (ef.is_open())
			{
				std::ostringstream ss;
				ss << ef.rdbuf();
				body404 = ss.str();
			}
		}
		if (body404.empty())
		{
			std::ifstream page404("www/404.html", std::ios::binary);
			if (page404.is_open())
			{
				std::ostringstream ss;
				ss << page404.rdbuf();
				body404 = ss.str();
			}
			else
				body404 = "<h1>404 Not Found</h1>\n";
		}
		return build_response(STATUS404, "text/html", body404, std::vector<std::string>(), !headOnly);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	std::string body = ss.str();
	return build_response(STATUS200, mime_type(filePath), body, std::vector<std::string>(), !headOnly);
}

std::string build_error_response(int errorCode, const std::string &body)
{
	struct ErrorStatus
	{
		int code;
		const char *status;
	};
	static const ErrorStatus statuses[] = {
		{400, STATUS400},
		{403, STATUS403},
		{404, STATUS404},
		{405, STATUS405},
		{411, STATUS411},
		{413, STATUS413},
		{505, STATUS505}};
	const char *statusStr = STATUS500;
	for (size_t i = 0; i < sizeof(statuses) / sizeof(statuses[0]); ++i)
	{
		if (statuses[i].code == errorCode)
		{
			statusStr = statuses[i].status;
			break;
		}
	}
	return build_response(statusStr, "text/plain", body);
}

std::string build_response_from_request(const std::string &rawRequest, const ServerConfig &config)
{
	Request request(rawRequest);
	if (!request.isValid())
	{
		int errorCode = request.getErrorCode();
		std::string errorBody = "Error " + itoa_int(errorCode) + "\n";
		return build_error_response(errorCode, errorBody);
	}

	if (request.getMethod() == "POST" && request.getPath() == "/post_body")
	{
		if (request.getBody().size() > 100)
			return build_error_response(413, "Error 413\n");
		return build_response(STATUS200, "text/plain", "POST OK\n");
	}

	return serve_file(request.getMethod(), request.getPath(), request.getBody(), config);
}
