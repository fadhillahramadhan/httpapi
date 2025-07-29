#include "httpapi/response.hpp"
#include "httpapi/utils.hpp"
#include <sstream>
#include <fstream>

namespace httpapi {

Response::Response() 
    : statusCode(200), statusMessage("OK"), headersSent_(false), ended_(false) {
    setDefaultHeaders();
}

Response& Response::status(int code) {
    statusCode = code;
    statusMessage = getStatusText(code);
    return *this;
}

Response& Response::set(const std::string& field, const std::string& value) {
    headers[Utils::normalizeHeaderName(field)] = value;
    return *this;
}

Response& Response::header(const std::string& field, const std::string& value) {
    return set(field, value);
}

Response& Response::send(const std::string& data) {
    body = data;
    if (!headersSent_) {
        set("Content-Length", std::to_string(data.length()));
        headersSent_ = true;
    }
    ended_ = true;
    return *this;
}

Response& Response::json(const std::string& data) {
    set("Content-Type", "application/json");
    return send(data);
}

Response& Response::sendFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return status(404).send("File not found");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Set appropriate content type based on file extension
    std::string extension = Utils::getFileExtension(path);
    std::string mimeType = "text/plain"; // Default MIME type
    if (extension == "html" || extension == "htm") {
        mimeType = "text/html";
    } else if (extension == "css") {
        mimeType = "text/css";
    } else if (extension == "js") {
        mimeType = "application/javascript";
    } else if (extension == "json") {
        mimeType = "application/json";
    } else if (extension == "png") {
        mimeType = "image/png";
    } else if (extension == "jpg" || extension == "jpeg") {
        mimeType = "image/jpeg";
    }
    set("Content-Type", mimeType);

    return send(content);
}

Response& Response::redirect(const std::string& url) {
    status(302);
    set("Location", url);
    return send("");
}

Response& Response::ok() {
    return status(200);
}

Response& Response::created() {
    return status(201);
}

Response& Response::noContent() {
    return status(204);
}

Response& Response::badRequest() {
    return status(400);
}

Response& Response::unauthorized() {
    return status(401);
}

Response& Response::forbidden() {
    return status(403);
}

Response& Response::notFound() {
    return status(404);
}

Response& Response::internalServerError() {
    return status(500);
}

std::string Response::toString() const {
    std::ostringstream response;
    
    // Status line
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    
    // Headers
    for (const auto& header : headers) {
        response << header.first << ": " << header.second << "\r\n";
    }
    
    // Empty line to separate headers from body
    response << "\r\n";
    
    // Body
    if (!body.empty()) {
        response << body;
    }
    
    return response.str();
}

void Response::clear() {
    statusCode = 200;
    statusMessage = "OK";
    headers.clear();
    body.clear();
    headersSent_ = false;
    ended_ = false;
    setDefaultHeaders();
}

void Response::setDefaultHeaders() {
    if (headers.find("Content-Type") == headers.end()) {
        set("Content-Type", "text/plain");
    }
    set("Server", "HttpApi/1.0");
    set("Connection", "close");
}

std::string Response::getStatusText(int code) const {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

} // namespace httpapi 