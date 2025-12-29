// =============================================================================
// Switch App Store - Settings Screen Implementation
// =============================================================================

#include "SettingsScreen.hpp"
#include "app.hpp"
#include "core/Input.hpp"
#include "ui/Theme.hpp"
#include "store/SettingsManager.hpp"
#include <algorithm>
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

SettingsScreen::SettingsScreen(App* app)
    : Screen(app)
{
    setupSettings();
}

SettingsScreen::~SettingsScreen() = default;

// =============================================================================
// Lifecycle
// =============================================================================

void SettingsScreen::onEnter() {
    m_scrollY = 0.0f;
    m_scrollVelocity = 0.0f;
    m_selectedSection = 0;
    m_selectedItem = 0;
    loadCurrentValues();
}

void SettingsScreen::onExit() {
    applySettings();
}

void SettingsScreen::onResolutionChanged(int width, int height, float scale) {
    // Recalculate layout
}

// =============================================================================
// Settings Setup
// =============================================================================

void SettingsScreen::setupSettings() {
    m_sections.clear();
    
    // -------------------------------------------------------------------------
    // Appearance Section
    // -------------------------------------------------------------------------
    SettingSection appearance;
    appearance.title = "外观";
    
    SettingItem darkMode;
    darkMode.id = "dark_mode";
    darkMode.title = "深色模式";
    darkMode.subtitle = "减少眼睛疲劳";
    darkMode.type = SettingItemType::Toggle;
    darkMode.toggleValue = false;
    darkMode.onToggle = [this](bool value) {
        SettingsManager::getInstance().setDarkMode(value);
        m_app->getTheme()->loadTheme(value ? "dark" : "light");
    };
    appearance.items.push_back(darkMode);
    
    SettingItem language;
    language.id = "language";
    language.title = "语言";
    language.subtitle = "简体中文";
    language.type = SettingItemType::Choice;
    language.choices = {"简体中文", "繁體中文", "English", "日本語"};
    language.selectedChoice = 0;
    language.onChoice = [](int index) {
        const char* langs[] = {"zh-CN", "zh-TW", "en-US", "ja-JP"};
        if (index >= 0 && index < 4) {
            SettingsManager::getInstance().setLanguage(langs[index]);
        }
    };
    appearance.items.push_back(language);
    
    m_sections.push_back(appearance);
    
    // -------------------------------------------------------------------------
    // Downloads Section
    // -------------------------------------------------------------------------
    SettingSection downloads;
    downloads.title = "下载";
    
    SettingItem downloadDir;
    downloadDir.id = "download_dir";
    downloadDir.title = "下载位置";
    downloadDir.subtitle = "sdmc:/switch/appstore/downloads";
    downloadDir.type = SettingItemType::Action;
    downloads.items.push_back(downloadDir);
    
    SettingItem installDir;
    installDir.id = "install_dir";
    installDir.title = "安装位置";
    installDir.subtitle = "sdmc:/switch";
    installDir.type = SettingItemType::Action;
    downloads.items.push_back(installDir);
    
    SettingItem autoUpdate;
    autoUpdate.id = "auto_update";
    autoUpdate.title = "自动检查更新";
    autoUpdate.subtitle = "启动时检查应用更新";
    autoUpdate.type = SettingItemType::Toggle;
    autoUpdate.toggleValue = true;
    autoUpdate.onToggle = [](bool value) {
        SettingsManager::getInstance().setAutoUpdate(value);
    };
    downloads.items.push_back(autoUpdate);
    
    m_sections.push_back(downloads);
    
    // -------------------------------------------------------------------------
    // Cache Section
    // -------------------------------------------------------------------------
    SettingSection cache;
    cache.title = "缓存";
    
    SettingItem cacheSize;
    cacheSize.id = "cache_size";
    cacheSize.title = "图片缓存大小";
    cacheSize.subtitle = "50 MB";
    cacheSize.type = SettingItemType::Choice;
    cacheSize.choices = {"25 MB", "50 MB", "100 MB", "200 MB"};
    cacheSize.selectedChoice = 1;
    cacheSize.onChoice = [](int index) {
        const int sizes[] = {25, 50, 100, 200};
        if (index >= 0 && index < 4) {
            SettingsManager::getInstance().setImageCacheSize(sizes[index]);
        }
    };
    cache.items.push_back(cacheSize);
    
    SettingItem clearCache;
    clearCache.id = "clear_cache";
    clearCache.title = "清除缓存";
    clearCache.subtitle = "释放存储空间";
    clearCache.type = SettingItemType::Action;
    clearCache.onAction = []() {
        // TODO: Clear cache
    };
    cache.items.push_back(clearCache);
    
    m_sections.push_back(cache);
    
    // -------------------------------------------------------------------------
    // About Section
    // -------------------------------------------------------------------------
    SettingSection about;
    about.title = "关于";
    
    SettingItem version;
    version.id = "version";
    version.title = "版本";
    version.subtitle = "1.0.0";
    version.type = SettingItemType::Info;
    about.items.push_back(version);
    
    SettingItem author;
    author.id = "author";
    author.title = "开发者";
    author.subtitle = "Switch App Store Team";
    author.type = SettingItemType::Info;
    about.items.push_back(author);
    
    SettingItem source;
    source.id = "source";
    source.title = "开源地址";
    source.subtitle = "github.com/yynickyeh/switch-appstore";
    source.type = SettingItemType::Action;
    about.items.push_back(source);
    
    m_sections.push_back(about);
    
    // Calculate content height
    m_contentHeight = 0;
    for (const auto& section : m_sections) {
        m_contentHeight += SECTION_HEADER_HEIGHT;
        m_contentHeight += section.items.size() * ITEM_HEIGHT;
        m_contentHeight += SECTION_SPACING;
    }
}

void SettingsScreen::loadCurrentValues() {
    auto& settings = SettingsManager::getInstance();
    
    // Update toggle values from settings
    for (auto& section : m_sections) {
        for (auto& item : section.items) {
            if (item.id == "dark_mode") {
                item.toggleValue = settings.isDarkMode();
            } else if (item.id == "auto_update") {
                item.toggleValue = settings.isAutoUpdateEnabled();
            } else if (item.id == "download_dir") {
                item.subtitle = settings.getDownloadDir();
            } else if (item.id == "install_dir") {
                item.subtitle = settings.getInstallDir();
            }
        }
    }
}

void SettingsScreen::applySettings() {
    SettingsManager::getInstance().save();
}

// =============================================================================
// Input Handling
// =============================================================================

void SettingsScreen::handleInput(const Input& input) {
    // D-pad navigation
    if (input.isPressed(Input::Button::DPadUp)) {
        if (m_selectedItem > 0) {
            m_selectedItem--;
        } else if (m_selectedSection > 0) {
            m_selectedSection--;
            m_selectedItem = static_cast<int>(m_sections[m_selectedSection].items.size()) - 1;
        }
    }
    
    if (input.isPressed(Input::Button::DPadDown)) {
        if (m_selectedItem < (int)m_sections[m_selectedSection].items.size() - 1) {
            m_selectedItem++;
        } else if (m_selectedSection < (int)m_sections.size() - 1) {
            m_selectedSection++;
            m_selectedItem = 0;
        }
    }
    
    // A button to toggle/activate
    if (input.isPressed(Input::Button::A)) {
        if (m_selectedSection < (int)m_sections.size() &&
            m_selectedItem < (int)m_sections[m_selectedSection].items.size()) {
            auto& item = m_sections[m_selectedSection].items[m_selectedItem];
            
            switch (item.type) {
                case SettingItemType::Toggle:
                    item.toggleValue = !item.toggleValue;
                    if (item.onToggle) {
                        item.onToggle(item.toggleValue);
                    }
                    break;
                    
                case SettingItemType::Choice:
                    item.selectedChoice = (item.selectedChoice + 1) % item.choices.size();
                    item.subtitle = item.choices[item.selectedChoice];
                    if (item.onChoice) {
                        item.onChoice(item.selectedChoice);
                    }
                    break;
                    
                case SettingItemType::Action:
                    if (item.onAction) {
                        item.onAction();
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    // Analog stick scrolling
    float stickY = input.getLeftStick().y;
    if (stickY != 0.0f) {
        m_scrollVelocity = -stickY * 300.0f;
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

void SettingsScreen::update(float deltaTime) {
    // Apply scroll velocity
    if (m_scrollVelocity != 0.0f) {
        m_scrollY += m_scrollVelocity * deltaTime;
        m_scrollVelocity *= 0.95f;
        
        if (std::abs(m_scrollVelocity) < 0.5f) {
            m_scrollVelocity = 0.0f;
        }
    }
    
    // Clamp scroll
    float maxScroll = m_contentHeight - 500.0f;
    m_scrollY = std::clamp(m_scrollY, 0.0f, std::max(0.0f, maxScroll));
}

// =============================================================================
// Rendering
// =============================================================================

void SettingsScreen::render(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    float yOffset = HEADER_HEIGHT - m_scrollY;
    
    // Render sections
    for (size_t sectionIdx = 0; sectionIdx < m_sections.size(); sectionIdx++) {
        const auto& section = m_sections[sectionIdx];
        
        // Section header
        if (yOffset > -SECTION_HEADER_HEIGHT && yOffset < 720) {
            renderer.drawText(section.title, SIDE_PADDING, yOffset + 10, 14,
                             theme->textSecondaryColor(), FontWeight::Semibold);
        }
        yOffset += SECTION_HEADER_HEIGHT;
        
        // Section items
        for (size_t itemIdx = 0; itemIdx < section.items.size(); itemIdx++) {
            if (yOffset > -ITEM_HEIGHT && yOffset < 720) {
                bool isSelected = ((int)sectionIdx == m_selectedSection && 
                                   (int)itemIdx == m_selectedItem);
                renderItem(renderer, section.items[itemIdx], yOffset, isSelected);
            }
            yOffset += ITEM_HEIGHT;
        }
        
        yOffset += SECTION_SPACING;
    }
    
    // Render header on top
    renderHeader(renderer);
}

void SettingsScreen::renderHeader(Renderer& renderer) {
    Theme* theme = m_app->getTheme();
    
    // Background
    Color bgColor = theme->backgroundColor();
    bgColor.a = 240;
    renderer.drawRect(Rect(0, 0, 1280, HEADER_HEIGHT), bgColor);
    
    // Title
    renderer.drawText("设置", SIDE_PADDING, 18, 34, 
                     theme->textPrimaryColor(), FontWeight::Bold);
    
    // Separator
    renderer.drawLine(0, HEADER_HEIGHT, 1280, HEADER_HEIGHT, 
                     theme->separatorColor(), 1);
}

void SettingsScreen::renderItem(Renderer& renderer, const SettingItem& item, 
                                 float y, bool isSelected) {
    Theme* theme = m_app->getTheme();
    
    // Selection background
    if (isSelected) {
        renderer.drawRect(Rect(0, y, 1280, ITEM_HEIGHT), theme->getColor("selection"));
    }
    
    // Item background (card style)
    renderer.drawRect(Rect(SIDE_PADDING, y + 1, 1280 - SIDE_PADDING * 2, ITEM_HEIGHT - 2),
                     theme->cardBackgroundColor());
    
    // Title
    renderer.drawText(item.title, SIDE_PADDING + 16, y + 8, 16,
                     theme->textPrimaryColor(), FontWeight::Regular);
    
    // Subtitle
    if (!item.subtitle.empty() && item.type != SettingItemType::Toggle) {
        renderer.drawText(item.subtitle, SIDE_PADDING + 16, y + 28, 13,
                         theme->textSecondaryColor());
    }
    
    // Type-specific rendering
    float rightX = 1280 - SIDE_PADDING - 16;
    
    switch (item.type) {
        case SettingItemType::Toggle:
            renderToggle(renderer, item.toggleValue, rightX - 50, y + 10, isSelected);
            break;
            
        case SettingItemType::Choice:
            // Show current choice with chevron
            renderer.drawText(item.subtitle, rightX - 120, y + 16, 14,
                             theme->textSecondaryColor(), FontWeight::Regular,
                             TextAlign::Right);
            renderer.drawText(">", rightX, y + 14, 18, theme->textTertiaryColor());
            break;
            
        case SettingItemType::Action:
            // Chevron only
            renderer.drawText(">", rightX, y + 14, 18, theme->textTertiaryColor());
            break;
            
        case SettingItemType::Info:
            // Value on right
            renderer.drawText(item.subtitle, rightX, y + 16, 14,
                             theme->textSecondaryColor(), FontWeight::Regular,
                             TextAlign::Right);
            break;
            
        default:
            break;
    }
    
    // Separator line
    renderer.drawLine(SIDE_PADDING + 16, y + ITEM_HEIGHT - 1,
                     1280 - SIDE_PADDING, y + ITEM_HEIGHT - 1,
                     theme->separatorColor(), 1);
}

void SettingsScreen::renderToggle(Renderer& renderer, bool value, float x, float y, 
                                    bool focused) {
    Theme* theme = m_app->getTheme();
    
    // Toggle track
    float trackW = 50;
    float trackH = 30;
    Color trackColor = value ? theme->primaryColor() : theme->getColor("button_secondary_bg");
    renderer.drawRoundedRect(Rect(x, y, trackW, trackH), 15, trackColor);
    
    // Toggle knob
    float knobSize = 26;
    float knobX = value ? (x + trackW - knobSize - 2) : (x + 2);
    renderer.drawCircle(knobX + knobSize/2, y + trackH/2, knobSize/2, 
                        Color(255, 255, 255));
    
    // Focus ring
    if (focused) {
        renderer.drawRoundedRectOutline(Rect(x - 3, y - 3, trackW + 6, trackH + 6),
                                        18, theme->primaryColor(), 2);
    }
}
