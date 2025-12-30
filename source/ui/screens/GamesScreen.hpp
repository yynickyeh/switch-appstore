// =============================================================================
// Switch App Store - Games Screen
// =============================================================================
// The "Games" tab showing game categories in horizontal scrolling lists
// Inspired by Apple App Store's Games tab
// =============================================================================

#pragma once

#include "Screen.hpp"
#include "core/Renderer.hpp"
#include <vector>
#include <string>
#include <memory>

// Forward declarations
class App;
class ScrollView;
class HorizontalList;
class Card;

// =============================================================================
// GameItem - Data for a single game
// =============================================================================
struct GameItem {
    std::string id;
    std::string name;
    std::string developer;
    std::string category;
    std::string iconUrl;
    float rating = 0.0f;
    std::string size;
    int downloadCount = 0;  // Download count from server
};

// =============================================================================
// InstalledGameItem - Data for an installed game
// =============================================================================
struct InstalledGameItem {
    uint64_t titleId;
    std::string name;
    std::string author;
    std::string version;
    SDL_Texture* icon = nullptr;
};

// =============================================================================
// GameCategory - A category section with games
// =============================================================================
struct GameCategory {
    std::string title;
    std::vector<GameItem> games;
};

// =============================================================================
// GamesScreen - Game browsing screen
// =============================================================================
class GamesScreen : public Screen {
public:
    explicit GamesScreen(App* app);
    ~GamesScreen() override;
    
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
    static constexpr float HEADER_HEIGHT = 60.0f;
    static constexpr float TAB_BAR_HEIGHT = 70.0f;
    static constexpr float SIDE_PADDING = 40.0f;
    static constexpr float SECTION_SPACING = 30.0f;
    static constexpr float GAME_CARD_SIZE = 180.0f;
    static constexpr float CARD_SPACING = 16.0f;
    static constexpr float ICON_RADIUS = 22.0f;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    void renderHeader(Renderer& renderer);
    void renderCategory(Renderer& renderer, const GameCategory& category, 
                        float& yOffset, int categoryIndex);
    void renderGameCard(Renderer& renderer, const GameItem& game, 
                        float x, float y, bool isSelected);
    void loadDemoContent();
    
    // Installed games
    void loadInstalledGames(Renderer& renderer);
    void renderInstalledSection(Renderer& renderer, float& yOffset);
    void deleteSelectedGame();
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    // Content
    std::vector<GameCategory> m_categories;
    
    // Scroll state
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    
    // Selection (for controller)
    int m_selectedCategory = 0;
    int m_selectedGame = 0;
    
    // Horizontal scroll per category
    std::vector<float> m_categoryScrollX;
    
    // Installed games
    std::vector<InstalledGameItem> m_installedGames;
    int m_selectedInstalledGame = 0;
    bool m_showingInstalled = false;  // Toggle between store/installed view
    bool m_installedLoaded = false;
};
