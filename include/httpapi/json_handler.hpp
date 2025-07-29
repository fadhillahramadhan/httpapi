#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>

namespace httpapi {

class JsonHandler {
public:
    // Parse JSON string to map
    static std::unordered_map<std::string, std::any> parse(const std::string& json);
    
    // Convert map to JSON string
    static std::string stringify(const std::unordered_map<std::string, std::any>& data);
    
    // Helper methods for specific types
    static std::string stringify(const std::vector<std::string>& array);
    static std::string stringify(const std::vector<std::unordered_map<std::string, std::any>>& array);
    
    // Validation
    static bool isValid(const std::string& json);
    
private:
    // Helper methods for JSON parsing
    static std::string escapeString(const std::string& str);
    static std::string unescapeString(const std::string& str);
    static std::any parseValue(const std::string& json, size_t& pos);
    static std::unordered_map<std::string, std::any> parseObject(const std::string& json, size_t& pos);
    static std::vector<std::any> parseArray(const std::string& json, size_t& pos);
    static std::string parseString(const std::string& json, size_t& pos);
    static double parseNumber(const std::string& json, size_t& pos);
    static bool parseBoolean(const std::string& json, size_t& pos);
    static void parseNull(const std::string& json, size_t& pos);
    static void skipWhitespace(const std::string& json, size_t& pos);
};

} // namespace httpapi 