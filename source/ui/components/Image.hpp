// =============================================================================
// Switch App Store - Image Component
// =============================================================================
// Displays images loaded from network or local storage
// Supports async loading with placeholder, rounded corners, and fit modes
// Integrates with ImageCache for automatic loading and state tracking
// =============================================================================

#pragma once

#include "ui/Component.hpp"
#include "network/ImageCache.hpp"  // For ImageLoadState
#include <string>

// =============================================================================
// Image fit modes
// =============================================================================
enum class ImageFit {
    Fill,       // Stretch to fill (may distort)
    Contain,    // Scale to fit inside (may have letterbox)
    Cover,      // Scale to cover (may crop)
    ScaleDown   // Only scale down, never up
};

// =============================================================================
// Image - Image display component with loading animation support
// =============================================================================
class Image : public Component {
public:
    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    Image();
    explicit Image(const std::string& url);
    ~Image() override;
    
    // -------------------------------------------------------------------------
    // Component overrides
    // -------------------------------------------------------------------------
    void render(Renderer& renderer, Theme& theme) override;
    void update(float deltaTime) override;
    
    // Images typically don't receive focus
    bool canFocus() const override { return false; }
    
    // -------------------------------------------------------------------------
    // Image-specific properties
    // -------------------------------------------------------------------------
    
    // Set image source (URL or local path)
    void setSource(const std::string& url);
    const std::string& getSource() const { return m_url; }
    
    // Set the SDL_Texture directly (for cached images)
    void setTexture(SDL_Texture* texture);
    
    // Fit mode
    void setFitMode(ImageFit fit) { m_fitMode = fit; }
    ImageFit getFitMode() const { return m_fitMode; }
    
    // Corner radius (for rounded images like app icons)
    void setCornerRadius(int radius) { m_cornerRadius = radius; }
    int getCornerRadius() const { return m_cornerRadius; }
    
    // Placeholder color (shown while loading)
    void setPlaceholderColor(const Color& color) { m_placeholderColor = color; }
    
    // Is the image loaded?
    bool isLoaded() const { return m_texture != nullptr; }
    
    // Get current load state
    ImageLoadState getLoadState() const { return m_loadState; }
    
    // Enable/disable loading animation
    void setShowLoadingAnimation(bool show) { m_showLoadingAnimation = show; }
    bool getShowLoadingAnimation() const { return m_showLoadingAnimation; }
    
    // Enable/disable error placeholder
    void setShowErrorPlaceholder(bool show) { m_showErrorPlaceholder = show; }
    bool getShowErrorPlaceholder() const { return m_showErrorPlaceholder; }
    
private:
    std::string m_url;
    SDL_Texture* m_texture = nullptr;
    bool m_ownsTexture = false;     // Should we destroy the texture?
    ImageFit m_fitMode = ImageFit::Cover;
    int m_cornerRadius = 0;
    Color m_placeholderColor{200, 200, 200, 255};
    
    // Loading state - integrated with ImageCache
    ImageLoadState m_loadState = ImageLoadState::Idle;
    bool m_showLoadingAnimation = true;
    bool m_showErrorPlaceholder = true;
    float m_loadingSpinAngle = 0.0f;
    
    // Fade-in animation when image loads
    float m_fadeInProgress = 0.0f;
    bool m_useFadeIn = true;
    
    // Calculate source and destination rects based on fit mode
    void calculateRects(Rect& srcRect, Rect& dstRect, int texW, int texH);
    
    // Render different states
    void renderPlaceholder(Renderer& renderer, Theme& theme);
    void renderLoadingAnimation(Renderer& renderer, Theme& theme);
    void renderErrorPlaceholder(Renderer& renderer, Theme& theme);
};

