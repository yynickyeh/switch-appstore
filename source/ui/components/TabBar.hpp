// =============================================================================
// Switch App Store - TabBar Component
// =============================================================================
// Bottom tab navigation bar (like iOS App Store)
// Shows icons with labels for main app sections
// =============================================================================

#pragma once

#include "ui/Component.hpp"
#include <vector>
#include <string>

// =============================================================================
// Tab item data
// =============================================================================
struct TabItem {
    std::string label;
    // SDL_Texture* icon = nullptr;
    // SDL_Texture* iconSelected = nullptr;
};

// =============================================================================
// TabBar - Bottom navigation bar
// =============================================================================
class TabBar : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    TabBar();
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void handleInput(const Input& input) override;
    void render(Renderer& renderer, Theme& theme) override;
    
    // TabBar can receive focus for controller navigation
    bool canFocus() const override { return true; }
    
    // -------------------------------------------------------------------------
    // Tab management
    // -------------------------------------------------------------------------
    
    // Add a tab
    void addTab(const std::string& label);
    
    // Get/set selected tab
    int getSelectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);
    
    // Tab count
    int getTabCount() const { return static_cast<int>(m_tabs.size()); }
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    using TabCallback = std::function<void(int)>;
    void setOnTabChanged(TabCallback callback) { m_onTabChanged = callback; }
    
    // -------------------------------------------------------------------------
    // Styling
    // -------------------------------------------------------------------------
    
    // Height (default 70 for 720p)
    void setHeight(float height) { m_bounds.h = height; }
    
    // Icon size (default 28 for 720p)
    void setIconSize(float size) { m_iconSize = size; }
    
    // Font size for labels
    void setFontSize(int size) { m_fontSize = size; }
    
private:
    std::vector<TabItem> m_tabs;
    int m_selectedIndex = 0;
    
    // Styling
    float m_iconSize = 28.0f;
    int m_fontSize = 12;
    
    // Callbacks
    TabCallback m_onTabChanged;
    
    // Animation
    float m_transitionProgress = 0.0f;
    int m_previousIndex = 0;
};
