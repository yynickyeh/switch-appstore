// =============================================================================
// Switch App Store - Today Screen
// =============================================================================
// The main "Today" tab, featuring large cards and featured content
// Inspired by Apple App Store's Today tab
// =============================================================================

#pragma once

#include "Screen.hpp"
#include "core/Renderer.hpp"
#include <string>
#include <vector>

// Forward declaration
class App;

// =============================================================================
// Featured Card Data
// =============================================================================
struct FeaturedCard {
    std::string tag;           // e.g., "GAME OF THE DAY"
    std::string title;         // Main title
    std::string subtitle;      // Description
    std::string gameId;        // Game ID for navigation
    Color backgroundColor;     // Card background color
    // SDL_Texture* backgroundImage;  // Background image (loaded async)
};

// =============================================================================
// TodayScreen - Main Today tab view
// =============================================================================
class TodayScreen : public Screen {
public:
    explicit TodayScreen(App* app);
    ~TodayScreen() override;
    
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
    // Layout constants (720p base, scaled for 1080p)
    // -------------------------------------------------------------------------
    static constexpr float HEADER_HEIGHT = 60.0f;
    static constexpr float TAB_BAR_HEIGHT = 70.0f;
    static constexpr float SIDE_PADDING = 40.0f;
    static constexpr float CARD_SPACING = 20.0f;
    static constexpr float LARGE_CARD_HEIGHT = 400.0f;
    static constexpr float SMALL_CARD_HEIGHT = 200.0f;
    static constexpr float CARD_RADIUS = 16.0f;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Render the header (date and title)
    void renderHeader(Renderer& renderer);
    
    // Render the bottom tab bar
    void renderTabBar(Renderer& renderer);
    
    // Render a large featured card
    void renderLargeCard(Renderer& renderer, const FeaturedCard& card, 
                         float x, float y, float width, float height);
    
    // Render small cards row
    void renderSmallCards(Renderer& renderer, float y);
    
    // Load demo content (placeholder until API is connected)
    void loadDemoContent();
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    // Content data
    std::vector<FeaturedCard> m_featuredCards;
    
    // Scroll state
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    float m_maxScrollY = 0.0f;
    
    // Selected index (for controller navigation)
    int m_selectedIndex = 0;
    
    // Animation state
    float m_headerOpacity = 1.0f;
    
    // Current date string
    std::string m_dateString;
};
