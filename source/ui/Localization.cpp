// =============================================================================
// Switch App Store - Localization Implementation
// =============================================================================

#include "Localization.hpp"
#include <algorithm>

// =============================================================================
// Singleton
// =============================================================================

Localization& Localization::getInstance() {
    static Localization instance;
    return instance;
}

// =============================================================================
// Initialization
// =============================================================================

void Localization::init() {
    loadLanguage(m_currentLang);
}

// =============================================================================
// Language Management
// =============================================================================

void Localization::setLanguage(Language lang) {
    if (m_currentLang != lang) {
        m_currentLang = lang;
        loadLanguage(lang);
    }
}

void Localization::setLanguage(const std::string& langCode) {
    if (langCode == "zh-CN") setLanguage(Language::ZH_CN);
    else if (langCode == "zh-TW") setLanguage(Language::ZH_TW);
    else if (langCode == "en-US") setLanguage(Language::EN_US);
    else if (langCode == "ja-JP") setLanguage(Language::JA_JP);
}

std::string Localization::getLanguageCode() const {
    return getLanguageCode(m_currentLang);
}

std::string Localization::getLanguageName() const {
    return getLanguageName(m_currentLang);
}

const std::vector<Language>& Localization::getAvailableLanguages() {
    static std::vector<Language> langs = {
        Language::ZH_CN, Language::ZH_TW, Language::EN_US, Language::JA_JP
    };
    return langs;
}

std::string Localization::getLanguageName(Language lang) {
    switch (lang) {
        case Language::ZH_CN: return "简体中文";
        case Language::ZH_TW: return "繁體中文";
        case Language::EN_US: return "English";
        case Language::JA_JP: return "日本語";
    }
    return "Unknown";
}

std::string Localization::getLanguageCode(Language lang) {
    switch (lang) {
        case Language::ZH_CN: return "zh-CN";
        case Language::ZH_TW: return "zh-TW";
        case Language::EN_US: return "en-US";
        case Language::JA_JP: return "ja-JP";
    }
    return "en-US";
}

// =============================================================================
// String Access
// =============================================================================

const std::string& Localization::get(const std::string& key) const {
    auto it = m_strings.find(key);
    if (it != m_strings.end()) {
        return it->second;
    }
    return m_emptyString;
}

std::string Localization::format(const std::string& key, const std::string& arg1) const {
    std::string result = get(key);
    size_t pos = result.find("%s");
    if (pos != std::string::npos) {
        result.replace(pos, 2, arg1);
    }
    return result;
}

std::string Localization::format(const std::string& key, const std::string& arg1,
                                  const std::string& arg2) const {
    std::string result = format(key, arg1);
    size_t pos = result.find("%s");
    if (pos != std::string::npos) {
        result.replace(pos, 2, arg2);
    }
    return result;
}

// =============================================================================
// Language Loading
// =============================================================================

void Localization::loadLanguage(Language lang) {
    m_strings.clear();
    
    switch (lang) {
        case Language::ZH_CN: loadChineseSimplified(); break;
        case Language::ZH_TW: loadChineseTraditional(); break;
        case Language::EN_US: loadEnglish(); break;
        case Language::JA_JP: loadJapanese(); break;
    }
}

void Localization::loadChineseSimplified() {
    // Navigation
    m_strings["nav.today"] = "今日";
    m_strings["nav.games"] = "游戏";
    m_strings["nav.search"] = "搜索";
    m_strings["nav.settings"] = "设置";
    
    // Today screen
    m_strings["today.title"] = "今日";
    m_strings["today.featured"] = "今日精选";
    m_strings["today.popular"] = "热门推荐";
    m_strings["today.new"] = "新游戏";
    
    // Games screen
    m_strings["games.title"] = "游戏";
    m_strings["games.all"] = "全部游戏";
    m_strings["games.popular"] = "热门游戏";
    m_strings["games.new"] = "新游戏推荐";
    m_strings["games.action"] = "动作游戏";
    m_strings["games.puzzle"] = "益智游戏";
    
    // Search screen
    m_strings["search.title"] = "搜索";
    m_strings["search.placeholder"] = "搜索游戏和应用";
    m_strings["search.hot"] = "热门搜索";
    m_strings["search.recent"] = "最近搜索";
    m_strings["search.no_results"] = "未找到结果";
    
    // Detail screen
    m_strings["detail.version"] = "版本";
    m_strings["detail.size"] = "大小";
    m_strings["detail.category"] = "分类";
    m_strings["detail.developer"] = "开发者";
    m_strings["detail.release_date"] = "发布日期";
    m_strings["detail.description"] = "简介";
    m_strings["detail.screenshots"] = "截图";
    m_strings["detail.download"] = "下载";
    m_strings["detail.install"] = "安装";
    m_strings["detail.open"] = "打开";
    m_strings["detail.update"] = "更新";
    m_strings["detail.downloading"] = "下载中...";
    m_strings["detail.installing"] = "安装中...";
    
    // Settings screen
    m_strings["settings.title"] = "设置";
    m_strings["settings.appearance"] = "外观";
    m_strings["settings.dark_mode"] = "深色模式";
    m_strings["settings.language"] = "语言";
    m_strings["settings.downloads"] = "下载";
    m_strings["settings.download_dir"] = "下载位置";
    m_strings["settings.install_dir"] = "安装位置";
    m_strings["settings.auto_update"] = "自动检查更新";
    m_strings["settings.cache"] = "缓存";
    m_strings["settings.cache_size"] = "图片缓存大小";
    m_strings["settings.clear_cache"] = "清除缓存";
    m_strings["settings.about"] = "关于";
    m_strings["settings.version"] = "版本";
    m_strings["settings.author"] = "开发者";
    m_strings["settings.source"] = "开源地址";
    
    // Common
    m_strings["common.ok"] = "确定";
    m_strings["common.cancel"] = "取消";
    m_strings["common.back"] = "返回";
    m_strings["common.loading"] = "加载中...";
    m_strings["common.error"] = "错误";
    m_strings["common.success"] = "成功";
    m_strings["common.retry"] = "重试";
}

void Localization::loadChineseTraditional() {
    // Start with simplified, then override
    loadChineseSimplified();
    
    // Override with traditional characters
    m_strings["nav.today"] = "今日";
    m_strings["nav.games"] = "遊戲";
    m_strings["nav.search"] = "搜尋";
    m_strings["nav.settings"] = "設定";
    
    m_strings["games.title"] = "遊戲";
    m_strings["games.all"] = "全部遊戲";
    m_strings["games.popular"] = "熱門遊戲";
    m_strings["games.new"] = "新遊戲推薦";
    
    m_strings["search.placeholder"] = "搜尋遊戲和應用程式";
    m_strings["search.hot"] = "熱門搜尋";
    m_strings["search.recent"] = "最近搜尋";
    m_strings["search.no_results"] = "未找到結果";
    
    m_strings["detail.description"] = "簡介";
    m_strings["settings.appearance"] = "外觀";
    m_strings["settings.downloads"] = "下載";
    m_strings["settings.clear_cache"] = "清除快取";
    m_strings["common.loading"] = "載入中...";
    m_strings["common.error"] = "錯誤";
}

void Localization::loadEnglish() {
    // Navigation
    m_strings["nav.today"] = "Today";
    m_strings["nav.games"] = "Games";
    m_strings["nav.search"] = "Search";
    m_strings["nav.settings"] = "Settings";
    
    // Today screen
    m_strings["today.title"] = "Today";
    m_strings["today.featured"] = "Featured Today";
    m_strings["today.popular"] = "Popular";
    m_strings["today.new"] = "New Games";
    
    // Games screen
    m_strings["games.title"] = "Games";
    m_strings["games.all"] = "All Games";
    m_strings["games.popular"] = "Popular Games";
    m_strings["games.new"] = "New Releases";
    m_strings["games.action"] = "Action Games";
    m_strings["games.puzzle"] = "Puzzle Games";
    
    // Search screen
    m_strings["search.title"] = "Search";
    m_strings["search.placeholder"] = "Search games and apps";
    m_strings["search.hot"] = "Trending";
    m_strings["search.recent"] = "Recent Searches";
    m_strings["search.no_results"] = "No results found";
    
    // Detail screen
    m_strings["detail.version"] = "Version";
    m_strings["detail.size"] = "Size";
    m_strings["detail.category"] = "Category";
    m_strings["detail.developer"] = "Developer";
    m_strings["detail.release_date"] = "Release Date";
    m_strings["detail.description"] = "Description";
    m_strings["detail.screenshots"] = "Screenshots";
    m_strings["detail.download"] = "Download";
    m_strings["detail.install"] = "Install";
    m_strings["detail.open"] = "Open";
    m_strings["detail.update"] = "Update";
    m_strings["detail.downloading"] = "Downloading...";
    m_strings["detail.installing"] = "Installing...";
    
    // Settings screen
    m_strings["settings.title"] = "Settings";
    m_strings["settings.appearance"] = "Appearance";
    m_strings["settings.dark_mode"] = "Dark Mode";
    m_strings["settings.language"] = "Language";
    m_strings["settings.downloads"] = "Downloads";
    m_strings["settings.download_dir"] = "Download Location";
    m_strings["settings.install_dir"] = "Install Location";
    m_strings["settings.auto_update"] = "Check for Updates";
    m_strings["settings.cache"] = "Cache";
    m_strings["settings.cache_size"] = "Image Cache Size";
    m_strings["settings.clear_cache"] = "Clear Cache";
    m_strings["settings.about"] = "About";
    m_strings["settings.version"] = "Version";
    m_strings["settings.author"] = "Developer";
    m_strings["settings.source"] = "Source Code";
    
    // Common
    m_strings["common.ok"] = "OK";
    m_strings["common.cancel"] = "Cancel";
    m_strings["common.back"] = "Back";
    m_strings["common.loading"] = "Loading...";
    m_strings["common.error"] = "Error";
    m_strings["common.success"] = "Success";
    m_strings["common.retry"] = "Retry";
}

void Localization::loadJapanese() {
    // Navigation
    m_strings["nav.today"] = "Today";
    m_strings["nav.games"] = "ゲーム";
    m_strings["nav.search"] = "検索";
    m_strings["nav.settings"] = "設定";
    
    // Today screen
    m_strings["today.title"] = "Today";
    m_strings["today.featured"] = "今日のおすすめ";
    m_strings["today.popular"] = "人気ゲーム";
    m_strings["today.new"] = "新着ゲーム";
    
    // Games screen
    m_strings["games.title"] = "ゲーム";
    m_strings["games.all"] = "すべてのゲーム";
    m_strings["games.popular"] = "人気ゲーム";
    m_strings["games.new"] = "新着リリース";
    m_strings["games.action"] = "アクション";
    m_strings["games.puzzle"] = "パズル";
    
    // Search screen
    m_strings["search.title"] = "検索";
    m_strings["search.placeholder"] = "ゲームとアプリを検索";
    m_strings["search.hot"] = "トレンド";
    m_strings["search.recent"] = "最近の検索";
    m_strings["search.no_results"] = "結果が見つかりません";
    
    // Detail screen
    m_strings["detail.version"] = "バージョン";
    m_strings["detail.size"] = "サイズ";
    m_strings["detail.category"] = "カテゴリ";
    m_strings["detail.developer"] = "開発者";
    m_strings["detail.release_date"] = "リリース日";
    m_strings["detail.description"] = "説明";
    m_strings["detail.screenshots"] = "スクリーンショット";
    m_strings["detail.download"] = "ダウンロード";
    m_strings["detail.install"] = "インストール";
    m_strings["detail.open"] = "開く";
    m_strings["detail.update"] = "アップデート";
    m_strings["detail.downloading"] = "ダウンロード中...";
    m_strings["detail.installing"] = "インストール中...";
    
    // Settings screen
    m_strings["settings.title"] = "設定";
    m_strings["settings.appearance"] = "外観";
    m_strings["settings.dark_mode"] = "ダークモード";
    m_strings["settings.language"] = "言語";
    m_strings["settings.downloads"] = "ダウンロード";
    m_strings["settings.download_dir"] = "ダウンロード先";
    m_strings["settings.install_dir"] = "インストール先";
    m_strings["settings.auto_update"] = "アップデートを確認";
    m_strings["settings.cache"] = "キャッシュ";
    m_strings["settings.cache_size"] = "画像キャッシュサイズ";
    m_strings["settings.clear_cache"] = "キャッシュを消去";
    m_strings["settings.about"] = "このアプリについて";
    m_strings["settings.version"] = "バージョン";
    m_strings["settings.author"] = "開発者";
    m_strings["settings.source"] = "ソースコード";
    
    // Common
    m_strings["common.ok"] = "OK";
    m_strings["common.cancel"] = "キャンセル";
    m_strings["common.back"] = "戻る";
    m_strings["common.loading"] = "読み込み中...";
    m_strings["common.error"] = "エラー";
    m_strings["common.success"] = "成功";
    m_strings["common.retry"] = "再試行";
}
