// =============================================================================
// Switch App Store - Image Cache
// =============================================================================
// Manages downloading and caching of images from network
// Supports async loading with placeholders and disk/memory caching
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

// Forward declaration
class Renderer;

// =============================================================================
// ImageCache - Manages image loading and caching
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
    
    // Evict old entries if over size limit
    void evictIfNeeded();
    
    // -------------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------------
    
    SDL_Renderer* m_renderer = nullptr;
    std::string m_cacheDir;
    
    // Memory cache: URL -> texture
    std::unordered_map<std::string, SDL_Texture*> m_memoryCache;
    
    // Track texture sizes for memory management
    std::unordered_map<std::string, size_t> m_textureSizes;
    size_t m_currentCacheSize = 0;
    size_t m_maxMemoryCacheSize = 50 * 1024 * 1024;  // 50 MB default
    
    // Load queue
    std::queue<std::string> m_loadQueue;
    
    // HTTP client for downloads
    std::unique_ptr<HttpClient> m_httpClient;
    
    // Set of URLs currently being loaded (to avoid duplicates)
    std::unordered_map<std::string, bool> m_loadingUrls;
};
