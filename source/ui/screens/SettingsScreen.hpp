// =============================================================================
// Switch App Store - Settings Screen
// =============================================================================
// Settings page with theme toggle, language selection, download options
// iOS style grouped settings with toggle switches
// =============================================================================

#pragma once

#include "Screen.hpp"
#include <string>
#include <vector>
#include <functional>

// Forward declarations
class App;

// =============================================================================
// Setting item types
// =============================================================================
enum class SettingItemType {
    Toggle,     // On/Off switch
    Choice,     // Selection from list
    Action,     // Tappable action
    Info,       // Display only
    Slider      // Value slider
};

// =============================================================================
// Setting item data
// =============================================================================
struct SettingItem {
    std::string id;
    std::string title;
    std::string subtitle;
    SettingItemType type;
    
    // For toggle
    bool toggleValue = false;
    
    // For choice
    std::vector<std::string> choices;
    int selectedChoice = 0;
    
    // For slider
    float sliderValue = 0.5f;
    float sliderMin = 0.0f;
    float sliderMax = 1.0f;
    
    // Callbacks
    std::function<void(bool)> onToggle;
    std::function<void(int)> onChoice;
    std::function<void()> onAction;
    std::function<void(float)> onSlider;
};

// =============================================================================
// Setting section
// =============================================================================
struct SettingSection {
    std::string title;
    std::vector<SettingItem> items;
};

// =============================================================================
// SettingsScreen - Settings page
// =============================================================================
class SettingsScreen : public Screen {
public:
    explicit SettingsScreen(App* app);
    ~SettingsScreen() override;
    
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
    static constexpr float SIDE_PADDING = 16.0f;
    static constexpr float SECTION_HEADER_HEIGHT = 40.0f;
    static constexpr float ITEM_HEIGHT = 50.0f;
    static constexpr float SECTION_SPACING = 24.0f;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    void setupSettings();
    void renderHeader(Renderer& renderer);
    void renderSection(Renderer& renderer, const SettingSection& section, float& yOffset);
    void renderItem(Renderer& renderer, const SettingItem& item, float y, bool isSelected);
    void renderToggle(Renderer& renderer, bool value, float x, float y, bool focused);
    void applySettings();
    void loadCurrentValues();
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    std::vector<SettingSection> m_sections;
    
    // Navigation
    int m_selectedSection = 0;
    int m_selectedItem = 0;
    
    // Scroll
    float m_scrollY = 0.0f;
    float m_scrollVelocity = 0.0f;
    float m_contentHeight = 0.0f;
    
    // Animation
    float m_toggleAnimProgress = 0.0f;
};
