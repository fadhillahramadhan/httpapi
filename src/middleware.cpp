#include "httpapi/middleware.hpp"
#include "httpapi/utils.hpp"
#include <iostream>

namespace httpapi {

Middleware::Middleware() {
}

Middleware::~Middleware() {
}

void Middleware::use(MiddlewareFunction middleware) {
    middlewareChain_.emplace_back("", middleware);
}

void Middleware::use(const std::string& path, MiddlewareFunction middleware) {
    middlewareChain_.emplace_back(path, middleware);
}

void Middleware::execute(Request& req, Response& res, std::function<void()> next) {
    if (middlewareChain_.empty()) {
        next();
        return;
    }
    
    executeNext(req, res, middlewareChain_.begin(), next);
}

void Middleware::clear() {
    middlewareChain_.clear();
}

size_t Middleware::getMiddlewareCount() const {
    return middlewareChain_.size();
}

bool Middleware::pathMatches(const std::string& middlewarePath, const std::string& requestPath) {
    if (middlewarePath.empty()) {
        return true; // Global middleware
    }
    
    // Simple path matching - can be enhanced with regex
    return requestPath.find(middlewarePath) == 0;
}

void Middleware::executeNext(Request& req, Response& res, 
                           std::vector<MiddlewareEntry>::iterator current,
                           std::function<void()> finalNext) {
    if (current == middlewareChain_.end()) {
        finalNext();
        return;
    }
    
    auto& entry = *current;
    auto next = [this, &req, &res, current, finalNext]() mutable {
        executeNext(req, res, ++current, finalNext);
    };
    
    if (pathMatches(entry.path, req.path)) {
        entry.function(req, res, next);
    } else {
        next();
    }
}

} // namespace httpapi 