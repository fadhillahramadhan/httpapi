#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <any>

namespace httpapi {

class Request {
public:
    Request();
    
    // HTTP method
    std::string method;
    std::string url;
    std::string path;
    std::string queryString;
    std::string protocol;
    
    // Headers
    std::unordered_map<std::string, std::string> headers;
    
    // Body
    std::string body;
    
    // Parameters (from URL and body)
    std::unordered_map<std::string, std::string> params;
    std::unordered_map<std::string, std::string> queryParams;
    
    // Express.js style properties
    std::string get(const std::string& header) const;
    std::string param(const std::string& name) const;
    std::string query(const std::string& name) const;
    
    // Body parsing
    template<typename T>
    T getBody() const;
    
    // Utility methods
    bool is(const std::string& type) const;
    std::string getContentType() const;
    size_t getContentLength() const;
    
    // Internal use
    void setParam(const std::string& name, const std::string& value);
    void setQueryParam(const std::string& name, const std::string& value);
    void parseQueryString();
    void parseBody();
    
private:
    std::unordered_map<std::string, std::any> locals_;
};

// Template implementation for body parsing
template<typename T>
T Request::getBody() const {
    // Default implementation - can be specialized for different types
    return T();
}

} // namespace httpapi 