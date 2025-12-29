// =============================================================================
// Switch App Store - Store Manager
// =============================================================================
// Manages app store sources (repositories) and game catalogs
// Supports multiple sources with priority and caching
// =============================================================================

#pragma once

#include "network/HttpClient.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

// =============================================================================
// App/Game entry from store catalog
// =============================================================================
struct StoreEntry {
    std::string id;
    std::string name;
    std::string developer;
    std::string description;
    std::string category;
    std::string version;
    std::string titleId;
    std::string iconUrl;
    std::vector<std::string> screenshotUrls;
    std::string downloadUrl;
    size_t fileSize = 0;
    float rating = 0.0f;
    int downloadCount = 0;
    std::string releaseDate;
    std::vector<std::string> languages;
    
    // Format file size as string
    std::string getFormattedSize() const;
};

// =============================================================================
// Store source (repository)
// =============================================================================
struct StoreSource {
    std::string id;
    std::string name;
    std::string url;           // API endpoint
    std::string iconUrl;
    bool enabled = true;
    int priority = 0;          // Higher = checked first
    
    // Last update time
    uint64_t lastUpdated = 0;
};

// =============================================================================
// Category definition
// =============================================================================
struct StoreCategory {
    std::string id;
    std::string name;
    std::string iconName;
};

// =============================================================================
// StoreManager - Main store management class
// =============================================================================
class StoreManager {
public:
    // -------------------------------------------------------------------------
    // Singleton access
    // -------------------------------------------------------------------------
    static StoreManager& getInstance();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    void init(const std::string& configPath);
    void shutdown();
    
    // -------------------------------------------------------------------------
    // Source management
    // -------------------------------------------------------------------------
    
    // Get all sources
    const std::vector<StoreSource>& getSources() const { return m_sources; }
    
    // Add a new source
    bool addSource(const StoreSource& source);
    
    // Remove a source by ID
    void removeSource(const std::string& id);
    
    // Enable/disable a source
    void setSourceEnabled(const std::string& id, bool enabled);
    
    // -------------------------------------------------------------------------
    // Catalog access
    // -------------------------------------------------------------------------
    
    // Get all entries (combined from all sources)
    const std::vector<StoreEntry>& getAllEntries() const { return m_entries; }
    
    // Get entries by category
    std::vector<const StoreEntry*> getEntriesByCategory(const std::string& category) const;
    
    // Get featured entries
    std::vector<const StoreEntry*> getFeaturedEntries(int count = 5) const;
    
    // Search entries
    std::vector<const StoreEntry*> search(const std::string& query) const;
    
    // Get entry by ID
    const StoreEntry* getEntry(const std::string& id) const;
    
    // -------------------------------------------------------------------------
    // Categories
    // -------------------------------------------------------------------------
    
    const std::vector<StoreCategory>& getCategories() const { return m_categories; }
    
    // -------------------------------------------------------------------------
    // Refresh
    // -------------------------------------------------------------------------
    
    // Refresh catalog from all sources (async-like, call in update loop)
    void refresh();
    
    // Check if refresh is needed
    bool needsRefresh() const;
    
    // Check if currently refreshing
    bool isRefreshing() const { return m_isRefreshing; }
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    using RefreshCallback = std::function<void(bool success)>;
    void setOnRefreshComplete(RefreshCallback callback) { m_onRefreshComplete = callback; }
    
private:
    StoreManager() = default;
    ~StoreManager() = default;
    
    StoreManager(const StoreManager&) = delete;
    StoreManager& operator=(const StoreManager&) = delete;
    
    // Load/save configuration
    void loadConfig();
    void saveConfig();
    
    // Parse catalog JSON
    void parseCatalog(const std::string& json, const std::string& sourceId);
    
    // Add default source
    void addDefaultSource();
    
    // -------------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------------
    
    std::string m_configPath;
    std::vector<StoreSource> m_sources;
    std::vector<StoreEntry> m_entries;
    std::vector<StoreCategory> m_categories;
    
    std::unique_ptr<HttpClient> m_httpClient;
    
    bool m_isRefreshing = false;
    uint64_t m_lastRefreshTime = 0;
    static constexpr uint64_t REFRESH_INTERVAL = 3600;  // 1 hour
    
    RefreshCallback m_onRefreshComplete;
};
