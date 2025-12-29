// =============================================================================
// Switch App Store - HorizontalList Component
// =============================================================================
// Horizontally scrolling list (like App Store game rows)
// Supports momentum scrolling and snap-to-item
// =============================================================================

#pragma once

#include "ui/Component.hpp"
#include <vector>

// =============================================================================
// HorizontalList - Horizontal scrolling container
// =============================================================================
class HorizontalList : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    HorizontalList();
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void handleInput(const Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer, Theme& theme) override;
    
    // -------------------------------------------------------------------------
    // Horizontal list properties
    // -------------------------------------------------------------------------
    
    // Item sizing
    void setItemWidth(float width) { m_itemWidth = width; }
    void setItemSpacing(float spacing) { m_itemSpacing = spacing; }
    
    // Content width (or calculate from items)
    void setContentWidth(float width) { m_contentWidth = width; }
    float getContentWidth() const { return m_contentWidth; }
    
    // Current scroll position
    float getScrollX() const { return m_scrollX; }
    void setScrollX(float x);
    
    // Scroll to a specific item index
    void scrollToItem(int index, bool animated = true);
    
    // Enable snap-to-item behavior
    void setSnapEnabled(bool enabled) { m_snapEnabled = enabled; }
    
    // Left/Right padding
    void setPadding(float left, float right) { 
        m_paddingLeft = left; 
        m_paddingRight = right; 
    }
    
private:
    // Scroll state
    float m_scrollX = 0.0f;
    float m_scrollVelocity = 0.0f;
    float m_contentWidth = 1000.0f;
    
    // Touch tracking
    bool m_touchTracking = false;
    float m_touchStartX = 0.0f;
    float m_lastTouchX = 0.0f;
    
    // Animation
    float m_targetScrollX = 0.0f;
    bool m_animating = false;
    
    // Item properties
    float m_itemWidth = 180.0f;
    float m_itemSpacing = 16.0f;
    bool m_snapEnabled = false;
    float m_paddingLeft = 0.0f;
    float m_paddingRight = 0.0f;
    
    // Physics
    static constexpr float FRICTION = 0.95f;
    static constexpr float MIN_VELOCITY = 0.5f;
    
    float getMaxScrollX() const;
    int getSnapIndex() const;
};
