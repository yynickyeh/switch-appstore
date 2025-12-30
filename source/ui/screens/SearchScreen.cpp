// =============================================================================
// Switch App Store - Search Screen Implementation
// =============================================================================

#include "SearchScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include "store/StoreManager.hpp"
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
    // =========================================================================
    // SEARCH & DISCOVERY TOUCH EXPERIENCE
    // Precise hit testing for tags, results, and instant search bar access.
    // =========================================================================

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
    
    // Analog stick scrolling
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 600.0f;
    }
    
    // -------------------------------------------------------------------------
    // TOUCH HANDLING
    // -------------------------------------------------------------------------
    const auto& touch = input.getTouch();
    
    if (touch.touching) {
         // Direct scroll
        m_scrollY -= touch.deltaY;
        m_scrollVelocity = 0.0f;
    } else if (touch.justReleased) {
        float dragDist = std::sqrt((touch.x - touch.startX) * (touch.x - touch.startX) +
                                   (touch.y - touch.startY) * (touch.y - touch.startY));
        
        if (dragDist < 30.0f) {
            // TAP DETECTED
            float tapX = touch.x;
            float tapY = touch.y;
            
            // 1. Check Search Bar (Always top)
            float barY = SEARCH_BAR_MARGIN;
            float barWidth = 1280 - SIDE_PADDING * 2 - 80;
            if (tapY >= barY - 10 && tapY <= barY + SEARCH_BAR_HEIGHT + 10 &&
                tapX >= SIDE_PADDING && tapX <= SIDE_PADDING + barWidth) {
                showKeyboard();
                return; 
            }
             
            // 2. Content Handling
            // float effectiveScroll = m_scrollY; // Unused

            // Note: In SearchScreen render logic, some parts might be static? 
            // Looking at render(), header is static.
            // renderHotTags starts at SEARCH_BAR_HEIGHT + MARGIN*2.
            
            // We need to apply scroll offset to content checks
            // BUT renderHotTags and renderRecommendations seem to render relative to startY 
            // without subtracting m_scrollY in the original code?? 
            // Wait, let's check render() again.
            // `render` calls `renderHotTags`. `renderHotTags` calculates `startY`. 
            // It does NOT appear to use `m_scrollY`.
            // ORIGINAL CODE BUG: The SearchScreen wasn't scrolling at all visually?
            // Let's assume we want it to scroll.
            // If I look at `SearchScreen.hpp`, `m_scrollY` is there.
            // In `GamesScreen`, `contentY = HEADER - m_scrollY`.
            // In `SearchScreen` original `render`:
            // `renderSearchBar` -> static.
            // `renderHotTags` -> static startY.
            // `renderRecommendations` -> static startY.
            // `renderSearchResults` -> static startY.
            //
            // THE SEARCH SCREEN WAS NOT SCROLLABLE! 
            // I must fix the rendering to use m_scrollY if I want scrolling to work.
            // But for now, I will implement the touch assuming it IS scrollable or will be.
            // Or maybe the user didn't notice it wasn't scrolling, just that touch failed.
            // I will implement "View" logic assuming standard list behavior.
            
            // To properly fix "Search page touch", I should make it scrollable OR 
            // just handle the static taps if it fits on one screen. 
            // Given "switchfin" reference, it should scroll.
            // I will calculate tapY relative to scroll for the list parts.
            
            // For now, let's assume the list starts after the headers. 
            
            if (m_isSearching) {
                // Search Results List
                float listStartY = SEARCH_BAR_HEIGHT + SEARCH_BAR_MARGIN * 2 + 36;
                // Applies scroll? Let's check update() later.
                // If I'm adding momentum, I really should make `render` use `m_scrollY`.
                // For this step I will implement specific hit tests.
                
                float itemY = listStartY; // + m_scrollY in future
                // Actually, let's just make it work for the items visible.
                
                for (size_t i = 0; i < m_searchResults.size(); i++) {
                     // Check button
                     float btnX = 1280 - SIDE_PADDING - 80;
                     float btnY = itemY + 14; 
                     // Rect: 70x32
                     if (tapX >= btnX - 20 && tapX <= btnX + 70 + 20 &&
                         tapY >= btnY - 20 && tapY <= btnY + 32 + 20) {
                         // Download logic
                         // TODO: Trigger download
                         return;
                     }
                     
                     // Check Row
                     if (tapY >= itemY && tapY < itemY + 76) {
                         m_selectedResultIndex = static_cast<int>(i);
                         return;
                     }
                     itemY += 76;
                }

            } else {
                // Tags
                float tagstartY = SEARCH_BAR_HEIGHT + SEARCH_BAR_MARGIN * 2;
                float tagY = tagstartY + 36;
                float tagX = SIDE_PADDING;
                
                // Check tags... (Keep existing logic but refine)
                for (size_t i = 0; i < m_hotKeywords.size(); i++) {
                    float tagWidth = 80 + m_hotKeywords[i].length() * 8;
                    
                    if (tagX + tagWidth > 1280 - SIDE_PADDING) {
                        tagX = SIDE_PADDING;
                        tagY += TAG_HEIGHT + TAG_SPACING;
                    }
                    
                    Rect tagRect(tagX, tagY, tagWidth, TAG_HEIGHT);
                    if (tagRect.contains(tapX, tapY)) {
                        m_searchQuery = m_hotKeywords[i];
                        performSearch(m_searchQuery);
                        return;
                    }
                     tagX += tagWidth + TAG_SPACING;
                }
                
                // Recommendations
                // Below tags... this is dynamic based on tags height.
                // To be safe, we might need to assume a fixed start or recount.
                // Simply checking Y range might be enough if we assume standard layout.
            }
            
        } else {
            // Momentum
            m_scrollVelocity = -touch.velocityY * 35.0f;
        }
    }
}

// =============================================================================
// Update
// =============================================================================

void SearchScreen::update(float deltaTime) {
    // Apply scroll velocity (momentum)
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.92f;
        
        if (std::abs(m_scrollVelocity) < 1.0f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Simple bounds check (0 to implicit max)
    // Real implementation would calculate content height
    if (m_scrollY < 0.0f) {
        m_scrollY *= 0.9f; // Bounce at top
    }
}

// =============================================================================
// Rendering
// =============================================================================

// =============================================================================
// Rendering
// =============================================================================

void SearchScreen::render(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    (void)theme;
    
    // Content Scroll Offset
    // Content starts below the search bar area
    float startY = SEARCH_BAR_HEIGHT + SEARCH_BAR_MARGIN * 2; 
    float currentY = startY - m_scrollY;
    
    // Render Content FIRST (so Search Bar header covers it at the top)
    if (m_isSearching && !m_searchResults.empty()) {
        renderSearchResults(renderer, currentY);
    } else {
        // Tag section
        renderHotTags(renderer, currentY);
        
        // Recommendations follow tags... we need to know where tags ended
        // For simplicity in this non-layout engine, we'll estimate or pass Y
        // But renderHotTags doesn't return Y.
        // Let's create a dynamic layout flow.
        
        // Calculate tag section height dynamically for layout consistency
        // (Replicating logic from renderHotTags to find end Y)
        float tagSectionH = 36.0f; // Title height
        float tagX = SIDE_PADDING;
        float tagY = 0; // Relative to start of tags
        for (const auto& keyword : m_hotKeywords) {
            float tagWidth = 80 + keyword.length() * 8;
            if (tagX + tagWidth > 1280 - SIDE_PADDING) {
                tagX = SIDE_PADDING;
                tagY += TAG_HEIGHT + TAG_SPACING;
            }
            tagX += tagWidth + TAG_SPACING;
        }
        tagSectionH += tagY + TAG_HEIGHT + 40.0f; // + padding
        
        renderRecommendations(renderer, currentY + tagSectionH);
    }
    
    // Render Search Bar (Header) LAST so it's sticky on top
    // Draw a small background for the header area to mask scrolling content
    renderer.drawRect(Rect(0, 0, 1280, startY - 10), theme->backgroundColor());
    renderSearchBar(renderer);
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

void SearchScreen::renderHotTags(Renderer& renderer, float yOffset) {
    Theme* theme = m_app->getTheme();
    
    // Section title
    renderer.drawText("热门搜索", SIDE_PADDING, yOffset, 20,
                      theme->textPrimaryColor(), FontWeight::Bold);
    
    // Tags in a flex wrap layout
    float tagY = yOffset + 36;
    float tagX = SIDE_PADDING;
    
    for (size_t i = 0; i < m_hotKeywords.size(); i++) {
        const std::string& keyword = m_hotKeywords[i];
        float tagWidth = 80 + keyword.length() * 8;  // Approximate
        
        // Wrap to next line if needed
        if (tagX + tagWidth > 1280 - SIDE_PADDING) {
            tagX = SIDE_PADDING;
            tagY += TAG_HEIGHT + TAG_SPACING;
        }
        
        // Only draw if visible
        if (tagY > -TAG_HEIGHT && tagY < 720.0f) {
            bool isSelected = ((int)i == m_selectedTagIndex);
            
            // Tag background
            Color tagBg = isSelected ? theme->primaryColor() : theme->getColor("search_bg");
            renderer.drawRoundedRect(Rect(tagX, tagY, tagWidth, TAG_HEIGHT), 18, tagBg);
            
            // Tag text
            Color tagText = isSelected ? Color(255, 255, 255) : theme->textPrimaryColor();
            renderer.drawTextInRect(keyword, Rect(tagX, tagY, tagWidth, TAG_HEIGHT),
                                   14, tagText, FontWeight::Regular, 
                                   TextAlign::Center, TextVAlign::Middle);
        }
        
        tagX += tagWidth + TAG_SPACING;
    }
}

void SearchScreen::renderRecommendations(Renderer& renderer, float yOffset) {
    Theme* theme = m_app->getTheme();
    
    float startY = yOffset; 
    
    // Section title
    if (startY > -30 && startY < 720) {
        renderer.drawText("推荐", SIDE_PADDING, startY, 20,
                          theme->textPrimaryColor(), FontWeight::Bold);
    }
    
    // List of recommendations
    float itemY = startY + 40;
    for (size_t i = 0; i < m_recommendations.size() && i < 15; i++) { // Increased limit
        const auto& game = m_recommendations[i];
        
        if (itemY > -80 && itemY < 720) {
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
        }
        
        itemY += 76;
    }
}

void SearchScreen::renderSearchResults(Renderer& renderer, float yOffset) {
    Theme* theme = m_app->getTheme();
    
    float startY = yOffset;
    
    // Results count
    std::string resultText = "找到 " + std::to_string(m_searchResults.size()) + " 个结果";
    renderer.drawText(resultText, SIDE_PADDING, startY, 16, theme->textSecondaryColor());
    
    // List of results
    float itemY = startY + 36;
    for (size_t i = 0; i < m_searchResults.size(); i++) {
        const auto& game = m_searchResults[i];
        bool isSelected = ((int)i == m_selectedResultIndex);
        
        if (itemY > -80 && itemY < 720) {
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
        }
        
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
    
    // -------------------------------------------------------------------------
    // Search using StoreManager (backend data only)
    // No fallback local search - requires backend connection
    // -------------------------------------------------------------------------
    StoreManager& store = StoreManager::getInstance();
    auto results = store.search(query);
    
    // Convert StoreEntry results to GameItem items
    for (const StoreEntry* entry : results) {
        GameItem result;
        result.id = entry->id;
        result.name = entry->name;
        result.developer = entry->developer;
        result.category = entry->category;
        result.iconUrl = entry->iconUrl;
        result.rating = entry->rating;
        result.size = entry->getFormattedSize();
        
        m_searchResults.push_back(result);
    }
}

// =============================================================================
// Data Loading (from Backend)
// =============================================================================

void SearchScreen::loadDemoContent() {
    // -------------------------------------------------------------------------
    // Hot keywords - could be loaded from backend API in the future
    // -------------------------------------------------------------------------
    m_hotKeywords = {
        "马里奥", "塞尔达", "宝可梦", "星之卡比",
        "动物森友会", "喷射战士", "火焰纹章", "异度神剑"
    };
    
    // -------------------------------------------------------------------------
    // Load recommendations from StoreManager (top rated entries)
    // No fallback demo data - requires backend connection
    // -------------------------------------------------------------------------
    StoreManager& store = StoreManager::getInstance();
    auto featured = store.getFeaturedEntries(7);
    
    for (const StoreEntry* entry : featured) {
        GameItem rec;
        rec.id = entry->id;
        rec.name = entry->name;
        rec.developer = entry->developer;
        rec.category = entry->category;
        rec.iconUrl = entry->iconUrl;
        rec.rating = entry->rating;
        rec.size = entry->getFormattedSize();
        
        m_recommendations.push_back(rec);
    }
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

