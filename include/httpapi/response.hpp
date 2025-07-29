#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

namespace httpapi {

class Response {
public:
    Response();
    
    // Status
    int statusCode;
    std::string statusMessage;
    
    // Headers
    std::unordered_map<std::string, std::string> headers;
    
    // Body
    std::string body;
    
    // Express.js style methods
    Response& status(int code);
    Response& set(const std::string& field, const std::string& value);
    Response& header(const std::string& field, const std::string& value);
    
    // Sending responses
    Response& send(const std::string& data);
    Response& json(const std::string& data);
    Response& sendFile(const std::string& path);
    Response& redirect(const std::string& url);
    
    // Status helpers
    Response& ok();
    Response& created();
    Response& noContent();
    Response& badRequest();
    Response& unauthorized();
    Response& forbidden();
    Response& notFound();
    Response& internalServerError();
    
    // Utility methods
    std::string toString() const;
    void clear();
    
    // Internal use
    void setDefaultHeaders();
    std::string getStatusText(int code) const;
    
private:
    bool headersSent_;
    bool ended_;
};

} // namespace httpapi 