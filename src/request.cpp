#include "httpapi/request.hpp"
#include "httpapi/utils.hpp"
#include <sstream>
#include <algorithm>

namespace httpapi {

Request::Request() {
    // Initialize default values
}

std::string Request::get(const std::string& header) const {
    auto it = headers.find(Utils::normalizeHeaderName(header));
    return (it != headers.end()) ? it->second : "";
}

std::string Request::param(const std::string& name) const {
    auto it = params.find(name);
    return (it != params.end()) ? it->second : "";
}

std::string Request::query(const std::string& name) const {
    auto it = queryParams.find(name);
    return (it != queryParams.end()) ? it->second : "";
}

bool Request::is(const std::string& type) const {
    std::string contentType = getContentType();
    return contentType.find(type) != std::string::npos;
}

std::string Request::getContentType() const {
    return get("content-type");
}

size_t Request::getContentLength() const {
    std::string contentLength = get("content-length");
    return contentLength.empty() ? 0 : std::stoul(contentLength);
}

void Request::setParam(const std::string& name, const std::string& value) {
    params[name] = value;
}

void Request::setQueryParam(const std::string& name, const std::string& value) {
    queryParams[name] = value;
}

void Request::parseQueryString() {
    if (queryString.empty()) {
        return;
    }

    std::vector<std::string> pairs = Utils::split(queryString, '&');
    for (const auto& pair : pairs) {
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string name = Utils::urlDecode(pair.substr(0, equalPos));
            std::string value = Utils::urlDecode(pair.substr(equalPos + 1));
            queryParams[name] = value;
        }
    }
}

void Request::parseBody() {
    // This is a basic implementation
    // In a real framework, you'd want to handle different content types
    // like application/json, application/x-www-form-urlencoded, etc.
    
    std::string contentType = getContentType();
    if (contentType.find("application/json") != std::string::npos) {
        // Parse JSON body
        // This would be implemented with a JSON parser
    } else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
        // Parse form data
        std::vector<std::string> pairs = Utils::split(body, '&');
        for (const auto& pair : pairs) {
            size_t equalPos = pair.find('=');
            if (equalPos != std::string::npos) {
                std::string name = Utils::urlDecode(pair.substr(0, equalPos));
                std::string value = Utils::urlDecode(pair.substr(equalPos + 1));
                params[name] = value;
            }
        }
    }
}

} // namespace httpapi 