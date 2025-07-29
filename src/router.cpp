#include "httpapi/router.hpp"
#include "httpapi/utils.hpp"
#include <iostream>

namespace httpapi {

Route::Route(const std::string& method, const std::string& path, 
             std::function<void(Request&, Response&)> handler)
    : method(method), path(path), handler(handler) {
    
    // Convert Express.js style path to regex
    pattern = pathToRegex(path, paramNames);
    regex = std::regex(pattern);
}

bool Route::matches(const std::string& requestMethod, const std::string& requestPath) const {
    if (method != requestMethod) {
        return false;
    }
    
    std::smatch match;
    return std::regex_match(requestPath, match, regex);
}

void Route::extractParams(const std::string& requestPath, Request& req) const {
    std::smatch match;
    if (std::regex_match(requestPath, match, regex)) {
        for (size_t i = 1; i < match.size() && i - 1 < paramNames.size(); ++i) {
            req.setParam(paramNames[i - 1], match[i].str());
        }
    }
}

std::string Route::pathToRegex(const std::string& path, std::vector<std::string>& paramNames) {
    std::string regex = "^";
    std::string currentParam = "";
    bool inParam = false;
    
    for (size_t i = 0; i < path.length(); ++i) {
        char c = path[i];
        
        if (c == ':') {
            inParam = true;
            currentParam = "";
        } else if (inParam && (c == '/' || i == path.length() - 1)) {
            if (i == path.length() - 1 && c != '/') {
                currentParam += c;
            }
            paramNames.push_back(currentParam);
            regex += "([^/]+)";
            inParam = false;
        } else if (inParam) {
            currentParam += c;
        } else {
            // Escape special regex characters
            if (c == '.' || c == '+' || c == '*' || c == '?' || c == '^' || c == '$' || c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}' || c == '|' || c == '\\') {
                regex += '\\';
            }
            regex += c;
        }
    }
    
    if (inParam) {
        paramNames.push_back(currentParam);
        regex += "([^/]+)";
    }
    
    regex += "$";
    return regex;
}

Router::Router() {
}

Router::~Router() {
}

void Router::get(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_.push_back(std::make_unique<Route>("GET", path, handler));
}

void Router::post(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_.push_back(std::make_unique<Route>("POST", path, handler));
}

void Router::put(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_.push_back(std::make_unique<Route>("PUT", path, handler));
}

void Router::delete_(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_.push_back(std::make_unique<Route>("DELETE", path, handler));
}

void Router::patch(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_.push_back(std::make_unique<Route>("PATCH", path, handler));
}

bool Router::handleRequest(Request& req, Response& res) {
    for (const auto& route : routes_) {
        if (route->matches(req.method, req.path)) {
            route->extractParams(req.path, req);
            route->handler(req, res);
            return true;
        }
    }
    return false;
}

void Router::clear() {
    routes_.clear();
}

size_t Router::getRouteCount() const {
    return routes_.size();
}

} // namespace httpapi 