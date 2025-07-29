#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <regex>
#include <memory>

#include "request.hpp"
#include "response.hpp"

namespace httpapi {

struct Route {
    std::string method;
    std::string path;
    std::string pattern;
    std::regex regex;
    std::vector<std::string> paramNames;
    std::function<void(Request&, Response&)> handler;
    
    Route(const std::string& method, const std::string& path, 
          std::function<void(Request&, Response&)> handler);
    
    bool matches(const std::string& method, const std::string& path) const;
    void extractParams(const std::string& path, Request& req) const;
    
    // Helper method for path to regex conversion
    static std::string pathToRegex(const std::string& path, std::vector<std::string>& paramNames);
};

class Router {
public:
    Router();
    ~Router();
    
    // Route registration
    void get(const std::string& path, std::function<void(Request&, Response&)> handler);
    void post(const std::string& path, std::function<void(Request&, Response&)> handler);
    void put(const std::string& path, std::function<void(Request&, Response&)> handler);
    void delete_(const std::string& path, std::function<void(Request&, Response&)> handler);
    void patch(const std::string& path, std::function<void(Request&, Response&)> handler);
    
    // Route matching
    bool handleRequest(Request& req, Response& res);
    
    // Utility methods
    void clear();
    size_t getRouteCount() const;
    
private:
    std::vector<std::unique_ptr<Route>> routes_;
    
    // Helper methods
    std::string pathToRegex(const std::string& path, std::vector<std::string>& paramNames);
    std::string escapeRegex(const std::string& str);
};

} // namespace httpapi 