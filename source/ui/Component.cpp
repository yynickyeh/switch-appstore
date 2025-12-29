// =============================================================================
// Switch App Store - Component Implementation
// =============================================================================

#include "Component.hpp"

// =============================================================================
// Constructor
// =============================================================================

Component::Component() = default;

// =============================================================================
// Core Lifecycle
// =============================================================================

void Component::handleInput(const Input& input) {
    // Handle input for children (in reverse order for proper z-ordering)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->isVisible() && (*it)->isEnabled()) {
            (*it)->handleInput(input);
        }
    }
    
    // Handle touch input
    const auto& touch = input.getTouch();
    
    if (touch.justTouched && hitTest(touch.x, touch.y)) {
        m_isPressed = true;
        m_pressAnimProgress = 0.0f;
    }
    
    if (touch.justReleased) {
        if (m_isPressed && hitTest(touch.x, touch.y)) {
            fireTap();
        }
        m_isPressed = false;
    }
    
    // Handle A button for focused component
    if (m_focused && m_enabled) {
        if (input.isPressed(Input::Button::A)) {
            fireTap();
        }
    }
}

void Component::update(float deltaTime) {
    // Update press animation
    if (m_isPressed) {
        m_pressAnimProgress += deltaTime * 10.0f;
        if (m_pressAnimProgress > 1.0f) m_pressAnimProgress = 1.0f;
        m_scale = 1.0f - 0.03f * m_pressAnimProgress;  // Scale down to 97%
    } else {
        if (m_scale < 1.0f) {
            m_scale += deltaTime * 5.0f;
            if (m_scale > 1.0f) m_scale = 1.0f;
        }
    }
    
    // Update children
    updateChildren(deltaTime);
}

void Component::updateChildren(float deltaTime) {
    for (auto& child : m_children) {
        if (child->isVisible()) {
            child->update(deltaTime);
        }
    }
}

void Component::renderChildren(Renderer& renderer, Theme& theme) {
    for (auto& child : m_children) {
        if (child->isVisible()) {
            child->render(renderer, theme);
        }
    }
}

// =============================================================================
// Bounds Management
// =============================================================================

void Component::setPosition(float x, float y) {
    m_bounds.x = x;
    m_bounds.y = y;
}

void Component::setSize(float width, float height) {
    m_bounds.w = width;
    m_bounds.h = height;
}

void Component::setBounds(float x, float y, float width, float height) {
    m_bounds.x = x;
    m_bounds.y = y;
    m_bounds.w = width;
    m_bounds.h = height;
}

void Component::setBounds(const Rect& bounds) {
    m_bounds = bounds;
}

bool Component::containsPoint(float x, float y) const {
    Rect screenBounds = getScreenBounds();
    return screenBounds.contains(x, y);
}

Rect Component::getScreenBounds() const {
    Rect result = m_bounds;
    
    // Add parent offset
    Component* parent = m_parent;
    while (parent) {
        result.x += parent->getX();
        result.y += parent->getY();
        parent = parent->getParent();
    }
    
    return result;
}

// =============================================================================
// Focus Management
// =============================================================================

void Component::setFocused(bool focused) {
    if (m_focused != focused) {
        m_focused = focused;
        if (focused && m_onFocus) {
            m_onFocus();
        } else if (!focused && m_onBlur) {
            m_onBlur();
        }
    }
}

// =============================================================================
// Children Management
// =============================================================================

void Component::addChild(std::unique_ptr<Component> child) {
    child->setParent(this);
    m_children.push_back(std::move(child));
}

void Component::removeChild(Component* child) {
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == child) {
            (*it)->setParent(nullptr);
            m_children.erase(it);
            return;
        }
    }
}

// =============================================================================
// Events
// =============================================================================

void Component::fireTap() {
    if (m_onTap) {
        m_onTap();
    }
}
