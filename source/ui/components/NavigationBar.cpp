// =============================================================================
// Switch App Store - NavigationBar Implementation
// =============================================================================

#include "NavigationBar.hpp"
#include "ui/Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

NavigationBar::NavigationBar() {
    m_bounds.h = 44.0f;  // Standard height
}

// =============================================================================
// Input Handling
// =============================================================================

void NavigationBar::handleInput(const Input& input) {
    // Handle B button as back
    if (m_showBackButton && input.isPressed(Input::Button::B)) {
        if (m_onBack) {
            m_onBack();
        }
    }
    
    // Handle touch on back button
    const auto& touch = input.getTouch();
    if (m_showBackButton) {
        // Back button area is left 100px
        Rect backArea(m_bounds.x, m_bounds.y, 100, m_bounds.h);
        
        if (touch.justTouched && backArea.contains(touch.x, touch.y)) {
            m_backButtonPressed = true;
        }
        
        if (touch.justReleased && m_backButtonPressed) {
            if (backArea.contains(touch.x, touch.y) && m_onBack) {
                m_onBack();
            }
            m_backButtonPressed = false;
        }
    }
}

// =============================================================================
// Rendering
// =============================================================================

void NavigationBar::render(Renderer& renderer, Theme& theme) {
    if (!m_visible) return;
    
    // Draw background with configurable opacity
    Color bgColor = theme.backgroundColor();
    bgColor.a = static_cast<Uint8>(240 * m_backgroundOpacity);
    renderer.drawRect(m_bounds, bgColor);
    
    // Draw bottom separator
    if (m_backgroundOpacity > 0.5f) {
        Color sepColor = theme.separatorColor();
        sepColor.a = static_cast<Uint8>(sepColor.a * m_backgroundOpacity);
        renderer.drawLine(
            m_bounds.x, m_bounds.y + m_bounds.h,
            m_bounds.x + m_bounds.w, m_bounds.y + m_bounds.h,
            sepColor, 1
        );
    }
    
    // Draw back button if shown
    if (m_showBackButton) {
        Color backColor = theme.primaryColor();
        if (m_backButtonPressed) {
            backColor.a = 180;
        }
        
        // Back arrow (simple < symbol for now)
        float arrowX = m_bounds.x + 16;
        float arrowY = m_bounds.y + m_bounds.h / 2;
        renderer.drawText("<", arrowX, arrowY - 10, 20, backColor, FontWeight::Regular);
        
        // Back title
        renderer.drawText(
            m_backButtonTitle,
            arrowX + 16,
            arrowY - 8,
            17,
            backColor,
            FontWeight::Regular
        );
    }
    
    // Draw title
    if (m_largeTitle) {
        // Large title (left-aligned, larger font)
        renderer.drawText(
            m_title,
            m_bounds.x + 20,
            m_bounds.y + m_bounds.h - 10,
            34,
            theme.textPrimaryColor(),
            FontWeight::Bold,
            TextAlign::Left
        );
    } else {
        // Regular title (centered)
        renderer.drawTextInRect(
            m_title,
            m_bounds,
            17,
            theme.textPrimaryColor(),
            FontWeight::Semibold,
            TextAlign::Center,
            TextVAlign::Middle
        );
    }
}
