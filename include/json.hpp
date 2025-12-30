// =============================================================================
// Simple JSON Parser for Switch App Store
// =============================================================================
// A lightweight JSON parser optimized for the specific API responses
// of this application. For a full-featured parser, consider nlohmann/json.
// =============================================================================

#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cctype>

namespace json {

// =============================================================================
// JSON Value Types
// =============================================================================
enum class ValueType {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

// Forward declaration
class Value;

// =============================================================================
// JSON Value Class
// =============================================================================
class Value {
public:
    ValueType type = ValueType::Null;
    
    // Value storage
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<Value> arrayValue;
    std::map<std::string, Value> objectValue;
    
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    Value() = default;
    Value(bool b) : type(ValueType::Bool), boolValue(b) {}
    Value(double n) : type(ValueType::Number), numberValue(n) {}
    Value(int n) : type(ValueType::Number), numberValue(static_cast<double>(n)) {}
    Value(const std::string& s) : type(ValueType::String), stringValue(s) {}
    Value(const char* s) : type(ValueType::String), stringValue(s) {}
    
    // -------------------------------------------------------------------------
    // Type checking
    // -------------------------------------------------------------------------
    bool isNull() const { return type == ValueType::Null; }
    bool isBool() const { return type == ValueType::Bool; }
    bool isNumber() const { return type == ValueType::Number; }
    bool isString() const { return type == ValueType::String; }
    bool isArray() const { return type == ValueType::Array; }
    bool isObject() const { return type == ValueType::Object; }
    
    // -------------------------------------------------------------------------
    // Value access
    // -------------------------------------------------------------------------
    bool asBool(bool defaultVal = false) const {
        return type == ValueType::Bool ? boolValue : defaultVal;
    }
    
    double asNumber(double defaultVal = 0.0) const {
        return type == ValueType::Number ? numberValue : defaultVal;
    }
    
    int asInt(int defaultVal = 0) const {
        return type == ValueType::Number ? static_cast<int>(numberValue) : defaultVal;
    }
    
    const std::string& asString(const std::string& defaultVal = "") const {
        static std::string empty;
        return type == ValueType::String ? stringValue : (defaultVal.empty() ? empty : defaultVal);
    }
    
    // -------------------------------------------------------------------------
    // Array access
    // -------------------------------------------------------------------------
    size_t size() const {
        if (type == ValueType::Array) return arrayValue.size();
        if (type == ValueType::Object) return objectValue.size();
        return 0;
    }
    
    const Value& operator[](size_t index) const {
        static Value nullValue;
        if (type != ValueType::Array || index >= arrayValue.size()) {
            return nullValue;
        }
        return arrayValue[index];
    }
    
    // -------------------------------------------------------------------------
    // Object access
    // -------------------------------------------------------------------------
    const Value& operator[](const std::string& key) const {
        static Value nullValue;
        if (type != ValueType::Object) return nullValue;
        auto it = objectValue.find(key);
        return it != objectValue.end() ? it->second : nullValue;
    }
    
    bool contains(const std::string& key) const {
        return type == ValueType::Object && objectValue.find(key) != objectValue.end();
    }
};

// =============================================================================
// JSON Parser
// =============================================================================
class Parser {
public:
    // -------------------------------------------------------------------------
    // Parse a JSON string into a Value
    // -------------------------------------------------------------------------
    static Value parse(const std::string& json) {
        size_t pos = 0;
        return parseValue(json, pos);
    }
    
private:
    // -------------------------------------------------------------------------
    // Skip whitespace
    // -------------------------------------------------------------------------
    static void skipWhitespace(const std::string& json, size_t& pos) {
        while (pos < json.size() && std::isspace(json[pos])) {
            pos++;
        }
    }
    
    // -------------------------------------------------------------------------
    // Parse any JSON value
    // -------------------------------------------------------------------------
    static Value parseValue(const std::string& json, size_t& pos) {
        skipWhitespace(json, pos);
        
        if (pos >= json.size()) return Value();
        
        char c = json[pos];
        
        if (c == '{') return parseObject(json, pos);
        if (c == '[') return parseArray(json, pos);
        if (c == '"') return parseString(json, pos);
        if (c == 't' || c == 'f') return parseBool(json, pos);
        if (c == 'n') return parseNull(json, pos);
        if (c == '-' || std::isdigit(c)) return parseNumber(json, pos);
        
        return Value();
    }
    
    // -------------------------------------------------------------------------
    // Parse object
    // -------------------------------------------------------------------------
    static Value parseObject(const std::string& json, size_t& pos) {
        Value obj;
        obj.type = ValueType::Object;
        
        pos++; // Skip '{'
        skipWhitespace(json, pos);
        
        while (pos < json.size() && json[pos] != '}') {
            // Parse key
            skipWhitespace(json, pos);
            if (json[pos] != '"') break;
            
            Value keyVal = parseString(json, pos);
            std::string key = keyVal.stringValue;
            
            // Skip ':'
            skipWhitespace(json, pos);
            if (pos >= json.size() || json[pos] != ':') break;
            pos++;
            
            // Parse value
            Value value = parseValue(json, pos);
            obj.objectValue[key] = value;
            
            // Skip ',' if present
            skipWhitespace(json, pos);
            if (pos < json.size() && json[pos] == ',') {
                pos++;
            }
        }
        
        if (pos < json.size() && json[pos] == '}') pos++;
        return obj;
    }
    
    // -------------------------------------------------------------------------
    // Parse array
    // -------------------------------------------------------------------------
    static Value parseArray(const std::string& json, size_t& pos) {
        Value arr;
        arr.type = ValueType::Array;
        
        pos++; // Skip '['
        skipWhitespace(json, pos);
        
        while (pos < json.size() && json[pos] != ']') {
            Value value = parseValue(json, pos);
            arr.arrayValue.push_back(value);
            
            skipWhitespace(json, pos);
            if (pos < json.size() && json[pos] == ',') {
                pos++;
            }
        }
        
        if (pos < json.size() && json[pos] == ']') pos++;
        return arr;
    }
    
    // -------------------------------------------------------------------------
    // Parse string
    // -------------------------------------------------------------------------
    static Value parseString(const std::string& json, size_t& pos) {
        pos++; // Skip opening '"'
        
        std::string result;
        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                pos++;
                char escaped = json[pos];
                switch (escaped) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        // Unicode escape - simplified handling
                        if (pos + 4 < json.size()) {
                            pos += 4; // Skip 4 hex digits
                        }
                        result += '?'; // Placeholder for unicode
                        break;
                    }
                    default: result += escaped; break;
                }
            } else {
                result += json[pos];
            }
            pos++;
        }
        
        if (pos < json.size() && json[pos] == '"') pos++;
        
        Value val;
        val.type = ValueType::String;
        val.stringValue = result;
        return val;
    }
    
    // -------------------------------------------------------------------------
    // Parse number
    // -------------------------------------------------------------------------
    static Value parseNumber(const std::string& json, size_t& pos) {
        size_t start = pos;
        
        if (json[pos] == '-') pos++;
        
        while (pos < json.size() && std::isdigit(json[pos])) pos++;
        
        if (pos < json.size() && json[pos] == '.') {
            pos++;
            while (pos < json.size() && std::isdigit(json[pos])) pos++;
        }
        
        if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
            pos++;
            if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) pos++;
            while (pos < json.size() && std::isdigit(json[pos])) pos++;
        }
        
        std::string numStr = json.substr(start, pos - start);
        
        Value val;
        val.type = ValueType::Number;
        val.numberValue = std::atof(numStr.c_str());
        return val;
    }
    
    // -------------------------------------------------------------------------
    // Parse boolean
    // -------------------------------------------------------------------------
    static Value parseBool(const std::string& json, size_t& pos) {
        Value val;
        val.type = ValueType::Bool;
        
        if (json.substr(pos, 4) == "true") {
            val.boolValue = true;
            pos += 4;
        } else if (json.substr(pos, 5) == "false") {
            val.boolValue = false;
            pos += 5;
        }
        
        return val;
    }
    
    // -------------------------------------------------------------------------
    // Parse null
    // -------------------------------------------------------------------------
    static Value parseNull(const std::string& json, size_t& pos) {
        if (json.substr(pos, 4) == "null") {
            pos += 4;
        }
        return Value();
    }
};

// =============================================================================
// Convenience function
// =============================================================================
inline Value parse(const std::string& json) {
    return Parser::parse(json);
}

} // namespace json
