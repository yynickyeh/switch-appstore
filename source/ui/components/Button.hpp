// =============================================================================
// Switch App Store - Button Component
// =============================================================================
// iOS-style button with various styles (filled, outline, text)
// Supports press animations and haptic feedback
// =============================================================================

#pragma once

#include "ui/Component.hpp"
#include <string>

// =============================================================================
// Button styles (matching iOS)
// =============================================================================
enum class ButtonStyle {
    Filled,         // Solid background (primary action, like "获取" button)
    Outline,        // Border only (secondary action)
    Text,           // Text only (tertiary action, like "查看全部")
    Capsule         // Pill-shaped filled (App Store download button)
};

// =============================================================================
// Button - Interactive button component
// =============================================================================
class Button : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    Button();
    explicit Button(const std::string& text, ButtonStyle style = ButtonStyle::Filled);
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void render(Renderer& renderer, Theme& theme) override;
    
    // -------------------------------------------------------------------------
    // Button-specific properties
    // -------------------------------------------------------------------------
    
    // Text
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }
    
    // Style
    void setStyle(ButtonStyle style) { m_style = style; }
    ButtonStyle getStyle() const { return m_style; }
    
    // Font size
    void setFontSize(int size) { m_fontSize = size; }
    int getFontSize() const { return m_fontSize; }
    
    // Corner radius
    void setCornerRadius(int radius) { m_cornerRadius = radius; }
    int getCornerRadius() const { return m_cornerRadius; }
    
    // Custom colors (optional, uses theme colors by default)
    void setBackgroundColor(const Color& color) { m_bgColor = color; m_useCustomBgColor = true; }
    void setTextColor(const Color& color) { m_textColor = color; m_useCustomTextColor = true; }
    
private:
    std::string m_text = "Button";
    ButtonStyle m_style = ButtonStyle::Filled;
    int m_fontSize = 16;
    int m_cornerRadius = 8;
    
    // Custom colors
    Color m_bgColor;
    Color m_textColor;
    bool m_useCustomBgColor = false;
    bool m_useCustomTextColor = false;
};
