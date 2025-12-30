#include "TitleManager.hpp"
#include <SDL2/SDL_image.h>
#include <cstring>
#include <cstdio>

TitleManager& TitleManager::getInstance() {
    static TitleManager instance;
    return instance;
}

bool TitleManager::init() {
    if (m_initialized) return true;
    
    Result rc = nsInitialize();
    if (R_SUCCEEDED(rc)) {
        m_initialized = true;
        return true;
    }
    return false;
}

void TitleManager::exit() {
    if (m_initialized) {
        nsExit();
        m_initialized = false;
    }
}

std::vector<InstalledApp> TitleManager::getInstalledApps(SDL_Renderer* renderer) {
    std::vector<InstalledApp> apps;
    if (!m_initialized) return apps;
    
    const int MAX_TITLES = 512;
    NsApplicationRecord* records = new NsApplicationRecord[MAX_TITLES];
    s32 count = 0;
    
    // Get all application records
    Result rc = nsListApplicationRecord(records, MAX_TITLES, 0, &count);
    
    if (R_SUCCEEDED(rc)) {
        for (int i = 0; i < count; i++) {
            InstalledApp app;
            app.titleId = records[i].application_id;
            
            // Skip self? Maybe not needed.
            
            // Get control data (icon + nacp)
            NsApplicationControlData* controlData = new NsApplicationControlData;
            u64 outSize = 0; // Use u64 for libnx size parameter
            
            rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, 
                                             app.titleId, controlData, sizeof(NsApplicationControlData), &outSize);
            
            if (R_SUCCEEDED(rc)) {
                NacpLanguageEntry* langEntry = nullptr;
                rc = nacpGetLanguageEntry(&controlData->nacp, &langEntry);
                
                if (R_SUCCEEDED(rc) && langEntry) {
                    app.name = langEntry->name;
                    app.author = langEntry->author;
                } else {
                    char idBuf[32];
                    snprintf(idBuf, sizeof(idBuf), "%016lX", app.titleId);
                    app.name = std::string("Title: ") + idBuf;
                    app.author = "Unknown";
                }
                
                app.version = controlData->nacp.display_version;
                app.size_str = "Installed"; // Size calc is complex, skip for now
                
                // Load icon from JPEG data in controlData->icon
                if (renderer) {
                    SDL_RWops* rw = SDL_RWFromMem(controlData->icon, sizeof(controlData->icon));
                    if (rw) {
                        SDL_Surface* surface = IMG_Load_RW(rw, 1); // 1 = auto-close
                        if (surface) {
                            app.icon = SDL_CreateTextureFromSurface(renderer, surface);
                            SDL_FreeSurface(surface);
                        }
                    }
                }
            } else {
                continue; // Skip titles with invalid control data
            }
            
            delete controlData;
            apps.push_back(app);
        }
    }
    
    delete[] records;
    return apps;
}

SDL_Texture* TitleManager::getIcon(uint64_t titleId) {
    return nullptr; // Not implemented yet
}
