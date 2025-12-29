// =============================================================================
// Switch App Store - Localization System
// =============================================================================
// Multi-language support with string resource management
// =============================================================================

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// Supported languages
// =============================================================================
enum class Language {
    ZH_CN,  // 简体中文
    ZH_TW,  // 繁體中文
    EN_US,  // English
    JA_JP   // 日本語
};

// =============================================================================
// Localization - String resource manager
// =============================================================================
class Localization {
public:
    // -------------------------------------------------------------------------
    // Singleton access
    // -------------------------------------------------------------------------
    static Localization& getInstance();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    void init();
    
    // -------------------------------------------------------------------------
    // Language management
    // -------------------------------------------------------------------------
    
    void setLanguage(Language lang);
    void setLanguage(const std::string& langCode);
    Language getLanguage() const { return m_currentLang; }
    std::string getLanguageCode() const;
    std::string getLanguageName() const;
    
    // Get available languages
    static const std::vector<Language>& getAvailableLanguages();
    static std::string getLanguageName(Language lang);
    static std::string getLanguageCode(Language lang);
    
    // -------------------------------------------------------------------------
    // String access
    // -------------------------------------------------------------------------
    
    // Get localized string by key
    const std::string& get(const std::string& key) const;
    
    // Shorthand operator
    const std::string& operator[](const std::string& key) const { return get(key); }
    
    // Format string with arguments (simple %s replacement)
    std::string format(const std::string& key, const std::string& arg1) const;
    std::string format(const std::string& key, const std::string& arg1, 
                       const std::string& arg2) const;
    
private:
    Localization() = default;
    ~Localization() = default;
    
    Localization(const Localization&) = delete;
    Localization& operator=(const Localization&) = delete;
    
    void loadLanguage(Language lang);
    void loadChineseSimplified();
    void loadChineseTraditional();
    void loadEnglish();
    void loadJapanese();
    
    // -------------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------------
    
    Language m_currentLang = Language::ZH_CN;
    std::unordered_map<std::string, std::string> m_strings;
    std::string m_emptyString;
};

// =============================================================================
// Convenience macro for getting localized strings
// =============================================================================
#define L(key) Localization::getInstance().get(key)
