// =============================================================================
// Switch App Store - Image Cache
// =============================================================================
// Manages downloading and caching of images from network
// Supports async loading with placeholders and disk/memory caching
// Features LRU (Least Recently Used) cache eviction strategy
// =============================================================================

#pragma once

#include "HttpClient.hpp"
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <functional>
#include <memory>
#include <chrono>

// Forward declaration
class Renderer;

// =============================================================================
// Image Load State - Tracks the loading status of images
// =============================================================================
enum class ImageLoadState {
    Idle,       // Not started loading
    Loading,    // Currently being downloaded/loaded
    Loaded,     // Successfully loaded and cached
    Failed      // Failed to load (network error, invalid format, etc.)
};

// =============================================================================
// Cache Entry - Stores texture with LRU metadata
// =============================================================================
struct CacheEntry {
    SDL_Texture* texture = nullptr;     // The actual texture
    size_t size = 0;                    // Approximate memory size in bytes
    uint64_t lastAccess = 0;            // Last access timestamp (for LRU)
    uint64_t insertTime = 0;            // When this entry was created
    ImageLoadState state = ImageLoadState::Idle;  // Current load state
};

// =============================================================================
// ImageCache - Manages image loading and caching with LRU eviction
// =============================================================================
class ImageCache {
public:
    // -------------------------------------------------------------------------
    // Singleton access
    // -------------------------------------------------------------------------
    static ImageCache& getInstance();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    void init(SDL_Renderer* renderer, const std::string& cacheDir);
    void shutdown();
    
    // -------------------------------------------------------------------------
    // Image loading
    // -------------------------------------------------------------------------
    
    // Get a cached image texture (returns nullptr if not cached)
    // This also updates the LRU access time for the entry
    SDL_Texture* getCached(const std::string& url);
    
    // Request an image to be loaded (async)
    // Returns immediately - texture will be available later via getCached
    void requestImage(const std::string& url);
    
    // Load an image synchronously (blocks until loaded)
    SDL_Texture* loadSync(const std::string& url);
    
    // Load image from local file
    SDL_Texture* loadFromFile(const std::string& path);
    
    // Load image from memory buffer
    SDL_Texture* loadFromMemory(const uint8_t* data, size_t size);
    
    // -------------------------------------------------------------------------
    // Cache management
    // -------------------------------------------------------------------------
    
    // Clear all memory cache
    void clearMemoryCache();
    
    // Clear disk cache
    void clearDiskCache();
    
    // Get memory cache size in bytes (approximate)
    size_t getMemoryCacheSize() const;
    
    // Set maximum memory cache size
    void setMaxMemoryCacheSize(size_t bytes) { m_maxMemoryCacheSize = bytes; }
    
    // Get the number of cached entries
    size_t getCacheEntryCount() const { return m_cacheEntries.size(); }
    
    // -------------------------------------------------------------------------
    // Load state management
    // -------------------------------------------------------------------------
    
    // Callback type for image state changes: (url, newState)
    using ImageStateCallback = std::function<void(const std::string&, ImageLoadState)>;
    
    // Set the callback for state changes (loading, loaded, failed)
    void setStateCallback(ImageStateCallback callback) { m_stateCallback = callback; }
    
    // Get the current load state for a URL
    ImageLoadState getLoadState(const std::string& url) const;
    
    // -------------------------------------------------------------------------
    // Processing pending loads
    // -------------------------------------------------------------------------
    
    // Process one pending load (call from main thread each frame)
    void processOne();
    
    // Check if any images are loading
    bool hasQueuedLoads() const { return !m_loadQueue.empty(); }
    
private:
    ImageCache() = default;
    ~ImageCache();
    
    // Prevent copying
    ImageCache(const ImageCache&) = delete;
    ImageCache& operator=(const ImageCache&) = delete;
    
    // Get cache file path for URL
    std::string getCachePath(const std::string& url) const;
    
    // Generate hash for URL
    std::string hashUrl(const std::string& url) const;
    
    // Evict old entries if over size limit (LRU strategy)
    void evictIfNeeded();
    
    // Get current timestamp in milliseconds
    uint64_t getCurrentTimestamp() const;
    
    // -------------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------------
    
    SDL_Renderer* m_renderer = nullptr;
    std::string m_cacheDir;
    
    // LRU Cache: URL -> CacheEntry (contains texture, size, access times, state)
    std::unordered_map<std::string, CacheEntry> m_cacheEntries;
    
    // Cache size tracking
    size_t m_currentCacheSize = 0;
    size_t m_maxMemoryCacheSize = 50 * 1024 * 1024;  // 50 MB default
    
    // Load queue for async loading
    std::queue<std::string> m_loadQueue;
    
    // HTTP client for downloads
    std::unique_ptr<HttpClient> m_httpClient;
    
    // Set of URLs currently being loaded (to avoid duplicate requests)
    std::unordered_map<std::string, bool> m_loadingUrls;
    
    // Callback for state changes (optional)
    ImageStateCallback m_stateCallback;
};
