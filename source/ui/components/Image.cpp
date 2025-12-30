// =============================================================================
// Switch App Store - Image Implementation
// =============================================================================
// Implements image rendering with loading states, animations, and error handling
// Integrates with ImageCache for automatic async loading
// =============================================================================

#include "Image.hpp"
#include "ui/Theme.hpp"
#include "network/ImageCache.hpp"
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

Image::Image() = default;

Image::Image(const std::string& url)
    : m_url(url)
{
    // Request the image to be loaded via ImageCache
    if (!url.empty()) {
        ImageCache::getInstance().requestImage(url);
        m_loadState = ImageLoadState::Loading;
    }
}

Image::~Image() {
    if (m_ownsTexture && m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

// =============================================================================
// Source Management
// =============================================================================

void Image::setSource(const std::string& url) {
    if (m_url == url) return;
    
    // Release old texture if we own it
    if (m_ownsTexture && m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
    
    m_url = url;
    m_ownsTexture = false;
    m_fadeInProgress = 0.0f;
    
    // -------------------------------------------------------------------------
    // Request loading via ImageCache - this automatically handles:
    // - Checking if already cached
    // - Queueing for async download
    // - Notifying via state callback when done
    // -------------------------------------------------------------------------
    if (!url.empty()) {
        ImageCache::getInstance().requestImage(url);
        m_loadState = ImageLoadState::Loading;
    } else {
        m_loadState = ImageLoadState::Idle;
    }
}

void Image::setTexture(SDL_Texture* texture) {
    // Release old texture if we own it
    if (m_ownsTexture && m_texture) {
        SDL_DestroyTexture(m_texture);
    }
    
    m_texture = texture;
    m_ownsTexture = false;
    m_loadState = texture ? ImageLoadState::Loaded : ImageLoadState::Idle;
    m_fadeInProgress = 1.0f;  // No fade-in for directly set textures
}

// =============================================================================
// Update
// =============================================================================

void Image::update(float deltaTime) {
    Component::update(deltaTime);
    
    // -------------------------------------------------------------------------
    // Check ImageCache for texture availability
    // This polls the cache each frame to see if the image has loaded
    // -------------------------------------------------------------------------
    if (!m_texture && !m_url.empty()) {
        SDL_Texture* cached = ImageCache::getInstance().getCached(m_url);
        if (cached) {
            m_texture = cached;
            m_ownsTexture = false;  // ImageCache owns this texture
            m_loadState = ImageLoadState::Loaded;
            m_fadeInProgress = 0.0f;  // Start fade-in
        } else {
            // Update load state from cache
            m_loadState = ImageCache::getInstance().getLoadState(m_url);
        }
    }
    
    // -------------------------------------------------------------------------
    // Animate loading spinner (rotates at 1 revolution per second)
    // -------------------------------------------------------------------------
    if (m_loadState == ImageLoadState::Loading && m_showLoadingAnimation) {
        m_loadingSpinAngle += deltaTime * 360.0f;
        if (m_loadingSpinAngle >= 360.0f) {
            m_loadingSpinAngle -= 360.0f;
        }
    }
    
    // -------------------------------------------------------------------------
    // Fade-in animation when image loads (0.25 second fade)
    // -------------------------------------------------------------------------
    if (m_useFadeIn && m_texture && m_fadeInProgress < 1.0f) {
        m_fadeInProgress += deltaTime * 4.0f;  // 4.0 = 1/0.25s
        if (m_fadeInProgress > 1.0f) {
            m_fadeInProgress = 1.0f;
        }
    }
}

// =============================================================================
// Rendering
// =============================================================================

void Image::render(Renderer& renderer, Theme& theme) {
    if (!m_visible) return;
    
    // -------------------------------------------------------------------------
    // Render based on current load state
    // -------------------------------------------------------------------------
    switch (m_loadState) {
        case ImageLoadState::Idle:
        case ImageLoadState::Loading:
            renderPlaceholder(renderer, theme);
            if (m_showLoadingAnimation && m_loadState == ImageLoadState::Loading) {
                renderLoadingAnimation(renderer, theme);
            }
            break;
            
        case ImageLoadState::Failed:
            if (m_showErrorPlaceholder) {
                renderErrorPlaceholder(renderer, theme);
            } else {
                renderPlaceholder(renderer, theme);
            }
            break;
            
        case ImageLoadState::Loaded:
            if (m_texture) {
                // Get texture dimensions
                int texW, texH;
                SDL_QueryTexture(m_texture, nullptr, nullptr, &texW, &texH);
                
                // Calculate source and destination rectangles
                Rect srcRect, dstRect;
                calculateRects(srcRect, dstRect, texW, texH);
                
                // Apply fade-in by drawing placeholder underneath
                if (m_fadeInProgress < 1.0f) {
                    renderPlaceholder(renderer, theme);
                }
                
                // Draw the image with fade-in opacity
                // Note: SDL_SetTextureAlphaMod would be ideal here
                renderer.drawTexture(m_texture, srcRect, dstRect);
            } else {
                renderPlaceholder(renderer, theme);
            }
            break;
    }
}

void Image::renderPlaceholder(Renderer& renderer, Theme& theme) {
    // Draw rounded rect placeholder
    if (m_cornerRadius > 0) {
        renderer.drawRoundedRect(m_bounds, m_cornerRadius, m_placeholderColor);
    } else {
        renderer.drawRect(m_bounds, m_placeholderColor);
    }
}

void Image::renderLoadingAnimation(Renderer& renderer, Theme& theme) {
    // -------------------------------------------------------------------------
    // Draw a spinning loading indicator in the center
    // Uses 8 dots arranged in a circle with varying opacity
    // The dots rotate smoothly to indicate loading progress
    // -------------------------------------------------------------------------
    float centerX = m_bounds.x + m_bounds.w / 2;
    float centerY = m_bounds.y + m_bounds.h / 2;
    float radius = std::min(m_bounds.w, m_bounds.h) / 6;
    
    // Minimum radius constraint
    if (radius < 8.0f) radius = 8.0f;
    
    // Convert angle to radians
    float angleRad = m_loadingSpinAngle * 3.14159f / 180.0f;
    
    // Draw 8 dots in a circle with varying opacity
    for (int i = 0; i < 8; i++) {
        float dotAngle = angleRad + (i * 3.14159f / 4.0f);  // 2*PI/8
        float dotX = centerX + std::cos(dotAngle) * radius;
        float dotY = centerY + std::sin(dotAngle) * radius;
        float dotSize = 2.0f + (i * 0.3f);
        
        // Fade from transparent to opaque around the circle
        Uint8 alpha = static_cast<Uint8>(60 + (195 * (i + 1) / 8.0f));
        
        // Use theme-aware color or neutral gray
        Color dotColor(100, 100, 100, alpha);
        renderer.drawCircle(dotX, dotY, dotSize, dotColor);
    }
}

void Image::renderErrorPlaceholder(Renderer& renderer, Theme& theme) {
    // -------------------------------------------------------------------------
    // Draw an error placeholder with an X mark to indicate load failure
    // Uses a muted red/gray color to indicate the error state
    // -------------------------------------------------------------------------
    
    // Background
    Color errorBgColor(230, 220, 220, 255);  // Light pinkish gray
    if (m_cornerRadius > 0) {
        renderer.drawRoundedRect(m_bounds, m_cornerRadius, errorBgColor);
    } else {
        renderer.drawRect(m_bounds, errorBgColor);
    }
    
    // Draw X mark in the center
    float centerX = m_bounds.x + m_bounds.w / 2;
    float centerY = m_bounds.y + m_bounds.h / 2;
    float size = std::min(m_bounds.w, m_bounds.h) / 4;
    
    // Minimum size constraint
    if (size < 12.0f) size = 12.0f;
    
    Color errorColor(180, 100, 100, 180);  // Muted red
    
    // Draw X using two diagonal lines
    renderer.drawLine(
        centerX - size, centerY - size,
        centerX + size, centerY + size,
        errorColor, 2
    );
    renderer.drawLine(
        centerX + size, centerY - size,
        centerX - size, centerY + size,
        errorColor, 2
    );
}

// =============================================================================
// Rect Calculation
// =============================================================================

void Image::calculateRects(Rect& srcRect, Rect& dstRect, int texW, int texH) {
    // Source rect (portion of texture to draw)
    srcRect = Rect(0, 0, static_cast<float>(texW), static_cast<float>(texH));
    
    // Destination rect (where to draw on screen)
    dstRect = m_bounds;
    
    float boundsRatio = m_bounds.w / m_bounds.h;
    float texRatio = static_cast<float>(texW) / texH;
    
    switch (m_fitMode) {
        case ImageFit::Fill:
            // Use full source and destination (stretch)
            break;
            
        case ImageFit::Contain:
            // Scale to fit inside bounds
            if (texRatio > boundsRatio) {
                // Texture is wider - fit width
                float newH = m_bounds.w / texRatio;
                dstRect.y += (m_bounds.h - newH) / 2;
                dstRect.h = newH;
            } else {
                // Texture is taller - fit height
                float newW = m_bounds.h * texRatio;
                dstRect.x += (m_bounds.w - newW) / 2;
                dstRect.w = newW;
            }
            break;
            
        case ImageFit::Cover:
            // Scale to cover bounds (may crop)
            if (texRatio > boundsRatio) {
                // Texture is wider - crop sides
                float newW = texH * boundsRatio;
                srcRect.x = (texW - newW) / 2;
                srcRect.w = newW;
            } else {
                // Texture is taller - crop top/bottom
                float newH = texW / boundsRatio;
                srcRect.y = (texH - newH) / 2;
                srcRect.h = newH;
            }
            break;
            
        case ImageFit::ScaleDown:
            // Like Contain, but don't scale up
            if (texW <= m_bounds.w && texH <= m_bounds.h) {
                // Center without scaling
                dstRect.x += (m_bounds.w - texW) / 2;
                dstRect.y += (m_bounds.h - texH) / 2;
                dstRect.w = static_cast<float>(texW);
                dstRect.h = static_cast<float>(texH);
            } else {
                // Scale down like Contain
                if (texRatio > boundsRatio) {
                    float newH = m_bounds.w / texRatio;
                    dstRect.y += (m_bounds.h - newH) / 2;
                    dstRect.h = newH;
                } else {
                    float newW = m_bounds.h * texRatio;
                    dstRect.x += (m_bounds.w - newW) / 2;
                    dstRect.w = newW;
                }
            }
            break;
    }
}
