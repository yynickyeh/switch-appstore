// =============================================================================
// Switch App Store - HorizontalList Implementation
// =============================================================================

#include "HorizontalList.hpp"
#include "ui/Theme.hpp"
#include <cmath>
#include <algorithm>

// =============================================================================
// Constructor
// =============================================================================

HorizontalList::HorizontalList() = default;

// =============================================================================
// Input Handling
// =============================================================================

void HorizontalList::handleInput(const Input& input) {
    const auto& touch = input.getTouch();
    
    // Handle touch scrolling
    if (touch.justTouched && containsPoint(touch.x, touch.y)) {
        m_touchTracking = true;
        m_touchStartX = touch.x;
        m_lastTouchX = touch.x;
        m_scrollVelocity = 0.0f;
        m_animating = false;
    }
    
    if (m_touchTracking && touch.touching) {
        float delta = m_lastTouchX - touch.x;
        m_scrollX += delta;
        m_scrollVelocity = delta;
        m_lastTouchX = touch.x;
    }
    
    if (touch.justReleased && m_touchTracking) {
        m_touchTracking = false;
        
        // If snap enabled, snap to nearest item
        if (m_snapEnabled) {
            int snapIndex = getSnapIndex();
            scrollToItem(snapIndex, true);
        }
    }
    
    // Handle L/R buttons for scrolling
    if (input.isPressed(Input::Button::L)) {
        m_scrollVelocity = -15.0f;
    }
    if (input.isPressed(Input::Button::R)) {
        m_scrollVelocity = 15.0f;
    }
    
    // Pass input to children
    for (auto& child : m_children) {
        if (child->isVisible()) {
            child->handleInput(input);
        }
    }
}

// =============================================================================
// Update
// =============================================================================

void HorizontalList::update(float deltaTime) {
    // Handle animated scrolling
    if (m_animating) {
        float diff = m_targetScrollX - m_scrollX;
        if (std::abs(diff) < 1.0f) {
            m_scrollX = m_targetScrollX;
            m_animating = false;
        } else {
            m_scrollX += diff * deltaTime * 10.0f;
        }
    } else if (!m_touchTracking) {
        // Apply momentum scrolling
        if (m_scrollVelocity != 0.0f) {
            m_scrollX += m_scrollVelocity;
            m_scrollVelocity *= FRICTION;
            
            if (std::abs(m_scrollVelocity) < MIN_VELOCITY) {
                m_scrollVelocity = 0.0f;
                
                // Snap when momentum ends
                if (m_snapEnabled) {
                    int snapIndex = getSnapIndex();
                    scrollToItem(snapIndex, true);
                }
            }
        }
    }
    
    // Clamp scroll position
    float maxScroll = getMaxScrollX();
    m_scrollX = std::clamp(m_scrollX, 0.0f, maxScroll);
    
    // Update children
    for (auto& child : m_children) {
        if (child->isVisible()) {
            child->update(deltaTime);
        }
    }
}

// =============================================================================
// Rendering
// =============================================================================

void HorizontalList::render(Renderer& renderer, Theme& theme) {
    if (!m_visible) return;
    
    // Set clip rect
    renderer.setClipRect(m_bounds);
    
    // Render children with scroll offset
    for (auto& child : m_children) {
        if (child->isVisible()) {
            // Offset child for scrolling
            float originalX = child->getX();
            child->setPosition(originalX + m_bounds.x + m_paddingLeft - m_scrollX, child->getY());
            
            // Only render if visible
            Rect childBounds = child->getBounds();
            if (childBounds.x + childBounds.w > m_bounds.x &&
                childBounds.x < m_bounds.x + m_bounds.w) {
                child->render(renderer, theme);
            }
            
            // Restore position
            child->setPosition(originalX, child->getY());
        }
    }
    
    // Clear clip rect
    renderer.clearClipRect();
}

// =============================================================================
// Scroll Control
// =============================================================================

void HorizontalList::setScrollX(float x) {
    m_scrollX = x;
    m_scrollVelocity = 0.0f;
    m_animating = false;
}

void HorizontalList::scrollToItem(int index, bool animated) {
    float targetX = index * (m_itemWidth + m_itemSpacing);
    targetX = std::clamp(targetX, 0.0f, getMaxScrollX());
    
    if (animated) {
        m_targetScrollX = targetX;
        m_animating = true;
        m_scrollVelocity = 0.0f;
    } else {
        setScrollX(targetX);
    }
}

float HorizontalList::getMaxScrollX() const {
    return std::max(0.0f, m_contentWidth - m_bounds.w + m_paddingLeft + m_paddingRight);
}

int HorizontalList::getSnapIndex() const {
    float itemStep = m_itemWidth + m_itemSpacing;
    return static_cast<int>((m_scrollX + itemStep / 2) / itemStep);
}
