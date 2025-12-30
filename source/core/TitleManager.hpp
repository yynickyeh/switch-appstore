#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <switch.h>

struct InstalledApp {
    uint64_t titleId;
    std::string name;
    std::string author;
    std::string version;
    std::string size_str; // e.g. "1.2 GB"
    SDL_Texture* icon = nullptr; // Texture owner
};

class TitleManager {
public:
    static TitleManager& getInstance();
    
    // Initialize services
    bool init();
    void exit();
    
    // Get list of installed applications
    std::vector<InstalledApp> getInstalledApps(SDL_Renderer* renderer);
    
    // Get icon for a specific title ID (if not already loaded)
    SDL_Texture* getIcon(uint64_t titleId);

private:
    TitleManager() = default;
    ~TitleManager() = default;
    
    TitleManager(const TitleManager&) = delete;
    TitleManager& operator=(const TitleManager&) = delete;
    
    bool m_initialized = false;
};
