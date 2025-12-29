// =============================================================================
// Switch App Store - ScrollView Implementation
// =============================================================================

#include "ScrollView.hpp"
#include "ui/Theme.hpp"
#include <cmath>
#include <algorithm>

// =============================================================================
// Constructor
// =============================================================================

ScrollView::ScrollView() = default;

// =============================================================================
// Input Handling
// =============================================================================

void ScrollView::handleInput(const Input& input) {
    const auto& touch = input.getTouch();
    
    // Handle touch scrolling
    if (touch.justTouched && containsPoint(touch.x, touch.y)) {
        m_touchTracking = true;
        m_touchStartY = touch.y;
        m_lastTouchY = touch.y;
        m_scrollVelocity = 0.0f;
        m_animating = false;
    }
    
    if (m_touchTracking && touch.touching) {
        float delta = m_lastTouchY - touch.y;
        m_scrollY += delta;
        m_scrollVelocity = delta;  // Store for momentum
        m_lastTouchY = touch.y;
    }
    
    if (touch.justReleased && m_touchTracking) {
        m_touchTracking = false;
        // Momentum continues with m_scrollVelocity
    }
    
    // Handle analog stick scrolling
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f && !m_touchTracking) {
        m_scrollVelocity = -stickY * 20.0f;
        m_animating = false;
    }
    
    // Handle D-pad for page scrolling
    if (input.isPressed(Input::Button::ZL)) {
        scrollToTop();
    }
    if (input.isPressed(Input::Button::ZR)) {
        scrollToBottom();
    }
    
    // Pass input to visible children
    for (auto& child : m_children) {
        if (child->isVisible()) {
            // Adjust touch coordinates for scroll offset
            // TODO: Properly transform input for children
            child->handleInput(input);
        }
    }
}

// =============================================================================
// Update
// =============================================================================

void ScrollView::update(float deltaTime) {
    // Handle animated scrolling (scrollTo)
    if (m_animating) {
        float diff = m_targetScrollY - m_scrollY;
        if (std::abs(diff) < 1.0f) {
            m_scrollY = m_targetScrollY;
            m_animating = false;
        } else {
            m_scrollY += diff * deltaTime * 10.0f;  // Ease out
        }
    } else if (!m_touchTracking) {
        // Apply momentum scrolling
        if (m_scrollVelocity != 0.0f) {
            m_scrollY += m_scrollVelocity;
            m_scrollVelocity *= FRICTION;
            
            if (std::abs(m_scrollVelocity) < MIN_VELOCITY) {
                m_scrollVelocity = 0.0f;
            }
        }
    }
    
    // Apply bounds with elastic effect
    applyBoundsElastic(deltaTime);
    
    // Update children with scroll offset
    for (auto& child : m_children) {
        if (child->isVisible()) {
            child->update(deltaTime);
        }
    }
}

void ScrollView::applyBoundsElastic(float deltaTime) {
    float maxScroll = getMaxScrollY();
    
    if (m_elasticEnabled) {
        // Allow overscroll with rubber-band effect
        if (m_scrollY < 0) {
            if (!m_touchTracking) {
                // Bounce back
                m_scrollY *= (1.0f - deltaTime * 5.0f);
                if (m_scrollY > -0.5f) m_scrollY = 0.0f;
            } else {
                // Rubber-band resistance
                m_scrollY = -ELASTIC_LIMIT * (1.0f - std::exp(m_scrollY / ELASTIC_LIMIT));
            }
        } else if (m_scrollY > maxScroll && maxScroll > 0) {
            float over = m_scrollY - maxScroll;
            if (!m_touchTracking) {
                // Bounce back
                m_scrollY = maxScroll + over * (1.0f - deltaTime * 5.0f);
                if (over < 0.5f) m_scrollY = maxScroll;
            } else {
                // Rubber-band resistance
                m_scrollY = maxScroll + ELASTIC_LIMIT * (1.0f - std::exp(-over / ELASTIC_LIMIT));
            }
        }
    } else {
        // Hard clamp
        m_scrollY = std::clamp(m_scrollY, 0.0f, maxScroll);
    }
}

float ScrollView::getMaxScrollY() const {
    return std::max(0.0f, m_contentHeight - m_bounds.h);
}

// =============================================================================
// Rendering
// =============================================================================

void ScrollView::render(Renderer& renderer, Theme& theme) {
    if (!m_visible) return;
    
    // Set clip rect to scroll view bounds
    renderer.setClipRect(m_bounds);
    
    // Render children with scroll offset
    for (auto& child : m_children) {
        if (child->isVisible()) {
            // Temporarily offset child for scrolling
            float originalY = child->getY();
            child->setPosition(child->getX(), originalY - m_scrollY);
            
            // Only render if visible within scroll view
            Rect childBounds = child->getBounds();
            if (childBounds.y + childBounds.h > m_bounds.y &&
                childBounds.y < m_bounds.y + m_bounds.h) {
                child->render(renderer, theme);
            }
            
            // Restore original position
            child->setPosition(child->getX(), originalY);
        }
    }
    
    // Clear clip rect
    renderer.clearClipRect();
    
    // Render scroll indicator
    if (m_showIndicator && m_contentHeight > m_bounds.h) {
        renderScrollIndicator(renderer, theme);
    }
}

void ScrollView::renderScrollIndicator(Renderer& renderer, Theme& theme) {
    float maxScroll = getMaxScrollY();
    if (maxScroll <= 0) return;
    
    // Calculate indicator size and position
    float viewRatio = m_bounds.h / m_contentHeight;
    float indicatorHeight = std::max(30.0f, m_bounds.h * viewRatio);
    
    float scrollRatio = m_scrollY / maxScroll;
    scrollRatio = std::clamp(scrollRatio, 0.0f, 1.0f);
    
    float indicatorY = m_bounds.y + scrollRatio * (m_bounds.h - indicatorHeight);
    float indicatorX = m_bounds.x + m_bounds.w - 6;
    
    // Draw indicator
    Color indicatorColor = theme.textSecondaryColor();
    indicatorColor.a = 100;
    
    renderer.drawRoundedRect(
        Rect(indicatorX, indicatorY, 4, indicatorHeight),
        2, indicatorColor
    );
}

// =============================================================================
// Scroll Control
// =============================================================================

void ScrollView::setScrollY(float y) {
    m_scrollY = y;
    m_scrollVelocity = 0.0f;
    m_animating = false;
}

void ScrollView::scrollToTop(bool animated) {
    if (animated) {
        m_targetScrollY = 0.0f;
        m_animating = true;
        m_scrollVelocity = 0.0f;
    } else {
        setScrollY(0.0f);
    }
}

void ScrollView::scrollToBottom(bool animated) {
    float maxScroll = getMaxScrollY();
    if (animated) {
        m_targetScrollY = maxScroll;
        m_animating = true;
        m_scrollVelocity = 0.0f;
    } else {
        setScrollY(maxScroll);
    }
}
