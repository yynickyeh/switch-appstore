// =============================================================================
// Switch App Store - Theme Implementation
// =============================================================================

#include "Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

Theme::Theme() {
    // Load light theme by default (matching iOS default)
    loadTheme("light");
}

// =============================================================================
// Theme Loading
// =============================================================================

void Theme::loadTheme(const std::string& themeName) {
    m_currentTheme = themeName;
    m_colors.clear();
    
    if (themeName == "dark") {
        loadDarkTheme();
    } else {
        loadLightTheme();
    }
}

void Theme::toggleTheme() {
    if (m_currentTheme == "light") {
        loadTheme("dark");
    } else {
        loadTheme("light");
    }
}

// =============================================================================
// Color Accessors
// =============================================================================

const Color& Theme::getColor(const std::string& name) const {
    auto it = m_colors.find(name);
    if (it != m_colors.end()) {
        return it->second;
    }
    return m_defaultColor;
}

// =============================================================================
// Light Theme (iOS App Store style)
// =============================================================================

void Theme::loadLightTheme() {
    // Primary accent color (iOS blue)
    m_colors["primary"] = Color::fromHex(0x007AFF);
    
    // Backgrounds
    m_colors["background"] = Color::fromHex(0xF2F2F7);
    m_colors["card_bg"] = Color::fromHex(0xFFFFFF);
    m_colors["tab_bar_bg"] = Color(249, 249, 249, 240);  // Semi-transparent
    
    // Text colors
    m_colors["text_primary"] = Color::fromHex(0x000000);
    m_colors["text_secondary"] = Color::fromHex(0x8E8E93);
    m_colors["text_tertiary"] = Color::fromHex(0xC7C7CC);
    
    // Separators
    m_colors["separator"] = Color::fromHex(0xC6C6C8);
    
    // Semantic colors
    m_colors["success"] = Color::fromHex(0x34C759);
    m_colors["warning"] = Color::fromHex(0xFF9500);
    m_colors["danger"] = Color::fromHex(0xFF3B30);
    
    // Special colors
    m_colors["star"] = Color::fromHex(0xFFD700);
    m_colors["link"] = Color::fromHex(0x007AFF);
    
    // Button colors
    m_colors["button_bg"] = Color::fromHex(0x007AFF);
    m_colors["button_text"] = Color::fromHex(0xFFFFFF);
    m_colors["button_secondary_bg"] = Color::fromHex(0xE5E5EA);
    m_colors["button_secondary_text"] = Color::fromHex(0x007AFF);
    
    // Search bar
    m_colors["search_bg"] = Color::fromHex(0xE5E5EA);
    m_colors["search_text"] = Color::fromHex(0x000000);
    m_colors["search_placeholder"] = Color::fromHex(0x8E8E93);
    
    // Card shadows
    m_colors["shadow"] = Color(0, 0, 0, 40);
    
    // Selection/highlight
    m_colors["selection"] = Color(0, 122, 255, 30);
    m_colors["highlight"] = Color::fromHex(0xE5E5EA);
}

// =============================================================================
// Dark Theme
// =============================================================================

void Theme::loadDarkTheme() {
    // Primary accent color (brighter blue for dark mode)
    m_colors["primary"] = Color::fromHex(0x0A84FF);
    
    // Backgrounds
    m_colors["background"] = Color::fromHex(0x000000);
    m_colors["card_bg"] = Color::fromHex(0x1C1C1E);
    m_colors["tab_bar_bg"] = Color(30, 30, 30, 240);  // Semi-transparent
    
    // Text colors
    m_colors["text_primary"] = Color::fromHex(0xFFFFFF);
    m_colors["text_secondary"] = Color::fromHex(0x8E8E93);
    m_colors["text_tertiary"] = Color::fromHex(0x48484A);
    
    // Separators
    m_colors["separator"] = Color::fromHex(0x38383A);
    
    // Semantic colors
    m_colors["success"] = Color::fromHex(0x30D158);
    m_colors["warning"] = Color::fromHex(0xFF9F0A);
    m_colors["danger"] = Color::fromHex(0xFF453A);
    
    // Special colors
    m_colors["star"] = Color::fromHex(0xFFD700);
    m_colors["link"] = Color::fromHex(0x0A84FF);
    
    // Button colors
    m_colors["button_bg"] = Color::fromHex(0x0A84FF);
    m_colors["button_text"] = Color::fromHex(0xFFFFFF);
    m_colors["button_secondary_bg"] = Color::fromHex(0x2C2C2E);
    m_colors["button_secondary_text"] = Color::fromHex(0x0A84FF);
    
    // Search bar
    m_colors["search_bg"] = Color::fromHex(0x1C1C1E);
    m_colors["search_text"] = Color::fromHex(0xFFFFFF);
    m_colors["search_placeholder"] = Color::fromHex(0x8E8E93);
    
    // Card shadows (less visible in dark mode)
    m_colors["shadow"] = Color(0, 0, 0, 80);
    
    // Selection/highlight
    m_colors["selection"] = Color(10, 132, 255, 40);
    m_colors["highlight"] = Color::fromHex(0x2C2C2E);
}
