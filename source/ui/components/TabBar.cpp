// =============================================================================
// Switch App Store - TabBar Implementation
// =============================================================================

#include "TabBar.hpp"
#include "ui/Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

TabBar::TabBar() {
    m_bounds.h = 70.0f;  // Default height for 720p
}

// =============================================================================
// Tab Management
// =============================================================================

void TabBar::addTab(const std::string& label) {
    TabItem item;
    item.label = label;
    m_tabs.push_back(item);
}

void TabBar::setSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_tabs.size()) && index != m_selectedIndex) {
        m_previousIndex = m_selectedIndex;
        m_selectedIndex = index;
        m_transitionProgress = 0.0f;
        
        if (m_onTabChanged) {
            m_onTabChanged(index);
        }
    }
}

// =============================================================================
// Input Handling
// =============================================================================

void TabBar::handleInput(const Input& input) {
    // Touch handling
    const auto& touch = input.getTouch();
    if (touch.justReleased && containsPoint(touch.x, touch.y)) {
        // Determine which tab was touched
        float tabWidth = m_bounds.w / m_tabs.size();
        int touchedIndex = static_cast<int>((touch.x - m_bounds.x) / tabWidth);
        if (touchedIndex >= 0 && touchedIndex < static_cast<int>(m_tabs.size())) {
            setSelectedIndex(touchedIndex);
        }
    }
    
    // Controller navigation (L/R to switch tabs)
    if (input.isPressed(Input::Button::L)) {
        if (m_selectedIndex > 0) {
            setSelectedIndex(m_selectedIndex - 1);
        }
    }
    if (input.isPressed(Input::Button::R)) {
        if (m_selectedIndex < static_cast<int>(m_tabs.size()) - 1) {
            setSelectedIndex(m_selectedIndex + 1);
        }
    }
}

// =============================================================================
// Rendering
// =============================================================================

void TabBar::render(Renderer& renderer, Theme& theme) {
    if (!m_visible || m_tabs.empty()) return;
    
    // Draw background (semi-transparent blur effect simulation)
    Color bgColor = theme.getColor("tab_bar_bg");
    renderer.drawRect(m_bounds, bgColor);
    
    // Draw top separator line
    Color sepColor = theme.separatorColor();
    renderer.drawLine(m_bounds.x, m_bounds.y, 
                      m_bounds.x + m_bounds.w, m_bounds.y, 
                      sepColor, 1);
    
    // Calculate tab dimensions
    float tabWidth = m_bounds.w / m_tabs.size();
    
    // Draw each tab
    for (size_t i = 0; i < m_tabs.size(); i++) {
        float tabX = m_bounds.x + i * tabWidth;
        float tabCenterX = tabX + tabWidth / 2;
        
        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        
        // Get color based on selection state
        Color iconColor, textColor;
        if (isSelected) {
            iconColor = theme.primaryColor();
            textColor = theme.primaryColor();
        } else {
            iconColor = theme.textSecondaryColor();
            textColor = theme.textSecondaryColor();
        }
        
        // Draw placeholder icon (circle for now)
        float iconY = m_bounds.y + 12;
        renderer.drawCircle(tabCenterX, iconY + m_iconSize / 2, m_iconSize / 2, iconColor);
        
        // Draw label
        float labelY = m_bounds.y + 12 + m_iconSize + 6;
        renderer.drawText(
            m_tabs[i].label,
            tabCenterX,
            labelY,
            m_fontSize,
            textColor,
            FontWeight::Regular,
            TextAlign::Center
        );
    }
    
    // Draw focus indicator if focused
    if (m_focused) {
        float focusX = m_bounds.x + m_selectedIndex * tabWidth;
        Color focusColor = theme.primaryColor();
        focusColor.a = 60;
        renderer.drawRect(Rect(focusX, m_bounds.y, tabWidth, m_bounds.h), focusColor);
    }
}
