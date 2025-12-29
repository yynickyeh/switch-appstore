// =============================================================================
// Switch App Store - Image Implementation
// =============================================================================

#include "Image.hpp"
#include "ui/Theme.hpp"
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

Image::Image() = default;

Image::Image(const std::string& url)
    : m_url(url)
{
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
    m_isLoading = true;
    m_ownsTexture = false;
    
    // TODO: Start async loading via ImageCache
}

void Image::setTexture(SDL_Texture* texture) {
    // Release old texture if we own it
    if (m_ownsTexture && m_texture) {
        SDL_DestroyTexture(m_texture);
    }
    
    m_texture = texture;
    m_ownsTexture = false;
    m_isLoading = false;
}

// =============================================================================
// Update
// =============================================================================

void Image::update(float deltaTime) {
    Component::update(deltaTime);
    
    // Animate loading indicator
    if (m_isLoading) {
        m_loadingAnimation += deltaTime * 2.0f;
        if (m_loadingAnimation > 6.28f) {  // 2*PI
            m_loadingAnimation -= 6.28f;
        }
    }
}

// =============================================================================
// Rendering
// =============================================================================

void Image::render(Renderer& renderer, Theme& theme) {
    if (!m_visible) return;
    
    // If no texture, draw placeholder
    if (!m_texture) {
        // Draw rounded rect placeholder
        if (m_cornerRadius > 0) {
            renderer.drawRoundedRect(m_bounds, m_cornerRadius, m_placeholderColor);
        } else {
            renderer.drawRect(m_bounds, m_placeholderColor);
        }
        
        // Draw loading indicator if loading
        if (m_isLoading) {
            float centerX = m_bounds.x + m_bounds.w / 2;
            float centerY = m_bounds.y + m_bounds.h / 2;
            float radius = std::min(m_bounds.w, m_bounds.h) / 6;
            
            // Simple spinning indicator (8 dots)
            for (int i = 0; i < 8; i++) {
                float angle = m_loadingAnimation + (i * 0.785f);  // 2*PI/8
                float dotX = centerX + std::cos(angle) * radius;
                float dotY = centerY + std::sin(angle) * radius;
                float dotSize = 3.0f + (i * 0.5f);
                Uint8 alpha = static_cast<Uint8>(255 * (i + 1) / 8.0f);
                
                renderer.drawCircle(dotX, dotY, dotSize, Color(150, 150, 150, alpha));
            }
        }
        return;
    }
    
    // Get texture dimensions
    int texW, texH;
    SDL_QueryTexture(m_texture, nullptr, nullptr, &texW, &texH);
    
    // Calculate source and destination rectangles
    Rect srcRect, dstRect;
    calculateRects(srcRect, dstRect, texW, texH);
    
    // Apply opacity
    Uint8 alpha = static_cast<Uint8>(255 * m_opacity);
    
    // Draw the image
    // Note: For rounded corners, we'd need to use a stencil or render to texture
    // For now, just draw the texture
    renderer.drawTexture(m_texture, srcRect, dstRect);
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
