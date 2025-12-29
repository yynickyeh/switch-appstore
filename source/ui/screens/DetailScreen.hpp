// =============================================================================
// Switch App Store - Detail Screen
// =============================================================================
// Game detail page showing full info, screenshots, description, and install
// Inspired by Apple App Store's app detail page
// =============================================================================

#pragma once

#include "Screen.hpp"
#include "GamesScreen.hpp"  // For GameItem
#include "core/Renderer.hpp"
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class App;

// =============================================================================
// Extended game info for detail page
// =============================================================================
struct GameDetail {
    GameItem basic;
    std::string description;
    std::vector<std::string> screenshots;
    std::string version;
    std::string releaseDate;
    std::string titleId;
    std::vector<std::string> languages;
    std::string players;
    float downloadProgress = 0.0f;  // 0.0 = not started, 1.0 = complete
    bool isInstalled = false;
    bool isDownloading = false;
};

// =============================================================================
// DetailScreen - Game detail and install page
// =============================================================================
class DetailScreen : public Screen {
public:
    DetailScreen(App* app, const GameItem& game);
    ~DetailScreen() override;
    
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
    static constexpr float NAV_BAR_HEIGHT = 44.0f;
    static constexpr float SIDE_PADDING = 20.0f;
    static constexpr float ICON_SIZE = 128.0f;
    static constexpr float ICON_RADIUS = 28.0f;
    static constexpr float SCREENSHOT_WIDTH = 300.0f;
    static constexpr float SCREENSHOT_HEIGHT = 169.0f;
    static constexpr float SECTION_SPACING = 24.0f;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    void renderNavBar(Renderer& renderer);
    void renderHeader(Renderer& renderer, float& yOffset);
    void renderScreenshots(Renderer& renderer, float& yOffset);
    void renderDescription(Renderer& renderer, float& yOffset);
    void renderInfo(Renderer& renderer, float& yOffset);
    void renderInstallButton(Renderer& renderer);
    
    void startDownload();
    void loadGameDetail(const GameItem& game);
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    GameDetail m_detail;
    
    // Scroll state
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    
    // Screenshot scroll
    float m_screenshotScrollX = 0.0f;
    int m_selectedScreenshot = 0;
    
    // UI state
    bool m_showFullDescription = false;
    bool m_installButtonFocused = false;
    
    // Callback for navigation
    std::function<void()> m_onBack;
};
