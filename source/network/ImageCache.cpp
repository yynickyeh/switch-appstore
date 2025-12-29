// =============================================================================
// Switch App Store - Image Cache Implementation
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
    // Free all cached textures
    for (auto& pair : m_memoryCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    m_memoryCache.clear();
    m_textureSizes.clear();
    m_currentCacheSize = 0;
    m_httpClient.reset();
}

// =============================================================================
// Image Loading
// =============================================================================

SDL_Texture* ImageCache::getCached(const std::string& url) {
    auto it = m_memoryCache.find(url);
    if (it != m_memoryCache.end()) {
        return it->second;
    }
    return nullptr;
}

void ImageCache::requestImage(const std::string& url) {
    // Check if already cached
    if (m_memoryCache.find(url) != m_memoryCache.end()) {
        return;
    }
    
    // Check if already queued
    if (m_loadingUrls.find(url) != m_loadingUrls.end()) {
        return;
    }
    
    // Add to queue
    m_loadQueue.push(url);
    m_loadingUrls[url] = true;
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
            m_memoryCache[url] = tex;
            return tex;
        }
    }
    
    // Download from network
    if (!m_httpClient) return nullptr;
    
    std::vector<uint8_t> data = m_httpClient->downloadData(url);
    if (data.empty()) return nullptr;
    
    // Save to disk cache
    FILE* file = fopen(cachePath.c_str(), "wb");
    if (file) {
        fwrite(data.data(), 1, data.size(), file);
        fclose(file);
    }
    
    // Load into texture
    SDL_Texture* tex = loadFromMemory(data.data(), data.size());
    if (tex) {
        m_memoryCache[url] = tex;
        
        // Track size
        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        size_t size = w * h * 4;  // Approximate RGBA size
        m_textureSizes[url] = size;
        m_currentCacheSize += size;
        
        evictIfNeeded();
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
    for (auto& pair : m_memoryCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    m_memoryCache.clear();
    m_textureSizes.clear();
    m_currentCacheSize = 0;
}

void ImageCache::clearDiskCache() {
    // Simple implementation: just clear our tracking
    // Real implementation would scan and delete files in m_cacheDir
}

size_t ImageCache::getMemoryCacheSize() const {
    return m_currentCacheSize;
}

void ImageCache::evictIfNeeded() {
    // Simple LRU-ish eviction: just remove oldest entries
    // Real implementation would track access times
    while (m_currentCacheSize > m_maxMemoryCacheSize && !m_memoryCache.empty()) {
        auto it = m_memoryCache.begin();
        
        if (it->second) {
            SDL_DestroyTexture(it->second);
        }
        
        auto sizeIt = m_textureSizes.find(it->first);
        if (sizeIt != m_textureSizes.end()) {
            m_currentCacheSize -= sizeIt->second;
            m_textureSizes.erase(sizeIt);
        }
        
        m_memoryCache.erase(it);
    }
}

// =============================================================================
// Helpers
// =============================================================================

std::string ImageCache::getCachePath(const std::string& url) const {
    return m_cacheDir + "/" + hashUrl(url);
}

std::string ImageCache::hashUrl(const std::string& url) const {
    // Simple hash for filename
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
        if (urlExt == ".jpg" || urlExt == ".jpeg" || 
            urlExt == ".png" || urlExt == ".webp") {
            ext = urlExt;
        }
    }
    
    return std::string(buf) + ext;
}
