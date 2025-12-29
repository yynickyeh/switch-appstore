// =============================================================================
// Switch App Store - Label Implementation
// =============================================================================

#include "Label.hpp"
#include "ui/Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

Label::Label() = default;

Label::Label(const std::string& text, int fontSize)
    : m_text(text)
    , m_fontSize(fontSize)
{
}

// =============================================================================
// Rendering
// =============================================================================

void Label::render(Renderer& renderer, Theme& theme) {
    if (!m_visible || m_text.empty()) return;
    
    // Get color
    Color color;
    if (m_useCustomColor) {
        color = m_customColor;
    } else {
        color = theme.getColor(m_colorName);
    }
    
    // Apply opacity
    color.a = static_cast<Uint8>(color.a * m_opacity);
    
    // Render text within bounds
    renderer.drawTextInRect(
        m_text, m_bounds, m_fontSize, color,
        m_fontWeight, m_textAlign, m_verticalAlign
    );
}
