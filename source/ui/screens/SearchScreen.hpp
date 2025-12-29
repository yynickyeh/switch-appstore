// =============================================================================
// Switch App Store - Search Screen
// =============================================================================
// Search page with search bar, hot keywords, and search results
// Inspired by Apple App Store's Search tab
// =============================================================================

#pragma once

#include "Screen.hpp"
#include "core/Renderer.hpp"
#include "GamesScreen.hpp"  // For GameItem
#include <vector>
#include <string>

// Forward declarations
class App;

// =============================================================================
// SearchScreen - Search and discovery screen
// =============================================================================
class SearchScreen : public Screen {
public:
    explicit SearchScreen(App* app);
    ~SearchScreen() override;
    
    // -------------------------------------------------------------------------
    // Screen lifecycle
    // -------------------------------------------------------------------------
    void onEnter() override;
    void onExit() override;
    void onResolutionChanged(int width, int height, float scale) override;
    
    // -------------------------------------------------------------------------
    // Update and render
    // -------------------------------------------------------------------------
    void handleInput(const Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer) override;
    
private:
    // -------------------------------------------------------------------------
    // Layout constants (720p base)
    // -------------------------------------------------------------------------
    static constexpr float SEARCH_BAR_HEIGHT = 44.0f;
    static constexpr float SEARCH_BAR_MARGIN = 16.0f;
    static constexpr float TAB_BAR_HEIGHT = 70.0f;
    static constexpr float SIDE_PADDING = 20.0f;
    static constexpr float TAG_HEIGHT = 36.0f;
    static constexpr float TAG_SPACING = 10.0f;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    void renderSearchBar(Renderer& renderer);
    void renderHotTags(Renderer& renderer);
    void renderRecommendations(Renderer& renderer);
    void renderSearchResults(Renderer& renderer);
    void performSearch(const std::string& query);
    void loadDemoContent();
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    // Search state
    std::string m_searchQuery;
    bool m_isSearching = false;
    bool m_searchBarFocused = false;
    
    // Hot keywords
    std::vector<std::string> m_hotKeywords;
    
    // Search results
    std::vector<GameItem> m_searchResults;
    
    // Recommendations (shown when no search)
    std::vector<GameItem> m_recommendations;
    
    // Selection
    int m_selectedTagIndex = -1;
    int m_selectedResultIndex = 0;
    
    // Scroll
    float m_scrollY = 0.0f;
};
