// =============================================================================
// Switch App Store - Today Screen Implementation
// =============================================================================

#include "TodayScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include <ctime>

// =============================================================================
// Constructor & Destructor
// =============================================================================

TodayScreen::TodayScreen(App* app)
    : Screen(app)
{
    // Generate date string
    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);
    
    // Format: "12月 29日 星期日" or similar
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%m月 %d日", tm);
    m_dateString = buffer;
    
    // Load demo content
    loadDemoContent();
}

TodayScreen::~TodayScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void TodayScreen::onEnter() {
    // Reset scroll position
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
}

void TodayScreen::onExit() {
    // Nothing to clean up
}

void TodayScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate max scroll based on content height
    // This will be more dynamic once we have real content
    m_maxScrollY = 1000.0f;  // Placeholder
}

// =============================================================================
// Input Handling
// =============================================================================

void TodayScreen::handleInput(const Input& input) {
    // Handle scrolling with analog stick
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 500.0f;  // Scroll speed
    }
    
    // Handle D-pad navigation
    if (input.isPressed(Input::Button::DPadUp)) {
        m_selectedIndex = (m_selectedIndex > 0) ? m_selectedIndex - 1 : 0;
    }
    if (input.isPressed(Input::Button::DPadDown)) {
        m_selectedIndex = (m_selectedIndex < (int)m_featuredCards.size() - 1) 
                          ? m_selectedIndex + 1 : m_selectedIndex;
    }
    
    // Handle selection with A button
    if (input.isPressed(Input::Button::A)) {
        // TODO: Navigate to detail screen
    }
    
    // Handle touch input for scrolling
    const auto& touch = input.getTouch();
    if (touch.touching) {
        m_scrollVelocity = -touch.deltaY;
    }
}

// =============================================================================
// Update
// =============================================================================

void TodayScreen::update(float deltaTime) {
    // Apply scroll velocity with friction
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.95f;  // Friction
        
        // Stop if velocity is very small
        if (std::abs(m_scrollVelocity) < 0.5f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll bounds with elastic bounce back
    if (m_scrollY < 0.0f) {
        m_scrollY *= 0.9f;  // Bounce back
    }
    if (m_scrollY > m_maxScrollY) {
        m_scrollY = m_maxScrollY + (m_scrollY - m_maxScrollY) * 0.9f;
    }
    
    // Update header opacity based on scroll
    m_headerOpacity = 1.0f - (m_scrollY / 100.0f);
    if (m_headerOpacity < 0.0f) m_headerOpacity = 0.0f;
    if (m_headerOpacity > 1.0f) m_headerOpacity = 1.0f;
}

// =============================================================================
// Rendering
// =============================================================================

void TodayScreen::render(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    float scale = m_app->getScale();
    
    // Screen dimensions
    float screenWidth = 1280.0f;  // Base width (720p)
    float screenHeight = 720.0f;
    
    // -------------------------------------------------------------------------
    // Render scrollable content
    // -------------------------------------------------------------------------
    
    float contentY = HEADER_HEIGHT - m_scrollY;
    
    // Render featured cards
    for (size_t i = 0; i < m_featuredCards.size(); ++i) {
        const auto& card = m_featuredCards[i];
        
        float cardX = SIDE_PADDING;
        float cardWidth = screenWidth - (SIDE_PADDING * 2);
        float cardHeight = (i == 0) ? LARGE_CARD_HEIGHT : SMALL_CARD_HEIGHT;
        
        // Check if card is visible
        if (contentY + cardHeight > 0 && contentY < screenHeight - TAB_BAR_HEIGHT) {
            bool isSelected = (int)i == m_selectedIndex;
            renderLargeCard(renderer, card, cardX, contentY, cardWidth, cardHeight);
            
            // Draw selection highlight if selected
            if (isSelected) {
                renderer.drawRoundedRectOutline(
                    Rect(cardX - 4, contentY - 4, cardWidth + 8, cardHeight + 8),
                    CARD_RADIUS + 4,
                    theme->primaryColor(),
                    3
                );
            }
        }
        
        contentY += cardHeight + CARD_SPACING;
    }
    
    // -------------------------------------------------------------------------
    // Render fixed UI elements (on top of scrollable content)
    // -------------------------------------------------------------------------
    
    renderHeader(renderer);
    renderTabBar(renderer);
}

// =============================================================================
// Header Rendering
// =============================================================================

void TodayScreen::renderHeader(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Header background (semi-transparent)
    Color bgColor = theme->backgroundColor();
    bgColor.a = static_cast<Uint8>(230 * m_headerOpacity);
    renderer.drawRect(Rect(0, 0, 1280, HEADER_HEIGHT), bgColor);
    
    // Date text (small, gray)
    Uint8 alpha = static_cast<Uint8>(255 * m_headerOpacity);
    Color dateColor = theme->textSecondaryColor();
    dateColor.a = alpha;
    renderer.drawText(m_dateString, SIDE_PADDING, 12, 14, dateColor);
    
    // "Today" title (large, bold)
    Color titleColor = theme->textPrimaryColor();
    titleColor.a = alpha;
    renderer.drawText("Today", SIDE_PADDING, 28, 34, titleColor, FontWeight::Bold);
}

// =============================================================================
// Tab Bar Rendering
// =============================================================================

void TodayScreen::renderTabBar(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float screenWidth = 1280.0f;
    float screenHeight = 720.0f;
    float tabBarY = screenHeight - TAB_BAR_HEIGHT;
    
    // Tab bar background (semi-transparent with blur effect simulation)
    renderer.drawRect(
        Rect(0, tabBarY, screenWidth, TAB_BAR_HEIGHT),
        theme->getColor("tab_bar_bg")
    );
    
    // Separator line at top of tab bar
    renderer.drawLine(0, tabBarY, screenWidth, tabBarY, theme->separatorColor(), 1);
    
    // Tab items
    const char* tabLabels[] = {"Today", "游戏", "App", "Arcade", "搜索"};
    const int tabCount = 5;
    float tabWidth = screenWidth / tabCount;
    
    for (int i = 0; i < tabCount; ++i) {
        float tabX = i * tabWidth + tabWidth / 2;
        float iconY = tabBarY + 12;
        float labelY = tabBarY + 48;
        
        // Determine if this tab is selected
        bool isSelected = (i == 0);  // Today is currently selected
        Color color = isSelected ? theme->primaryColor() : theme->textSecondaryColor();
        
        // Draw placeholder icon (circle for now)
        renderer.drawCircle(tabX, iconY + 14, 14, color);
        
        // Draw label
        renderer.drawText(tabLabels[i], tabX, labelY, 12, color, 
                         FontWeight::Regular, TextAlign::Center);
    }
}

// =============================================================================
// Card Rendering
// =============================================================================

void TodayScreen::renderLargeCard(Renderer& renderer, const FeaturedCard& card,
                                   float x, float y, float width, float height) {
    Theme* theme = m_app->getTheme();
    
    // Draw card shadow
    renderer.drawShadow(Rect(x, y, width, height), CARD_RADIUS, 12, 0, 4, 
                        theme->getColor("shadow"));
    
    // Draw card background
    renderer.drawRoundedRect(Rect(x, y, width, height), CARD_RADIUS, 
                             card.backgroundColor);
    
    // Draw content (positioned at bottom with gradient overlay)
    float textPadding = 20.0f;
    float textY = y + height - 100;
    
    // Gradient overlay at bottom for text readability
    renderer.drawGradientRect(
        Rect(x, textY - 40, width, 140),
        Color(0, 0, 0, 0),
        Color(0, 0, 0, 180)
    );
    
    // Tag (small uppercase text)
    renderer.drawText(card.tag, x + textPadding, textY, 12, 
                     Color(255, 255, 255, 200), FontWeight::Bold);
    
    // Title (large white text)
    renderer.drawText(card.title, x + textPadding, textY + 20, 28, 
                     Color(255, 255, 255, 255), FontWeight::Bold);
    
    // Subtitle (smaller white text)
    renderer.drawText(card.subtitle, x + textPadding, textY + 54, 16, 
                     Color(255, 255, 255, 200));
}

// =============================================================================
// Demo Content
// =============================================================================

void TodayScreen::loadDemoContent() {
    // Create demo featured cards
    m_featuredCards.push_back({
        "GAME OF THE DAY",
        "塞尔达传说：旷野之息",
        "探索广袤的海拉鲁大陆",
        "0100000000010000",
        Color::fromHex(0x1B5E20)  // Dark green
    });
    
    m_featuredCards.push_back({
        "NEW RELEASE",
        "超级马力欧 奥德赛",
        "跨越世界的冒险之旅",
        "0100000000010001",
        Color::fromHex(0xB71C1C)  // Deep red
    });
    
    m_featuredCards.push_back({
        "TRENDING",
        "宝可梦 朱/紫",
        "全新开放世界宝可梦冒险",
        "0100000000010002",
        Color::fromHex(0x4A148C)  // Deep purple
    });
    
    m_featuredCards.push_back({
        "EDITOR'S CHOICE",
        "动物森友会",
        "打造你的理想岛屿生活",
        "0100000000010003",
        Color::fromHex(0x00695C)  // Teal
    });
    
    // Calculate max scroll
    m_maxScrollY = m_featuredCards.size() * (LARGE_CARD_HEIGHT + CARD_SPACING) 
                   - 400.0f;
}
