// =============================================================================
// Switch App Store - Game Installer
// =============================================================================
// Manages installation of NRO files to SD card
// Handles file operations and verification
// =============================================================================

#pragma once

#include <string>
#include <vector>
#include <functional>

// =============================================================================
// Installation status
// =============================================================================
enum class InstallStatus {
    None,
    Preparing,
    Copying,
    Verifying,
    Completed,
    Failed
};

// =============================================================================
// Installed game info
// =============================================================================
struct InstalledGame {
    std::string id;
    std::string name;
    std::string path;       // Full path to NRO
    std::string version;
    std::string iconPath;
    size_t fileSize = 0;
    uint64_t installDate = 0;
};

// =============================================================================
// Installation progress
// =============================================================================
struct InstallProgress {
    InstallStatus status = InstallStatus::None;
    std::string currentFile;
    size_t bytesWritten = 0;
    size_t totalBytes = 0;
    std::string error;
    
    float getProgress() const {
        if (totalBytes == 0) return 0.0f;
        return static_cast<float>(bytesWritten) / totalBytes;
    }
};

// =============================================================================
// Callbacks
// =============================================================================
using InstallProgressCallback = std::function<void(const InstallProgress&)>;
using InstallCompleteCallback = std::function<void(bool success, const std::string& error)>;

// =============================================================================
// GameInstaller - NRO installation manager
// =============================================================================
class GameInstaller {
public:
    // -------------------------------------------------------------------------
    // Singleton access
    // -------------------------------------------------------------------------
    static GameInstaller& getInstance();
    
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    void init(const std::string& installDir);
    
    // -------------------------------------------------------------------------
    // Installation
    // -------------------------------------------------------------------------
    
    // Install from downloaded file
    bool install(const std::string& sourcePath, const std::string& gameName,
                 InstallProgressCallback onProgress = nullptr);
    
    // Uninstall a game
    bool uninstall(const std::string& gameId);
    
    // Check if installation is in progress
    bool isInstalling() const { return m_progress.status != InstallStatus::None &&
                                        m_progress.status != InstallStatus::Completed &&
                                        m_progress.status != InstallStatus::Failed; }
    
    // Get current progress
    const InstallProgress& getProgress() const { return m_progress; }
    
    // -------------------------------------------------------------------------
    // Installed games
    // -------------------------------------------------------------------------
    
    // Get all installed games
    const std::vector<InstalledGame>& getInstalledGames() const { return m_installedGames; }
    
    // Check if a game is installed
    bool isInstalled(const std::string& gameId) const;
    
    // Get installed game by ID
    const InstalledGame* getInstalledGame(const std::string& gameId) const;
    
    // Scan for installed games
    void scanInstalledGames();
    
    // -------------------------------------------------------------------------
    // Verification
    // -------------------------------------------------------------------------
    
    // Verify NRO file integrity
    bool verifyNro(const std::string& path);
    
    // Get NRO metadata (name, version)
    bool getNroInfo(const std::string& path, std::string& name, std::string& version);
    
private:
    GameInstaller() = default;
    ~GameInstaller() = default;
    
    GameInstaller(const GameInstaller&) = delete;
    GameInstaller& operator=(const GameInstaller&) = delete;
    
    // Copy file with progress
    bool copyFileWithProgress(const std::string& src, const std::string& dst,
                               InstallProgressCallback onProgress);
    
    // Generate unique game ID
    std::string generateGameId(const std::string& name);
    
    // Save/load installed games database
    void loadDatabase();
    void saveDatabase();
    
    // -------------------------------------------------------------------------
    // Members
    // -------------------------------------------------------------------------
    
    std::string m_installDir;
    std::string m_databasePath;
    std::vector<InstalledGame> m_installedGames;
    InstallProgress m_progress;
};
