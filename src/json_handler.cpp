#include "httpapi/json_handler.hpp"
#include "httpapi/utils.hpp"
#include <sstream>
#include <iomanip>

namespace httpapi {

std::unordered_map<std::string, std::any> JsonHandler::parse(const std::string& json) {
    std::unordered_map<std::string, std::any> result;
    size_t pos = 0;
    
    skipWhitespace(json, pos);
    if (pos >= json.length() || json[pos] != '{') {
        return result;
    }
    
    pos++; // Skip '{'
    skipWhitespace(json, pos);
    
    while (pos < json.length() && json[pos] != '}') {
        // Parse key
        skipWhitespace(json, pos);
        if (json[pos] != '"') {
            break;
        }
        
        std::string key = parseString(json, pos);
        skipWhitespace(json, pos);
        
        if (pos >= json.length() || json[pos] != ':') {
            break;
        }
        pos++; // Skip ':'
        
        // Parse value
        skipWhitespace(json, pos);
        result[key] = parseValue(json, pos);
        
        skipWhitespace(json, pos);
        if (pos < json.length() && json[pos] == ',') {
            pos++; // Skip ','
        }
    }
    
    return result;
}

std::string JsonHandler::stringify(const std::unordered_map<std::string, std::any>& data) {
    std::ostringstream result;
    result << "{";
    
    bool first = true;
    for (const auto& pair : data) {
        if (!first) {
            result << ",";
        }
        first = false;
        
        result << "\"" << escapeString(pair.first) << "\":";
        
        if (pair.second.type() == typeid(std::string)) {
            result << "\"" << escapeString(std::any_cast<std::string>(pair.second)) << "\"";
        } else if (pair.second.type() == typeid(int)) {
            result << std::any_cast<int>(pair.second);
        } else if (pair.second.type() == typeid(double)) {
            result << std::any_cast<double>(pair.second);
        } else if (pair.second.type() == typeid(bool)) {
            result << (std::any_cast<bool>(pair.second) ? "true" : "false");
        } else {
            result << "null";
        }
    }
    
    result << "}";
    return result.str();
}

std::string JsonHandler::stringify(const std::vector<std::string>& array) {
    std::ostringstream result;
    result << "[";
    
    for (size_t i = 0; i < array.size(); ++i) {
        if (i > 0) {
            result << ",";
        }
        result << "\"" << escapeString(array[i]) << "\"";
    }
    
    result << "]";
    return result.str();
}

std::string JsonHandler::stringify(const std::vector<std::unordered_map<std::string, std::any>>& array) {
    std::ostringstream result;
    result << "[";
    
    for (size_t i = 0; i < array.size(); ++i) {
        if (i > 0) {
            result << ",";
        }
        result << stringify(array[i]);
    }
    
    result << "]";
    return result.str();
}

bool JsonHandler::isValid(const std::string& json) {
    try {
        size_t pos = 0;
        skipWhitespace(json, pos);
        parseValue(json, pos);
        skipWhitespace(json, pos);
        return pos >= json.length();
    } catch (...) {
        return false;
    }
}

std::string JsonHandler::escapeString(const std::string& str) {
    std::ostringstream result;
    for (char c : str) {
        switch (c) {
            case '"': result << "\\\""; break;
            case '\\': result << "\\\\"; break;
            case '\b': result << "\\b"; break;
            case '\f': result << "\\f"; break;
            case '\n': result << "\\n"; break;
            case '\r': result << "\\r"; break;
            case '\t': result << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 32) {
                    result << "\\u" << std::hex << std::setw(4) << std::setfill('0') 
                           << static_cast<int>(static_cast<unsigned char>(c));
                } else {
                    result << c;
                }
                break;
        }
    }
    return result.str();
}

std::string JsonHandler::unescapeString(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': result += '"'; i++; break;
                case '\\': result += '\\'; i++; break;
                case 'b': result += '\b'; i++; break;
                case 'f': result += '\f'; i++; break;
                case 'n': result += '\n'; i++; break;
                case 'r': result += '\r'; i++; break;
                case 't': result += '\t'; i++; break;
                case 'u':
                    if (i + 5 < str.length()) {
                        std::string hex = str.substr(i + 2, 4);
                        try {
                            int unicode = std::stoi(hex, nullptr, 16);
                            result += static_cast<char>(unicode);
                            i += 5;
                        } catch (...) {
                            result += '\\';
                        }
                    } else {
                        result += '\\';
                    }
                    break;
                default:
                    result += '\\';
                    break;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

std::any JsonHandler::parseValue(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);
    
    if (pos >= json.length()) {
        throw std::runtime_error("Unexpected end of JSON");
    }
    
    char c = json[pos];
    
    if (c == '{') {
        std::unordered_map<std::string, std::any> obj = parseObject(json, pos);
        return obj;
    } else if (c == '[') {
        std::vector<std::any> arr = parseArray(json, pos);
        return arr;
    } else if (c == '"') {
        std::string str = parseString(json, pos);
        return str;
    } else if (c == 't' || c == 'f') {
        bool b = parseBoolean(json, pos);
        return b;
    } else if (c == 'n') {
        parseNull(json, pos);
        return std::any();
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        double num = parseNumber(json, pos);
        return num;
    } else {
        throw std::runtime_error("Unexpected character in JSON");
    }
}

std::unordered_map<std::string, std::any> JsonHandler::parseObject(const std::string& json, size_t& pos) {
    std::unordered_map<std::string, std::any> result;
    
    if (json[pos] != '{') {
        throw std::runtime_error("Expected '{'");
    }
    pos++; // Skip '{'
    
    skipWhitespace(json, pos);
    
    while (pos < json.length() && json[pos] != '}') {
        // Parse key
        skipWhitespace(json, pos);
        if (json[pos] != '"') {
            throw std::runtime_error("Expected '\"' for object key");
        }
        
        std::string key = parseString(json, pos);
        skipWhitespace(json, pos);
        
        if (json[pos] != ':') {
            throw std::runtime_error("Expected ':' after object key");
        }
        pos++; // Skip ':'
        
        // Parse value
        skipWhitespace(json, pos);
        result[key] = parseValue(json, pos);
        
        skipWhitespace(json, pos);
        if (pos < json.length() && json[pos] == ',') {
            pos++; // Skip ','
        }
    }
    
    if (pos >= json.length() || json[pos] != '}') {
        throw std::runtime_error("Expected '}'");
    }
    pos++; // Skip '}'
    
    return result;
}

std::vector<std::any> JsonHandler::parseArray(const std::string& json, size_t& pos) {
    std::vector<std::any> result;
    
    if (json[pos] != '[') {
        throw std::runtime_error("Expected '['");
    }
    pos++; // Skip '['
    
    skipWhitespace(json, pos);
    
    while (pos < json.length() && json[pos] != ']') {
        result.push_back(parseValue(json, pos));
        
        skipWhitespace(json, pos);
        if (pos < json.length() && json[pos] == ',') {
            pos++; // Skip ','
        }
    }
    
    if (pos >= json.length() || json[pos] != ']') {
        throw std::runtime_error("Expected ']'");
    }
    pos++; // Skip ']'
    
    return result;
}

std::string JsonHandler::parseString(const std::string& json, size_t& pos) {
    if (json[pos] != '"') {
        throw std::runtime_error("Expected '\"'");
    }
    pos++; // Skip '"'
    
    std::string result;
    while (pos < json.length() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.length()) {
            pos++; // Skip '\'
            switch (json[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += '\\'; result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        pos++;
    }
    
    if (pos >= json.length() || json[pos] != '"') {
        throw std::runtime_error("Expected '\"'");
    }
    pos++; // Skip '"'
    
    return result;
}

double JsonHandler::parseNumber(const std::string& json, size_t& pos) {
    size_t start = pos;
    
    if (json[pos] == '-') {
        pos++;
    }
    
    while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9') {
        pos++;
    }
    
    if (pos < json.length() && json[pos] == '.') {
        pos++;
        while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9') {
            pos++;
        }
    }
    
    if (pos < json.length() && (json[pos] == 'e' || json[pos] == 'E')) {
        pos++;
        if (pos < json.length() && (json[pos] == '+' || json[pos] == '-')) {
            pos++;
        }
        while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9') {
            pos++;
        }
    }
    
    std::string numStr = json.substr(start, pos - start);
    return std::stod(numStr);
}

bool JsonHandler::parseBoolean(const std::string& json, size_t& pos) {
    if (pos + 3 < json.length() && json.substr(pos, 4) == "true") {
        pos += 4;
        return true;
    } else if (pos + 4 < json.length() && json.substr(pos, 5) == "false") {
        pos += 5;
        return false;
    } else {
        throw std::runtime_error("Expected 'true' or 'false'");
    }
}

void JsonHandler::parseNull(const std::string& json, size_t& pos) {
    if (pos + 3 < json.length() && json.substr(pos, 4) == "null") {
        pos += 4;
    } else {
        throw std::runtime_error("Expected 'null'");
    }
}

void JsonHandler::skipWhitespace(const std::string& json, size_t& pos) {
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || 
                                  json[pos] == '\n' || json[pos] == '\r')) {
        pos++;
    }
}

} // namespace httpapi 