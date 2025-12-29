// =============================================================================
// Switch App Store - Card Implementation
// =============================================================================

#include "Card.hpp"
#include "ui/Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

Card::Card() = default;

// =============================================================================
// Rendering
// =============================================================================

void Card::render(Renderer& renderer, Theme& theme) {
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
    
    // Draw shadow first (behind the card)
    if (m_shadowEnabled && m_opacity > 0.5f) {
        Color shadowColor = theme.getColor("shadow");
        shadowColor.a = static_cast<Uint8>(shadowColor.a * m_opacity);
        
        renderer.drawShadow(
            drawBounds, m_cornerRadius, m_shadowBlur,
            m_shadowOffsetX, m_shadowOffsetY, shadowColor
        );
    }
    
    // Get background color
    Color bgColor;
    if (m_useCustomBgColor) {
        bgColor = m_bgColor;
    } else {
        bgColor = theme.cardBackgroundColor();
    }
    bgColor.a = static_cast<Uint8>(bgColor.a * m_opacity);
    
    // Draw background
    if (m_hasGradient) {
        Color topColor = m_gradientTop;
        Color bottomColor = m_gradientBottom;
        topColor.a = static_cast<Uint8>(topColor.a * m_opacity);
        bottomColor.a = static_cast<Uint8>(bottomColor.a * m_opacity);
        
        renderer.drawGradientRoundedRect(drawBounds, m_cornerRadius, topColor, bottomColor);
    } else {
        renderer.drawRoundedRect(drawBounds, m_cornerRadius, bgColor);
    }
    
    // Draw border if enabled
    if (m_hasBorder) {
        Color borderColor = m_borderColor;
        borderColor.a = static_cast<Uint8>(borderColor.a * m_opacity);
        renderer.drawRoundedRectOutline(drawBounds, m_cornerRadius, borderColor, m_borderWidth);
    }
    
    // Draw focus ring if focused
    if (m_focused) {
        Color focusColor = theme.primaryColor();
        focusColor.a = 180;
        renderer.drawRoundedRectOutline(
            Rect(drawBounds.x - 3, drawBounds.y - 3,
                 drawBounds.w + 6, drawBounds.h + 6),
            m_cornerRadius + 3, focusColor, 3
        );
    }
    
    // Render children on top of card
    renderChildren(renderer, theme);
}
