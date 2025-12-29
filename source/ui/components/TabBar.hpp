// =============================================================================
// Switch App Store - TabBar Component
// =============================================================================
// iOS-style bottom navigation bar with proper encapsulation
// Designed to be rendered at the App/Router level, independent of Screens
// =============================================================================

#pragma once

#include "core/Renderer.hpp"
#include <string>
#include <vector>
#include <functional>

// Forward declarations
class App;
class Input;

// =============================================================================
// TabItem - Single tab entry
// =============================================================================
struct TabItem {
    std::string id;
    std::string label;
    std::string iconName;  // For future icon support
};

// =============================================================================
// TabBar - Bottom navigation component
// =============================================================================
// This component follows Apple Human Interface Guidelines:
// - Fixed at bottom of screen
// - 5 tabs max (iOS standard)
// - Selected state clearly indicated
// - Touch targets at least 44x44 points
// =============================================================================
class TabBar {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    explicit TabBar(App* app);
    ~TabBar() = default;
    
    // -------------------------------------------------------------------------
    // Tab management
    // -------------------------------------------------------------------------
    
    // Add a tab (call during initialization)
    void addTab(const std::string& id, const std::string& label, 
                const std::string& iconName = "");
    
    // Get/set selected tab index
    int getSelectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);
    
    // Get tab count
    size_t getTabCount() const { return m_tabs.size(); }
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    using TabChangeCallback = std::function<void(int oldIndex, int newIndex)>;
    void setOnTabChange(TabChangeCallback callback) { m_onTabChange = callback; }
    
    // -------------------------------------------------------------------------
    // Input handling
    // -------------------------------------------------------------------------
    
    // Handle input for tab switching
    // Returns true if input was consumed
    bool handleInput(const Input& input);
    
    // -------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------
    
    // Render the tab bar
    // Should be called AFTER screen rendering to appear on top
    void render(Renderer& renderer);
    
    // -------------------------------------------------------------------------
    // Layout constants (based on iOS HIG)
    // -------------------------------------------------------------------------
    static constexpr float HEIGHT = 70.0f;           // Tab bar height
    static constexpr float SAFE_AREA_BOTTOM = 0.0f;  // Safe area for notch devices
    
    // Get the usable screen height (accounting for tab bar)
    static float getContentHeight(float screenHeight) {
        return screenHeight - HEIGHT - SAFE_AREA_BOTTOM;
    }
    
private:
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    void renderBackground(Renderer& renderer);
    void renderTabs(Renderer& renderer);
    void renderTab(Renderer& renderer, const TabItem& tab, float x, float y,
                   float width, bool isSelected);
    
    // -------------------------------------------------------------------------
    // Touch handling
    // -------------------------------------------------------------------------
    
    int hitTestTabs(float touchX, float touchY) const;
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    App* m_app;
    std::vector<TabItem> m_tabs;
    int m_selectedIndex = 0;
    
    // Animation
    float m_selectionAnimProgress = 0.0f;
    int m_previousIndex = 0;
    
    // Callbacks
    TabChangeCallback m_onTabChange;
    
    // Layout cache
    float m_screenWidth = 1280.0f;
    float m_screenHeight = 720.0f;
};
