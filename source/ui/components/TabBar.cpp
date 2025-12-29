// =============================================================================
// Switch App Store - TabBar Component Implementation
// =============================================================================

#include "TabBar.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include "ui/Router.hpp"

// =============================================================================
// Constructor
// =============================================================================

TabBar::TabBar(App* app) 
    : m_app(app) 
{
}

// =============================================================================
// Tab Management
// =============================================================================

void TabBar::addTab(const std::string& id, const std::string& label,
                    const std::string& iconName) {
    TabItem tab;
    tab.id = id;
    tab.label = label;
    tab.iconName = iconName;
    m_tabs.push_back(tab);
}

void TabBar::setSelectedIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return;
    if (index == m_selectedIndex) return;
    
    int oldIndex = m_selectedIndex;
    m_previousIndex = m_selectedIndex;
    m_selectedIndex = index;
    m_selectionAnimProgress = 0.0f;
    
    if (m_onTabChange) {
        m_onTabChange(oldIndex, index);
    }
}

// =============================================================================
// Input Handling
// =============================================================================

bool TabBar::handleInput(const Input& input) {
    // L/R shoulder buttons for quick tab switching (Apple HIG pattern)
    if (input.isPressed(Input::Button::L)) {
        if (m_selectedIndex > 0) {
            setSelectedIndex(m_selectedIndex - 1);
            return true;
        }
    }
    
    if (input.isPressed(Input::Button::R)) {
        if (m_selectedIndex < static_cast<int>(m_tabs.size()) - 1) {
            setSelectedIndex(m_selectedIndex + 1);
            return true;
        }
    }
    
    // Touch input for direct tab selection
    const auto& touch = input.getTouch();
    if (touch.justReleased) {
        int hitIndex = hitTestTabs(touch.x, touch.y);
        if (hitIndex >= 0 && hitIndex != m_selectedIndex) {
            setSelectedIndex(hitIndex);
            return true;
        }
    }
    
    return false;
}

int TabBar::hitTestTabs(float touchX, float touchY) const {
    if (m_tabs.empty()) return -1;
    
    float tabBarY = m_screenHeight - HEIGHT;
    
    // Check if touch is within tab bar vertical bounds
    if (touchY < tabBarY || touchY > m_screenHeight) {
        return -1;
    }
    
    // Calculate which tab was touched
    float tabWidth = m_screenWidth / m_tabs.size();
    int index = static_cast<int>(touchX / tabWidth);
    
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        return index;
    }
    
    return -1;
}

// =============================================================================
// Rendering
// =============================================================================

void TabBar::render(Renderer& renderer) {
    if (m_tabs.empty()) return;
    
    renderBackground(renderer);
    renderTabs(renderer);
}

void TabBar::renderBackground(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float tabBarY = m_screenHeight - HEIGHT;
    
    // Background with blur effect simulation (semi-transparent)
    // Using the theme's tab bar background color
    renderer.drawRect(
        Rect(0, tabBarY, m_screenWidth, HEIGHT),
        theme->getColor("tab_bar_bg")
    );
    
    // Top separator line (1px, subtle)
    renderer.drawLine(0, tabBarY, m_screenWidth, tabBarY, 
                     theme->separatorColor(), 1);
}

void TabBar::renderTabs(Renderer& renderer) {
    if (m_tabs.empty()) return;
    
    float tabWidth = m_screenWidth / m_tabs.size();
    float tabBarY = m_screenHeight - HEIGHT;
    
    for (size_t i = 0; i < m_tabs.size(); i++) {
        float tabX = i * tabWidth;
        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        
        renderTab(renderer, m_tabs[i], tabX, tabBarY, tabWidth, isSelected);
    }
}

void TabBar::renderTab(Renderer& renderer, const TabItem& tab, float x, float y,
                        float width, bool isSelected) {
    Theme* theme = m_app->getTheme();
    
    // Calculate center position
    float centerX = x + width / 2.0f;
    
    // Icon area (top portion of tab)
    float iconY = y + 12.0f;
    float iconSize = 28.0f;
    
    // Determine color based on selection state
    Color color = isSelected ? theme->primaryColor() : theme->textSecondaryColor();
    
    // Draw placeholder icon (circle for now)
    // In production, would load actual SF Symbols or custom icons
    renderer.drawCircle(centerX, iconY + iconSize / 2.0f, iconSize / 2.0f, color);
    
    // Icon inner detail (simulating icon shape)
    if (isSelected) {
        renderer.drawCircle(centerX, iconY + iconSize / 2.0f, iconSize / 4.0f, 
                           theme->getColor("tab_bar_bg"));
    }
    
    // Label text
    float labelY = y + 48.0f;
    renderer.drawText(tab.label, centerX, labelY, 11, color,
                     FontWeight::Regular, TextAlign::Center);
    
    // Selection indicator dot (subtle, below icon for visual enhancement)
    if (isSelected) {
        renderer.drawCircle(centerX, y + 8.0f, 2.0f, theme->primaryColor());
    }
}
