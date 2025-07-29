#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

#include "request.hpp"
#include "response.hpp"

namespace httpapi {

class Middleware {
public:
    using MiddlewareFunction = std::function<void(Request&, Response&, std::function<void()>)>;
    
    Middleware();
    ~Middleware();
    
    // Add middleware
    void use(MiddlewareFunction middleware);
    void use(const std::string& path, MiddlewareFunction middleware);
    
    // Execute middleware chain
    void execute(Request& req, Response& res, std::function<void()> next);
    
    // Clear all middleware
    void clear();
    
    // Get middleware count
    size_t getMiddlewareCount() const;
    
private:
    struct MiddlewareEntry {
        std::string path;
        MiddlewareFunction function;
        
        MiddlewareEntry(const std::string& path, MiddlewareFunction func)
            : path(path), function(func) {}
    };
    
    std::vector<MiddlewareEntry> middlewareChain_;
    
    // Helper methods
    bool pathMatches(const std::string& middlewarePath, const std::string& requestPath);
    void executeNext(Request& req, Response& res, 
                    std::vector<MiddlewareEntry>::iterator current,
                    std::function<void()> finalNext);
};

} // namespace httpapi 