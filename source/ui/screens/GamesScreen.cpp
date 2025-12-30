// =============================================================================
// Switch App Store - Games Screen Implementation
// =============================================================================
// Shows game categories from store + installed games section
// =============================================================================

#include "GamesScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "core/TitleManager.hpp"
#include "ui/Theme.hpp"
#include "store/StoreManager.hpp"
#include <algorithm>
#include <cmath>
#include <switch.h>

// =============================================================================
// Constructor & Destructor
// =============================================================================

GamesScreen::GamesScreen(App* app)
    : Screen(app)
{
    loadDemoContent();
}

GamesScreen::~GamesScreen() {
    // Cleanup installed game textures
    for (auto& game : m_installedGames) {
        if (game.icon) {
            SDL_DestroyTexture(game.icon);
        }
    }
}

// =============================================================================
// Lifecycle
// =============================================================================

void GamesScreen::onEnter() {
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
    m_selectedCategory = 0;
    m_selectedGame = 0;
    m_selectedInstalledGame = 0;
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
    // Y button to toggle between store and installed view
    if (input.isPressed(Input::Button::Y)) {
        m_showingInstalled = !m_showingInstalled;
        m_selectedInstalledGame = 0;
        m_selectedCategory = 0;
        m_selectedGame = 0;
        return;
    }
    
    // X button to delete (when showing installed)
    if (m_showingInstalled && input.isPressed(Input::Button::X)) {
        deleteSelectedGame();
        return;
    }
    
    // Vertical scrolling with analog stick
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 500.0f;
    }
    
    // D-pad navigation
    if (m_showingInstalled) {
        // Navigate installed games list
        if (input.isPressed(Input::Button::DPadUp)) {
            if (m_selectedInstalledGame > 0) m_selectedInstalledGame--;
        }
        if (input.isPressed(Input::Button::DPadDown)) {
            if (m_selectedInstalledGame < static_cast<int>(m_installedGames.size()) - 1) {
                m_selectedInstalledGame++;
            }
        }
    } else {
        // Navigate store categories
        float stickX = input.getLeftStick().x;
        if (stickX != 0.0f && m_selectedCategory < (int)m_categoryScrollX.size()) {
            m_categoryScrollX[m_selectedCategory] += stickX * 10.0f;
            m_categoryScrollX[m_selectedCategory] = std::max(0.0f, m_categoryScrollX[m_selectedCategory]);
        }
        
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
            if (m_selectedGame > 0) m_selectedGame--;
        }
        if (input.isPressed(Input::Button::DPadRight)) {
            if (m_selectedCategory < (int)m_categories.size() &&
                m_selectedGame < (int)m_categories[m_selectedCategory].games.size() - 1) {
                m_selectedGame++;
            }
        }
    }
    
    // Touch scrolling
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

void GamesScreen::update(float deltaTime) {
    // Apply scroll velocity
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.92f;
        
        if (std::abs(m_scrollVelocity) < 1.0f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll bounds
    float maxScroll = m_showingInstalled ? 
                      m_installedGames.size() * 88.0f - 400.0f :
                      m_categories.size() * 280.0f - 400.0f;
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
    (void)theme;
    
    // Load installed games lazily
    if (!m_installedLoaded) {
        loadInstalledGames(renderer);
        m_installedLoaded = true;
    }
    
    float contentY = HEADER_HEIGHT - m_scrollY;
    
    if (m_showingInstalled) {
        // Render installed games list
        renderInstalledSection(renderer, contentY);
    } else {
        // Render store categories
        for (size_t i = 0; i < m_categories.size(); i++) {
            if (contentY > -300.0f && contentY < 720.0f) {
                renderCategory(renderer, m_categories[i], contentY, static_cast<int>(i));
            }
            contentY += 280.0f;
        }
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
    std::string title = m_showingInstalled ? "已安装游戏" : "游戏";
    renderer.drawText(title, SIDE_PADDING, 20, 34, 
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Toggle hint
    std::string hint = m_showingInstalled ? "按Y查看商店 · 按X删除" : "按Y查看已安装";
    renderer.drawText(hint, 1280 - SIDE_PADDING - 200, 30, 14,
                     theme->textSecondaryColor());
    
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
    
    // Get horizontal scroll
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
    
    yOffset += GAME_CARD_SIZE + 60;
}

void GamesScreen::renderGameCard(Renderer& renderer, const GameItem& game,
                                  float x, float y, bool isSelected) {
    Theme* theme = m_app->getTheme();
    
    // Icon placeholder
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
    
    // Game name
    renderer.drawTextInRect(
        game.name,
        Rect(x, y + GAME_CARD_SIZE + 8, GAME_CARD_SIZE, 36),
        14, theme->textPrimaryColor(),
        FontWeight::Semibold, TextAlign::Left, TextVAlign::Top
    );
    
    // Category
    renderer.drawText(game.category, x, y + GAME_CARD_SIZE + 32, 12,
                     theme->textSecondaryColor());
}

// =============================================================================
// Installed Games Section
// =============================================================================

void GamesScreen::renderInstalledSection(Renderer& renderer, float& yOffset) {
    Theme* theme = m_app->getTheme();
    
    if (m_installedGames.empty()) {
        renderer.drawText("未找到已安装的游戏", 640, 300, 20,
                         theme->textSecondaryColor(), FontWeight::Regular, TextAlign::Center);
        return;
    }
    
    float itemHeight = 88.0f;
    float screenHeight = 720.0f - 70.0f;  // Minus tab bar
    
    for (size_t i = 0; i < m_installedGames.size(); i++) {
        float itemY = yOffset + i * itemHeight;
        
        if (itemY > -itemHeight && itemY < screenHeight) {
            const InstalledGameItem& game = m_installedGames[i];
            bool isSelected = (static_cast<int>(i) == m_selectedInstalledGame);
            
            // Selection highlight
            if (isSelected) {
                renderer.drawRect(Rect(0, itemY, 1280, itemHeight),
                                 theme->getColor("selection"));
            }
            
            // Icon
            float iconX = SIDE_PADDING;
            float iconY = itemY + 14;
            float iconSize = 60.0f;
            
            if (game.icon) {
                renderer.drawTexture(game.icon, Rect(iconX, iconY, iconSize, iconSize));
            } else {
                renderer.drawRoundedRect(Rect(iconX, iconY, iconSize, iconSize),
                                        12, Color::fromHex(0x007AFF));
            }
            
            // Name
            float textX = iconX + iconSize + 16;
            renderer.drawText(game.name, textX, itemY + 18, 17,
                             theme->textPrimaryColor(), FontWeight::Semibold);
            
            // Author
            renderer.drawText(game.author, textX, itemY + 42, 13,
                             theme->textSecondaryColor());
            
            // Version and Title ID
            char idBuf[32];
            snprintf(idBuf, sizeof(idBuf), "%016lX", game.titleId);
            std::string info = "v" + game.version + " · " + idBuf;
            renderer.drawText(info, textX, itemY + 62, 12,
                             theme->textTertiaryColor());
            
            // Delete button (when selected)
            if (isSelected) {
                float btnX = 1280 - SIDE_PADDING - 70;
                float btnY = itemY + 28;
                renderer.drawRoundedRect(Rect(btnX, btnY, 60, 32), 16, Color::fromHex(0xFF3B30));
                renderer.drawTextInRect("删除", Rect(btnX, btnY, 60, 32),
                                       14, Color(255, 255, 255), FontWeight::Semibold,
                                       TextAlign::Center, TextVAlign::Middle);
            }
            
            // Separator
            renderer.drawLine(SIDE_PADDING, itemY + itemHeight - 1,
                             1280 - SIDE_PADDING, itemY + itemHeight - 1,
                             theme->separatorColor(), 1);
        }
    }
}

void GamesScreen::loadInstalledGames(Renderer& renderer) {
    TitleManager& tm = TitleManager::getInstance();
    if (!tm.init()) return;
    
    auto apps = tm.getInstalledApps(renderer.getSDLRenderer());
    
    m_installedGames.clear();
    for (const auto& app : apps) {
        InstalledGameItem item;
        item.titleId = app.titleId;
        item.name = app.name;
        item.author = app.author;
        item.version = app.version;
        item.icon = app.icon;
        
        m_installedGames.push_back(item);
    }
}

void GamesScreen::deleteSelectedGame() {
    if (m_selectedInstalledGame < 0 || 
        m_selectedInstalledGame >= static_cast<int>(m_installedGames.size())) {
        return;
    }
    
    InstalledGameItem& game = m_installedGames[m_selectedInstalledGame];
    
    // Delete using ns service
    Result rc = nsDeleteApplicationCompletely(game.titleId);
    
    if (R_SUCCEEDED(rc)) {
        // Free texture
        if (game.icon) {
            SDL_DestroyTexture(game.icon);
        }
        
        // Remove from list
        m_installedGames.erase(m_installedGames.begin() + m_selectedInstalledGame);
        
        // Adjust selection
        if (m_selectedInstalledGame >= static_cast<int>(m_installedGames.size())) {
            m_selectedInstalledGame = static_cast<int>(m_installedGames.size()) - 1;
        }
        if (m_selectedInstalledGame < 0) m_selectedInstalledGame = 0;
    }
}

// =============================================================================
// Data Loading (from Backend)
// =============================================================================

void GamesScreen::loadDemoContent() {
    StoreManager& store = StoreManager::getInstance();
    const auto& categories = store.getCategories();
    
    for (const auto& storeCategory : categories) {
        auto entries = store.getEntriesByCategory(storeCategory.id);
        
        if (entries.empty()) continue;
        
        GameCategory category;
        category.title = storeCategory.name;
        
        for (const StoreEntry* entry : entries) {
            GameItem item;
            item.id = entry->id;
            item.name = entry->name;
            item.developer = entry->developer;
            item.category = storeCategory.name;
            item.iconUrl = entry->iconUrl;
            item.rating = entry->rating;
            item.size = entry->getFormattedSize();
            
            category.games.push_back(item);
        }
        
        m_categories.push_back(category);
        m_categoryScrollX.push_back(0.0f);
    }
}
