// =============================================================================
// Switch App Store - App Header
// =============================================================================
// Main application class that manages the entire app lifecycle
// Handles window creation, rendering, input, and screen navigation
// =============================================================================

#pragma once

#include <SDL2/SDL.h>
#include <memory>

// Forward declarations
class Router;
class Renderer;
class Input;
class Theme;

// =============================================================================
// App - Main application controller
// =============================================================================
// This class is the heart of the application. It creates and manages:
// - The SDL window and renderer
// - The rendering subsystem (Renderer)
// - The input handling subsystem (Input)
// - The screen navigation system (Router)
// - The theming system (Theme)
// =============================================================================
class App {
public:
    // -------------------------------------------------------------------------
    // Constructor & Destructor
    // -------------------------------------------------------------------------
    App();
    ~App();
    
    // -------------------------------------------------------------------------
    // Lifecycle methods
    // -------------------------------------------------------------------------
    
    // Initialize the application - creates window, loads resources
    // Returns true on success, false on failure
    bool init();
    
    // Run the main application loop
    // This method blocks until the user exits the app
    void run();
    
    // -------------------------------------------------------------------------
    // Accessors for subsystems (used by screens and components)
    // -------------------------------------------------------------------------
    Renderer* getRenderer() const { return m_renderer.get(); }
    Input* getInput() const { return m_input.get(); }
    Router* getRouter() const { return m_router.get(); }
    Theme* getTheme() const { return m_theme.get(); }
    
    // Get the current scale factor (1.0 = 720p, 1.5 = 1080p)
    float getScale() const { return m_scale; }
    
    // Check if running in docked mode (1080p) or handheld (720p)
    bool isDocked() const { return m_isDocked; }
    
private:
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Process all pending input events
    void handleEvents();
    
    // Update game logic (animations, transitions, etc.)
    void update(float deltaTime);
    
    // Render the current frame
    void render();
    
    // Check and handle resolution changes (dock/undock)
    void checkResolutionChange();
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    // SDL resources
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_sdlRenderer = nullptr;
    
    // Subsystems (using unique_ptr for automatic cleanup)
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<Router> m_router;
    std::unique_ptr<Theme> m_theme;
    
    // State
    bool m_running = false;           // Is the main loop running?
    bool m_isDocked = false;          // Is the Switch docked (1080p)?
    float m_scale = 1.0f;             // Scale factor (1.0 = 720p, 1.5 = 1080p)
    
    // Timing
    Uint64 m_lastFrameTime = 0;       // For delta time calculation
    
    // Window dimensions
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
};
