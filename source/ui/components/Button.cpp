// =============================================================================
// Switch App Store - Button Implementation
// =============================================================================

#include "Button.hpp"
#include "ui/Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

Button::Button() = default;

Button::Button(const std::string& text, ButtonStyle style)
    : m_text(text)
    , m_style(style)
{
    // Set default size based on style
    if (style == ButtonStyle::Capsule) {
        m_cornerRadius = 16;  // More rounded for capsule
        m_bounds.w = 80;
        m_bounds.h = 32;
    }
}

// =============================================================================
// Rendering
// =============================================================================

void Button::render(Renderer& renderer, Theme& theme) {
    if (!m_visible) return;
    
    // Calculate scaled bounds for press animation
    float scaledW = m_bounds.w * m_scale;
    float scaledH = m_bounds.h * m_scale;
    float offsetX = (m_bounds.w - scaledW) / 2;
    float offsetY = (m_bounds.h - scaledH) / 2;
    
    Rect drawBounds(
        m_bounds.x + offsetX,
        m_bounds.y + offsetY,
        scaledW,
        scaledH
    );
    
    // Get colors from theme
    Color bgColor = m_useCustomBgColor ? m_bgColor : theme.getColor("button_bg");
    Color textColor = m_useCustomTextColor ? m_textColor : theme.getColor("button_text");
    
    // Adjust colors based on state
    if (!m_enabled) {
        bgColor.a = 128;
        textColor.a = 128;
    } else if (m_focused) {
        // Slightly brighter when focused
        bgColor.r = std::min(255, bgColor.r + 20);
        bgColor.g = std::min(255, bgColor.g + 20);
        bgColor.b = std::min(255, bgColor.b + 20);
    }
    
    // Render based on style
    switch (m_style) {
        case ButtonStyle::Filled:
        case ButtonStyle::Capsule: {
            // Draw filled background
            renderer.drawRoundedRect(drawBounds, m_cornerRadius, bgColor);
            
            // Draw text centered
            renderer.drawTextInRect(
                m_text, drawBounds, m_fontSize, textColor,
                FontWeight::Semibold, TextAlign::Center, TextVAlign::Middle
            );
            break;
        }
        
        case ButtonStyle::Outline: {
            // Draw outline only
            Color outlineColor = m_useCustomBgColor ? m_bgColor : theme.primaryColor();
            renderer.drawRoundedRectOutline(drawBounds, m_cornerRadius, outlineColor, 2);
            
            // Text uses the outline color
            textColor = outlineColor;
            renderer.drawTextInRect(
                m_text, drawBounds, m_fontSize, textColor,
                FontWeight::Semibold, TextAlign::Center, TextVAlign::Middle
            );
            break;
        }
        
        case ButtonStyle::Text: {
            // No background, just text
            textColor = m_useCustomTextColor ? m_textColor : theme.primaryColor();
            renderer.drawTextInRect(
                m_text, drawBounds, m_fontSize, textColor,
                FontWeight::Regular, TextAlign::Center, TextVAlign::Middle
            );
            break;
        }
    }
    
    // Draw focus ring if focused
    if (m_focused) {
        Color focusColor = theme.primaryColor();
        focusColor.a = 100;
        renderer.drawRoundedRectOutline(
            Rect(drawBounds.x - 3, drawBounds.y - 3, 
                 drawBounds.w + 6, drawBounds.h + 6),
            m_cornerRadius + 3, focusColor, 2
        );
    }
}
