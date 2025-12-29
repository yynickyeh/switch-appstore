// =============================================================================
// Switch App Store - HTTP Client
// =============================================================================
// HTTP client wrapper using libcurl for making network requests
// Supports GET, POST, downloads with progress, and JSON parsing
// =============================================================================

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <memory>

// =============================================================================
// HTTP Response structure
// =============================================================================
struct HttpResponse {
    int statusCode = 0;
    std::string body;
    std::map<std::string, std::string> headers;
    std::string error;
    
    bool isSuccess() const { return statusCode >= 200 && statusCode < 300; }
    bool isError() const { return statusCode >= 400 || !error.empty(); }
};

// =============================================================================
// HTTP Request Options
// =============================================================================
struct HttpOptions {
    std::map<std::string, std::string> headers;
    int timeoutSeconds = 30;
    bool followRedirects = true;
    std::string userAgent = "SwitchAppStore/1.0";
    
    // For POST requests
    std::string contentType = "application/json";
};

// =============================================================================
// Progress callback for downloads
// =============================================================================
using ProgressCallback = std::function<void(size_t downloaded, size_t total)>;
using CompletionCallback = std::function<void(const HttpResponse&)>;

// =============================================================================
// HttpClient - Main HTTP client class
// =============================================================================
class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    // Initialize curl (call once at app startup)
    static bool init();
    
    // Cleanup curl (call once at app shutdown)
    static void cleanup();
    
    // -------------------------------------------------------------------------
    // Synchronous requests
    // -------------------------------------------------------------------------
    
    // Perform a GET request
    HttpResponse get(const std::string& url, const HttpOptions& options = {});
    
    // Perform a POST request
    HttpResponse post(const std::string& url, const std::string& body,
                      const HttpOptions& options = {});
    
    // Download a file to disk
    bool downloadFile(const std::string& url, const std::string& outputPath,
                      ProgressCallback onProgress = nullptr);
    
    // Download data to memory
    std::vector<uint8_t> downloadData(const std::string& url,
                                       ProgressCallback onProgress = nullptr);
    
    // -------------------------------------------------------------------------
    // JSON helpers
    // -------------------------------------------------------------------------
    
    // Parse JSON from response (simplified, returns empty string on error)
    // For real implementation, use nlohmann/json or similar
    static std::string getJsonValue(const std::string& json, const std::string& key);
    
    // -------------------------------------------------------------------------
    // Utilities
    // -------------------------------------------------------------------------
    
    // URL encode a string
    static std::string urlEncode(const std::string& str);
    
    // Build query string from parameters
    static std::string buildQueryString(const std::map<std::string, std::string>& params);
    
private:
    // CURL handle (reused for connection pooling)
    void* m_curl = nullptr;
    
    // For write callbacks
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t writeFileCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int progressCallback(void* clientp, double dltotal, double dlnow,
                                double ultotal, double ulnow);
    
    // Progress callback context
    struct ProgressContext {
        ProgressCallback callback;
    };
};
