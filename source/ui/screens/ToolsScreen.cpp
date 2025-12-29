// =============================================================================
// Switch App Store - Tools Screen Implementation
// =============================================================================

#include "ToolsScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

ToolsScreen::ToolsScreen(App* app)
    : Screen(app)
{
    loadDemoContent();
}

ToolsScreen::~ToolsScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void ToolsScreen::onEnter() {
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
    m_selectedIndex = 0;
}

void ToolsScreen::onExit() {
    // Nothing to clean up
}

void ToolsScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate max scroll
    m_maxScrollY = std::max(0.0f, m_tools.size() * ITEM_HEIGHT - 
                            (720.0f - HEADER_HEIGHT - TAB_BAR_HEIGHT));
}

// =============================================================================
// Input Handling
// =============================================================================

void ToolsScreen::handleInput(const Input& input) {
    // D-pad navigation
    if (input.isPressed(Input::Button::DPadUp)) {
        if (m_selectedIndex > 0) {
            m_selectedIndex--;
        }
    }
    if (input.isPressed(Input::Button::DPadDown)) {
        if (m_selectedIndex < static_cast<int>(m_tools.size()) - 1) {
            m_selectedIndex++;
        }
    }
    
    // A button to download/open
    if (input.isPressed(Input::Button::A)) {
        // TODO: Download or open the selected tool
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

void ToolsScreen::update(float deltaTime) {
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

void ToolsScreen::render(Renderer& renderer) {
    renderToolsList(renderer);
    renderHeader(renderer);
}

void ToolsScreen::renderHeader(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Background
    Color bgColor = theme->backgroundColor();
    bgColor.a = 240;
    renderer.drawRect(Rect(0, 0, 1280, HEADER_HEIGHT), bgColor);
    
    // Title
    renderer.drawText("工具", SIDE_PADDING, 20, 34, 
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Subtitle
    renderer.drawText("实用的自制软件工具和系统增强", SIDE_PADDING, 54, 14,
                     theme->textSecondaryColor());
    
    // Separator
    renderer.drawLine(0, HEADER_HEIGHT, 1280, HEADER_HEIGHT, 
                     theme->separatorColor(), 1);
}

void ToolsScreen::renderToolsList(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float contentY = HEADER_HEIGHT - m_scrollY;
    float screenHeight = 720.0f - TAB_BAR_HEIGHT;
    
    for (size_t i = 0; i < m_tools.size(); i++) {
        float itemY = contentY + i * ITEM_HEIGHT;
        
        // Only render visible items
        if (itemY > -ITEM_HEIGHT && itemY < screenHeight) {
            bool isSelected = (static_cast<int>(i) == m_selectedIndex);
            renderToolItem(renderer, m_tools[i], itemY, isSelected);
        }
    }
}

void ToolsScreen::renderToolItem(Renderer& renderer, const ToolItem& tool,
                                  float y, bool isSelected) {
    Theme* theme = m_app->getTheme();
    
    // Selection highlight
    if (isSelected) {
        renderer.drawRect(Rect(0, y, 1280, ITEM_HEIGHT),
                         theme->getColor("selection"));
    }
    
    // Icon placeholder (rounded rect)
    float iconX = SIDE_PADDING;
    float iconY = y + 14;
    float iconSize = 60.0f;
    renderer.drawRoundedRect(Rect(iconX, iconY, iconSize, iconSize), 
                            12, Color::fromHex(0x5856D6));  // Purple for tools
    
    // Tool icon symbol (wrench)
    renderer.drawCircle(iconX + iconSize/2, iconY + iconSize/2, 12, 
                       Color(255, 255, 255));
    
    // Tool name
    float textX = iconX + iconSize + 16;
    renderer.drawText(tool.name, textX, y + 18, 17,
                     theme->textPrimaryColor(), FontWeight::Semibold);
    
    // Description
    renderer.drawText(tool.description, textX, y + 42, 13,
                     theme->textSecondaryColor());
    
    // Version and size
    std::string info = "v" + tool.version + " · " + tool.size;
    renderer.drawText(info, textX, y + 62, 12,
                     theme->textTertiaryColor());
    
    // Download/Open button
    float btnX = 1280 - SIDE_PADDING - 76;
    float btnY = y + 28;
    Color btnColor = tool.isInstalled ? 
                     theme->getColor("button_secondary_bg") : 
                     theme->primaryColor();
    renderer.drawRoundedRect(Rect(btnX, btnY, 70, 32), 16, btnColor);
    
    Color btnTextColor = tool.isInstalled ?
                         theme->primaryColor() : Color(255, 255, 255);
    std::string btnText = tool.isInstalled ? "打开" : "获取";
    renderer.drawTextInRect(btnText, Rect(btnX, btnY, 70, 32),
                           14, btnTextColor, FontWeight::Semibold,
                           TextAlign::Center, TextVAlign::Middle);
    
    // Separator line
    renderer.drawLine(SIDE_PADDING, y + ITEM_HEIGHT - 1, 
                     1280 - SIDE_PADDING, y + ITEM_HEIGHT - 1,
                     theme->separatorColor(), 1);
}

// =============================================================================
// Demo Content
// =============================================================================

void ToolsScreen::loadDemoContent() {
    m_tools = {
        {"1", "Goldleaf", "XorTroll", "多功能文件管理器和NSP安装器", "", "0.10.0", "2.3MB", true},
        {"2", "Tinfoil", "Blawar", "游戏安装器和文件管理", "", "16.0", "8.5MB", true},
        {"3", "NXDumpTool", "DarkMatterCore", "游戏卡带和数字游戏转储工具", "", "2.0.0", "1.2MB", false},
        {"4", "JKSV", "J-D-K", "存档管理器 - 备份和还原游戏存档", "", "2024.01", "1.5MB", true},
        {"5", "Hekate", "CTCaer", "自定义启动引导器", "", "6.0.7", "2.1MB", true},
        {"6", "Atmosphere", "SciresM", "自制系统固件", "", "1.6.2", "1.8MB", true},
        {"7", "SysDVR", "exelix11", "视频流工具 - 通过USB/网络串流画面", "", "6.0", "3.2MB", false},
        {"8", "NX-Shell", "Joel16", "文件管理器 - 浏览和管理SD卡文件", "", "4.0", "856KB", false},
        {"9", "sys-clk", "retronx-team", "系统超频工具", "", "2.0.0", "128KB", false},
        {"10", "EdiZon", "WerWolv", "存档编辑器和金手指管理器", "", "3.1.0", "5.6MB", false},
    };
    
    // Calculate max scroll
    m_maxScrollY = std::max(0.0f, m_tools.size() * ITEM_HEIGHT - 
                            (720.0f - HEADER_HEIGHT - TAB_BAR_HEIGHT));
}
