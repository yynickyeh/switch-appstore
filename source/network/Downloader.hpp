// =============================================================================
// Switch App Store - Downloader
// =============================================================================
// Manages game/app downloads with queue, pause/resume, and progress tracking
// =============================================================================

#pragma once

#include "HttpClient.hpp"
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <memory>

// =============================================================================
// Download status
// =============================================================================
enum class DownloadStatus {
    Queued,         // Waiting in queue
    Downloading,    // Currently downloading
    Paused,         // Paused by user
    Completed,      // Successfully completed
    Failed,         // Failed with error
    Cancelled       // Cancelled by user
};

// =============================================================================
// Download item
// =============================================================================
struct DownloadItem {
    std::string id;             // Unique identifier
    std::string name;           // Display name
    std::string url;            // Download URL
    std::string outputPath;     // Output file path
    
    DownloadStatus status = DownloadStatus::Queued;
    size_t totalBytes = 0;
    size_t downloadedBytes = 0;
    std::string error;
    
    // Progress percentage (0.0 - 1.0)
    float getProgress() const {
        if (totalBytes == 0) return 0.0f;
        return static_cast<float>(downloadedBytes) / totalBytes;
    }
    
    // Format progress as string (e.g., "45.2 MB / 100.0 MB")
    std::string getProgressString() const;
};

// =============================================================================
// Callbacks
// =============================================================================
using DownloadProgressCallback = std::function<void(const DownloadItem&)>;
using DownloadCompleteCallback = std::function<void(const DownloadItem&, bool success)>;

// =============================================================================
// Downloader - Download manager
// =============================================================================
class Downloader {
public:
    // -------------------------------------------------------------------------
    // Singleton access
    // -------------------------------------------------------------------------
    static Downloader& getInstance();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    void init(const std::string& downloadDir);
    void shutdown();
    
    // -------------------------------------------------------------------------
    // Queue management
    // -------------------------------------------------------------------------
    
    // Add a download to the queue
    std::string addDownload(const std::string& name, const std::string& url,
                            const std::string& filename = "");
    
    // Remove a download from queue (cancels if in progress)
    void removeDownload(const std::string& id);
    
    // Clear completed downloads from list
    void clearCompleted();
    
    // Get all downloads
    const std::vector<DownloadItem>& getDownloads() const { return m_downloads; }
    
    // Get a specific download by ID
    DownloadItem* getDownload(const std::string& id);
    
    // -------------------------------------------------------------------------
    // Download control
    // -------------------------------------------------------------------------
    
    // Pause a download
    void pause(const std::string& id);
    
    // Resume a paused download
    void resume(const std::string& id);
    
    // Pause all downloads
    void pauseAll();
    
    // Resume all paused downloads
    void resumeAll();
    
    // -------------------------------------------------------------------------
    // Processing
    // -------------------------------------------------------------------------
    
    // Process downloads (call each frame)
    void update();
    
    // Check if any download is active
    bool hasActiveDownload() const;
    
    // Get number of queued downloads
    size_t getQueuedCount() const;
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    void setOnProgress(DownloadProgressCallback callback) { m_onProgress = callback; }
    void setOnComplete(DownloadCompleteCallback callback) { m_onComplete = callback; }
    
    // -------------------------------------------------------------------------
    // Settings
    // -------------------------------------------------------------------------
    
    // Maximum concurrent downloads
    void setMaxConcurrent(int max) { m_maxConcurrent = max; }
    int getMaxConcurrent() const { return m_maxConcurrent; }
    
private:
    Downloader() = default;
    ~Downloader() = default;
    
    Downloader(const Downloader&) = delete;
    Downloader& operator=(const Downloader&) = delete;
    
    // Internal download processing
    void startNextDownload();
    void processActiveDownload();
    
    // Generate unique ID
    std::string generateId();
    
    // -------------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------------
    
    std::string m_downloadDir;
    std::vector<DownloadItem> m_downloads;
    std::unique_ptr<HttpClient> m_httpClient;
    
    int m_maxConcurrent = 1;
    int m_currentDownloads = 0;
    int m_nextId = 1;
    
    // Currently active download index
    int m_activeIndex = -1;
    
    // Callbacks
    DownloadProgressCallback m_onProgress;
    DownloadCompleteCallback m_onComplete;
    
    // Request to cancel current download
    bool m_cancelRequested = false;
};
