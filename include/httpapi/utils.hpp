#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace httpapi {

class Utils {
public:
    // String utilities
    static std::string toLowerCase(const std::string& str);
    static std::string toUpperCase(const std::string& str);
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& parts, const std::string& separator);
    
    // URL utilities
    static std::string urlDecode(const std::string& str);
    static std::string urlEncode(const std::string& str);
    static std::pair<std::string, std::string> parseUrl(const std::string& url);
    
    // HTTP utilities
    static std::string getHttpStatusText(int statusCode);
    static std::string getHttpMethod(const std::string& method);
    static bool isValidHttpMethod(const std::string& method);
    
    // Header utilities
    static std::string normalizeHeaderName(const std::string& name);
    static std::unordered_map<std::string, std::string> parseHeaders(const std::string& headerText);
    
    // Time utilities
    static std::string getCurrentTime();
    static std::string formatTime(const std::string& format);
    
    // File utilities
    static std::string getFileSize(const std::string& path);
    static std::string getFileExtension(const std::string& filename);
    static bool isDirectory(const std::string& path);
    
    // Validation utilities
    static bool isValidPath(const std::string& path);
    static bool isSafePath(const std::string& path);
    static bool containsPathTraversal(const std::string& path);
    
private:
    static std::unordered_map<int, std::string> statusTexts_;
    static std::unordered_map<std::string, std::string> mimeTypes_;
    
    static void initializeStatusTexts();
    static void initializeMimeTypes();
};

} // namespace httpapi 