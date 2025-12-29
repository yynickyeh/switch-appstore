// =============================================================================
// Switch App Store - Detail Screen Implementation
// =============================================================================

#include "DetailScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include <algorithm>
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

DetailScreen::DetailScreen(App* app, const GameItem& game)
    : Screen(app)
{
    loadGameDetail(game);
}

DetailScreen::~DetailScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void DetailScreen::onEnter() {
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
    m_screenshotScrollX = 0.0f;
}

void DetailScreen::onExit() {
    // Nothing to clean up
}

void DetailScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate layout
}

// =============================================================================
// Input Handling
// =============================================================================

void DetailScreen::handleInput(const Input& input) {
    // B button to go back
    if (input.isPressed(Input::Button::B)) {
        if (m_onBack) {
            m_onBack();
        }
        return;
    }
    
    // Vertical scrolling
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 500.0f;
    }
    
    // Screenshot scrolling with L/R
    if (input.isPressed(Input::Button::L)) {
        if (m_selectedScreenshot > 0) {
            m_selectedScreenshot--;
        }
    }
    if (input.isPressed(Input::Button::R)) {
        if (m_selectedScreenshot < (int)m_detail.screenshots.size() - 1) {
            m_selectedScreenshot++;
        }
    }
    
    // A button to install/download
    if (input.isPressed(Input::Button::A)) {
        if (!m_detail.isInstalled && !m_detail.isDownloading) {
            startDownload();
        }
    }
    
    // Touch scrolling
    const auto& touch = input.getTouch();
    if (touch.touching) {
        m_scrollVelocity = -touch.deltaY;
    }
}

// =============================================================================
// Update
// =============================================================================

void DetailScreen::update(float deltaTime) {
    // Apply scroll velocity
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.95f;
        
        if (std::abs(m_scrollVelocity) < 0.5f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll
    m_scrollY = std::max(0.0f, m_scrollY);
    
    // Animate screenshot scroll
    float targetScrollX = m_selectedScreenshot * (SCREENSHOT_WIDTH + 16);
    m_screenshotScrollX += (targetScrollX - m_screenshotScrollX) * deltaTime * 8.0f;
    
    // Simulate download progress
    if (m_detail.isDownloading) {
        m_detail.downloadProgress += deltaTime * 0.1f;  // 10 seconds for demo
        if (m_detail.downloadProgress >= 1.0f) {
            m_detail.downloadProgress = 1.0f;
            m_detail.isDownloading = false;
            m_detail.isInstalled = true;
        }
    }
}

// =============================================================================
// Rendering
// =============================================================================

void DetailScreen::render(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float yOffset = NAV_BAR_HEIGHT + 20 - m_scrollY;
    
    // Render sections
    renderHeader(renderer, yOffset);
    renderScreenshots(renderer, yOffset);
    renderDescription(renderer, yOffset);
    renderInfo(renderer, yOffset);
    
    // Fixed elements
    renderNavBar(renderer);
    renderInstallButton(renderer);
}

void DetailScreen::renderNavBar(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Background
    Color bgColor = theme->backgroundColor();
    bgColor.a = 240;
    renderer.drawRect(Rect(0, 0, 1280, NAV_BAR_HEIGHT), bgColor);
    
    // Back button
    renderer.drawText("< 返回", SIDE_PADDING, 12, 17, theme->primaryColor());
    
    // Separator
    renderer.drawLine(0, NAV_BAR_HEIGHT, 1280, NAV_BAR_HEIGHT,
                     theme->separatorColor(), 1);
}

void DetailScreen::renderHeader(Renderer& renderer, float& yOffset) {
    Theme* theme = m_app->getTheme();
    
    // Game icon
    renderer.drawRoundedRect(
        Rect(SIDE_PADDING, yOffset, ICON_SIZE, ICON_SIZE),
        ICON_RADIUS, Color::fromHex(0xE5E5EA)
    );
    
    float textX = SIDE_PADDING + ICON_SIZE + 16;
    
    // Game name
    renderer.drawText(m_detail.basic.name, textX, yOffset, 24,
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Developer
    renderer.drawText(m_detail.basic.developer, textX, yOffset + 32, 16,
                     theme->textSecondaryColor());
    
    // Rating and size
    std::string ratingStr = "★★★★★ (" + std::to_string(m_detail.basic.rating).substr(0, 3) + ")";
    renderer.drawText(ratingStr, textX, yOffset + 56, 14,
                     theme->getColor("star"));
    
    renderer.drawText(m_detail.basic.size, textX + 140, yOffset + 56, 14,
                     theme->textSecondaryColor());
    
    yOffset += ICON_SIZE + SECTION_SPACING;
}

void DetailScreen::renderScreenshots(Renderer& renderer, float& yOffset) {
    Theme* theme = m_app->getTheme();
    
    // Section title
    renderer.drawText("预览", SIDE_PADDING, yOffset, 20,
                     theme->textPrimaryColor(), FontWeight::Bold);
    yOffset += 32;
    
    // Screenshot carousel
    float startX = SIDE_PADDING - m_screenshotScrollX;
    
    for (size_t i = 0; i < m_detail.screenshots.size(); i++) {
        float x = startX + i * (SCREENSHOT_WIDTH + 16);
        
        // Only render visible screenshots
        if (x > -SCREENSHOT_WIDTH && x < 1280) {
            // Screenshot placeholder
            renderer.drawRoundedRect(
                Rect(x, yOffset, SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT),
                12, Color::fromHex(0xD1D1D6)
            );
            
            // Selection indicator
            if ((int)i == m_selectedScreenshot) {
                renderer.drawRoundedRectOutline(
                    Rect(x - 3, yOffset - 3, SCREENSHOT_WIDTH + 6, SCREENSHOT_HEIGHT + 6),
                    15, theme->primaryColor(), 2
                );
            }
        }
    }
    
    // Page dots
    float dotsY = yOffset + SCREENSHOT_HEIGHT + 12;
    float dotsStartX = 640 - m_detail.screenshots.size() * 6;
    
    for (size_t i = 0; i < m_detail.screenshots.size(); i++) {
        Color dotColor = ((int)i == m_selectedScreenshot) ?
            theme->primaryColor() : theme->textTertiaryColor();
        renderer.drawCircle(dotsStartX + i * 12, dotsY, 4, dotColor);
    }
    
    yOffset += SCREENSHOT_HEIGHT + 36;
}

void DetailScreen::renderDescription(Renderer& renderer, float& yOffset) {
    Theme* theme = m_app->getTheme();
    
    // Section title
    renderer.drawText("描述", SIDE_PADDING, yOffset, 20,
                     theme->textPrimaryColor(), FontWeight::Bold);
    yOffset += 32;
    
    // Description text (truncated unless expanded)
    std::string displayText = m_detail.description;
    if (!m_showFullDescription && displayText.length() > 200) {
        displayText = displayText.substr(0, 200) + "...";
    }
    
    renderer.drawTextInRect(
        displayText,
        Rect(SIDE_PADDING, yOffset, 1280 - SIDE_PADDING * 2, 100),
        15, theme->textPrimaryColor()
    );
    
    yOffset += 80;
    
    // "More" button if truncated
    if (m_detail.description.length() > 200) {
        renderer.drawText(m_showFullDescription ? "收起 ▲" : "更多 ▼",
                         1280 - SIDE_PADDING - 60, yOffset, 14,
                         theme->primaryColor());
    }
    
    yOffset += SECTION_SPACING;
}

void DetailScreen::renderInfo(Renderer& renderer, float& yOffset) {
    Theme* theme = m_app->getTheme();
    
    // Section title
    renderer.drawText("信息", SIDE_PADDING, yOffset, 20,
                     theme->textPrimaryColor(), FontWeight::Bold);
    yOffset += 32;
    
    // Info rows
    struct InfoRow {
        const char* label;
        std::string value;
    };
    
    std::vector<InfoRow> rows = {
        {"开发商", m_detail.basic.developer},
        {"大小", m_detail.basic.size},
        {"类别", m_detail.basic.category},
        {"版本", m_detail.version},
        {"Title ID", m_detail.titleId},
    };
    
    for (const auto& row : rows) {
        renderer.drawText(row.label, SIDE_PADDING, yOffset, 15,
                         theme->textSecondaryColor());
        renderer.drawText(row.value, SIDE_PADDING + 120, yOffset, 15,
                         theme->textPrimaryColor());
        
        yOffset += 28;
        
        // Separator
        renderer.drawLine(SIDE_PADDING, yOffset, 1280 - SIDE_PADDING, yOffset,
                         theme->separatorColor(), 1);
        yOffset += 8;
    }
    
    yOffset += 100;  // Bottom padding for tab bar
}

void DetailScreen::renderInstallButton(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Button position (fixed at bottom right, above tab bar)
    float btnX = 1280 - SIDE_PADDING - 100;
    float btnY = 720 - 70 - 50;  // Above tab bar
    float btnW = 90;
    float btnH = 36;
    
    if (m_detail.isInstalled) {
        // Show "Open" button
        renderer.drawRoundedRect(Rect(btnX, btnY, btnW, btnH), 18,
                                theme->getColor("button_secondary_bg"));
        renderer.drawTextInRect("打开", Rect(btnX, btnY, btnW, btnH), 16,
                               theme->primaryColor(), FontWeight::Semibold,
                               TextAlign::Center, TextVAlign::Middle);
    } else if (m_detail.isDownloading) {
        // Show progress ring
        float centerX = btnX + btnW / 2;
        float centerY = btnY + btnH / 2;
        float radius = 14;
        
        // Background ring
        renderer.drawCircleOutline(centerX, centerY, radius,
                                   theme->getColor("button_secondary_bg"), 3);
        
        // Progress (simplified - just a colored portion)
        // TODO: Implement proper arc drawing
        renderer.drawCircle(centerX, centerY, radius - 4, theme->primaryColor());
        
        // Percentage text
        int percent = static_cast<int>(m_detail.downloadProgress * 100);
        renderer.drawTextInRect(
            std::to_string(percent) + "%",
            Rect(btnX, btnY + btnH + 4, btnW, 20), 12,
            theme->textSecondaryColor(), FontWeight::Regular,
            TextAlign::Center, TextVAlign::Top
        );
    } else {
        // Show "Get" button
        Color btnColor = m_installButtonFocused ?
            Color::fromHex(0x0056B3) : theme->primaryColor();
        renderer.drawRoundedRect(Rect(btnX, btnY, btnW, btnH), 18, btnColor);
        renderer.drawTextInRect("获取", Rect(btnX, btnY, btnW, btnH), 16,
                               Color(255, 255, 255), FontWeight::Semibold,
                               TextAlign::Center, TextVAlign::Middle);
    }
}

// =============================================================================
// Actions
// =============================================================================

void DetailScreen::startDownload() {
    m_detail.isDownloading = true;
    m_detail.downloadProgress = 0.0f;
    // TODO: Start actual download via Downloader
}

void DetailScreen::loadGameDetail(const GameItem& game) {
    m_detail.basic = game;
    m_detail.description = "踏上一场冒险之旅，探索广阔的开放世界。在这款游戏中，你将"
                           "体验前所未有的自由与乐趣。穿越山川河流，发现隐藏的宝藏，"
                           "与各种敌人战斗，解开神秘的谜题。无论是新手玩家还是资深玩家，"
                           "都能在这个世界中找到属于自己的乐趣。准备好开始你的冒险了吗？";
    
    // Demo screenshots (placeholders)
    m_detail.screenshots = {"shot1.jpg", "shot2.jpg", "shot3.jpg", "shot4.jpg"};
    
    m_detail.version = "1.6.0";
    m_detail.releaseDate = "2017-03-03";
    m_detail.titleId = "0100" + game.id + "00000" + game.id + "000";
    m_detail.languages = {"中文", "英语", "日语"};
    m_detail.players = "1人";
    m_detail.isInstalled = false;
    m_detail.isDownloading = false;
    m_detail.downloadProgress = 0.0f;
}
