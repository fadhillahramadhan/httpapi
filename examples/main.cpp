#include "httpapi/http_server.hpp"
#include "httpapi/json_handler.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace httpapi;

int main() {
    std::cout << "Starting HttpApi Server (Express.js-like C++ Framework)" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    HttpServer app;
    
    // Middleware - Logger
    app.use([](Request& req, Response& res, std::function<void()> next) {
        std::cout << "[" << req.method << " " << req.path << "]" << std::endl;
        next();
    });
    
    // Middleware - CORS
    app.use([](Request& req, Response& res, std::function<void()> next) {
        res.header("Access-Control-Allow-Origin", "*");
        res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        if (req.method == "OPTIONS") {
            res.status(200).send("");
            return;
        }
        next();
    });
    
    // Basic routes
    app.get("/", [](Request& req, Response& res) {
        res.json("{\"message\": \"Welcome to HttpApi - Express.js-like C++ Framework!\", \"version\": \"1.0.0\"}");
    });
    
    app.get("/hello", [](Request& req, Response& res) {
        res.send("Hello, World!");
    });
    
    app.get("/hello/:name", [](Request& req, Response& res) {
        std::string name = req.param("name");
        res.json("{\"message\": \"Hello, " + name + "!\", \"method\": \"GET\"}");
    });
    
    // JSON API routes
    app.get("/api/users", [](Request& req, Response& res) {
        std::vector<std::unordered_map<std::string, std::any>> users = {
            {{"id", 1}, {"name", "John Doe"}, {"email", "john@example.com"}},
            {{"id", 2}, {"name", "Jane Smith"}, {"email", "jane@example.com"}},
            {{"id", 3}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}}
        };
        
        res.json(JsonHandler::stringify(users));
    });
    
    app.get("/api/users/:id", [](Request& req, Response& res) {
        std::string id = req.param("id");
        
        // Simulate database lookup
        if (id == "1") {
            std::unordered_map<std::string, std::any> user = {
                {"id", 1},
                {"name", "John Doe"},
                {"email", "john@example.com"},
                {"created_at", "2024-01-01"}
            };
            res.json(JsonHandler::stringify(user));
        } else {
            res.status(404).json("{\"error\": \"User not found\"}");
        }
    });
    
    app.post("/api/users", [](Request& req, Response& res) {
        // Parse JSON body
        auto userData = JsonHandler::parse(req.body);
        
        if (userData.find("name") == userData.end() || userData.find("email") == userData.end()) {
            res.status(400).json("{\"error\": \"Name and email are required\"}");
            return;
        }
        
        // Simulate creating user
        std::unordered_map<std::string, std::any> newUser = {
            {"id", 4},
            {"name", std::any_cast<std::string>(userData["name"])},
            {"email", std::any_cast<std::string>(userData["email"])},
            {"created_at", "2024-01-01"}
        };
        
        res.status(201).json(JsonHandler::stringify(newUser));
    });
    
    app.put("/api/users/:id", [](Request& req, Response& res) {
        std::string id = req.param("id");
        auto userData = JsonHandler::parse(req.body);
        
        if (id == "1") {
            std::unordered_map<std::string, std::any> updatedUser = {
                {"id", 1},
                {"name", std::any_cast<std::string>(userData["name"])},
                {"email", std::any_cast<std::string>(userData["email"])},
                {"updated_at", "2024-01-01"}
            };
            res.json(JsonHandler::stringify(updatedUser));
        } else {
            res.status(404).json("{\"error\": \"User not found\"}");
        }
    });
    
    app.delete_("/api/users/:id", [](Request& req, Response& res) {
        std::string id = req.param("id");
        
        if (id == "1") {
            res.status(204).send("");
        } else {
            res.status(404).json("{\"error\": \"User not found\"}");
        }
    });
    
    // Query parameters example
    app.get("/search", [](Request& req, Response& res) {
        std::string query = req.query("q");
        std::string page = req.query("page");
        
        if (query.empty()) {
            res.status(400).json("{\"error\": \"Query parameter 'q' is required\"}");
            return;
        }
        
        std::unordered_map<std::string, std::any> result = {
            {"query", query},
            {"page", page.empty() ? 1 : std::stoi(page)},
            {"results", std::vector<std::string>{"result1", "result2", "result3"}}
        };
        
        res.json(JsonHandler::stringify(result));
    });
    
    // File upload simulation
    app.post("/upload", [](Request& req, Response& res) {
        std::string contentType = req.get("content-type");
        
        if (contentType.find("multipart/form-data") != std::string::npos) {
            res.json("{\"message\": \"File upload simulation\", \"files\": 1}");
        } else {
            res.status(400).json("{\"error\": \"Invalid content type\"}");
        }
    });
    
    // Error handling example
    app.get("/error", [](Request& req, Response& res) {
        res.status(500).json("{\"error\": \"Internal server error simulation\"}");
    });
    
    // Redirect example
    app.get("/old-page", [](Request& req, Response& res) {
        res.redirect("/new-page");
    });
    
    app.get("/new-page", [](Request& req, Response& res) {
        res.send("This is the new page!");
    });
    
    // Static file serving
    app.static_("/static", "./static");
    
    // Start server
    app.listen(3000, "127.0.0.1");
    app.start();
    
    std::cout << "Server is running on http://127.0.0.1:3000" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /                    - Welcome message" << std::endl;
    std::cout << "  GET  /hello               - Hello World" << std::endl;
    std::cout << "  GET  /hello/:name         - Hello with parameter" << std::endl;
    std::cout << "  GET  /api/users           - Get all users" << std::endl;
    std::cout << "  GET  /api/users/:id       - Get user by ID" << std::endl;
    std::cout << "  POST /api/users           - Create new user" << std::endl;
    std::cout << "  PUT  /api/users/:id       - Update user" << std::endl;
    std::cout << "  DELETE /api/users/:id     - Delete user" << std::endl;
    std::cout << "  GET  /search?q=term&page=1 - Search with query params" << std::endl;
    std::cout << "  POST /upload              - File upload simulation" << std::endl;
    std::cout << "  GET  /error               - Error simulation" << std::endl;
    std::cout << "  GET  /old-page            - Redirect example" << std::endl;
    std::cout << "  GET  /static/*            - Static file serving" << std::endl;
    std::cout << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Keep the server running
    while (app.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
} 