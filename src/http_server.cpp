#include "httpapi/http_server.hpp"
#include "httpapi/utils.hpp"
#include "httpapi/static_files.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace httpapi {

HttpServer::HttpServer() 
    : serverSocket_(INVALID_SOCKET), running_(false), port_(3000), host_("0.0.0.0") {
    router_ = std::make_unique<Router>();
    initializeWinsock();
}

HttpServer::~HttpServer() {
    stop();
    cleanupWinsock();
}

HttpServer& HttpServer::listen(int port, const std::string& host) {
    port_ = port;
    host_ = host;
    return *this;
}

HttpServer& HttpServer::set(const std::string& setting, const std::string& value) {
    settings_[setting] = value;
    return *this;
}

HttpServer& HttpServer::get(const std::string& path, RequestHandler handler) {
    router_->get(path, handler);
    return *this;
}

HttpServer& HttpServer::post(const std::string& path, RequestHandler handler) {
    router_->post(path, handler);
    return *this;
}

HttpServer& HttpServer::put(const std::string& path, RequestHandler handler) {
    router_->put(path, handler);
    return *this;
}

HttpServer& HttpServer::delete_(const std::string& path, RequestHandler handler) {
    router_->delete_(path, handler);
    return *this;
}

HttpServer& HttpServer::patch(const std::string& path, RequestHandler handler) {
    router_->patch(path, handler);
    return *this;
}

HttpServer& HttpServer::use(MiddlewareFunction middleware) {
    globalMiddleware_.push_back(middleware);
    return *this;
}

HttpServer& HttpServer::use(const std::string& path, MiddlewareFunction middleware) {
    globalMiddleware_.push_back(middleware);
    return *this;
}

HttpServer& HttpServer::static_(const std::string& path, const std::string& directory) {
    staticPaths_[path] = directory;
    return *this;
}

void HttpServer::start() {
    if (running_) {
        return;
    }

    // Create socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        closesocket(serverSocket_);
        return;
    }

    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host_.c_str());
    serverAddr.sin_port = htons(port_);

    if (bind(serverSocket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(serverSocket_);
        return;
    }

    // Listen for connections
    int listenResult = ::listen(serverSocket_, SOMAXCONN);
    if (listenResult < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(serverSocket_);
        return;
    }

    std::cout << "Server listening on " << host_ << ":" << port_ << std::endl;
    running_ = true;

    // Start server thread
    serverThread_ = std::thread([this]() {
        while (running_) {
            struct sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running_) {
                    std::cerr << "Failed to accept connection" << std::endl;
                }
                continue;
            }

            // Handle client in a new thread
            std::thread([this, clientSocket]() {
                handleClient(clientSocket);
            }).detach();
        }
    });
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    
    if (serverSocket_ != INVALID_SOCKET) {
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
    }

    if (serverThread_.joinable()) {
        serverThread_.join();
    }

    std::cout << "Server stopped" << std::endl;
}

bool HttpServer::isRunning() const {
    return running_;
}

void HttpServer::handleClient(SOCKET clientSocket) {
    std::string requestData = readRequest(clientSocket);
    if (requestData.empty()) {
        closesocket(clientSocket);
        return;
    }

    Request req;
    Response res;

    // Parse request
    std::istringstream requestStream(requestData);
    std::string requestLine;
    std::getline(requestStream, requestLine);

    // Parse request line
    std::vector<std::string> parts = Utils::split(Utils::trim(requestLine), ' ');
    if (parts.size() >= 3) {
        req.method = Utils::toUpperCase(parts[0]);
        req.url = parts[1];
        req.protocol = parts[2];

        // Parse URL
        auto urlParts = Utils::parseUrl(req.url);
        req.path = urlParts.first;
        req.queryString = urlParts.second;
    }

    // Parse headers
    std::string line;
    while (std::getline(requestStream, line) && line != "\r") {
        line = Utils::trim(line);
        if (line.empty()) break;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = Utils::normalizeHeaderName(line.substr(0, colonPos));
            std::string value = Utils::trim(line.substr(colonPos + 1));
            req.headers[name] = value;
        }
    }

    // Parse body
    std::string body;
    while (std::getline(requestStream, line)) {
        body += line + "\n";
    }
    req.body = body;

    // Parse query parameters
    req.parseQueryString();

    // Process request through middleware and router
    processRequest(req, res);

    // Send response
    std::string responseStr = res.toString();
    sendResponse(clientSocket, responseStr);

    closesocket(clientSocket);
}

void HttpServer::processRequest(Request& req, Response& res) {
    // Set default headers
    res.setDefaultHeaders();

    // Check static files first
    for (const auto& staticPath : staticPaths_) {
        if (req.path.find(staticPath.first) == 0) {
            StaticFileHandler staticHandler;
            staticHandler.setStaticPath(staticPath.first, staticPath.second);
            
            std::string content, contentType;
            int statusCode;
            if (staticHandler.serveFile(req.path, content, contentType, statusCode)) {
                res.status(statusCode);
                res.header("Content-Type", contentType);
                res.send(content);
                return;
            }
        }
    }

    // Execute global middleware
    size_t middlewareIndex = 0;
    std::function<void()> next = [&]() {
        if (middlewareIndex < globalMiddleware_.size()) {
            auto& middleware = globalMiddleware_[middlewareIndex++];
            middleware(req, res, next);
        } else {
            // All middleware executed, now handle the request
            if (!router_->handleRequest(req, res)) {
                res.status(404).send("Not Found");
            }
        }
    };

    if (!globalMiddleware_.empty()) {
        next();
    } else {
        // No middleware, handle request directly
        if (!router_->handleRequest(req, res)) {
            res.status(404).send("Not Found");
        }
    }
}

std::string HttpServer::readRequest(SOCKET socket) {
    std::string request;
    char buffer[4096];
    int bytesRead;

    while ((bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        request += buffer;

        // Check if we've received the complete request
        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    return request;
}

void HttpServer::sendResponse(SOCKET socket, const std::string& response) {
    send(socket, response.c_str(), response.length(), 0);
}

bool HttpServer::initializeWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void HttpServer::cleanupWinsock() {
    WSACleanup();
}

} // namespace httpapi 