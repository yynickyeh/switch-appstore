// =============================================================================
// Switch App Store - Emulators Screen Implementation
// =============================================================================

#include "EmulatorsScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

EmulatorsScreen::EmulatorsScreen(App* app)
    : Screen(app)
{
    loadDemoContent();
}

EmulatorsScreen::~EmulatorsScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void EmulatorsScreen::onEnter() {
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
    m_selectedIndex = 0;
}

void EmulatorsScreen::onExit() {
    // Nothing to clean up
}

void EmulatorsScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate max scroll
    int rows = (m_emulators.size() + COLUMNS - 1) / COLUMNS;
    m_maxScrollY = std::max(0.0f, rows * (CARD_HEIGHT + CARD_SPACING) - 
                            (720.0f - HEADER_HEIGHT - TAB_BAR_HEIGHT - 40));
}

// =============================================================================
// Input Handling
// =============================================================================

void EmulatorsScreen::handleInput(const Input& input) {
    int cols = COLUMNS;
    int total = static_cast<int>(m_emulators.size());
    
    // D-pad navigation (grid)
    if (input.isPressed(Input::Button::DPadUp)) {
        if (m_selectedIndex >= cols) {
            m_selectedIndex -= cols;
        }
    }
    if (input.isPressed(Input::Button::DPadDown)) {
        if (m_selectedIndex + cols < total) {
            m_selectedIndex += cols;
        }
    }
    if (input.isPressed(Input::Button::DPadLeft)) {
        if (m_selectedIndex > 0) {
            m_selectedIndex--;
        }
    }
    if (input.isPressed(Input::Button::DPadRight)) {
        if (m_selectedIndex < total - 1) {
            m_selectedIndex++;
        }
    }
    
    // A button to download/open
    if (input.isPressed(Input::Button::A)) {
        // TODO: Download or open the selected emulator
    }
    
    // Analog stick scrolling
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 500.0f;
    }
    
    // Touch scrolling - direct 1:1 mapping
    const auto& touch = input.getTouch();
    if (touch.touching) {
        m_scrollY -= touch.deltaY;
        m_scrollVelocity = 0.0f;
    } else if (touch.justReleased) {
        m_scrollVelocity = -touch.velocityY * 30.0f;
    }
}

// =============================================================================
// Update
// =============================================================================

void EmulatorsScreen::update(float deltaTime) {
    // Apply scroll velocity (momentum)
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.92f;
        
        if (std::abs(m_scrollVelocity) < 1.0f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll bounds with bounce
    if (m_scrollY < 0.0f) {
        m_scrollY *= 0.9f;
    }
    if (m_scrollY > m_maxScrollY) {
        m_scrollY = m_maxScrollY + (m_scrollY - m_maxScrollY) * 0.9f;
    }
}

// =============================================================================
// Rendering
// =============================================================================

void EmulatorsScreen::render(Renderer& renderer) {
    renderEmulatorGrid(renderer);
    renderHeader(renderer);
}

void EmulatorsScreen::renderHeader(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Background
    Color bgColor = theme->backgroundColor();
    bgColor.a = 240;
    renderer.drawRect(Rect(0, 0, 1280, HEADER_HEIGHT), bgColor);
    
    // Title
    renderer.drawText("模拟器", SIDE_PADDING, 20, 34, 
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Subtitle
    renderer.drawText("经典游戏模拟器 - 重温复古游戏", SIDE_PADDING, 54, 14,
                     theme->textSecondaryColor());
    
    // Separator
    renderer.drawLine(0, HEADER_HEIGHT, 1280, HEADER_HEIGHT, 
                     theme->separatorColor(), 1);
}

void EmulatorsScreen::renderEmulatorGrid(Renderer& renderer) {
    float contentY = HEADER_HEIGHT + 20 - m_scrollY;
    float screenHeight = 720.0f - TAB_BAR_HEIGHT;
    
    // Calculate card width based on screen width with padding
    float totalWidth = 1280 - SIDE_PADDING * 2;
    float cardWidth = (totalWidth - CARD_SPACING * (COLUMNS - 1)) / COLUMNS;
    
    for (size_t i = 0; i < m_emulators.size(); i++) {
        int row = i / COLUMNS;
        int col = i % COLUMNS;
        
        float cardX = SIDE_PADDING + col * (cardWidth + CARD_SPACING);
        float cardY = contentY + row * (CARD_HEIGHT + CARD_SPACING);
        
        // Only render visible items
        if (cardY > -CARD_HEIGHT && cardY < screenHeight) {
            bool isSelected = (static_cast<int>(i) == m_selectedIndex);
            renderEmulatorCard(renderer, m_emulators[i], cardX, cardY, isSelected);
        }
    }
}

void EmulatorsScreen::renderEmulatorCard(Renderer& renderer, const EmulatorItem& emu,
                                          float x, float y, bool isSelected) {
    Theme* theme = m_app->getTheme();
    float cardWidth = (1280 - SIDE_PADDING * 2 - CARD_SPACING * (COLUMNS - 1)) / COLUMNS;
    
    // Card shadow
    renderer.drawShadow(Rect(x, y, cardWidth, CARD_HEIGHT), CARD_RADIUS, 
                       8, 0, 4, theme->getColor("shadow"));
    
    // Card background with emulator brand color
    renderer.drawRoundedRect(Rect(x, y, cardWidth, CARD_HEIGHT), CARD_RADIUS, 
                            emu.themeColor);
    
    // Selection outline
    if (isSelected) {
        renderer.drawRoundedRectOutline(
            Rect(x - 3, y - 3, cardWidth + 6, CARD_HEIGHT + 6),
            CARD_RADIUS + 3, theme->primaryColor(), 3
        );
    }
    
    // Emulator name
    renderer.drawText(emu.name, x + 16, y + 16, 20,
                     Color(255, 255, 255), FontWeight::Bold);
    
    // Supported systems
    renderer.drawText(emu.systems, x + 16, y + 44, 13,
                     Color(255, 255, 255, 200));
    
    // Version and developer
    std::string info = "v" + emu.version + " · " + emu.developer;
    renderer.drawText(info, x + 16, y + 68, 11,
                     Color(255, 255, 255, 160));
    
    // Install status badge
    float badgeX = x + cardWidth - 60;
    float badgeY = y + 16;
    if (emu.isInstalled) {
        renderer.drawRoundedRect(Rect(badgeX, badgeY, 44, 22), 11,
                                Color(255, 255, 255, 50));
        renderer.drawTextInRect("已装", Rect(badgeX, badgeY, 44, 22),
                               11, Color(255, 255, 255), FontWeight::Regular,
                               TextAlign::Center, TextVAlign::Middle);
    }
}

// =============================================================================
// Demo Content
// =============================================================================

void EmulatorsScreen::loadDemoContent() {
    m_emulators = {
        {"1", "RetroArch", "libretro", "多合一模拟器 (NES, SNES, GBA, PS1等)", 
         "", "1.16.0", "180MB", Color::fromHex(0x2D2D2D), true},
        
        {"2", "mGBA", "endrift", "GBA, GB, GBC",
         "", "0.10.3", "4.2MB", Color::fromHex(0x6B8E23), true},
        
        {"3", "melonDS", "Arisotura", "Nintendo DS",
         "", "0.9.5", "3.8MB", Color::fromHex(0xE91E63), false},
        
        {"4", "PPSSPP", "Henrik Rydgård", "PlayStation Portable",
         "", "1.16.6", "35MB", Color::fromHex(0x2196F3), true},
        
        {"5", "Dolphin", "Dolphin Team", "GameCube, Wii",
         "", "5.0", "45MB", Color::fromHex(0x03A9F4), false},
        
        {"6", "Citra", "Citra Team", "Nintendo 3DS",
         "", "2088", "28MB", Color::fromHex(0xF44336), false},
        
        {"7", "DuckStation", "stenzek", "PlayStation 1",
         "", "0.1", "12MB", Color::fromHex(0x9C27B0), true},
        
        {"8", "SNES9x", "snes9x team", "Super Nintendo",
         "", "1.62.3", "2.1MB", Color::fromHex(0x673AB7), true},
        
        {"9", "Mupen64Plus", "mupen64plus", "Nintendo 64",
         "", "2.5.9", "8.5MB", Color::fromHex(0x4CAF50), false},
        
        {"10", "PCSX2", "PCSX2 Team", "PlayStation 2",
         "", "1.7.5", "65MB", Color::fromHex(0x3F51B5), false},
        
        {"11", "Flycast", "flyinghead", "Dreamcast, NAOMI",
         "", "2.3", "15MB", Color::fromHex(0xFF5722), false},
        
        {"12", "ScummVM", "ScummVM Team", "经典冒险游戏",
         "", "2.8.0", "42MB", Color::fromHex(0x795548), true},
    };
    
    // Calculate max scroll
    int rows = (m_emulators.size() + COLUMNS - 1) / COLUMNS;
    m_maxScrollY = std::max(0.0f, rows * (CARD_HEIGHT + CARD_SPACING) - 
                            (720.0f - HEADER_HEIGHT - TAB_BAR_HEIGHT - 40));
}
