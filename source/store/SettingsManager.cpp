// =============================================================================
// Switch App Store - Settings Manager Implementation
// =============================================================================

#include "SettingsManager.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// =============================================================================
// Singleton
// =============================================================================

SettingsManager& SettingsManager::getInstance() {
    static SettingsManager instance;
    return instance;
}

// =============================================================================
// Initialization
// =============================================================================

void SettingsManager::init(const std::string& settingsPath) {
    m_settingsPath = settingsPath;
    setDefaults();
    load();
}

void SettingsManager::setDefaults() {
    m_settings["theme_dark"] = "false";
    m_settings["language"] = "zh-CN";
    m_settings["auto_update"] = "true";
    m_settings["download_dir"] = "sdmc:/switch/appstore/downloads";
    m_settings["install_dir"] = "sdmc:/switch";
    m_settings["max_downloads"] = "1";
    m_settings["image_cache_mb"] = "50";
}

// =============================================================================
// Load/Save
// =============================================================================

void SettingsManager::load() {
    FILE* file = fopen(m_settingsPath.c_str(), "r");
    if (!file) return;
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Parse "key=value" format
        char* eq = strchr(line, '=');
        if (!eq) continue;
        
        *eq = '\0';
        std::string key = line;
        std::string value = eq + 1;
        
        // Remove newline
        while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
            value.pop_back();
        }
        
        // Trim whitespace
        while (!key.empty() && key.back() == ' ') key.pop_back();
        while (!key.empty() && key.front() == ' ') key.erase(0, 1);
        while (!value.empty() && value.front() == ' ') value.erase(0, 1);
        
        m_settings[key] = value;
    }
    
    fclose(file);
}

void SettingsManager::save() {
    FILE* file = fopen(m_settingsPath.c_str(), "w");
    if (!file) return;
    
    for (const auto& pair : m_settings) {
        fprintf(file, "%s=%s\n", pair.first.c_str(), pair.second.c_str());
    }
    
    fclose(file);
}

// =============================================================================
// Boolean Settings
// =============================================================================

bool SettingsManager::getBool(const std::string& key, bool defaultValue) const {
    auto it = m_settings.find(key);
    if (it == m_settings.end()) return defaultValue;
    
    return it->second == "true" || it->second == "1";
}

void SettingsManager::setBool(const std::string& key, bool value) {
    m_settings[key] = value ? "true" : "false";
    
    if (m_onChange) {
        m_onChange(key);
    }
}

// =============================================================================
// Integer Settings
// =============================================================================

int SettingsManager::getInt(const std::string& key, int defaultValue) const {
    auto it = m_settings.find(key);
    if (it == m_settings.end()) return defaultValue;
    
    return atoi(it->second.c_str());
}

void SettingsManager::setInt(const std::string& key, int value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    m_settings[key] = buf;
    
    if (m_onChange) {
        m_onChange(key);
    }
}

// =============================================================================
// Float Settings
// =============================================================================

float SettingsManager::getFloat(const std::string& key, float defaultValue) const {
    auto it = m_settings.find(key);
    if (it == m_settings.end()) return defaultValue;
    
    return static_cast<float>(atof(it->second.c_str()));
}

void SettingsManager::setFloat(const std::string& key, float value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.4f", value);
    m_settings[key] = buf;
    
    if (m_onChange) {
        m_onChange(key);
    }
}

// =============================================================================
// String Settings
// =============================================================================

std::string SettingsManager::getString(const std::string& key, 
                                        const std::string& defaultValue) const {
    auto it = m_settings.find(key);
    if (it == m_settings.end()) return defaultValue;
    
    return it->second;
}

void SettingsManager::setString(const std::string& key, const std::string& value) {
    m_settings[key] = value;
    
    if (m_onChange) {
        m_onChange(key);
    }
}
