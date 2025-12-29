// =============================================================================
// Switch App Store - Card Component
// =============================================================================
// iOS-style card container with shadow, rounded corners, and optional tap handling
// Used for game cards, featured cards, and other content containers
// =============================================================================

#pragma once

#include "ui/Component.hpp"

// =============================================================================
// Card - Container component with styling
// =============================================================================
class Card : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    Card();
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void render(Renderer& renderer, Theme& theme) override;
    
    // -------------------------------------------------------------------------
    // Card-specific properties
    // -------------------------------------------------------------------------
    
    // Corner radius
    void setCornerRadius(int radius) { m_cornerRadius = radius; }
    int getCornerRadius() const { return m_cornerRadius; }
    
    // Shadow
    void setShadowEnabled(bool enabled) { m_shadowEnabled = enabled; }
    bool isShadowEnabled() const { return m_shadowEnabled; }
    
    void setShadowOffset(int offsetX, int offsetY) { 
        m_shadowOffsetX = offsetX; 
        m_shadowOffsetY = offsetY; 
    }
    
    void setShadowBlur(int blur) { m_shadowBlur = blur; }
    
    // Background color (if not set, uses theme's card_bg color)
    void setBackgroundColor(const Color& color) { 
        m_bgColor = color; 
        m_useCustomBgColor = true; 
    }
    
    // Optional background gradient
    void setGradient(const Color& topColor, const Color& bottomColor) {
        m_gradientTop = topColor;
        m_gradientBottom = bottomColor;
        m_hasGradient = true;
    }
    void clearGradient() { m_hasGradient = false; }
    
    // Border
    void setBorderColor(const Color& color) { 
        m_borderColor = color; 
        m_hasBorder = true; 
    }
    void setBorderWidth(int width) { m_borderWidth = width; }
    
private:
    int m_cornerRadius = 16;
    
    // Shadow
    bool m_shadowEnabled = true;
    int m_shadowOffsetX = 0;
    int m_shadowOffsetY = 4;
    int m_shadowBlur = 12;
    
    // Background
    Color m_bgColor;
    bool m_useCustomBgColor = false;
    
    // Gradient
    bool m_hasGradient = false;
    Color m_gradientTop;
    Color m_gradientBottom;
    
    // Border
    bool m_hasBorder = false;
    Color m_borderColor;
    int m_borderWidth = 1;
};
