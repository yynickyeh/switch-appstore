// =============================================================================
// Switch App Store - Downloader Implementation
// =============================================================================

#include "Downloader.hpp"
#include <cstdio>
#include <sys/stat.h>

// =============================================================================
// DownloadItem helpers
// =============================================================================

std::string DownloadItem::getProgressString() const {
    auto formatSize = [](size_t bytes) -> std::string {
        char buf[32];
        if (bytes >= 1024 * 1024 * 1024) {
            snprintf(buf, sizeof(buf), "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
        } else if (bytes >= 1024 * 1024) {
            snprintf(buf, sizeof(buf), "%.1f MB", bytes / (1024.0 * 1024.0));
        } else if (bytes >= 1024) {
            snprintf(buf, sizeof(buf), "%.1f KB", bytes / 1024.0);
        } else {
            snprintf(buf, sizeof(buf), "%zu B", bytes);
        }
        return std::string(buf);
    };
    
    return formatSize(downloadedBytes) + " / " + formatSize(totalBytes);
}

// =============================================================================
// Singleton
// =============================================================================

Downloader& Downloader::getInstance() {
    static Downloader instance;
    return instance;
}

// =============================================================================
// Initialization
// =============================================================================

void Downloader::init(const std::string& downloadDir) {
    m_downloadDir = downloadDir;
    m_httpClient = std::make_unique<HttpClient>();
    
    // Create download directory
    mkdir(downloadDir.c_str(), 0755);
}

void Downloader::shutdown() {
    // Cancel any active download
    m_cancelRequested = true;
    m_httpClient.reset();
    m_downloads.clear();
}

// =============================================================================
// Queue Management
// =============================================================================

std::string Downloader::addDownload(const std::string& name, const std::string& url,
                                     const std::string& filename) {
    DownloadItem item;
    item.id = generateId();
    item.name = name;
    item.url = url;
    
    // Generate output path
    std::string outFile = filename.empty() ? (item.id + ".nro") : filename;
    item.outputPath = m_downloadDir + "/" + outFile;
    
    item.status = DownloadStatus::Queued;
    item.totalBytes = 0;
    item.downloadedBytes = 0;
    
    m_downloads.push_back(item);
    
    return item.id;
}

void Downloader::removeDownload(const std::string& id) {
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        if (it->id == id) {
            // If this is the active download, request cancel
            if (it->status == DownloadStatus::Downloading) {
                m_cancelRequested = true;
            }
            
            // Mark as cancelled but don't remove yet
            // (let update() clean it up)
            it->status = DownloadStatus::Cancelled;
            break;
        }
    }
}

void Downloader::clearCompleted() {
    auto it = m_downloads.begin();
    while (it != m_downloads.end()) {
        if (it->status == DownloadStatus::Completed ||
            it->status == DownloadStatus::Cancelled) {
            it = m_downloads.erase(it);
        } else {
            ++it;
        }
    }
}

DownloadItem* Downloader::getDownload(const std::string& id) {
    for (auto& item : m_downloads) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

// =============================================================================
// Download Control
// =============================================================================

void Downloader::pause(const std::string& id) {
    DownloadItem* item = getDownload(id);
    if (item && item->status == DownloadStatus::Downloading) {
        m_cancelRequested = true;
        item->status = DownloadStatus::Paused;
    }
}

void Downloader::resume(const std::string& id) {
    DownloadItem* item = getDownload(id);
    if (item && item->status == DownloadStatus::Paused) {
        item->status = DownloadStatus::Queued;
    }
}

void Downloader::pauseAll() {
    for (auto& item : m_downloads) {
        if (item.status == DownloadStatus::Downloading ||
            item.status == DownloadStatus::Queued) {
            item.status = DownloadStatus::Paused;
        }
    }
    m_cancelRequested = true;
}

void Downloader::resumeAll() {
    for (auto& item : m_downloads) {
        if (item.status == DownloadStatus::Paused) {
            item.status = DownloadStatus::Queued;
        }
    }
}

// =============================================================================
// Processing
// =============================================================================

void Downloader::update() {
    // Clean up cancelled downloads
    auto it = m_downloads.begin();
    while (it != m_downloads.end()) {
        if (it->status == DownloadStatus::Cancelled) {
            // Delete partial file
            remove(it->outputPath.c_str());
            it = m_downloads.erase(it);
        } else {
            ++it;
        }
    }
    
    // Start next download if none active
    if (m_currentDownloads < m_maxConcurrent) {
        startNextDownload();
    }
}

void Downloader::startNextDownload() {
    // Find next queued download
    int index = -1;
    for (size_t i = 0; i < m_downloads.size(); i++) {
        if (m_downloads[i].status == DownloadStatus::Queued) {
            index = static_cast<int>(i);
            break;
        }
    }
    
    if (index < 0) return;
    
    DownloadItem& item = m_downloads[index];
    item.status = DownloadStatus::Downloading;
    m_activeIndex = index;
    m_currentDownloads++;
    m_cancelRequested = false;
    
    // Download with progress callback
    bool success = m_httpClient->downloadFile(
        item.url,
        item.outputPath,
        [this, index](size_t downloaded, size_t total) {
            if (index < static_cast<int>(m_downloads.size())) {
                m_downloads[index].downloadedBytes = downloaded;
                m_downloads[index].totalBytes = total;
                
                if (m_onProgress) {
                    m_onProgress(m_downloads[index]);
                }
            }
        }
    );
    
    // Update status
    if (m_cancelRequested) {
        // Was cancelled during download
        if (item.status != DownloadStatus::Paused) {
            item.status = DownloadStatus::Cancelled;
        }
    } else if (success) {
        item.status = DownloadStatus::Completed;
        item.downloadedBytes = item.totalBytes;
    } else {
        item.status = DownloadStatus::Failed;
        item.error = "Download failed";
    }
    
    m_currentDownloads--;
    m_activeIndex = -1;
    
    // Fire completion callback
    if (m_onComplete) {
        m_onComplete(item, success);
    }
}

bool Downloader::hasActiveDownload() const {
    for (const auto& item : m_downloads) {
        if (item.status == DownloadStatus::Downloading) {
            return true;
        }
    }
    return false;
}

size_t Downloader::getQueuedCount() const {
    size_t count = 0;
    for (const auto& item : m_downloads) {
        if (item.status == DownloadStatus::Queued) {
            count++;
        }
    }
    return count;
}

// =============================================================================
// Helpers
// =============================================================================

std::string Downloader::generateId() {
    char buf[16];
    snprintf(buf, sizeof(buf), "dl_%d", m_nextId++);
    return std::string(buf);
}
