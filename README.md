# HttpApi - Express.js-like C++ HTTP Framework

A modern, high-performance HTTP server framework for C++ that provides Express.js-like functionality with a familiar API.

## Features

- **Express.js-like API**: Familiar routing and middleware patterns
- **HTTP Server**: Full HTTP/1.1 server implementation
- **Routing**: Support for all HTTP methods (GET, POST, PUT, DELETE, PATCH)
- **Middleware**: Chainable middleware functions
- **Static File Serving**: Serve static files from directories
- **JSON Support**: Built-in JSON parsing and generation
- **URL Parameters**: Dynamic route parameters (`/users/:id`)
- **Query Parameters**: Automatic parsing of query strings
- **CORS Support**: Built-in CORS middleware
- **Error Handling**: Comprehensive error handling
- **Multi-threaded**: Concurrent request handling
- **Cross-platform**: Windows support (with Winsock)

## Quick Start

### Prerequisites

- C++17 compatible compiler (MSVC, GCC, Clang)
- CMake 3.16 or higher
- Windows (for Winsock support)

### Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Running the Example

```bash
./httpapi_app
```

The server will start on `http://127.0.0.1:3000`

## API Reference

### Basic Server Setup

```cpp
#include "httpapi/http_server.hpp"

using namespace httpapi;

int main() {
    HttpServer app;

    // Define routes
    app.get("/", [](Request& req, Response& res) {
        res.send("Hello, World!");
    });

    // Start server
    app.listen(3000, "127.0.0.1");
    app.start();

    return 0;
}
```

### Routing

#### HTTP Methods

```cpp
// GET requests
app.get("/users", [](Request& req, Response& res) {
    res.json("{\"users\": []}");
});

// POST requests
app.post("/users", [](Request& req, Response& res) {
    // Handle user creation
    res.status(201).json("{\"id\": 1}");
});

// PUT requests
app.put("/users/:id", [](Request& req, Response& res) {
    std::string id = req.param("id");
    res.json("{\"updated\": true}");
});

// DELETE requests
app.delete_("/users/:id", [](Request& req, Response& res) {
    res.status(204).send("");
});

// PATCH requests
app.patch("/users/:id", [](Request& req, Response& res) {
    res.json("{\"patched\": true}");
});
```

#### Route Parameters

```cpp
app.get("/users/:id/posts/:postId", [](Request& req, Response& res) {
    std::string userId = req.param("id");
    std::string postId = req.param("postId");
    res.json("{\"userId\": \"" + userId + "\", \"postId\": \"" + postId + "\"}");
});
```

#### Query Parameters

```cpp
app.get("/search", [](Request& req, Response& res) {
    std::string query = req.query("q");
    std::string page = req.query("page");
    res.json("{\"query\": \"" + query + "\", \"page\": " + page + "}");
});
```

### Middleware

#### Global Middleware

```cpp
// Logger middleware
app.use([](Request& req, Response& res, std::function<void()> next) {
    std::cout << req.method << " " << req.path << std::endl;
    next();
});

// CORS middleware
app.use([](Request& req, Response& res, std::function<void()> next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    next();
});
```

#### Path-specific Middleware

```cpp
app.use("/api", [](Request& req, Response& res, std::function<void()> next) {
    // Authentication middleware for /api routes
    std::string auth = req.get("authorization");
    if (auth.empty()) {
        res.status(401).json("{\"error\": \"Unauthorized\"}");
        return;
    }
    next();
});
```

### Request Object

```cpp
// HTTP method
std::string method = req.method;  // "GET", "POST", etc.

// URL and path
std::string url = req.url;        // "/users/123?page=1"
std::string path = req.path;      // "/users/123"
std::string query = req.query;    // "page=1"

// Headers
std::string contentType = req.get("content-type");
std::string userAgent = req.get("user-agent");

// Body
std::string body = req.body;

// Parameters
std::string userId = req.param("id");
std::string page = req.query("page");

// Content type checking
if (req.is("application/json")) {
    // Handle JSON request
}
```

### Response Object

#### Sending Responses

```cpp
// Send plain text
res.send("Hello, World!");

// Send JSON
res.json("{\"message\": \"Success\"}");

// Send file
res.sendFile("./public/index.html");

// Redirect
res.redirect("/new-page");
```

#### Status Codes

```cpp
// Status helpers
res.ok();                    // 200
res.created();               // 201
res.noContent();             // 204
res.badRequest();            // 400
res.unauthorized();          // 401
res.forbidden();             // 403
res.notFound();              // 404
res.internalServerError();   // 500

// Custom status
res.status(418).send("I'm a teapot");
```

#### Headers

```cpp
res.header("Content-Type", "application/json");
res.set("X-Custom-Header", "value");
```

### Static File Serving

```cpp
// Serve static files from a directory
app.static_("/static", "./public");
app.static_("/images", "./assets/images");
```

### JSON Handling

```cpp
#include "httpapi/json_handler.hpp"

// Parse JSON
auto data = JsonHandler::parse(req.body);

// Generate JSON
std::unordered_map<std::string, std::any> user = {
    {"id", 1},
    {"name", "John Doe"},
    {"email", "john@example.com"}
};
std::string json = JsonHandler::stringify(user);
res.json(json);
```

## Example Applications

### REST API

```cpp
HttpServer app;

// Get all users
app.get("/api/users", [](Request& req, Response& res) {
    std::vector<std::unordered_map<std::string, std::any>> users = {
        {{"id", 1}, {"name", "John"}, {"email", "john@example.com"}},
        {{"id", 2}, {"name", "Jane"}, {"email", "jane@example.com"}}
    };
    res.json(JsonHandler::stringify(users));
});

// Get user by ID
app.get("/api/users/:id", [](Request& req, Response& res) {
    std::string id = req.param("id");
    // Database lookup logic here
    res.json("{\"id\": " + id + ", \"name\": \"John\"}");
});

// Create user
app.post("/api/users", [](Request& req, Response& res) {
    auto userData = JsonHandler::parse(req.body);
    // Database insert logic here
    res.status(201).json("{\"id\": 3, \"created\": true}");
});

// Update user
app.put("/api/users/:id", [](Request& req, Response& res) {
    std::string id = req.param("id");
    auto userData = JsonHandler::parse(req.body);
    // Database update logic here
    res.json("{\"id\": " + id + ", \"updated\": true}");
});

// Delete user
app.delete_("/api/users/:id", [](Request& req, Response& res) {
    std::string id = req.param("id");
    // Database delete logic here
    res.status(204).send("");
});
```

### File Upload Handler

```cpp
app.post("/upload", [](Request& req, Response& res) {
    std::string contentType = req.get("content-type");

    if (contentType.find("multipart/form-data") != std::string::npos) {
        // Handle file upload
        res.json("{\"message\": \"File uploaded successfully\"}");
    } else {
        res.status(400).json("{\"error\": \"Invalid content type\"}");
    }
});
```

## Project Structure

```
httpapi/
├── CMakeLists.txt          # Main build configuration
├── README.md              # This file
├── include/
│   └── httpapi/
│       ├── http_server.hpp # Main server class
│       ├── request.hpp     # Request object
│       ├── response.hpp    # Response object
│       ├── router.hpp      # Routing system
│       ├── middleware.hpp  # Middleware system
│       ├── json_handler.hpp # JSON utilities
│       ├── static_files.hpp # Static file serving
│       └── utils.hpp       # Utility functions
├── src/
│   ├── CMakeLists.txt
│   ├── http_server.cpp     # Server implementation
│   ├── request.cpp         # Request implementation
│   ├── response.cpp        # Response implementation
│   ├── router.cpp          # Router implementation
│   ├── middleware.cpp      # Middleware implementation
│   ├── json_handler.cpp    # JSON implementation
│   ├── static_files.cpp    # Static files implementation
│   └── utils.cpp           # Utilities implementation
└── examples/
    ├── CMakeLists.txt
    └── main.cpp           # Example application
```

## Performance

- **Multi-threaded**: Each request is handled in a separate thread
- **Non-blocking**: Asynchronous request processing
- **Memory efficient**: Minimal memory overhead per request
- **Fast routing**: Optimized route matching with regex

## Security Features

- **Path traversal protection**: Prevents directory traversal attacks
- **Safe static file serving**: Validates file paths
- **Input validation**: Built-in parameter validation
- **CORS support**: Configurable cross-origin resource sharing

## Limitations

- Currently Windows-only (uses Winsock)
- Basic HTTP/1.1 implementation
- No HTTPS support (can be added with OpenSSL)
- Limited to single-threaded event loop (can be enhanced)

## Future Enhancements

- Linux/macOS support
- HTTPS/SSL support
- WebSocket support
- Template engine integration
- Database connectors
- Session management
- Rate limiting
- Compression middleware
- Logging framework integration

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by Express.js
- Built with modern C++17 features
- Uses Winsock for Windows networking
