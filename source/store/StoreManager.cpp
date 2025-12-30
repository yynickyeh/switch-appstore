// =============================================================================
// Switch App Store - Store Manager Implementation
// =============================================================================

#include "StoreManager.hpp"
#include "json.hpp"
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
    std::string lastError;
    
    for (const auto& source : m_sources) {
        if (!source.enabled) continue;
        
        // -------------------------------------------------------------------------
        // Construct the API URL. The server uses /api/catalog, not /api/catalog.json
        // -------------------------------------------------------------------------
        std::string apiUrl = source.url + "/api/catalog";
        HttpResponse response = m_httpClient->get(apiUrl);
        
        if (response.isSuccess()) {
            parseCatalog(response.body, source.id, source.url);
            anySuccess = true;
        } else {
            lastError = response.error;
            if (lastError.empty()) {
                lastError = "HTTP " + std::to_string(response.statusCode);
            }
        }
    }
    
    m_lastRefreshTime = static_cast<uint64_t>(time(nullptr));
    m_isRefreshing = false;
    
    if (m_onRefreshComplete) {
        m_onRefreshComplete(anySuccess, anySuccess ? "" : lastError);
    }
}

// =============================================================================
// JSON Parsing (Simplified)
// =============================================================================

void StoreManager::parseCatalog(const std::string& jsonStr, const std::string& sourceId, const std::string& baseUrl) {
    // -------------------------------------------------------------------------
    // Parse the JSON response from /api/catalog
    // Expected format: { success: true, data: { games: [...], categories: [...] } }
    // -------------------------------------------------------------------------
    
    json::Value root = json::parse(jsonStr);
    
    // Check for success
    if (!root["success"].asBool(false)) {
        return;
    }
    
    // Get the data object
    const json::Value& data = root["data"];
    if (data.isNull()) {
        return;
    }
    
    // Parse games array
    const json::Value& games = data["games"];
    if (games.isArray()) {
        for (size_t i = 0; i < games.size(); ++i) {
            const json::Value& game = games[i];
            
            StoreEntry entry;
            entry.id = game["id"].asString();
            entry.name = game["name"].asString();
            entry.developer = game["developer"].asString();
            entry.description = game["description"].asString();
            entry.category = game["category"].asString();
            entry.version = game["version"].asString();
            entry.titleId = game["titleId"].asString();
            // Helper to resolve relative URLs
            auto resolveUrl = [&](std::string url) {
                if (url.empty()) return url;
                if (url.front() == '/') {
                    return baseUrl + url;
                }
                return url;
            };

            entry.iconUrl = resolveUrl(game["iconUrl"].asString());
            entry.downloadUrl = resolveUrl(game["downloadUrl"].asString());
            entry.fileSize = static_cast<size_t>(game["fileSize"].asNumber(0));
            entry.rating = static_cast<float>(game["rating"].asNumber(0.0));
            entry.downloadCount = game["downloadCount"].asInt(0);
            entry.releaseDate = game["releaseDate"].asString();
            
            // Parse screenshot URLs array
            const json::Value& screenshots = game["screenshotUrls"];
            if (screenshots.isArray()) {
                for (size_t j = 0; j < screenshots.size(); ++j) {
                    entry.screenshotUrls.push_back(resolveUrl(screenshots[j].asString()));
                }
            }
            
            // Parse languages array
            const json::Value& languages = game["languages"];
            if (languages.isArray()) {
                for (size_t j = 0; j < languages.size(); ++j) {
                    entry.languages.push_back(languages[j].asString());
                }
            }
            
            m_entries.push_back(entry);
        }
    }
    
    // Parse categories array
    const json::Value& categories = data["categories"];
    if (categories.isArray()) {
        m_categories.clear();
        for (size_t i = 0; i < categories.size(); ++i) {
            const json::Value& cat = categories[i];
            
            StoreCategory category;
            category.id = cat["id"].asString();
            category.name = cat["name"].asString();
            category.iconName = cat["icon"].asString();
            
            m_categories.push_back(category);
        }
    }
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
    // -------------------------------------------------------------------------
    // Add default source - points to local development server
    // In production, this could be loaded from config or set by user
    // -------------------------------------------------------------------------
    StoreSource defaultSource;
    defaultSource.id = "local_dev_server";
    defaultSource.name = "Local Development Server";
    defaultSource.url = "http://124.156.197.94:5090";  // Local Express server
    defaultSource.enabled = true;
    defaultSource.priority = 100;
    
    m_sources.push_back(defaultSource);
}

// =============================================================================
// Download Statistics
// =============================================================================

void StoreManager::reportDownload(const std::string& gameId, 
                                   DownloadReportCallback callback) {
    // -------------------------------------------------------------------------
    // Send POST request to /api/catalog/download/:gameId to increment
    // the server-side download counter. Upon success, update local entry.
    // -------------------------------------------------------------------------
    
    if (!m_httpClient) {
        if (callback) callback(false, 0);
        return;
    }
    
    // Find the first enabled source to get the base URL
    std::string baseUrl;
    for (const auto& source : m_sources) {
        if (source.enabled && !source.url.empty()) {
            baseUrl = source.url;
            break;
        }
    }
    
    if (baseUrl.empty()) {
        if (callback) callback(false, 0);
        return;
    }
    
    // Construct the API endpoint URL
    std::string apiUrl = baseUrl + "/api/catalog/download/" + gameId;
    
    // Send POST request (empty body for this endpoint)
    HttpResponse response = m_httpClient->post(apiUrl, "{}");
    
    if (response.isSuccess()) {
        // -------------------------------------------------------------------------
        // Parse the response JSON to extract the new download count
        // Expected format: { success: true, data: { newDownloadCount: N } }
        // -------------------------------------------------------------------------
        json::Value root = json::parse(response.body);
        
        if (root["success"].asBool(false)) {
            const json::Value& data = root["data"];
            int newCount = data["newDownloadCount"].asInt(0);
            
            // Update local entry with new count
            updateLocalDownloadCount(gameId, newCount);
            
            if (callback) callback(true, newCount);
            return;
        }
    }
    
    // Failed - call callback with failure
    if (callback) callback(false, 0);
}

void StoreManager::updateLocalDownloadCount(const std::string& gameId, int newCount) {
    // -------------------------------------------------------------------------
    // Find and update the local entry's download count
    // This keeps the local cache in sync with server data
    // -------------------------------------------------------------------------
    for (auto& entry : m_entries) {
        if (entry.id == gameId) {
            entry.downloadCount = newCount;
            break;
        }
    }
}

