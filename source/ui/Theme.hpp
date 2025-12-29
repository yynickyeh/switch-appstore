// =============================================================================
// Switch App Store - Theme Manager
// =============================================================================
// Manages color themes (Light/Dark mode) matching iOS App Store style
// =============================================================================

#pragma once

#include "core/Renderer.hpp"
#include <string>
#include <unordered_map>

// =============================================================================
// Theme - Color and style management
// =============================================================================
class Theme {
public:
    Theme();
    ~Theme() = default;
    
    // Load a theme by name ("light" or "dark")
    void loadTheme(const std::string& themeName);
    
    // Get the current theme name
    const std::string& getThemeName() const { return m_currentTheme; }
    
    // Toggle between light and dark mode
    void toggleTheme();
    
    // -------------------------------------------------------------------------
    // Color accessors
    // -------------------------------------------------------------------------
    
    // Get a named color (e.g., "primary", "background", "text")
    const Color& getColor(const std::string& name) const;
    
    // Convenience accessors for common colors
    const Color& primaryColor() const { return getColor("primary"); }
    const Color& backgroundColor() const { return getColor("background"); }
    const Color& cardBackgroundColor() const { return getColor("card_bg"); }
    const Color& textPrimaryColor() const { return getColor("text_primary"); }
    const Color& textSecondaryColor() const { return getColor("text_secondary"); }
    const Color& separatorColor() const { return getColor("separator"); }
    const Color& successColor() const { return getColor("success"); }
    const Color& warningColor() const { return getColor("warning"); }
    const Color& dangerColor() const { return getColor("danger"); }
    
private:
    // Current theme name
    std::string m_currentTheme;
    
    // Color map
    std::unordered_map<std::string, Color> m_colors;
    
    // Default color for missing entries
    Color m_defaultColor{255, 0, 255, 255};  // Magenta for visibility
    
    // Load light theme colors
    void loadLightTheme();
    
    // Load dark theme colors
    void loadDarkTheme();
};
