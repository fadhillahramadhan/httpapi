#include "httpapi/utils.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <ctime>

namespace httpapi {

std::unordered_map<int, std::string> Utils::statusTexts_;
std::unordered_map<std::string, std::string> Utils::mimeTypes_;

std::string Utils::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Utils::toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string Utils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

std::string Utils::join(const std::vector<std::string>& parts, const std::string& separator) {
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            result += separator;
        }
        result += parts[i];
    }
    return result;
}

std::string Utils::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream iss(str.substr(i + 1, 2));
            iss >> std::hex >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string Utils::urlEncode(const std::string& str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    
    return escaped.str();
}

std::pair<std::string, std::string> Utils::parseUrl(const std::string& url) {
    size_t queryPos = url.find('?');
    if (queryPos == std::string::npos) {
        return {url, ""};
    }
    return {url.substr(0, queryPos), url.substr(queryPos + 1)};
}

std::string Utils::getHttpStatusText(int statusCode) {
    if (statusTexts_.empty()) {
        initializeStatusTexts();
    }
    
    auto it = statusTexts_.find(statusCode);
    return (it != statusTexts_.end()) ? it->second : "Unknown";
}

std::string Utils::getHttpMethod(const std::string& method) {
    return toUpperCase(method);
}

bool Utils::isValidHttpMethod(const std::string& method) {
    std::string upperMethod = toUpperCase(method);
    return upperMethod == "GET" || upperMethod == "POST" || upperMethod == "PUT" || 
           upperMethod == "DELETE" || upperMethod == "PATCH" || upperMethod == "HEAD" || 
           upperMethod == "OPTIONS";
}

std::string Utils::normalizeHeaderName(const std::string& name) {
    std::string result = toLowerCase(name);
    bool capitalize = true;
    
    for (char& c : result) {
        if (capitalize && isalpha(c)) {
            c = toupper(c);
            capitalize = false;
        } else if (c == '-') {
            capitalize = true;
        }
    }
    
    return result;
}

std::unordered_map<std::string, std::string> Utils::parseHeaders(const std::string& headerText) {
    std::unordered_map<std::string, std::string> headers;
    std::vector<std::string> lines = split(headerText, '\n');
    
    for (const auto& line : lines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        
        size_t colonPos = trimmed.find(':');
        if (colonPos != std::string::npos) {
            std::string name = normalizeHeaderName(trimmed.substr(0, colonPos));
            std::string value = trim(trimmed.substr(colonPos + 1));
            headers[name] = value;
        }
    }
    
    return headers;
}

std::string Utils::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

std::string Utils::formatTime(const std::string& format) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), format.c_str());
    return ss.str();
}

std::string Utils::getFileSize(const std::string& path) {
    try {
        std::filesystem::path filePath(path);
        if (std::filesystem::exists(filePath)) {
            return std::to_string(std::filesystem::file_size(filePath));
        }
    } catch (...) {
        // Ignore errors
    }
    return "0";
}

std::string Utils::getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        return toLowerCase(filename.substr(dotPos + 1));
    }
    return "";
}

bool Utils::isDirectory(const std::string& path) {
    try {
        return std::filesystem::is_directory(path);
    } catch (...) {
        return false;
    }
}

bool Utils::isValidPath(const std::string& path) {
    return !path.empty() && path[0] == '/';
}

bool Utils::isSafePath(const std::string& path) {
    return !containsPathTraversal(path);
}

bool Utils::containsPathTraversal(const std::string& path) {
    return path.find("..") != std::string::npos || 
           path.find("\\") != std::string::npos ||
           path.find("//") != std::string::npos;
}

void Utils::initializeStatusTexts() {
    statusTexts_ = {
        {200, "OK"},
        {201, "Created"},
        {204, "No Content"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"}
    };
}

void Utils::initializeMimeTypes() {
    mimeTypes_ = {
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
        {"mp4", "video/mp4"}
    };
}

} // namespace httpapi 