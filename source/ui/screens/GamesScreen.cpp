// =============================================================================
// Switch App Store - Games Screen Implementation
// =============================================================================

#include "GamesScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include <algorithm>
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

GamesScreen::GamesScreen(App* app)
    : Screen(app)
{
    loadDemoContent();
}

GamesScreen::~GamesScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void GamesScreen::onEnter() {
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
    m_selectedCategory = 0;
    m_selectedGame = 0;
}

void GamesScreen::onExit() {
    // Nothing to clean up
}

void GamesScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate layout if needed
}

// =============================================================================
// Input Handling
// =============================================================================

void GamesScreen::handleInput(const Input& input) {
    // Vertical scrolling with analog stick
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 500.0f;
    }
    
    // Horizontal scroll for current category
    float stickX = input.getLeftStick().x;
    if (stickX != 0.0f && m_selectedCategory < (int)m_categoryScrollX.size()) {
        m_categoryScrollX[m_selectedCategory] += stickX * 10.0f;
        // Clamp
        m_categoryScrollX[m_selectedCategory] = std::max(0.0f, m_categoryScrollX[m_selectedCategory]);
    }
    
    // D-pad navigation
    if (input.isPressed(Input::Button::DPadUp)) {
        if (m_selectedCategory > 0) {
            m_selectedCategory--;
            m_selectedGame = 0;
        }
    }
    if (input.isPressed(Input::Button::DPadDown)) {
        if (m_selectedCategory < (int)m_categories.size() - 1) {
            m_selectedCategory++;
            m_selectedGame = 0;
        }
    }
    if (input.isPressed(Input::Button::DPadLeft)) {
        if (m_selectedGame > 0) {
            m_selectedGame--;
        }
    }
    if (input.isPressed(Input::Button::DPadRight)) {
        if (m_selectedCategory < (int)m_categories.size() &&
            m_selectedGame < (int)m_categories[m_selectedCategory].games.size() - 1) {
            m_selectedGame++;
        }
    }
    
    // A button to select game (navigate to detail)
    if (input.isPressed(Input::Button::A)) {
        // TODO: Navigate to DetailScreen
    }
    
    // Touch scrolling - direct 1:1 mapping for responsiveness
    const auto& touch = input.getTouch();
    if (touch.touching) {
        // Direct scroll with touch delta
        m_scrollY -= touch.deltaY;  // Direct 1:1 scroll
        m_scrollVelocity = 0.0f;    // Stop inertia while touching
    } else if (touch.justReleased) {
        // Apply momentum from touch velocity
        m_scrollVelocity = -touch.velocityY * 30.0f;
    }
}

// =============================================================================
// Update
// =============================================================================

void GamesScreen::update(float deltaTime) {
    // Apply scroll velocity (momentum scrolling)
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.92f;  // Smoother friction
        
        if (std::abs(m_scrollVelocity) < 1.0f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll bounds
    float maxScroll = m_categories.size() * 280.0f - 400.0f;
    if (m_scrollY < 0.0f) {
        m_scrollY *= 0.9f;
    }
    if (m_scrollY > maxScroll) {
        m_scrollY = maxScroll + (m_scrollY - maxScroll) * 0.9f;
    }
}

// =============================================================================
// Rendering
// =============================================================================

void GamesScreen::render(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float contentY = HEADER_HEIGHT - m_scrollY;
    
    // Render each category section
    for (size_t i = 0; i < m_categories.size(); i++) {
        if (contentY > -300.0f && contentY < 720.0f) {
            renderCategory(renderer, m_categories[i], contentY, static_cast<int>(i));
        }
        contentY += 280.0f;  // Category height
    }
    
    // Render header on top
    renderHeader(renderer);
}

void GamesScreen::renderHeader(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Background
    Color bgColor = theme->backgroundColor();
    bgColor.a = 240;
    renderer.drawRect(Rect(0, 0, 1280, HEADER_HEIGHT), bgColor);
    
    // Title
    renderer.drawText("游戏", SIDE_PADDING, 20, 34, 
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Separator
    renderer.drawLine(0, HEADER_HEIGHT, 1280, HEADER_HEIGHT, 
                     theme->separatorColor(), 1);
}

void GamesScreen::renderCategory(Renderer& renderer, const GameCategory& category,
                                  float& yOffset, int categoryIndex) {
    Theme* theme = m_app->getTheme();
    
    // Category header
    renderer.drawText(category.title, SIDE_PADDING, yOffset, 22,
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // "查看全部 >" link
    renderer.drawText("查看全部 >", 1280 - SIDE_PADDING - 100, yOffset, 16,
                     theme->primaryColor());
    
    yOffset += 40;
    
    // Get horizontal scroll for this category
    float scrollX = 0.0f;
    if (categoryIndex < (int)m_categoryScrollX.size()) {
        scrollX = m_categoryScrollX[categoryIndex];
    }
    
    // Render games horizontally
    float gameX = SIDE_PADDING - scrollX;
    for (size_t i = 0; i < category.games.size(); i++) {
        if (gameX > -GAME_CARD_SIZE && gameX < 1280) {
            bool isSelected = (categoryIndex == m_selectedCategory && 
                              (int)i == m_selectedGame);
            renderGameCard(renderer, category.games[i], gameX, yOffset, isSelected);
        }
        gameX += GAME_CARD_SIZE + CARD_SPACING;
    }
    
    yOffset += GAME_CARD_SIZE + 60;  // Card + text height
}

void GamesScreen::renderGameCard(Renderer& renderer, const GameItem& game,
                                  float x, float y, bool isSelected) {
    Theme* theme = m_app->getTheme();
    
    // Icon placeholder (rounded rect)
    Color iconBg = Color::fromHex(0xE5E5EA);
    renderer.drawRoundedRect(Rect(x, y, GAME_CARD_SIZE, GAME_CARD_SIZE), 
                             ICON_RADIUS, iconBg);
    
    // Selection border
    if (isSelected) {
        renderer.drawRoundedRectOutline(
            Rect(x - 4, y - 4, GAME_CARD_SIZE + 8, GAME_CARD_SIZE + 8),
            ICON_RADIUS + 4, theme->primaryColor(), 3
        );
    }
    
    // Game name (below icon)
    renderer.drawTextInRect(
        game.name,
        Rect(x, y + GAME_CARD_SIZE + 8, GAME_CARD_SIZE, 36),
        14, theme->textPrimaryColor(),
        FontWeight::Semibold, TextAlign::Left, TextVAlign::Top
    );
    
    // Category/developer
    renderer.drawText(game.category, x, y + GAME_CARD_SIZE + 32, 12,
                     theme->textSecondaryColor());
}

// =============================================================================
// Demo Content
// =============================================================================

void GamesScreen::loadDemoContent() {
    // Hot games category
    GameCategory hotGames;
    hotGames.title = "热门游戏";
    hotGames.games = {
        {"1", "塞尔达传说：旷野之息", "Nintendo", "动作冒险", "", 4.9f, "14.5GB"},
        {"2", "超级马力欧 奥德赛", "Nintendo", "平台动作", "", 4.8f, "5.7GB"},
        {"3", "宝可梦 朱/紫", "Game Freak", "角色扮演", "", 4.5f, "7.0GB"},
        {"4", "斯普拉遁3", "Nintendo", "射击", "", 4.7f, "6.1GB"},
        {"5", "动物森友会", "Nintendo", "模拟经营", "", 4.9f, "6.8GB"},
    };
    m_categories.push_back(hotGames);
    m_categoryScrollX.push_back(0.0f);
    
    // New releases
    GameCategory newGames;
    newGames.title = "新游戏推荐";
    newGames.games = {
        {"6", "塞尔达传说：王国之泪", "Nintendo", "动作冒险", "", 4.9f, "16.2GB"},
        {"7", "皮克敏4", "Nintendo", "策略", "", 4.6f, "10.3GB"},
        {"8", "超级马力欧兄弟 惊奇", "Nintendo", "平台动作", "", 4.8f, "4.5GB"},
        {"9", "火焰纹章 Engage", "Intelligent Sys", "策略RPG", "", 4.4f, "12.0GB"},
    };
    m_categories.push_back(newGames);
    m_categoryScrollX.push_back(0.0f);
    
    // Indie games
    GameCategory indieGames;
    indieGames.title = "独立游戏精选";
    indieGames.games = {
        {"10", "空洞骑士", "Team Cherry", "银河恶魔城", "", 4.9f, "4.1GB"},
        {"11", "蔚蓝", "Matt Makes Games", "平台动作", "", 4.8f, "1.2GB"},
        {"12", "哈迪斯", "Supergiant Games", "动作Rogue", "", 4.9f, "5.8GB"},
        {"13", "星露谷物语", "ConcernedApe", "模拟经营", "", 4.7f, "1.0GB"},
    };
    m_categories.push_back(indieGames);
    m_categoryScrollX.push_back(0.0f);
    
    // Action games
    GameCategory actionGames;
    actionGames.title = "动作游戏";
    actionGames.games = {
        {"14", "猎天使魔女3", "PlatinumGames", "动作", "", 4.5f, "15.0GB"},
        {"15", "怪物猎人:崛起", "Capcom", "动作RPG", "", 4.7f, "13.3GB"},
        {"16", "黑暗之魂:重制版", "FromSoftware", "动作RPG", "", 4.6f, "4.0GB"},
    };
    m_categories.push_back(actionGames);
    m_categoryScrollX.push_back(0.0f);
}
