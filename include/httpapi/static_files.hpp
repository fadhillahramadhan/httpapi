#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

namespace httpapi {

class StaticFileHandler {
public:
    StaticFileHandler();
    ~StaticFileHandler();
    
    // Configure static file serving
    void setStaticPath(const std::string& urlPath, const std::string& filePath);
    
    // Serve static file
    bool serveFile(const std::string& requestPath, std::string& content, 
                  std::string& contentType, int& statusCode);
    
    // Get file extension
    static std::string getFileExtension(const std::string& filename);
    
    // Get MIME type
    static std::string getMimeType(const std::string& extension);
    
    // Check if file exists
    static bool fileExists(const std::string& path);
    
    // Read file content
    static bool readFile(const std::string& path, std::string& content);
    
private:
    std::unordered_map<std::string, std::string> staticPaths_;
    
    // Helper methods
    std::string resolvePath(const std::string& requestPath);
    bool isPathSafe(const std::string& path);
};

} // namespace httpapi 