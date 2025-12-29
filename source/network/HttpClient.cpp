// =============================================================================
// Switch App Store - HTTP Client Implementation
// =============================================================================

#include "HttpClient.hpp"
#include <curl/curl.h>
#include <cstdio>
#include <cstring>

// =============================================================================
// Static initialization
// =============================================================================

bool HttpClient::init() {
    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    return result == CURLE_OK;
}

void HttpClient::cleanup() {
    curl_global_cleanup();
}

// =============================================================================
// Constructor & Destructor
// =============================================================================

HttpClient::HttpClient() {
    m_curl = curl_easy_init();
}

HttpClient::~HttpClient() {
    if (m_curl) {
        curl_easy_cleanup(static_cast<CURL*>(m_curl));
        m_curl = nullptr;
    }
}

// =============================================================================
// Write Callbacks
// =============================================================================

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), realsize);
    return realsize;
}

size_t HttpClient::writeFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    FILE* file = static_cast<FILE*>(userp);
    return fwrite(contents, size, nmemb, file);
}

int HttpClient::progressCallback(void* clientp, double dltotal, double dlnow,
                                  double /* ultotal */, double /* ulnow */) {
    ProgressContext* ctx = static_cast<ProgressContext*>(clientp);
    if (ctx && ctx->callback && dltotal > 0) {
        ctx->callback(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
    }
    return 0;  // Return 0 to continue, non-zero to abort
}

// =============================================================================
// GET Request
// =============================================================================

HttpResponse HttpClient::get(const std::string& url, const HttpOptions& options) {
    HttpResponse response;
    
    if (!m_curl) {
        response.error = "CURL not initialized";
        return response;
    }
    
    CURL* curl = static_cast<CURL*>(m_curl);
    
    // Reset handle for reuse
    curl_easy_reset(curl);
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set write callback
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set options
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, options.timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, options.followRedirects ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, options.userAgent.c_str());
    
    // SSL options for Switch
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Set custom headers
    struct curl_slist* headerList = nullptr;
    for (const auto& header : options.headers) {
        std::string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Get response code
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    response.statusCode = static_cast<int>(httpCode);
    
    if (res != CURLE_OK) {
        response.error = curl_easy_strerror(res);
    } else {
        response.body = responseData;
    }
    
    // Cleanup headers
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    
    return response;
}

// =============================================================================
// POST Request
// =============================================================================

HttpResponse HttpClient::post(const std::string& url, const std::string& body,
                               const HttpOptions& options) {
    HttpResponse response;
    
    if (!m_curl) {
        response.error = "CURL not initialized";
        return response;
    }
    
    CURL* curl = static_cast<CURL*>(m_curl);
    
    // Reset handle
    curl_easy_reset(curl);
    
    // Set URL and POST
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());
    
    // Set write callback
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set options
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, options.timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, options.followRedirects ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, options.userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Set headers including Content-Type
    struct curl_slist* headerList = nullptr;
    headerList = curl_slist_append(headerList, 
                                    ("Content-Type: " + options.contentType).c_str());
    for (const auto& header : options.headers) {
        std::string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    response.statusCode = static_cast<int>(httpCode);
    
    if (res != CURLE_OK) {
        response.error = curl_easy_strerror(res);
    } else {
        response.body = responseData;
    }
    
    curl_slist_free_all(headerList);
    
    return response;
}

// =============================================================================
// Download File
// =============================================================================

bool HttpClient::downloadFile(const std::string& url, const std::string& outputPath,
                               ProgressCallback onProgress) {
    if (!m_curl) return false;
    
    CURL* curl = static_cast<CURL*>(m_curl);
    curl_easy_reset(curl);
    
    // Open output file
    FILE* file = fopen(outputPath.c_str(), "wb");
    if (!file) return false;
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    
    // Set progress callback
    ProgressContext ctx;
    ctx.callback = onProgress;
    if (onProgress) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &ctx);
    }
    
    // SSL and redirect options
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Perform download
    CURLcode res = curl_easy_perform(curl);
    
    fclose(file);
    
    if (res != CURLE_OK) {
        // Delete partial file on error
        remove(outputPath.c_str());
        return false;
    }
    
    return true;
}

// =============================================================================
// Download Data to Memory
// =============================================================================

std::vector<uint8_t> HttpClient::downloadData(const std::string& url,
                                               ProgressCallback onProgress) {
    std::vector<uint8_t> result;
    
    if (!m_curl) return result;
    
    CURL* curl = static_cast<CURL*>(m_curl);
    curl_easy_reset(curl);
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Use string buffer for simplicity
    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    
    // Progress callback
    ProgressContext ctx;
    ctx.callback = onProgress;
    if (onProgress) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &ctx);
    }
    
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        result.assign(buffer.begin(), buffer.end());
    }
    
    return result;
}

// =============================================================================
// JSON Helper (Simple implementation)
// =============================================================================

std::string HttpClient::getJsonValue(const std::string& json, const std::string& key) {
    // Very simple JSON value extraction (for basic use cases)
    // For production, use a proper JSON library
    
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";
    
    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";
    
    // Skip whitespace
    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && 
           (json[valueStart] == ' ' || json[valueStart] == '\t')) {
        valueStart++;
    }
    
    if (valueStart >= json.length()) return "";
    
    // Check value type
    if (json[valueStart] == '"') {
        // String value
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    } else if (json[valueStart] == '[' || json[valueStart] == '{') {
        // Array or object - find matching bracket
        // Simplified: just return empty for now
        return "";
    } else {
        // Number or boolean
        size_t valueEnd = json.find_first_of(",}]", valueStart);
        if (valueEnd == std::string::npos) return "";
        std::string value = json.substr(valueStart, valueEnd - valueStart);
        // Trim whitespace
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
            value.pop_back();
        }
        return value;
    }
}

// =============================================================================
// URL Encoding
// =============================================================================

std::string HttpClient::urlEncode(const std::string& str) {
    std::string result;
    result.reserve(str.length() * 3);
    
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else if (c == ' ') {
            result += '+';
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned char>(c));
            result += buf;
        }
    }
    
    return result;
}

std::string HttpClient::buildQueryString(const std::map<std::string, std::string>& params) {
    std::string result;
    bool first = true;
    
    for (const auto& param : params) {
        if (!first) result += '&';
        first = false;
        result += urlEncode(param.first) + '=' + urlEncode(param.second);
    }
    
    return result;
}
