// =============================================================================
// Switch App Store - Tools Screen
// =============================================================================
// Displays homebrew tools and utilities for the Nintendo Switch
// =============================================================================

#pragma once

#include "Screen.hpp"
#include "core/Renderer.hpp"
#include <vector>
#include <string>

// Forward declaration
class App;

// =============================================================================
// Tool item data structure
// =============================================================================
struct ToolItem {
    std::string id;
    std::string name;
    std::string developer;
    std::string description;
    std::string iconUrl;
    std::string version;
    std::string size;
    bool isInstalled = false;
};

// =============================================================================
// ToolsScreen - Homebrew tools and utilities
// =============================================================================
class ToolsScreen : public Screen {
public:
    explicit ToolsScreen(App* app);
    ~ToolsScreen() override;
    
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
    void renderToolsList(Renderer& renderer);
    void renderToolItem(Renderer& renderer, const ToolItem& tool, 
                        float y, bool isSelected);
    
    // Demo content
    void loadDemoContent();
    
    // Data
    std::vector<ToolItem> m_tools;
    
    // State
    int m_selectedIndex = 0;
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    float m_maxScrollY = 0.0f;
    
    // Layout constants
    static constexpr float HEADER_HEIGHT = 70.0f;
    static constexpr float ITEM_HEIGHT = 88.0f;
    static constexpr float SIDE_PADDING = 20.0f;
    static constexpr float TAB_BAR_HEIGHT = 70.0f;
};
