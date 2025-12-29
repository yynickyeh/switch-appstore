// =============================================================================
// Switch App Store - Settings Manager
// =============================================================================
// Manages application settings with persistent storage
// =============================================================================

#pragma once

#include <string>
#include <map>
#include <functional>

// =============================================================================
// Setting types
// =============================================================================
enum class SettingType {
    Bool,
    Int,
    Float,
    String
};

// =============================================================================
// SettingsManager - Application settings
// =============================================================================
class SettingsManager {
public:
    // -------------------------------------------------------------------------
    // Singleton access
    // -------------------------------------------------------------------------
    static SettingsManager& getInstance();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    void init(const std::string& settingsPath);
    void save();
    
    // -------------------------------------------------------------------------
    // Boolean settings
    // -------------------------------------------------------------------------
    bool getBool(const std::string& key, bool defaultValue = false) const;
    void setBool(const std::string& key, bool value);
    
    // -------------------------------------------------------------------------
    // Integer settings
    // -------------------------------------------------------------------------
    int getInt(const std::string& key, int defaultValue = 0) const;
    void setInt(const std::string& key, int value);
    
    // -------------------------------------------------------------------------
    // Float settings
    // -------------------------------------------------------------------------
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    void setFloat(const std::string& key, float value);
    
    // -------------------------------------------------------------------------
    // String settings
    // -------------------------------------------------------------------------
    std::string getString(const std::string& key, 
                          const std::string& defaultValue = "") const;
    void setString(const std::string& key, const std::string& value);
    
    // -------------------------------------------------------------------------
    // Predefined settings (convenience)
    // -------------------------------------------------------------------------
    
    // Theme
    bool isDarkMode() const { return getBool("theme_dark", false); }
    void setDarkMode(bool dark) { setBool("theme_dark", dark); }
    
    // Language
    std::string getLanguage() const { return getString("language", "zh-CN"); }
    void setLanguage(const std::string& lang) { setString("language", lang); }
    
    // Auto-update check
    bool isAutoUpdateEnabled() const { return getBool("auto_update", true); }
    void setAutoUpdate(bool enabled) { setBool("auto_update", enabled); }
    
    // Download location
    std::string getDownloadDir() const { 
        return getString("download_dir", "sdmc:/switch/appstore/downloads"); 
    }
    void setDownloadDir(const std::string& dir) { setString("download_dir", dir); }
    
    // Install location
    std::string getInstallDir() const { 
        return getString("install_dir", "sdmc:/switch"); 
    }
    void setInstallDir(const std::string& dir) { setString("install_dir", dir); }
    
    // Max concurrent downloads
    int getMaxDownloads() const { return getInt("max_downloads", 1); }
    void setMaxDownloads(int count) { setInt("max_downloads", count); }
    
    // Image cache size (MB)
    int getImageCacheSize() const { return getInt("image_cache_mb", 50); }
    void setImageCacheSize(int sizeMB) { setInt("image_cache_mb", sizeMB); }
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    using SettingChangeCallback = std::function<void(const std::string& key)>;
    void setOnChange(SettingChangeCallback callback) { m_onChange = callback; }
    
private:
    SettingsManager() = default;
    ~SettingsManager() = default;
    
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    
    void load();
    void setDefaults();
    
    std::string m_settingsPath;
    std::map<std::string, std::string> m_settings;
    SettingChangeCallback m_onChange;
};
