// =============================================================================
// Switch App Store - Screen Base Class
// =============================================================================
// Base class for all screens (pages) in the application
// =============================================================================

#pragma once

class App;
class Renderer;
class Input;

// =============================================================================
// Screen - Base class for all screens
// =============================================================================
class Screen {
public:
    // -------------------------------------------------------------------------
    // Constructor & Destructor
    // -------------------------------------------------------------------------
    explicit Screen(App* app) : m_app(app) {}
    virtual ~Screen() = default;
    
    // -------------------------------------------------------------------------
    // Lifecycle methods
    // -------------------------------------------------------------------------
    
    // Called when this screen becomes the active screen
    virtual void onEnter() {}
    
    // Called when this screen is about to be removed/hidden
    virtual void onExit() {}
    
    // Called when resolution changes (dock/undock)
    virtual void onResolutionChanged(int width, int height, float scale) {}
    
    // -------------------------------------------------------------------------
    // Update and render
    // -------------------------------------------------------------------------
    
    // Handle input events
    virtual void handleInput(const Input& input) = 0;
    
    // Update logic (animations, etc.)
    virtual void update(float deltaTime) = 0;
    
    // Render the screen
    virtual void render(Renderer& renderer) = 0;
    
protected:
    App* m_app;  // Reference to main app for accessing subsystems
};
