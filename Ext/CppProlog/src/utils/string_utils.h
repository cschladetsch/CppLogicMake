#pragma once

#include <string>
#include <vector>

namespace prolog::utils {

class StringUtils {
public:
    static std::string trim(const std::string& str);
    static std::string trimLeft(const std::string& str);
    static std::string trimRight(const std::string& str);
    
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    
    static std::string join(const std::vector<std::string>& strings, const std::string& separator);
    
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    static std::string toLowerCase(const std::string& str);
    static std::string toUpperCase(const std::string& str);
    
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);
    
    static bool isWhitespace(const std::string& str);
    static bool isAlphanumeric(const std::string& str);
    
    static std::string escape(const std::string& str);
    static std::string unescape(const std::string& str);
};

}