// =============================================================================
// Switch App Store - NRO Scanner
// =============================================================================
// Scans for .nro files on SD card and extracts metadata from NACP
// =============================================================================

#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>

// =============================================================================
// NRO Application Info
// =============================================================================
struct NroAppInfo {
    std::string path;           // Full path to .nro file
    std::string name;           // Application name from NACP
    std::string author;         // Author from NACP
    std::string version;        // Version from NACP
    std::string size_str;       // File size formatted
    size_t fileSize = 0;        // Raw file size
    SDL_Texture* icon = nullptr; // Loaded icon texture
};

// =============================================================================
// NroScanner - Scans for NRO files
// =============================================================================
class NroScanner {
public:
    static NroScanner& getInstance();
    
    // Scan directory for NRO files
    std::vector<NroAppInfo> scanDirectory(const std::string& path, SDL_Renderer* renderer);
    
    // Delete an NRO file
    bool deleteNro(const std::string& path);
    
private:
    NroScanner() = default;
    ~NroScanner() = default;
    
    NroScanner(const NroScanner&) = delete;
    NroScanner& operator=(const NroScanner&) = delete;
    
    // Parse NRO file to extract NACP and icon
    bool parseNroFile(const std::string& path, NroAppInfo& info, SDL_Renderer* renderer);
    
    // Format file size
    std::string formatFileSize(size_t bytes);
};
