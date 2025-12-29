// =============================================================================
// Switch App Store - Emulators Screen
// =============================================================================
// Displays retro gaming emulators for the Nintendo Switch
// =============================================================================

#pragma once

#include "Screen.hpp"
#include "core/Renderer.hpp"
#include <vector>
#include <string>

// Forward declaration
class App;

// =============================================================================
// Emulator item data structure
// =============================================================================
struct EmulatorItem {
    std::string id;
    std::string name;
    std::string developer;
    std::string systems;       // Supported systems (e.g., "GBA, NES, SNES")
    std::string iconUrl;
    std::string version;
    std::string size;
    Color themeColor;          // Brand color for the emulator
    bool isInstalled = false;
};

// =============================================================================
// EmulatorsScreen - Retro gaming emulators
// =============================================================================
class EmulatorsScreen : public Screen {
public:
    explicit EmulatorsScreen(App* app);
    ~EmulatorsScreen() override;
    
    // Lifecycle
    void onEnter() override;
    void onExit() override;
    void onResolutionChanged(int width, int height, float scale) override;
    
    // Update and render
    void handleInput(const Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer) override;
    
private:
    // Rendering helpers
    void renderHeader(Renderer& renderer);
    void renderEmulatorGrid(Renderer& renderer);
    void renderEmulatorCard(Renderer& renderer, const EmulatorItem& emu,
                            float x, float y, bool isSelected);
    
    // Demo content
    void loadDemoContent();
    
    // Data
    std::vector<EmulatorItem> m_emulators;
    
    // State
    int m_selectedIndex = 0;
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    float m_maxScrollY = 0.0f;
    
    // Layout constants
    static constexpr float HEADER_HEIGHT = 70.0f;
    static constexpr float SIDE_PADDING = 20.0f;
    static constexpr float TAB_BAR_HEIGHT = 70.0f;
    static constexpr float CARD_WIDTH = 400.0f;
    static constexpr float CARD_HEIGHT = 120.0f;
    static constexpr float CARD_SPACING = 16.0f;
    static constexpr float CARD_RADIUS = 16.0f;
    static constexpr int COLUMNS = 3;
};
