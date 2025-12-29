// =============================================================================
// Switch App Store - Router (Page Navigation)
// =============================================================================
// Manages screen navigation with animated transitions
// =============================================================================

#pragma once

#include <memory>
#include <vector>
#include <functional>

// Forward declarations
class Screen;
class Renderer;
class Input;
class App;
class TabBar;

// =============================================================================
// Router - Screen navigation and transition manager
// =============================================================================
class Router {
public:
    Router();
    ~Router();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    
    // Initialize the router with a reference to the app
    void init(App* app);
    
    // -------------------------------------------------------------------------
    // Screen management
    // -------------------------------------------------------------------------
    
    // Set the root screen (replaces all screens)
    void setRootScreen(std::unique_ptr<Screen> screen);
    
    // Push a new screen onto the stack (with transition animation)
    void push(std::unique_ptr<Screen> screen);
    
    // Pop the current screen (go back)
    void pop();
    
    // Pop to the root screen
    void popToRoot();
    
    // Get the current (topmost) screen
    Screen* currentScreen() const;
    
    // Get screen count
    size_t screenCount() const { return m_screens.size(); }
    
    // -------------------------------------------------------------------------
    // Tab navigation (for bottom tab bar)
    // -------------------------------------------------------------------------
    
    // Switch to a different tab
    void switchTab(int tabIndex);
    
    // Add a tab screen
    void addTabScreen(std::unique_ptr<Screen> screen);
    
    // Get current tab index
    int getCurrentTab() const { return m_currentTab; }
    
    // Get the TabBar component
    TabBar* getTabBar() const { return m_tabBar.get(); }
    
    // -------------------------------------------------------------------------
    // Update and render
    // -------------------------------------------------------------------------
    
    void handleInput(const Input& input);
    void update(float deltaTime);
    void render(Renderer& renderer);
    
    // -------------------------------------------------------------------------
    // Resolution change callback
    // -------------------------------------------------------------------------
    
    void onResolutionChanged(int width, int height, float scale);
    
private:
    // App reference
    App* m_app = nullptr;
    
    // Screen stack
    std::vector<std::unique_ptr<Screen>> m_screens;
    
    // Tab navigation
    int m_currentTab = 0;
    std::vector<std::unique_ptr<Screen>> m_tabScreens;
    
    // TabBar component (owned by Router)
    std::unique_ptr<TabBar> m_tabBar;
    
    // Transition animation state
    bool m_transitioning = false;
    float m_transitionProgress = 0.0f;
    float m_transitionDuration = 0.3f;
    bool m_transitionIsPush = true;
    
    // Pending screen for transitions
    std::unique_ptr<Screen> m_pendingScreen;
};

