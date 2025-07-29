#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "request.hpp"
#include "response.hpp"
#include "router.hpp"
#include "middleware.hpp"

namespace httpapi {

class HttpServer {
public:
    using RequestHandler = std::function<void(Request&, Response&)>;
    using MiddlewareFunction = std::function<void(Request&, Response&, std::function<void()>)>;

    HttpServer();
    ~HttpServer();

    // Server configuration
    HttpServer& listen(int port, const std::string& host = "0.0.0.0");
    HttpServer& set(const std::string& setting, const std::string& value);
    
    // Routing methods (Express.js style)
    HttpServer& get(const std::string& path, RequestHandler handler);
    HttpServer& post(const std::string& path, RequestHandler handler);
    HttpServer& put(const std::string& path, RequestHandler handler);
    HttpServer& delete_(const std::string& path, RequestHandler handler);
    HttpServer& patch(const std::string& path, RequestHandler handler);
    
    // Middleware support
    HttpServer& use(MiddlewareFunction middleware);
    HttpServer& use(const std::string& path, MiddlewareFunction middleware);
    
    // Static file serving
    HttpServer& static_(const std::string& path, const std::string& directory);
    
    // Server control
    void start();
    void stop();
    bool isRunning() const;

private:
    void handleClient(SOCKET clientSocket);
    void processRequest(Request& req, Response& res);
    std::string readRequest(SOCKET socket);
    void sendResponse(SOCKET socket, const std::string& response);
    
    // Server state
    SOCKET serverSocket_;
    std::atomic<bool> running_;
    std::thread serverThread_;
    
    // Configuration
    int port_;
    std::string host_;
    std::unordered_map<std::string, std::string> settings_;
    
    // Router and middleware
    std::unique_ptr<Router> router_;
    std::vector<MiddlewareFunction> globalMiddleware_;
    
    // Static file configuration
    std::unordered_map<std::string, std::string> staticPaths_;
    
    // Initialize Winsock
    bool initializeWinsock();
    void cleanupWinsock();
};

} // namespace httpapi 