// =============================================================================
// Switch App Store - Tools Screen Implementation
// =============================================================================
// Displays downloadable tools from store + installed NRO homebrew
// =============================================================================

#include "ToolsScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "core/NroScanner.hpp"
#include "store/StoreManager.hpp"
#include "ui/Theme.hpp"
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

ToolsScreen::ToolsScreen(App* app)
    : Screen(app)
{
    loadStoreTools();
}

ToolsScreen::~ToolsScreen() {
    // Cleanup textures
    for (auto& tool : m_installedTools) {
        if (tool.iconTexture) {
            SDL_DestroyTexture(tool.iconTexture);
        }
    }
}

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
    auto& tools = m_showingInstalled ? m_installedTools : m_storeTools;
    m_maxScrollY = std::max(0.0f, tools.size() * ITEM_HEIGHT - 
                            (720.0f - HEADER_HEIGHT - TAB_BAR_HEIGHT));
}

// =============================================================================
// Input Handling
// =============================================================================

void ToolsScreen::handleInput(const Input& input) {
    // =========================================================================
    // NATIVE TOUCH EXPERIENCE
    // Implementing direct manipulation and precise hit testing for tool actions.
    // =========================================================================
    
    auto& tools = m_showingInstalled ? m_installedTools : m_storeTools;
    
    // Y button to toggle between store and installed view
    if (input.isPressed(Input::Button::Y)) {
        m_showingInstalled = !m_showingInstalled;
        m_selectedIndex = 0;
        m_scrollY = 0.0f;
        return;
    }
    
    // X button to delete (when showing installed)
    if (m_showingInstalled && input.isPressed(Input::Button::X)) {
        deleteSelectedTool();
        return;
    }
    
    // A button to download (when showing store)
    if (!m_showingInstalled && input.isPressed(Input::Button::A)) {
        downloadSelectedTool();
        return;
    }
    
    // D-pad navigation
    if (input.isPressed(Input::Button::DPadUp)) {
        if (m_selectedIndex > 0) {
            m_selectedIndex--;
        }
    }
    if (input.isPressed(Input::Button::DPadDown)) {
        if (m_selectedIndex < static_cast<int>(tools.size()) - 1) {
            m_selectedIndex++;
        }
    }
    
    // Analog stick scrolling
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 600.0f; // Matches GamesScreen physics
    }
    
    // -------------------------------------------------------------------------
    // TOUCH HANDLING
    // -------------------------------------------------------------------------
    const auto& touch = input.getTouch();
    
    if (touch.touching) {
        // Direct scroll
        m_scrollY -= touch.deltaY;
        m_scrollVelocity = 0.0f;
    } else if (touch.justReleased) {
        // Tap vs Scroll detection
        float dragDist = std::sqrt((touch.x - touch.startX) * (touch.x - touch.startX) +
                                   (touch.y - touch.startY) * (touch.y - touch.startY));
        
        if (dragDist < 30.0f) {
            // TAP DETECTED
            float contentY = HEADER_HEIGHT - m_scrollY;
            float tapY = touch.y;
            float tapX = touch.x;
            
            // Check content area
            if (tapY > HEADER_HEIGHT && tapY < 720.0f - TAB_BAR_HEIGHT) {
                int tappedIndex = static_cast<int>((tapY - contentY) / ITEM_HEIGHT);
                
                if (tappedIndex >= 0 && tappedIndex < static_cast<int>(tools.size())) {
                    
                    // ---------------------------------------------------------
                    // Check for ACTION BUTTON tap
                    // The "Get" or "Delete" button is on the right side
                    // ---------------------------------------------------------
                    float itemTopY = contentY + tappedIndex * ITEM_HEIGHT;
                    float btnX = 1280 - SIDE_PADDING - 70;
                    float btnY = itemTopY + 28;
                    float btnW = 60;
                    float btnH = 32;
                    
                    // Add generous padding for touch target (Hit Slop)
                    if (tapX >= btnX - 20 && tapX <= btnX + btnW + 20 &&
                        tapY >= btnY - 20 && tapY <= btnY + btnH + 20) {
                        
                        // Select it first to be sure
                        m_selectedIndex = tappedIndex;
                        
                        // Trigger Action Immediately
                        if (m_showingInstalled) {
                            deleteSelectedTool();
                        } else {
                            downloadSelectedTool();
                        }
                    } else {
                        // Normal row tap
                        if (tappedIndex == m_selectedIndex) {
                            // Double tap approach (optional, but button is preferred)
                            // We kept this for keyboard/controller logic consistency if needed
                            // But for touch, the button above is the primary way.
                        } else {
                            m_selectedIndex = tappedIndex;
                        }
                    }
                }
            }
        } else {
            // Drag Momentum
            m_scrollVelocity = -touch.velocityY * 35.0f;
        }
    }
}

// =============================================================================
// Update
// =============================================================================

void ToolsScreen::update(float deltaTime) {
    auto& tools = m_showingInstalled ? m_installedTools : m_storeTools;
    
    // Apply scroll velocity
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.92f;
        
        if (std::abs(m_scrollVelocity) < 1.0f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll bounds
    float maxScroll = std::max(0.0f, tools.size() * ITEM_HEIGHT - 
                               (720.0f - HEADER_HEIGHT - TAB_BAR_HEIGHT));
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

void ToolsScreen::render(Renderer& renderer) {
    // Lazy load installed tools
    if (!m_installedLoaded) {
        loadNroTools(renderer);
        m_installedLoaded = true;
    }
    
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
    std::string title = m_showingInstalled ? "已安装工具" : "工具商店";
    renderer.drawText(title, SIDE_PADDING, 20, 34, 
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Hint
    std::string hint = m_showingInstalled ? 
                       "按Y查看商店 · 按X删除" : 
                       "按Y查看已安装 · 按A下载";
    renderer.drawText(hint, 1280 - SIDE_PADDING - 220, 30, 14,
                     theme->textSecondaryColor());
    
    // Separator
    renderer.drawLine(0, HEADER_HEIGHT, 1280, HEADER_HEIGHT, 
                     theme->separatorColor(), 1);
}

void ToolsScreen::renderToolsList(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    auto& tools = m_showingInstalled ? m_installedTools : m_storeTools;
    
    float contentY = HEADER_HEIGHT - m_scrollY;
    float screenHeight = 720.0f - TAB_BAR_HEIGHT;
    
    // Show message if empty
    if (tools.empty()) {
        std::string msg = m_showingInstalled ? 
                          "未找到NRO工具" : "无可用工具";
        renderer.drawText(msg, 640, 300, 20,
                         theme->textSecondaryColor(), FontWeight::Regular, TextAlign::Center);
        
        if (m_showingInstalled) {
            renderer.drawText("请将.nro文件放入 /switch/ 目录", 640, 330, 14,
                             theme->textTertiaryColor(), FontWeight::Regular, TextAlign::Center);
        }
        return;
    }
    
    for (size_t i = 0; i < tools.size(); i++) {
        float itemY = contentY + i * ITEM_HEIGHT;
        
        if (itemY > -ITEM_HEIGHT && itemY < screenHeight) {
            bool isSelected = (static_cast<int>(i) == m_selectedIndex);
            renderToolItem(renderer, tools[i], itemY, isSelected);
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
    
    // Icon
    float iconX = SIDE_PADDING;
    float iconY = y + 14;
    float iconSize = 60.0f;
    
    if (tool.iconTexture) {
        renderer.drawTexture(tool.iconTexture, Rect(iconX, iconY, iconSize, iconSize));
    } else {
        // Placeholder (purple for tools)
        renderer.drawRoundedRect(Rect(iconX, iconY, iconSize, iconSize), 
                                12, Color::fromHex(0x5856D6));
        renderer.drawCircle(iconX + iconSize/2, iconY + iconSize/2, 12, 
                           Color(255, 255, 255));
    }
    
    // Name
    float textX = iconX + iconSize + 16;
    renderer.drawText(tool.name, textX, y + 18, 17,
                     theme->textPrimaryColor(), FontWeight::Semibold);
    
    // Developer/description
    renderer.drawText(tool.developer.empty() ? tool.description : tool.developer, 
                     textX, y + 42, 13, theme->textSecondaryColor());
    
    // Version and size
    std::string info = tool.version.empty() ? tool.size : ("v" + tool.version + " · " + tool.size);
    renderer.drawText(info, textX, y + 62, 12, theme->textTertiaryColor());
    
    // Action button
    float btnX = 1280 - SIDE_PADDING - 70;
    float btnY = y + 28;
    
    if (m_showingInstalled) {
        // Delete button (red)
        if (isSelected) {
            renderer.drawRoundedRect(Rect(btnX, btnY, 60, 32), 16, Color::fromHex(0xFF3B30));
            renderer.drawTextInRect("删除", Rect(btnX, btnY, 60, 32),
                                   14, Color(255, 255, 255), FontWeight::Semibold,
                                   TextAlign::Center, TextVAlign::Middle);
        }
    } else {
        // Download button (blue)
        Color btnColor = tool.isInstalled ? 
                         theme->getColor("button_secondary_bg") : theme->primaryColor();
        renderer.drawRoundedRect(Rect(btnX, btnY, 60, 32), 16, btnColor);
        
        Color btnTextColor = tool.isInstalled ? theme->primaryColor() : Color(255, 255, 255);
        std::string btnText = tool.isInstalled ? "已安装" : "获取";
        renderer.drawTextInRect(btnText, Rect(btnX, btnY, 60, 32),
                               14, btnTextColor, FontWeight::Semibold,
                               TextAlign::Center, TextVAlign::Middle);
    }
    
    // Separator
    renderer.drawLine(SIDE_PADDING, y + ITEM_HEIGHT - 1, 
                     1280 - SIDE_PADDING, y + ITEM_HEIGHT - 1,
                     theme->separatorColor(), 1);
}

// =============================================================================
// Content Loading
// =============================================================================

void ToolsScreen::loadStoreTools() {
    StoreManager& store = StoreManager::getInstance();
    
    // Get tools category entries
    auto entries = store.getEntriesByCategory("tools");
    
    m_storeTools.clear();
    for (const StoreEntry* entry : entries) {
        ToolItem item;
        item.id = entry->id;
        item.name = entry->name;
        item.developer = entry->developer;
        item.description = entry->description;
        item.downloadUrl = entry->downloadUrl;
        item.version = entry->version;
        item.size = entry->getFormattedSize();
        item.isInstalled = false;  // TODO: Check if installed
        
        m_storeTools.push_back(item);
    }
}

void ToolsScreen::loadNroTools(Renderer& renderer) {
    auto nros = NroScanner::getInstance().scanDirectory("sdmc:/switch", renderer.getSDLRenderer());
    
    m_installedTools.clear();
    for (const auto& nro : nros) {
        ToolItem item;
        item.id = nro.path;
        item.name = nro.name;
        item.developer = nro.author;
        item.description = nro.path;
        item.filePath = nro.path;
        item.version = nro.version;
        item.size = nro.size_str;
        item.isInstalled = true;
        item.iconTexture = nro.icon;
        
        m_installedTools.push_back(item);
    }
}

// =============================================================================
// Actions
// =============================================================================

void ToolsScreen::deleteSelectedTool() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_installedTools.size())) {
        return;
    }
    
    ToolItem& tool = m_installedTools[m_selectedIndex];
    
    if (NroScanner::getInstance().deleteNro(tool.filePath)) {
        if (tool.iconTexture) {
            SDL_DestroyTexture(tool.iconTexture);
        }
        
        m_installedTools.erase(m_installedTools.begin() + m_selectedIndex);
        
        if (m_selectedIndex >= static_cast<int>(m_installedTools.size())) {
            m_selectedIndex = static_cast<int>(m_installedTools.size()) - 1;
        }
        if (m_selectedIndex < 0) m_selectedIndex = 0;
    }
}

void ToolsScreen::downloadSelectedTool() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_storeTools.size())) {
        return;
    }
    
    ToolItem& tool = m_storeTools[m_selectedIndex];
    
    if (tool.downloadUrl.empty()) return;
    
    // TODO: Implement actual download using GameInstaller
    // For now, just mark as "downloading"
    // GameInstaller::getInstance().downloadGame(tool.downloadUrl, "sdmc:/switch/" + tool.name + ".nro");
}
