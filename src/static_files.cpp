#include "httpapi/static_files.hpp"
#include "httpapi/utils.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace httpapi {

StaticFileHandler::StaticFileHandler() {
}

StaticFileHandler::~StaticFileHandler() {
}

void StaticFileHandler::setStaticPath(const std::string& urlPath, const std::string& filePath) {
    staticPaths_[urlPath] = filePath;
}

bool StaticFileHandler::serveFile(const std::string& requestPath, std::string& content, 
                                 std::string& contentType, int& statusCode) {
    std::string resolvedPath = resolvePath(requestPath);
    
    if (resolvedPath.empty() || !isPathSafe(resolvedPath)) {
        statusCode = 404;
        return false;
    }
    
    if (!fileExists(resolvedPath)) {
        statusCode = 404;
        return false;
    }
    
    if (!readFile(resolvedPath, content)) {
        statusCode = 500;
        return false;
    }
    
    std::string extension = getFileExtension(resolvedPath);
    contentType = getMimeType(extension);
    statusCode = 200;
    
    return true;
}

std::string StaticFileHandler::getFileExtension(const std::string& filename) {
    return Utils::getFileExtension(filename);
}

std::string StaticFileHandler::getMimeType(const std::string& extension) {
    static std::unordered_map<std::string, std::string> mimeTypes = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"txt", "text/plain"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif", "image/gif"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"pdf", "application/pdf"},
        {"zip", "application/zip"},
        {"mp3", "audio/mpeg"},
        {"mp4", "video/mp4"},
        {"woff", "font/woff"},
        {"woff2", "font/woff2"},
        {"ttf", "font/ttf"},
        {"eot", "application/vnd.ms-fontobject"},
        {"otf", "font/otf"}
    };
    
    auto it = mimeTypes.find(Utils::toLowerCase(extension));
    return (it != mimeTypes.end()) ? it->second : "application/octet-stream";
}

bool StaticFileHandler::fileExists(const std::string& path) {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    } catch (...) {
        return false;
    }
}

bool StaticFileHandler::readFile(const std::string& path, std::string& content) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        return true;
    } catch (...) {
        return false;
    }
}

std::string StaticFileHandler::resolvePath(const std::string& requestPath) {
    // Find the matching static path
    for (const auto& staticPath : staticPaths_) {
        if (requestPath.find(staticPath.first) == 0) {
            std::string relativePath = requestPath.substr(staticPath.first.length());
            if (relativePath.empty() || relativePath[0] != '/') {
                relativePath = "/" + relativePath;
            }
            
            std::string fullPath = staticPath.second + relativePath;
            
            // Normalize path
            try {
                std::filesystem::path normalizedPath = std::filesystem::canonical(fullPath);
                return normalizedPath.string();
            } catch (...) {
                return "";
            }
        }
    }
    
    return "";
}

bool StaticFileHandler::isPathSafe(const std::string& path) {
    // Check for path traversal attacks
    if (Utils::containsPathTraversal(path)) {
        return false;
    }
    
    // Check if path is within the allowed directory
    try {
        std::filesystem::path filePath(path);
        std::filesystem::path canonicalPath = std::filesystem::canonical(filePath);
        
        // For each static path, check if the canonical path is within the static directory
        for (const auto& staticPath : staticPaths_) {
            std::filesystem::path staticDir(staticPath.second);
            std::filesystem::path canonicalStaticDir = std::filesystem::canonical(staticDir);
            
            // Check if the file path starts with the static directory path
            std::string filePathStr = canonicalPath.string();
            std::string staticDirStr = canonicalStaticDir.string();
            
            if (filePathStr.find(staticDirStr) == 0) {
                return true;
            }
        }
        
        return false;
    } catch (...) {
        return false;
    }
}

} // namespace httpapi 