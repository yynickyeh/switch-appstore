// =============================================================================
// Switch App Store - Game Installer Implementation
// =============================================================================

#include "GameInstaller.hpp"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

// =============================================================================
// Singleton
// =============================================================================

GameInstaller& GameInstaller::getInstance() {
    static GameInstaller instance;
    return instance;
}

// =============================================================================
// Initialization
// =============================================================================

void GameInstaller::init(const std::string& installDir) {
    m_installDir = installDir;
    m_databasePath = installDir + "/installed.json";
    
    // Create install directory if needed
    mkdir(installDir.c_str(), 0755);
    
    // Load installed games database
    loadDatabase();
    
    // Scan for any manually added games
    scanInstalledGames();
}

// =============================================================================
// Installation
// =============================================================================

bool GameInstaller::install(const std::string& sourcePath, const std::string& gameName,
                             InstallProgressCallback onProgress) {
    m_progress = InstallProgress();
    m_progress.status = InstallStatus::Preparing;
    m_progress.currentFile = gameName;
    
    if (onProgress) onProgress(m_progress);
    
    // Generate destination path
    std::string gameId = generateGameId(gameName);
    std::string destPath = m_installDir + "/" + gameId + ".nro";
    
    // Check if source exists
    struct stat st;
    if (stat(sourcePath.c_str(), &st) != 0) {
        m_progress.status = InstallStatus::Failed;
        m_progress.error = "Source file not found";
        if (onProgress) onProgress(m_progress);
        return false;
    }
    
    m_progress.totalBytes = st.st_size;
    m_progress.status = InstallStatus::Copying;
    if (onProgress) onProgress(m_progress);
    
    // Copy file
    if (!copyFileWithProgress(sourcePath, destPath, onProgress)) {
        m_progress.status = InstallStatus::Failed;
        m_progress.error = "Failed to copy file";
        if (onProgress) onProgress(m_progress);
        return false;
    }
    
    // Verify
    m_progress.status = InstallStatus::Verifying;
    if (onProgress) onProgress(m_progress);
    
    if (!verifyNro(destPath)) {
        remove(destPath.c_str());
        m_progress.status = InstallStatus::Failed;
        m_progress.error = "NRO verification failed";
        if (onProgress) onProgress(m_progress);
        return false;
    }
    
    // Add to installed games
    InstalledGame game;
    game.id = gameId;
    game.name = gameName;
    game.path = destPath;
    game.fileSize = m_progress.totalBytes;
    game.installDate = static_cast<uint64_t>(time(nullptr));
    
    // Try to get version from NRO
    getNroInfo(destPath, game.name, game.version);
    
    m_installedGames.push_back(game);
    saveDatabase();
    
    m_progress.status = InstallStatus::Completed;
    m_progress.bytesWritten = m_progress.totalBytes;
    if (onProgress) onProgress(m_progress);
    
    return true;
}

bool GameInstaller::uninstall(const std::string& gameId) {
    auto it = std::find_if(m_installedGames.begin(), m_installedGames.end(),
                            [&gameId](const InstalledGame& g) { return g.id == gameId; });
    
    if (it == m_installedGames.end()) {
        return false;
    }
    
    // Delete the file
    if (remove(it->path.c_str()) != 0) {
        return false;
    }
    
    // Remove from database
    m_installedGames.erase(it);
    saveDatabase();
    
    return true;
}

// =============================================================================
// Installed Games
// =============================================================================

bool GameInstaller::isInstalled(const std::string& gameId) const {
    for (const auto& game : m_installedGames) {
        if (game.id == gameId) {
            return true;
        }
    }
    return false;
}

const InstalledGame* GameInstaller::getInstalledGame(const std::string& gameId) const {
    for (const auto& game : m_installedGames) {
        if (game.id == gameId) {
            return &game;
        }
    }
    return nullptr;
}

void GameInstaller::scanInstalledGames() {
    DIR* dir = opendir(m_installDir.c_str());
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        
        // Check for .nro extension
        if (name.length() > 4 && name.substr(name.length() - 4) == ".nro") {
            std::string fullPath = m_installDir + "/" + name;
            std::string gameId = name.substr(0, name.length() - 4);
            
            // Check if already in database
            if (isInstalled(gameId)) continue;
            
            // Add to database
            InstalledGame game;
            game.id = gameId;
            game.path = fullPath;
            
            // Get file size
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0) {
                game.fileSize = st.st_size;
            }
            
            // Try to get name from NRO metadata
            getNroInfo(fullPath, game.name, game.version);
            if (game.name.empty()) {
                game.name = gameId;
            }
            
            m_installedGames.push_back(game);
        }
    }
    
    closedir(dir);
    saveDatabase();
}

// =============================================================================
// Verification
// =============================================================================

bool GameInstaller::verifyNro(const std::string& path) {
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) return false;
    
    // Check NRO magic number (first 4 bytes at offset 0x10 should be "NRO0")
    fseek(file, 0x10, SEEK_SET);
    char magic[4];
    size_t read = fread(magic, 1, 4, file);
    fclose(file);
    
    if (read != 4) return false;
    
    return (magic[0] == 'N' && magic[1] == 'R' && 
            magic[2] == 'O' && magic[3] == '0');
}

bool GameInstaller::getNroInfo(const std::string& path, 
                                std::string& name, std::string& version) {
    // NRO assets contain NACP data with name/version
    // Simplified implementation - real one would parse NACP properly
    
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) return false;
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    
    // Read NRO header to find asset section
    fseek(file, 0, SEEK_SET);
    
    // For now, just return false (would need full NRO parsing)
    fclose(file);
    
    // Return default values
    if (name.empty()) {
        // Extract name from path
        size_t slash = path.rfind('/');
        size_t dot = path.rfind('.');
        if (slash != std::string::npos && dot != std::string::npos && dot > slash) {
            name = path.substr(slash + 1, dot - slash - 1);
        }
    }
    
    if (version.empty()) {
        version = "1.0.0";
    }
    
    return true;
}

// =============================================================================
// File Operations
// =============================================================================

bool GameInstaller::copyFileWithProgress(const std::string& src, const std::string& dst,
                                          InstallProgressCallback onProgress) {
    FILE* srcFile = fopen(src.c_str(), "rb");
    if (!srcFile) return false;
    
    FILE* dstFile = fopen(dst.c_str(), "wb");
    if (!dstFile) {
        fclose(srcFile);
        return false;
    }
    
    const size_t BUFFER_SIZE = 1024 * 1024;  // 1 MB buffer
    std::vector<char> buffer(BUFFER_SIZE);
    
    size_t totalWritten = 0;
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer.data(), 1, BUFFER_SIZE, srcFile)) > 0) {
        size_t written = fwrite(buffer.data(), 1, bytesRead, dstFile);
        if (written != bytesRead) {
            fclose(srcFile);
            fclose(dstFile);
            remove(dst.c_str());
            return false;
        }
        
        totalWritten += written;
        
        // Update progress
        m_progress.bytesWritten = totalWritten;
        if (onProgress) {
            onProgress(m_progress);
        }
    }
    
    fclose(srcFile);
    fclose(dstFile);
    
    return true;
}

std::string GameInstaller::generateGameId(const std::string& name) {
    // Simple ID generation: lowercase, replace spaces with underscores
    std::string id;
    for (char c : name) {
        if (isalnum(c)) {
            id += static_cast<char>(tolower(c));
        } else if (c == ' ' || c == '-') {
            id += '_';
        }
    }
    
    // Add timestamp to ensure uniqueness
    char buf[16];
    snprintf(buf, sizeof(buf), "_%lx", static_cast<unsigned long>(time(nullptr)));
    id += buf;
    
    return id;
}

// =============================================================================
// Database
// =============================================================================

void GameInstaller::loadDatabase() {
    FILE* file = fopen(m_databasePath.c_str(), "r");
    if (!file) return;
    
    // Read file (simplified JSON parsing would go here)
    fclose(file);
}

void GameInstaller::saveDatabase() {
    FILE* file = fopen(m_databasePath.c_str(), "w");
    if (!file) return;
    
    fprintf(file, "{\n  \"games\": [\n");
    for (size_t i = 0; i < m_installedGames.size(); i++) {
        const auto& g = m_installedGames[i];
        fprintf(file, "    {\"id\":\"%s\",\"name\":\"%s\",\"path\":\"%s\",\"version\":\"%s\"}%s\n",
                g.id.c_str(), g.name.c_str(), g.path.c_str(), g.version.c_str(),
                i < m_installedGames.size() - 1 ? "," : "");
    }
    fprintf(file, "  ]\n}\n");
    fclose(file);
}
