// =============================================================================
// Switch App Store - Label Component
// =============================================================================
// Text display component with various styles and alignment options
// =============================================================================

#pragma once

#include "ui/Component.hpp"
#include <string>

// =============================================================================
// Label - Text display component
// =============================================================================
class Label : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    Label();
    explicit Label(const std::string& text, int fontSize = 17);
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void render(Renderer& renderer, Theme& theme) override;
    
    // Labels typically don't receive focus
    bool canFocus() const override { return false; }
    
    // -------------------------------------------------------------------------
    // Label-specific properties
    // -------------------------------------------------------------------------
    
    // Text content
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }
    
    // Font settings
    void setFontSize(int size) { m_fontSize = size; }
    int getFontSize() const { return m_fontSize; }
    
    void setFontWeight(FontWeight weight) { m_fontWeight = weight; }
    FontWeight getFontWeight() const { return m_fontWeight; }
    
    // Alignment
    void setTextAlign(TextAlign align) { m_textAlign = align; }
    TextAlign getTextAlign() const { return m_textAlign; }
    
    void setVerticalAlign(TextVAlign align) { m_verticalAlign = align; }
    TextVAlign getVerticalAlign() const { return m_verticalAlign; }
    
    // Color (use "text_primary", "text_secondary", "text_tertiary" or custom)
    void setColorName(const std::string& colorName) { m_colorName = colorName; }
    void setCustomColor(const Color& color) { m_customColor = color; m_useCustomColor = true; }
    
    // Max lines (0 = unlimited)
    void setMaxLines(int maxLines) { m_maxLines = maxLines; }
    
private:
    std::string m_text;
    int m_fontSize = 17;
    FontWeight m_fontWeight = FontWeight::Regular;
    TextAlign m_textAlign = TextAlign::Left;
    TextVAlign m_verticalAlign = TextVAlign::Top;
    std::string m_colorName = "text_primary";
    Color m_customColor;
    bool m_useCustomColor = false;
    int m_maxLines = 0;
};
