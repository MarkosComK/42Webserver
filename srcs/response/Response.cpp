#include "../includes/Response.hpp"
#include "../includes/webserv.hpp"
#include "../includes/Utils.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cerrno>
#include <dirent.h>
#include <algorithm>

static std::string buildResponse(const std::string &status,
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

static bool replaceAll(std::string &text, const std::string &from, const std::string &to)
{
	if (from.empty())
		return false;
	bool replaced = false;
	size_t pos = 0;
	while ((pos = text.find(from, pos)) != std::string::npos)
	{
		text.replace(pos, from.length(), to);
		pos += to.length();
		replaced = true;
	}
	return replaced;
}

static std::string buildAutoindexBody(const std::string &dirPath, const std::string &requestPath)
{
	std::vector<std::string> names;
	DIR *dir = opendir(dirPath.c_str());
	if (!dir)
		return "";

	for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
	{
		std::string name = entry->d_name;
		if (name == ".")
			continue;
		names.push_back(name);
	}
	closedir(dir);

	std::sort(names.begin(), names.end());

	std::string baseUri = requestPath;
	if (baseUri.empty())
		baseUri = "/";
	if (baseUri[baseUri.size() - 1] != '/')
		baseUri += "/";

	std::ostringstream listItems;

	for (size_t i = 0; i < names.size(); ++i)
	{
		std::string entryName = names[i];
		std::string fullPath = dirPath;
		if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
			fullPath += "/";
		fullPath += entryName;

		bool isDir = false;
		struct stat entrySt;
		if (stat(fullPath.c_str(), &entrySt) == 0)
			isDir = S_ISDIR(entrySt.st_mode);

		std::string display = entryName + (isDir ? "/" : "");
		std::string href = baseUri + entryName + (isDir ? "/" : "");
		listItems << "<li><a href=\"" << href << "\">" << display << "</a></li>\n";
	}

	std::ifstream templateFile("www/autoindex.html", std::ios::binary);
	if (templateFile.is_open())
	{
		std::ostringstream tmpl;
		tmpl << templateFile.rdbuf();
		std::string html = tmpl.str();
		replaceAll(html, "{{TITLE}}", "Index of " + baseUri);
		replaceAll(html, "{{PATH}}", baseUri);
		replaceAll(html, "{{LIST_ITEMS}}", listItems.str());
		return html;
	}

	std::ostringstream ss;
	ss << "<!DOCTYPE html>\n";
	ss << "<html><head><meta charset=\"UTF-8\"><title>Index of " << baseUri << "</title></head><body>\n";
	ss << "<h1>Index of " << baseUri << "</h1>\n<ul>\n";
	ss << listItems.str();

	ss << "</ul>\n</body></html>\n";
	return ss.str();
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

static std::string build404PageResponse(const ServerConfig &config, const std::string &root, bool headOnly)
{
	std::string body404;
	std::map<int, std::string>::const_iterator ep = config.errorPages.find(404);
	if (ep != config.errorPages.end()) {
		std::ifstream ef((root + ep->second).c_str(), std::ios::binary);
		if (ef.is_open()) {
			std::ostringstream ss;
			ss << ef.rdbuf();
			body404 = ss.str();
		}
	}
	if (body404.empty()) {
		std::ifstream page404("www/404.html", std::ios::binary);
		if (page404.is_open()) {
			std::ostringstream ss;
			ss << page404.rdbuf();
			body404 = ss.str();
		} else {
			body404 = "<h1>404 Not Found</h1>\n";
		}
	}
	return buildResponse(STATUS404, "text/html", body404, std::vector<std::string>(), !headOnly);
}


// if a location starting with "/" exists, redirect to that path
static std::string redirectIfLocationExists(const std::string &method, const std::string &requestPath, const ServerConfig &config, const Location *loc, bool headOnly) {
	if (requestPath.size() > 1) {
		std::string withSlash = requestPath + "/";
		const Location *slashLoc = matchLocation(withSlash, config);
		if (slashLoc && !slashLoc->path.empty() && slashLoc->path[slashLoc->path.size() - 1] == '/' && (!loc || slashLoc->path.size() > loc->path.size())) {
			if (!isMethodAllowedForLocation(method, withSlash, config))
				return buildErrorResponse(405, "Error 405\n");
			std::vector<std::string> headers;
			headers.push_back("Location: " + withSlash);
			return buildResponse(STATUS301, "text/plain", "", headers, !headOnly);
		}
	}
	return "";
}

// if the location has a redirect configured, return the redirect response
static std::string checkLocationRedirect(const Location *loc, bool headOnly) {
	if (loc && loc->redirectCode != 0) {
		std::string body = "<html><body>Redirecting to <a href=\"" + loc->redirect + "\">" + loc->redirect + "</a></body></html>\n";
		std::string status = itoa_int(loc->redirectCode) + " Redirect";
		if (loc->redirectCode == 301)
			status = STATUS301;
		if (loc->redirectCode == 302)
			status = STATUS302;
		std::vector<std::string> headers;
		headers.push_back("Location: " + loc->redirect);
		return buildResponse(status, "text/html", body, headers, !headOnly);
	}
	return "";
}

// check if the location has a redirect configured, return the redirect response
static void checkRootIndex(const Location *loc, const ServerConfig &config, std::string &root, std::string &index) {
	if (loc && !loc->root.empty()) {
		root = loc->root;
	} else if (!config.root.empty()) {
		root = config.root;
	} else {
		root = "www"; // default root
	}
	if (loc && !loc->index.empty()) {
		index = loc->index;
	} else {
		index = "";
	}
}

// path relative to the location root
static std::string checkRelativePath(const Location *loc, const std::string &requestPath) {
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
	return relativePath;
}

// POST method: create or overwrite a file with the request body as content
static std::string postMethod(const std::string &relativePath, const std::string &requestBody, const Location *loc, const std::string &root) {
	if (relativePath == "/")
		return buildErrorResponse(400, "Error 400\n");
	if (hasUnsafePathSegments(relativePath))
		return buildErrorResponse(400, "Error 400\n");
	std::string uploadBase = root;
	if (loc && !loc->uploadDir.empty())
		uploadBase = loc->uploadDir;
	std::string targetPath = uploadBase + relativePath;
	struct stat targetSt;
	if (stat(targetPath.c_str(), &targetSt) == 0 && S_ISDIR(targetSt.st_mode))
		return buildErrorResponse(403, "Error 403\n");
	std::ofstream out(targetPath.c_str(), std::ios::binary | std::ios::trunc);
	if (!out.is_open()) {
		if (errno == EACCES || errno == EPERM)
			return buildErrorResponse(403, "Error 403\n");
		return buildErrorResponse(500, "Error 500\n");
	}
	if (!requestBody.empty())
		out.write(requestBody.data(), static_cast<std::streamsize>(requestBody.size()));
	if (!out.good()) {
		out.close();
		return buildErrorResponse(500, "Error 500\n");
	}
	out.close();
	return buildResponse(STATUS201, "text/plain", "Created\n");
}

// DELETE method: remove a file
static std::string deleteMethod(const std::string &relativePath, const Location *loc, const std::string &root) {
	if (relativePath == "/")
		return buildErrorResponse(403, "Error 403\n");
	if (hasUnsafePathSegments(relativePath))
		return buildErrorResponse(400, "Error 400\n");
	std::string deleteBase = root;
	if (loc && !loc->uploadDir.empty())
		deleteBase = loc->uploadDir;
	std::string targetPath = deleteBase + relativePath;
	struct stat targetSt;
	if (stat(targetPath.c_str(), &targetSt) != 0)
		return buildErrorResponse(404, "Error 404\n");
	if (S_ISDIR(targetSt.st_mode))
		return buildErrorResponse(403, "Error 403\n");
	if (std::remove(targetPath.c_str()) != 0) {
		if (errno == ENOENT)
			return buildErrorResponse(404, "Error 404\n");
		if (errno == EACCES || errno == EPERM)
			return buildErrorResponse(403, "Error 403\n");
		return buildErrorResponse(500, "Error 500\n");
	}
	return buildResponse(STATUS204, "", "");
}

// for GET and HEAD methods, send the file contents or directory listing
static std::string sendFileOrDirectoryContents(const std::string &filePath, const std::string &index, const std::string &root, const ServerConfig &config, bool headOnly, const Location *loc, const std::string &requestPath) {
	struct stat st;
	std::string finalPath = filePath;
	if (stat(finalPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		if (!index.empty()) {
			std::string indexPath = finalPath;
			if (indexPath.empty() || indexPath[indexPath.size() - 1] != '/')
				indexPath += "/";
			indexPath += index;

			std::ifstream indexFile(indexPath.c_str(), std::ios::binary);
			if (indexFile.is_open()) {
				std::ostringstream ss;
				ss << indexFile.rdbuf();
				return buildResponse(STATUS200, mime_type(indexPath), ss.str(), std::vector<std::string>(), !headOnly);
			}
		}

		if (loc && loc->autoindex) {
			std::string body = buildAutoindexBody(finalPath, requestPath);
			if (body.empty())
				return buildErrorResponse(500, "Error 500\n");
			return buildResponse(STATUS200, "text/html", body, std::vector<std::string>(), !headOnly);
		}

		return build404PageResponse(config, root, headOnly);
	}

	std::ifstream file(finalPath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		return build404PageResponse(config, root, headOnly);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	std::string body = ss.str();
	return buildResponse(STATUS200, mime_type(finalPath), body, std::vector<std::string>(), !headOnly);
}

std::string buildResourceResponse(const std::string &method, const std::string &requestPath, const std::string &requestBody, const ServerConfig &config)
{
	const bool headOnly = (method == "HEAD");
	const Location *loc = matchLocation(requestPath, config);

	std::string result = redirectIfLocationExists(method, requestPath, config, loc, headOnly);

	if (!result.empty())
		return result;
	// from this point, we know the location exists

	if (!isMethodAllowedForLocation(method, requestPath, config))
		return buildErrorResponse(405, "Error 405\n");
	// from this point, we know the method is allowed for the location

	result = checkLocationRedirect(loc, headOnly);
	if (!result.empty())
		return result;
	// from this point, we know there is no redirect configured for the location

	std::string root, index;
	checkRootIndex(loc, config, root, index);
	// from this point, we have the root and index for the location (either from the location or from the server config or default)

	std::string relativePath = checkRelativePath(loc, requestPath);

	std::string filePath;
	if (relativePath == "/") {
		filePath = root + "/" + index;
	} else {
		filePath = root + relativePath;
	}

	if (method == "POST")
		return postMethod(relativePath, requestBody, loc, root);

	if (method == "DELETE")
		return deleteMethod(relativePath, loc, root);

	return sendFileOrDirectoryContents(filePath, index, root, config, headOnly, loc, requestPath);
}

std::string buildErrorResponse(int errorCode, const std::string &body)
{
	struct ErrorStatus
	{
		int code;
		const char *status;
	};
	static const ErrorStatus statuses[] = {
		{200, STATUS200},
		{201, STATUS201},
		{202, STATUS202},
		{204, STATUS204},
		{301, STATUS301},
		{302, STATUS302},
		{304, STATUS304},
		{400, STATUS400},
		{401, STATUS401},
		{403, STATUS403},
		{404, STATUS404},
		{405, STATUS405},
		{411, STATUS411},
		{413, STATUS413},
		{500, STATUS500},
		{501, STATUS501},
		{502, STATUS502},
		{503, STATUS503},
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
	return buildResponse(statusStr, "text/plain", body);
}

std::string buildResponseFromRequest(const std::string &rawRequest, const ServerConfig &config)
{
	Request request(rawRequest);
	if (!request.isValid())
	{
		int errorCode = request.getErrorCode();
		std::string errorBody = "Error " + itoa_int(errorCode) + "\n";
		return buildErrorResponse(errorCode, errorBody);
	}

	if (request.getMethod() == "POST" && request.getPath() == "/post_body")
	{
		if (request.getBody().size() > 100)
			return buildErrorResponse(413, "Error 413\n");
		return buildResponse(STATUS200, "text/plain", "POST OK\n");
	}

	return buildResourceResponse(request.getMethod(), request.getPath(), request.getBody(), config);
}
