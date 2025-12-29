// =============================================================================
// Switch App Store - Main Entry Point
// =============================================================================
// A Nintendo Switch homebrew app store with Apple App Store style UI
// Built with SDL2 for graphics and libnx for Switch homebrew support
// =============================================================================

#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "app.hpp"

// =============================================================================
// Main entry point for the Switch homebrew application
// Initializes SDL2 subsystems, creates the app instance, and runs the main loop
// =============================================================================
int main(int argc, char* argv[]) {
    // -------------------------------------------------------------------------
    // Initialize Switch-specific services
    // -------------------------------------------------------------------------
    // Socket initialization is needed for network operations
    socketInitializeDefault();
    
    // Initialize romfs for bundled resources (fonts, icons, images)
    romfsInit();
    
    // -------------------------------------------------------------------------
    // Initialize SDL2 subsystems
    // -------------------------------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
        // SDL initialization failed - we can't continue
        return 1;
    }
    
    // Initialize SDL2_image for PNG/JPG loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL2_ttf for font rendering
    if (TTF_Init() < 0) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // -------------------------------------------------------------------------
    // Create and run the application
    // -------------------------------------------------------------------------
    {
        // Create app instance - this handles all the UI and logic
        App app;
        
        // Initialize the app (creates window, loads resources, etc.)
        if (app.init()) {
            // Run the main loop until user exits
            app.run();
        }
        
        // App destructor handles cleanup automatically
    }
    
    // -------------------------------------------------------------------------
    // Cleanup SDL2 subsystems
    // -------------------------------------------------------------------------
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    // -------------------------------------------------------------------------
    // Cleanup Switch-specific services
    // -------------------------------------------------------------------------
    romfsExit();
    socketExit();
    
    return 0;
}
