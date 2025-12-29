// =============================================================================
// Switch App Store - NavigationBar Component
// =============================================================================
// Top navigation bar with title, back button, and optional right items
// =============================================================================

#pragma once

#include "ui/Component.hpp"
#include <string>

// =============================================================================
// NavigationBar - Top navigation bar
// =============================================================================
class NavigationBar : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    NavigationBar();
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void handleInput(const Input& input) override;
    void render(Renderer& renderer, Theme& theme) override;
    
    // -------------------------------------------------------------------------
    // Navigation bar properties
    // -------------------------------------------------------------------------
    
    // Title
    void setTitle(const std::string& title) { m_title = title; }
    const std::string& getTitle() const { return m_title; }
    
    // Large title mode (like iOS)
    void setLargeTitle(bool large) { m_largeTitle = large; }
    bool isLargeTitle() const { return m_largeTitle; }
    
    // Back button
    void setShowBackButton(bool show) { m_showBackButton = show; }
    bool isBackButtonVisible() const { return m_showBackButton; }
    void setBackButtonTitle(const std::string& title) { m_backButtonTitle = title; }
    
    // Callbacks
    void setOnBack(Callback callback) { m_onBack = callback; }
    
    // Background opacity (for scroll-based transparency)
    void setBackgroundOpacity(float opacity) { m_backgroundOpacity = opacity; }
    
private:
    std::string m_title = "Title";
    std::string m_backButtonTitle = "返回";
    bool m_largeTitle = false;
    bool m_showBackButton = false;
    float m_backgroundOpacity = 1.0f;
    
    Callback m_onBack;
    
    // Animation
    bool m_backButtonPressed = false;
};
