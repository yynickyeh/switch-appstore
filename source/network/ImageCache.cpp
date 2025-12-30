// =============================================================================
// Switch App Store - Image Cache Implementation
// =============================================================================
// Implements LRU (Least Recently Used) cache eviction strategy
// Tracks image loading states and provides callbacks for UI updates
// =============================================================================

#include "ImageCache.hpp"
#include <SDL2/SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

// =============================================================================
// Singleton
// =============================================================================

ImageCache& ImageCache::getInstance() {
    static ImageCache instance;
    return instance;
}

ImageCache::~ImageCache() {
    shutdown();
}

// =============================================================================
// Initialization
// =============================================================================

void ImageCache::init(SDL_Renderer* renderer, const std::string& cacheDir) {
    m_renderer = renderer;
    m_cacheDir = cacheDir;
    m_httpClient = std::make_unique<HttpClient>();
    
    // Create cache directory if it doesn't exist
    mkdir(cacheDir.c_str(), 0755);
}

void ImageCache::shutdown() {
    // Free all cached textures from the new CacheEntry structure
    for (auto& pair : m_cacheEntries) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    m_cacheEntries.clear();
    m_currentCacheSize = 0;
    m_httpClient.reset();
}

// =============================================================================
// Image Loading
// =============================================================================

SDL_Texture* ImageCache::getCached(const std::string& url) {
    // -------------------------------------------------------------------------
    // Look up the cache entry and update its LRU timestamp if found
    // This is the core of the LRU algorithm - recently accessed items
    // get their timestamp updated so they're less likely to be evicted
    // -------------------------------------------------------------------------
    auto it = m_cacheEntries.find(url);
    if (it != m_cacheEntries.end() && it->second.texture) {
        // Update last access time for LRU tracking
        it->second.lastAccess = getCurrentTimestamp();
        return it->second.texture;
    }
    return nullptr;
}

void ImageCache::requestImage(const std::string& url) {
    // Check if already cached and loaded
    auto cacheIt = m_cacheEntries.find(url);
    if (cacheIt != m_cacheEntries.end() && 
        cacheIt->second.state == ImageLoadState::Loaded) {
        return;
    }
    
    // Check if already queued for loading
    if (m_loadingUrls.find(url) != m_loadingUrls.end()) {
        return;
    }
    
    // -------------------------------------------------------------------------
    // Create a new cache entry with Loading state
    // This allows UI components to show loading indicators
    // -------------------------------------------------------------------------
    CacheEntry entry;
    entry.state = ImageLoadState::Loading;
    entry.insertTime = getCurrentTimestamp();
    entry.lastAccess = entry.insertTime;
    m_cacheEntries[url] = entry;
    
    // Add to load queue
    m_loadQueue.push(url);
    m_loadingUrls[url] = true;
    
    // Notify listeners that loading has started
    if (m_stateCallback) {
        m_stateCallback(url, ImageLoadState::Loading);
    }
}

SDL_Texture* ImageCache::loadSync(const std::string& url) {
    // Check memory cache first
    auto cached = getCached(url);
    if (cached) return cached;
    
    // Check disk cache
    std::string cachePath = getCachePath(url);
    struct stat st;
    if (stat(cachePath.c_str(), &st) == 0) {
        // File exists in cache, load it
        SDL_Texture* tex = loadFromFile(cachePath);
        if (tex) {
            // -------------------------------------------------------------------------
            // Create cache entry with all LRU metadata
            // -------------------------------------------------------------------------
            CacheEntry entry;
            entry.texture = tex;
            entry.state = ImageLoadState::Loaded;
            
            // Calculate texture size for memory tracking
            int w, h;
            SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
            entry.size = w * h * 4;  // Approximate RGBA size
            
            uint64_t now = getCurrentTimestamp();
            entry.insertTime = now;
            entry.lastAccess = now;
            
            m_cacheEntries[url] = entry;
            m_currentCacheSize += entry.size;
            
            // Notify listeners
            if (m_stateCallback) {
                m_stateCallback(url, ImageLoadState::Loaded);
            }
            
            evictIfNeeded();
            return tex;
        }
    }
    
    // Download from network
    if (!m_httpClient) {
        // Mark as failed if we can't download
        if (m_cacheEntries.find(url) != m_cacheEntries.end()) {
            m_cacheEntries[url].state = ImageLoadState::Failed;
        }
        if (m_stateCallback) {
            m_stateCallback(url, ImageLoadState::Failed);
        }
        return nullptr;
    }
    
    std::vector<uint8_t> data = m_httpClient->downloadData(url);
    if (data.empty()) {
        // Mark as failed
        if (m_cacheEntries.find(url) != m_cacheEntries.end()) {
            m_cacheEntries[url].state = ImageLoadState::Failed;
        }
        if (m_stateCallback) {
            m_stateCallback(url, ImageLoadState::Failed);
        }
        return nullptr;
    }
    
    // Save to disk cache
    FILE* file = fopen(cachePath.c_str(), "wb");
    if (file) {
        fwrite(data.data(), 1, data.size(), file);
        fclose(file);
    }
    
    // Load into texture
    SDL_Texture* tex = loadFromMemory(data.data(), data.size());
    if (tex) {
        // -------------------------------------------------------------------------
        // Create cache entry with full LRU metadata
        // -------------------------------------------------------------------------
        CacheEntry entry;
        entry.texture = tex;
        entry.state = ImageLoadState::Loaded;
        
        // Calculate and track texture size
        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        entry.size = w * h * 4;  // Approximate RGBA size
        
        uint64_t now = getCurrentTimestamp();
        entry.insertTime = now;
        entry.lastAccess = now;
        
        m_cacheEntries[url] = entry;
        m_currentCacheSize += entry.size;
        
        // Notify listeners
        if (m_stateCallback) {
            m_stateCallback(url, ImageLoadState::Loaded);
        }
        
        evictIfNeeded();
    } else {
        // Mark as failed if texture creation failed
        if (m_cacheEntries.find(url) != m_cacheEntries.end()) {
            m_cacheEntries[url].state = ImageLoadState::Failed;
        }
        if (m_stateCallback) {
            m_stateCallback(url, ImageLoadState::Failed);
        }
    }
    
    return tex;
}

SDL_Texture* ImageCache::loadFromFile(const std::string& path) {
    if (!m_renderer) return nullptr;
    
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) return nullptr;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

SDL_Texture* ImageCache::loadFromMemory(const uint8_t* data, size_t size) {
    if (!m_renderer || !data || size == 0) return nullptr;
    
    SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
    if (!rw) return nullptr;
    
    SDL_Surface* surface = IMG_Load_RW(rw, 1);  // 1 = auto-close RWops
    if (!surface) return nullptr;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

// =============================================================================
// Processing
// =============================================================================

void ImageCache::processOne() {
    if (m_loadQueue.empty()) return;
    
    std::string url = m_loadQueue.front();
    m_loadQueue.pop();
    m_loadingUrls.erase(url);
    
    // Load synchronously (in real app, would use background thread)
    loadSync(url);
}

// =============================================================================
// Cache Management
// =============================================================================

void ImageCache::clearMemoryCache() {
    for (auto& pair : m_cacheEntries) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    m_cacheEntries.clear();
    m_currentCacheSize = 0;
}

void ImageCache::clearDiskCache() {
    // Simple implementation: just clear our tracking
    // Real implementation would scan and delete files in m_cacheDir
}

size_t ImageCache::getMemoryCacheSize() const {
    return m_currentCacheSize;
}

ImageLoadState ImageCache::getLoadState(const std::string& url) const {
    auto it = m_cacheEntries.find(url);
    if (it != m_cacheEntries.end()) {
        return it->second.state;
    }
    return ImageLoadState::Idle;
}

void ImageCache::evictIfNeeded() {
    // -------------------------------------------------------------------------
    // LRU (Least Recently Used) Cache Eviction
    // When cache exceeds the maximum size, we find and remove the entry
    // with the oldest (smallest) lastAccess timestamp
    // -------------------------------------------------------------------------
    while (m_currentCacheSize > m_maxMemoryCacheSize && !m_cacheEntries.empty()) {
        // Find the least recently used entry
        auto oldest = m_cacheEntries.begin();
        for (auto it = m_cacheEntries.begin(); it != m_cacheEntries.end(); ++it) {
            // Skip entries that are still loading
            if (it->second.state == ImageLoadState::Loading) continue;
            
            if (it->second.lastAccess < oldest->second.lastAccess) {
                oldest = it;
            }
        }
        
        // If we only have loading entries, don't evict
        if (oldest->second.state == ImageLoadState::Loading) {
            break;
        }
        
        // Free the texture
        if (oldest->second.texture) {
            SDL_DestroyTexture(oldest->second.texture);
        }
        
        // Update cache size and remove entry
        m_currentCacheSize -= oldest->second.size;
        m_cacheEntries.erase(oldest);
    }
}

// =============================================================================
// Helpers
// =============================================================================

uint64_t ImageCache::getCurrentTimestamp() const {
    // -------------------------------------------------------------------------
    // Get current time in milliseconds since epoch
    // Using steady_clock for monotonic timing (not affected by system time changes)
    // -------------------------------------------------------------------------
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()
    );
}

std::string ImageCache::getCachePath(const std::string& url) const {
    return m_cacheDir + "/" + hashUrl(url);
}

std::string ImageCache::hashUrl(const std::string& url) const {
    // Simple DJB2 hash for filename
    // Real implementation would use proper hash like SHA256
    unsigned long hash = 5381;
    for (char c : url) {
        hash = ((hash << 5) + hash) + c;
    }
    
    char buf[20];
    snprintf(buf, sizeof(buf), "%lx", hash);
    
    // Append extension from URL if present
    std::string ext = ".dat";
    size_t dotPos = url.rfind('.');
    if (dotPos != std::string::npos) {
        std::string urlExt = url.substr(dotPos);
        // Only use recognized image extensions
        if (urlExt == ".jpg" || urlExt == ".jpeg" || 
            urlExt == ".png" || urlExt == ".webp") {
            ext = urlExt;
        }
    }
    
    return std::string(buf) + ext;
}
