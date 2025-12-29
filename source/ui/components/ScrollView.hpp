// =============================================================================
// Switch App Store - ScrollView Component
// =============================================================================
// Vertical scrolling container with momentum scrolling and elastic bounce
// Clips children to its bounds
// =============================================================================

#pragma once

#include "ui/Component.hpp"

// =============================================================================
// ScrollView - Scrollable container
// =============================================================================
class ScrollView : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    ScrollView();
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void handleInput(const Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer, Theme& theme) override;
    
    // -------------------------------------------------------------------------
    // Scroll properties
    // -------------------------------------------------------------------------
    
    // Content height (total scrollable area)
    void setContentHeight(float height) { m_contentHeight = height; }
    float getContentHeight() const { return m_contentHeight; }
    
    // Current scroll position
    float getScrollY() const { return m_scrollY; }
    void setScrollY(float y);
    
    // Scroll to top/bottom
    void scrollToTop(bool animated = true);
    void scrollToBottom(bool animated = true);
    
    // Is currently scrolling?
    bool isScrolling() const { return m_scrollVelocity != 0.0f; }
    
    // Show/hide scroll indicator
    void setShowScrollIndicator(bool show) { m_showIndicator = show; }
    
    // Elastic overscroll (iOS bounce)
    void setElasticEnabled(bool enabled) { m_elasticEnabled = enabled; }
    
protected:
    // Scroll state
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    float m_contentHeight = 1000.0f;
    
    // Touch tracking
    bool m_touchTracking = false;
    float m_touchStartY = 0.0f;
    float m_lastTouchY = 0.0f;
    
    // Animation target for smooth scrolling
    float m_targetScrollY = 0.0f;
    bool m_animating = false;
    
    // Visual options
    bool m_showIndicator = true;
    bool m_elasticEnabled = true;
    
    // Physics constants
    static constexpr float FRICTION = 0.95f;
    static constexpr float MIN_VELOCITY = 0.5f;
    static constexpr float BOUNCE_FACTOR = 0.3f;
    static constexpr float ELASTIC_LIMIT = 100.0f;
    
private:
    // Calculate maximum scroll position
    float getMaxScrollY() const;
    
    // Apply bounds with elastic overscroll
    void applyBoundsElastic(float deltaTime);
    
    // Render scroll indicator
    void renderScrollIndicator(Renderer& renderer, Theme& theme);
};
