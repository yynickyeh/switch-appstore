// =============================================================================
// Switch App Store - Store Manager Implementation
// =============================================================================

#include "StoreManager.hpp"
#include <cstdio>
#include <ctime>
#include <algorithm>

// =============================================================================
// StoreEntry helpers
// =============================================================================

std::string StoreEntry::getFormattedSize() const {
    char buf[32];
    if (fileSize >= 1024ULL * 1024 * 1024) {
        snprintf(buf, sizeof(buf), "%.1f GB", fileSize / (1024.0 * 1024.0 * 1024.0));
    } else if (fileSize >= 1024 * 1024) {
        snprintf(buf, sizeof(buf), "%.1f MB", fileSize / (1024.0 * 1024.0));
    } else if (fileSize >= 1024) {
        snprintf(buf, sizeof(buf), "%.1f KB", fileSize / 1024.0);
    } else {
        snprintf(buf, sizeof(buf), "%zu B", fileSize);
    }
    return std::string(buf);
}

// =============================================================================
// Singleton
// =============================================================================

StoreManager& StoreManager::getInstance() {
    static StoreManager instance;
    return instance;
}

// =============================================================================
// Initialization
// =============================================================================

void StoreManager::init(const std::string& configPath) {
    m_configPath = configPath;
    m_httpClient = std::make_unique<HttpClient>();
    
    // Initialize default categories
    m_categories = {
        {"games", "游戏", "game"},
        {"homebrew", "自制软件", "app"},
        {"emulators", "模拟器", "gamepad"},
        {"tools", "工具", "tool"},
        {"themes", "主题", "palette"},
    };
    
    loadConfig();
    
    // Add default source if none exist
    if (m_sources.empty()) {
        addDefaultSource();
    }
}

void StoreManager::shutdown() {
    saveConfig();
    m_httpClient.reset();
    m_entries.clear();
    m_sources.clear();
}

// =============================================================================
// Source Management
// =============================================================================

bool StoreManager::addSource(const StoreSource& source) {
    // Check for duplicate ID
    for (const auto& s : m_sources) {
        if (s.id == source.id) {
            return false;
        }
    }
    
    m_sources.push_back(source);
    saveConfig();
    return true;
}

void StoreManager::removeSource(const std::string& id) {
    auto it = std::remove_if(m_sources.begin(), m_sources.end(),
                              [&id](const StoreSource& s) { return s.id == id; });
    if (it != m_sources.end()) {
        m_sources.erase(it, m_sources.end());
        saveConfig();
    }
}

void StoreManager::setSourceEnabled(const std::string& id, bool enabled) {
    for (auto& source : m_sources) {
        if (source.id == id) {
            source.enabled = enabled;
            saveConfig();
            break;
        }
    }
}

// =============================================================================
// Catalog Access
// =============================================================================

std::vector<const StoreEntry*> StoreManager::getEntriesByCategory(
    const std::string& category) const {
    std::vector<const StoreEntry*> result;
    for (const auto& entry : m_entries) {
        if (entry.category == category) {
            result.push_back(&entry);
        }
    }
    return result;
}

std::vector<const StoreEntry*> StoreManager::getFeaturedEntries(int count) const {
    std::vector<const StoreEntry*> result;
    
    // Sort by rating + download count (simple popularity score)
    std::vector<std::pair<float, const StoreEntry*>> scored;
    for (const auto& entry : m_entries) {
        float score = entry.rating * 10 + entry.downloadCount / 1000.0f;
        scored.push_back({score, &entry});
    }
    
    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    for (int i = 0; i < count && i < (int)scored.size(); i++) {
        result.push_back(scored[i].second);
    }
    
    return result;
}

std::vector<const StoreEntry*> StoreManager::search(const std::string& query) const {
    std::vector<const StoreEntry*> result;
    
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& entry : m_entries) {
        std::string lowerName = entry.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        std::string lowerDev = entry.developer;
        std::transform(lowerDev.begin(), lowerDev.end(), lowerDev.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos ||
            lowerDev.find(lowerQuery) != std::string::npos) {
            result.push_back(&entry);
        }
    }
    
    return result;
}

const StoreEntry* StoreManager::getEntry(const std::string& id) const {
    for (const auto& entry : m_entries) {
        if (entry.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

// =============================================================================
// Refresh
// =============================================================================

bool StoreManager::needsRefresh() const {
    uint64_t now = static_cast<uint64_t>(time(nullptr));
    return (now - m_lastRefreshTime) > REFRESH_INTERVAL;
}

void StoreManager::refresh() {
    if (m_isRefreshing) return;
    
    m_isRefreshing = true;
    m_entries.clear();
    
    bool anySuccess = false;
    
    for (const auto& source : m_sources) {
        if (!source.enabled) continue;
        
        std::string apiUrl = source.url + "/api/catalog.json";
        HttpResponse response = m_httpClient->get(apiUrl);
        
        if (response.isSuccess()) {
            parseCatalog(response.body, source.id);
            anySuccess = true;
        }
    }
    
    m_lastRefreshTime = static_cast<uint64_t>(time(nullptr));
    m_isRefreshing = false;
    
    if (m_onRefreshComplete) {
        m_onRefreshComplete(anySuccess);
    }
}

// =============================================================================
// JSON Parsing (Simplified)
// =============================================================================

void StoreManager::parseCatalog(const std::string& json, const std::string& sourceId) {
    // Very simplified JSON array parsing
    // Real implementation would use proper JSON library
    
    // Find "entries" array
    size_t pos = json.find("\"entries\"");
    if (pos == std::string::npos) return;
    
    // For demo, just add some mock entries
    // Real implementation would parse the actual JSON
    
    StoreEntry entry;
    entry.id = sourceId + "_1";
    entry.name = "示例游戏";
    entry.developer = "开发者";
    entry.category = "games";
    entry.version = "1.0.0";
    entry.rating = 4.5f;
    entry.fileSize = 100 * 1024 * 1024;  // 100 MB
    m_entries.push_back(entry);
}

// =============================================================================
// Configuration
// =============================================================================

void StoreManager::loadConfig() {
    FILE* file = fopen(m_configPath.c_str(), "r");
    if (!file) return;
    
    // Read file content
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    std::string content(size, '\0');
    fread(&content[0], 1, size, file);
    fclose(file);
    
    // Parse sources from config (simplified)
    // Real implementation would parse proper JSON/config format
}

void StoreManager::saveConfig() {
    FILE* file = fopen(m_configPath.c_str(), "w");
    if (!file) return;
    
    // Write sources as simple format
    fprintf(file, "{\n  \"sources\": [\n");
    for (size_t i = 0; i < m_sources.size(); i++) {
        const auto& s = m_sources[i];
        fprintf(file, "    {\"id\":\"%s\",\"name\":\"%s\",\"url\":\"%s\",\"enabled\":%s}%s\n",
                s.id.c_str(), s.name.c_str(), s.url.c_str(),
                s.enabled ? "true" : "false",
                i < m_sources.size() - 1 ? "," : "");
    }
    fprintf(file, "  ]\n}\n");
    fclose(file);
}

void StoreManager::addDefaultSource() {
    StoreSource defaultSource;
    defaultSource.id = "homebrew_switch";
    defaultSource.name = "Switch Homebrew App Store";
    defaultSource.url = "https://switchbru.com/appstore";
    defaultSource.enabled = true;
    defaultSource.priority = 100;
    
    m_sources.push_back(defaultSource);
}
