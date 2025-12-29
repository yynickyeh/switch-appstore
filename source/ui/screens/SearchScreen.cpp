// =============================================================================
// Switch App Store - Search Screen Implementation
// =============================================================================

#include "SearchScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include <algorithm>
#include <switch.h>  // For swkbd (software keyboard)

// =============================================================================
// Constructor & Destructor
// =============================================================================

SearchScreen::SearchScreen(App* app)
    : Screen(app)
{
    loadDemoContent();
}

SearchScreen::~SearchScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void SearchScreen::onEnter() {
    m_scrollY = 0.0f;
    m_searchQuery.clear();
    m_isSearching = false;
    m_selectedTagIndex = -1;
    m_selectedResultIndex = 0;
}

void SearchScreen::onExit() {
    // Clear search state
}

void SearchScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate layout
}

// =============================================================================
// Input Handling
// =============================================================================

void SearchScreen::handleInput(const Input& input) {
    // D-pad navigation for tags
    if (!m_isSearching) {
        if (input.isPressed(Input::Button::DPadLeft)) {
            if (m_selectedTagIndex > 0) {
                m_selectedTagIndex--;
            }
        }
        if (input.isPressed(Input::Button::DPadRight)) {
            if (m_selectedTagIndex < (int)m_hotKeywords.size() - 1) {
                m_selectedTagIndex++;
            }
        }
        
        // A to select tag and search
        if (input.isPressed(Input::Button::A) && m_selectedTagIndex >= 0) {
            m_searchQuery = m_hotKeywords[m_selectedTagIndex];
            performSearch(m_searchQuery);
        }
    } else {
        // Navigation in search results
        if (input.isPressed(Input::Button::DPadUp)) {
            if (m_selectedResultIndex > 0) {
                m_selectedResultIndex--;
            }
        }
        if (input.isPressed(Input::Button::DPadDown)) {
            if (m_selectedResultIndex < (int)m_searchResults.size() - 1) {
                m_selectedResultIndex++;
            }
        }
        
        // B to cancel search
        if (input.isPressed(Input::Button::B)) {
            m_isSearching = false;
            m_searchQuery.clear();
            m_searchResults.clear();
        }
        
        // A to select result
        if (input.isPressed(Input::Button::A) && !m_searchResults.empty()) {
            // TODO: Navigate to DetailScreen
        }
    }
    
    // X button to focus search bar (open keyboard on Switch)
    if (input.isPressed(Input::Button::X) || input.isPressed(Input::Button::Y)) {
        showKeyboard();
    }
    
    // Touch handling
    const auto& touch = input.getTouch();
    
    // Touch on search bar area to open keyboard
    if (touch.justReleased) {
        float barY = SEARCH_BAR_MARGIN;
        float barWidth = 1280 - SIDE_PADDING * 2 - 80;
        if (touch.y >= barY && touch.y <= barY + SEARCH_BAR_HEIGHT &&
            touch.x >= SIDE_PADDING && touch.x <= SIDE_PADDING + barWidth) {
            showKeyboard();
        }
    }
    
    // Touch handling for tags
    if (touch.justReleased && !m_isSearching) {
        // Check if touching a tag
        float tagY = SEARCH_BAR_HEIGHT + SEARCH_BAR_MARGIN * 2 + 30;
        float tagX = SIDE_PADDING;
        
        for (size_t i = 0; i < m_hotKeywords.size(); i++) {
            float tagWidth = 80 + m_hotKeywords[i].length() * 8;  // Approximate
            Rect tagRect(tagX, tagY, tagWidth, TAG_HEIGHT);
            
            if (tagRect.contains(touch.x, touch.y)) {
                m_searchQuery = m_hotKeywords[i];
                performSearch(m_searchQuery);
                break;
            }
            
            tagX += tagWidth + TAG_SPACING;
            if (tagX > 1200) {
                tagX = SIDE_PADDING;
                tagY += TAG_HEIGHT + TAG_SPACING;
            }
        }
    }
}

// =============================================================================
// Update
// =============================================================================

void SearchScreen::update(float deltaTime) {
    // Nothing special for now
}

// =============================================================================
// Rendering
// =============================================================================

void SearchScreen::render(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Search bar at top
    renderSearchBar(renderer);
    
    if (m_isSearching && !m_searchResults.empty()) {
        // Show search results
        renderSearchResults(renderer);
    } else {
        // Show hot tags and recommendations
        renderHotTags(renderer);
        renderRecommendations(renderer);
    }
}

void SearchScreen::renderSearchBar(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float barY = SEARCH_BAR_MARGIN;
    float barWidth = 1280 - SIDE_PADDING * 2 - 80;  // Leave space for cancel
    
    // Search bar background
    Color barBg = theme->getColor("search_bg");
    renderer.drawRoundedRect(
        Rect(SIDE_PADDING, barY, barWidth, SEARCH_BAR_HEIGHT),
        12, barBg
    );
    
    // Search icon placeholder
    Color iconColor = theme->textSecondaryColor();
    renderer.drawCircle(SIDE_PADDING + 24, barY + SEARCH_BAR_HEIGHT / 2, 8, iconColor);
    
    // Placeholder or query text
    std::string displayText = m_searchQuery.empty() ? 
        "游戏、App与更多内容" : m_searchQuery;
    Color textColor = m_searchQuery.empty() ? 
        theme->getColor("search_placeholder") : theme->getColor("search_text");
    
    renderer.drawText(displayText, SIDE_PADDING + 44, barY + 13, 16, textColor);
    
    // Cancel button (if searching)
    if (m_isSearching || !m_searchQuery.empty()) {
        renderer.drawText("取消", 1280 - SIDE_PADDING - 60, barY + 13, 17,
                         theme->primaryColor());
    }
    
    // Focus indicator
    if (m_searchBarFocused) {
        renderer.drawRoundedRectOutline(
            Rect(SIDE_PADDING - 2, barY - 2, barWidth + 4, SEARCH_BAR_HEIGHT + 4),
            14, theme->primaryColor(), 2
        );
    }
}

void SearchScreen::renderHotTags(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float startY = SEARCH_BAR_HEIGHT + SEARCH_BAR_MARGIN * 2;
    
    // Section title
    renderer.drawText("热门搜索", SIDE_PADDING, startY, 20,
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Tags in a flex wrap layout
    float tagY = startY + 36;
    float tagX = SIDE_PADDING;
    
    for (size_t i = 0; i < m_hotKeywords.size(); i++) {
        const std::string& keyword = m_hotKeywords[i];
        float tagWidth = 20 + keyword.length() * 12;  // Approximate Chinese char width
        
        // Wrap to next line if needed
        if (tagX + tagWidth > 1280 - SIDE_PADDING) {
            tagX = SIDE_PADDING;
            tagY += TAG_HEIGHT + TAG_SPACING;
        }
        
        bool isSelected = ((int)i == m_selectedTagIndex);
        
        // Tag background
        Color tagBg = isSelected ? theme->primaryColor() : theme->getColor("search_bg");
        renderer.drawRoundedRect(Rect(tagX, tagY, tagWidth, TAG_HEIGHT), 18, tagBg);
        
        // Tag text
        Color tagText = isSelected ? Color(255, 255, 255) : theme->textPrimaryColor();
        renderer.drawTextInRect(keyword, Rect(tagX, tagY, tagWidth, TAG_HEIGHT),
                               14, tagText, FontWeight::Regular, 
                               TextAlign::Center, TextVAlign::Middle);
        
        tagX += tagWidth + TAG_SPACING;
    }
}

void SearchScreen::renderRecommendations(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float startY = 280;  // Below tags
    
    // Section title
    renderer.drawText("推荐", SIDE_PADDING, startY, 20,
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // List of recommendations
    float itemY = startY + 40;
    for (size_t i = 0; i < m_recommendations.size() && i < 5; i++) {
        const auto& game = m_recommendations[i];
        
        // Icon placeholder
        renderer.drawRoundedRect(Rect(SIDE_PADDING, itemY, 60, 60), 12, 
                                Color::fromHex(0xE5E5EA));
        
        // Game info
        renderer.drawText(game.name, SIDE_PADDING + 76, itemY + 8, 16,
                         theme->textPrimaryColor(), FontWeight::Semibold);
        renderer.drawText(game.category, SIDE_PADDING + 76, itemY + 32, 14,
                         theme->textSecondaryColor());
        
        // Get button
        renderer.drawRoundedRect(
            Rect(1280 - SIDE_PADDING - 80, itemY + 14, 70, 32),
            16, theme->primaryColor()
        );
        renderer.drawTextInRect(
            "获取",
            Rect(1280 - SIDE_PADDING - 80, itemY + 14, 70, 32),
            14, Color(255, 255, 255),
            FontWeight::Semibold, TextAlign::Center, TextVAlign::Middle
        );
        
        itemY += 76;
    }
}

void SearchScreen::renderSearchResults(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float startY = SEARCH_BAR_HEIGHT + SEARCH_BAR_MARGIN * 2;
    
    // Results count
    std::string resultText = "找到 " + std::to_string(m_searchResults.size()) + " 个结果";
    renderer.drawText(resultText, SIDE_PADDING, startY, 16, theme->textSecondaryColor());
    
    // List of results
    float itemY = startY + 36;
    for (size_t i = 0; i < m_searchResults.size(); i++) {
        const auto& game = m_searchResults[i];
        bool isSelected = ((int)i == m_selectedResultIndex);
        
        // Selection highlight
        if (isSelected) {
            renderer.drawRect(
                Rect(0, itemY - 8, 1280, 76),
                theme->getColor("selection")
            );
        }
        
        // Icon placeholder
        renderer.drawRoundedRect(Rect(SIDE_PADDING, itemY, 60, 60), 12,
                                Color::fromHex(0xE5E5EA));
        
        // Game info
        renderer.drawText(game.name, SIDE_PADDING + 76, itemY + 8, 16,
                         theme->textPrimaryColor(), FontWeight::Semibold);
        renderer.drawText(game.developer + " · " + game.category, 
                         SIDE_PADDING + 76, itemY + 32, 14,
                         theme->textSecondaryColor());
        
        // Get button
        renderer.drawRoundedRect(
            Rect(1280 - SIDE_PADDING - 80, itemY + 14, 70, 32),
            16, theme->primaryColor()
        );
        renderer.drawTextInRect(
            "获取",
            Rect(1280 - SIDE_PADDING - 80, itemY + 14, 70, 32),
            14, Color(255, 255, 255),
            FontWeight::Semibold, TextAlign::Center, TextVAlign::Middle
        );
        
        itemY += 76;
    }
}

// =============================================================================
// Search
// =============================================================================

void SearchScreen::performSearch(const std::string& query) {
    m_isSearching = true;
    m_searchResults.clear();
    m_selectedResultIndex = 0;
    
    // Demo: filter recommendations by query
    for (const auto& game : m_recommendations) {
        // Simple contains check
        if (game.name.find(query) != std::string::npos ||
            game.category.find(query) != std::string::npos) {
            m_searchResults.push_back(game);
        }
    }
    
    // If no results, show all
    if (m_searchResults.empty()) {
        m_searchResults = m_recommendations;
    }
}

// =============================================================================
// Demo Content
// =============================================================================

void SearchScreen::loadDemoContent() {
    // Hot keywords
    m_hotKeywords = {
        "马里奥", "塞尔达", "宝可梦", "星之卡比",
        "动物森友会", "喷射战士", "火焰纹章", "异度神剑"
    };
    
    // Recommendations
    m_recommendations = {
        {"1", "塞尔达传说：旷野之息", "Nintendo", "动作冒险", "", 4.9f, "14.5GB"},
        {"2", "超级马力欧 奥德赛", "Nintendo", "平台动作", "", 4.8f, "5.7GB"},
        {"3", "宝可梦 朱/紫", "Game Freak", "角色扮演", "", 4.5f, "7.0GB"},
        {"4", "斯普拉遁3", "Nintendo", "射击", "", 4.7f, "6.1GB"},
        {"5", "动物森友会", "Nintendo", "模拟经营", "", 4.9f, "6.8GB"},
        {"6", "星之卡比 探索发现", "HAL Laboratory", "平台动作", "", 4.7f, "5.8GB"},
        {"7", "异度神剑3", "Monolith Soft", "角色扮演", "", 4.6f, "15.0GB"},
    };
}

// =============================================================================
// Software Keyboard
// =============================================================================

void SearchScreen::showKeyboard() {
    // Initialize swkbd config
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    
    if (R_SUCCEEDED(rc)) {
        // Configure keyboard
        swkbdConfigMakePresetDefault(&kbd);
        swkbdConfigSetGuideText(&kbd, "输入搜索关键词");
        swkbdConfigSetInitialText(&kbd, m_searchQuery.c_str());
        swkbdConfigSetStringLenMax(&kbd, 64);
        swkbdConfigSetStringLenMin(&kbd, 0);
        swkbdConfigSetType(&kbd, SwkbdType_Normal);
        
        // Show keyboard and get input
        char resultText[65] = {0};
        rc = swkbdShow(&kbd, resultText, sizeof(resultText));
        
        if (R_SUCCEEDED(rc)) {
            // User confirmed input
            m_searchQuery = resultText;
            
            if (!m_searchQuery.empty()) {
                performSearch(m_searchQuery);
            } else {
                // Empty query - clear search
                m_isSearching = false;
                m_searchResults.clear();
            }
        }
        
        swkbdClose(&kbd);
    }
}

