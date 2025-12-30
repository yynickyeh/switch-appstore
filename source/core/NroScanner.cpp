// =============================================================================
// Switch App Store - NRO Scanner Implementation
// =============================================================================

#include "NroScanner.hpp"
#include <SDL2/SDL_image.h>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <switch.h>

// NRO file format structures (renamed to avoid libnx conflicts)
// Based on switchbrew documentation

#pragma pack(push, 1)

// NRO Start header
struct LocalNroStart {
    uint32_t unused;
    uint32_t mod0Offset;
    uint8_t padding[8];
};

// NRO Header
struct LocalNroHeader {
    uint32_t magic;  // "NRO0"
    uint32_t version;
    uint32_t size;
    uint32_t flags;
    uint32_t textOffset;
    uint32_t textSize;
    uint32_t roOffset;
    uint32_t roSize;
    uint32_t dataOffset;
    uint32_t dataSize;
    uint32_t bssSize;
    uint32_t reserved1;
    uint8_t buildId[0x20];
    uint8_t reserved2[0x20];
};

// NRO Asset Header (ASET)
struct LocalNroAssetHeader {
    uint32_t magic;  // "ASET"
    uint32_t version;
    uint64_t iconOffset;
    uint64_t iconSize;
    uint64_t nacpOffset;
    uint64_t nacpSize;
    uint64_t romfsOffset;
    uint64_t romfsSize;
};

#pragma pack(pop)

// =============================================================================
// Singleton
// =============================================================================

NroScanner& NroScanner::getInstance() {
    static NroScanner instance;
    return instance;
}

// =============================================================================
// Directory Scanning
// =============================================================================

std::vector<NroAppInfo> NroScanner::scanDirectory(const std::string& path, SDL_Renderer* renderer) {
    std::vector<NroAppInfo> apps;
    
    DIR* dir = opendir(path.c_str());
    if (!dir) return apps;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // Check for .nro extension
        if (filename.length() > 4) {
            std::string ext = filename.substr(filename.length() - 4);
            if (ext == ".nro" || ext == ".NRO") {
                std::string fullPath = path + "/" + filename;
                
                NroAppInfo info;
                info.path = fullPath;
                
                // Try to parse NRO for metadata
                if (!parseNroFile(fullPath, info, renderer)) {
                    // If parsing fails, use filename as name
                    info.name = filename.substr(0, filename.length() - 4);
                    info.author = "Unknown";
                    info.version = "";
                }
                
                // Get file size
                struct stat st;
                if (stat(fullPath.c_str(), &st) == 0) {
                    info.fileSize = st.st_size;
                    info.size_str = formatFileSize(st.st_size);
                }
                
                apps.push_back(info);
            }
        }
    }
    
    closedir(dir);
    return apps;
}

// =============================================================================
// NRO Parsing
// =============================================================================

bool NroScanner::parseNroFile(const std::string& path, NroAppInfo& info, SDL_Renderer* renderer) {
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) return false;
    
    // Read NRO start
    LocalNroStart start;
    if (fread(&start, sizeof(start), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Read NRO header
    LocalNroHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Check magic
    if (header.magic != 0x304F524E) { // "NRO0" little-endian
        fclose(file);
        return false;
    }
    
    // Seek to end of NRO to find ASET section
    uint32_t nroSize = header.size;
    fseek(file, nroSize, SEEK_SET);
    
    // Read ASET header
    LocalNroAssetHeader aset;
    if (fread(&aset, sizeof(aset), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Check ASET magic
    if (aset.magic != 0x54455341) { // "ASET" little-endian
        fclose(file);
        return false;
    }
    
    // Read NACP if available
    if (aset.nacpSize > 0) {
        fseek(file, nroSize + aset.nacpOffset, SEEK_SET);
        
        // NACP is a fixed-size structure
        NacpStruct nacp;
        if (fread(&nacp, sizeof(nacp), 1, file) == 1) {
            // Get language entry (use first non-empty)
            for (int i = 0; i < 16; i++) {
                if (nacp.lang[i].name[0] != '\0') {
                    info.name = nacp.lang[i].name;
                    info.author = nacp.lang[i].author;
                    break;
                }
            }
            info.version = nacp.display_version;
        }
    }
    
    // Read icon if available
    if (aset.iconSize > 0 && renderer) {
        fseek(file, nroSize + aset.iconOffset, SEEK_SET);
        
        // Allocate buffer for icon (JPEG data)
        uint8_t* iconData = new uint8_t[aset.iconSize];
        if (fread(iconData, 1, aset.iconSize, file) == aset.iconSize) {
            SDL_RWops* rw = SDL_RWFromMem(iconData, aset.iconSize);
            if (rw) {
                SDL_Surface* surface = IMG_Load_RW(rw, 1);
                if (surface) {
                    info.icon = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                }
            }
        }
        delete[] iconData;
    }
    
    fclose(file);
    return !info.name.empty();
}

// =============================================================================
// Delete NRO
// =============================================================================

bool NroScanner::deleteNro(const std::string& path) {
    return (remove(path.c_str()) == 0);
}

// =============================================================================
// Utilities
// =============================================================================

std::string NroScanner::formatFileSize(size_t bytes) {
    char buf[32];
    if (bytes >= 1024ULL * 1024 * 1024) {
        snprintf(buf, sizeof(buf), "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    } else if (bytes >= 1024 * 1024) {
        snprintf(buf, sizeof(buf), "%.1f MB", bytes / (1024.0 * 1024.0));
    } else if (bytes >= 1024) {
        snprintf(buf, sizeof(buf), "%.1f KB", bytes / 1024.0);
    } else {
        snprintf(buf, sizeof(buf), "%zu B", bytes);
    }
    return std::string(buf);
}
