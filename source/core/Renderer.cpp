// =============================================================================
// Switch App Store - Renderer Implementation
// =============================================================================

#include "Renderer.hpp"
#include <algorithm>

// =============================================================================
// Constructor & Destructor
// =============================================================================

Renderer::Renderer(SDL_Renderer* sdlRenderer, float scale)
    : m_sdlRenderer(sdlRenderer)
    , m_scale(scale)
    , m_regularFontPath("romfs:/fonts/NotoSansCJKsc-Regular.otf")
    , m_boldFontPath("romfs:/fonts/NotoSansCJKsc-Regular.otf")
{
}

Renderer::~Renderer() {
    // Clean up all cached fonts
    for (auto& pair : m_fontCache) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    m_fontCache.clear();
}

bool Renderer::init() {
    // Fonts will be loaded on demand via getFont()
    // Just verify the font paths exist
    // For now, we'll use system fonts if custom fonts aren't available
    return true;
}

// =============================================================================
// Basic Drawing Operations
// =============================================================================

void Renderer::drawRect(const Rect& rect, const Color& color) {
    SDL_SetRenderDrawColor(m_sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_Rect sdlRect = rect.toSDL(m_scale);
    SDL_RenderFillRect(m_sdlRenderer, &sdlRect);
}

void Renderer::drawRectOutline(const Rect& rect, const Color& color, int thickness) {
    SDL_SetRenderDrawColor(m_sdlRenderer, color.r, color.g, color.b, color.a);
    
    int t = static_cast<int>(thickness * m_scale);
    SDL_Rect sdlRect = rect.toSDL(m_scale);
    
    // Draw four sides
    SDL_Rect top = {sdlRect.x, sdlRect.y, sdlRect.w, t};
    SDL_Rect bottom = {sdlRect.x, sdlRect.y + sdlRect.h - t, sdlRect.w, t};
    SDL_Rect left = {sdlRect.x, sdlRect.y, t, sdlRect.h};
    SDL_Rect right = {sdlRect.x + sdlRect.w - t, sdlRect.y, t, sdlRect.h};
    
    SDL_RenderFillRect(m_sdlRenderer, &top);
    SDL_RenderFillRect(m_sdlRenderer, &bottom);
    SDL_RenderFillRect(m_sdlRenderer, &left);
    SDL_RenderFillRect(m_sdlRenderer, &right);
}

void Renderer::drawRoundedRect(const Rect& rect, int radius, const Color& color) {
    // Use SDL2_gfx for rounded rectangles
    int x1 = static_cast<int>(rect.x * m_scale);
    int y1 = static_cast<int>(rect.y * m_scale);
    int x2 = static_cast<int>((rect.x + rect.w) * m_scale);
    int y2 = static_cast<int>((rect.y + rect.h) * m_scale);
    int r = static_cast<int>(radius * m_scale);
    
    roundedBoxRGBA(m_sdlRenderer, x1, y1, x2, y2, r, 
                   color.r, color.g, color.b, color.a);
}

void Renderer::drawRoundedRectOutline(const Rect& rect, int radius, 
                                       const Color& color, int thickness) {
    int x1 = static_cast<int>(rect.x * m_scale);
    int y1 = static_cast<int>(rect.y * m_scale);
    int x2 = static_cast<int>((rect.x + rect.w) * m_scale);
    int y2 = static_cast<int>((rect.y + rect.h) * m_scale);
    int r = static_cast<int>(radius * m_scale);
    
    roundedRectangleRGBA(m_sdlRenderer, x1, y1, x2, y2, r,
                         color.r, color.g, color.b, color.a);
}

void Renderer::drawCircle(float x, float y, float radius, const Color& color) {
    int cx = static_cast<int>(x * m_scale);
    int cy = static_cast<int>(y * m_scale);
    int r = static_cast<int>(radius * m_scale);
    
    filledCircleRGBA(m_sdlRenderer, cx, cy, r, 
                     color.r, color.g, color.b, color.a);
}

void Renderer::drawCircleOutline(float x, float y, float radius, 
                                  const Color& color, int thickness) {
    int cx = static_cast<int>(x * m_scale);
    int cy = static_cast<int>(y * m_scale);
    int r = static_cast<int>(radius * m_scale);
    
    circleRGBA(m_sdlRenderer, cx, cy, r, 
               color.r, color.g, color.b, color.a);
}

void Renderer::drawLine(float x1, float y1, float x2, float y2, 
                        const Color& color, int thickness) {
    int sx1 = static_cast<int>(x1 * m_scale);
    int sy1 = static_cast<int>(y1 * m_scale);
    int sx2 = static_cast<int>(x2 * m_scale);
    int sy2 = static_cast<int>(y2 * m_scale);
    int t = static_cast<int>(thickness * m_scale);
    
    if (t <= 1) {
        lineRGBA(m_sdlRenderer, sx1, sy1, sx2, sy2, 
                 color.r, color.g, color.b, color.a);
    } else {
        thickLineRGBA(m_sdlRenderer, sx1, sy1, sx2, sy2, t,
                      color.r, color.g, color.b, color.a);
    }
}

// =============================================================================
// Gradient Drawing
// =============================================================================

void Renderer::drawGradientRect(const Rect& rect, 
                                const Color& topColor, const Color& bottomColor) {
    SDL_Rect sdlRect = rect.toSDL(m_scale);
    
    // Draw gradient line by line
    for (int y = 0; y < sdlRect.h; y++) {
        float t = static_cast<float>(y) / sdlRect.h;
        
        Uint8 r = static_cast<Uint8>(topColor.r + (bottomColor.r - topColor.r) * t);
        Uint8 g = static_cast<Uint8>(topColor.g + (bottomColor.g - topColor.g) * t);
        Uint8 b = static_cast<Uint8>(topColor.b + (bottomColor.b - topColor.b) * t);
        Uint8 a = static_cast<Uint8>(topColor.a + (bottomColor.a - topColor.a) * t);
        
        SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, a);
        SDL_RenderDrawLine(m_sdlRenderer, 
                          sdlRect.x, sdlRect.y + y, 
                          sdlRect.x + sdlRect.w, sdlRect.y + y);
    }
}

void Renderer::drawGradientRoundedRect(const Rect& rect, int radius,
                                        const Color& topColor, const Color& bottomColor) {
    // For simplicity, just use solid color rounded rect for now
    // A proper implementation would need custom shader or more complex clipping
    Color avgColor(
        (topColor.r + bottomColor.r) / 2,
        (topColor.g + bottomColor.g) / 2,
        (topColor.b + bottomColor.b) / 2,
        (topColor.a + bottomColor.a) / 2
    );
    drawRoundedRect(rect, radius, avgColor);
}

// =============================================================================
// Text Rendering
// =============================================================================

void Renderer::drawText(const std::string& text, float x, float y, 
                        int fontSize, const Color& color,
                        FontWeight weight, TextAlign align) {
    if (text.empty()) return;
    
    TTF_Font* font = getFont(fontSize, weight);
    if (!font) return;
    
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    
    // Render text to surface
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;
    
    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_sdlRenderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    // Calculate position based on alignment
    int sx = static_cast<int>(x * m_scale);
    int sy = static_cast<int>(y * m_scale);
    
    if (align == TextAlign::Center) {
        sx -= surface->w / 2;
    } else if (align == TextAlign::Right) {
        sx -= surface->w;
    }
    
    SDL_Rect destRect = {sx, sy, surface->w, surface->h};
    SDL_RenderCopy(m_sdlRenderer, texture, nullptr, &destRect);
    
    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void Renderer::drawTextInRect(const std::string& text, const Rect& rect,
                              int fontSize, const Color& color,
                              FontWeight weight, TextAlign align,
                              TextVAlign valign) {
    if (text.empty()) return;
    
    TTF_Font* font = getFont(fontSize, weight);
    if (!font) return;
    
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    
    // Render text to surface
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_sdlRenderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect dstRect = rect.toSDL(m_scale);
    
    // Calculate horizontal position
    int textX = dstRect.x;
    if (align == TextAlign::Center) {
        textX = dstRect.x + (dstRect.w - surface->w) / 2;
    } else if (align == TextAlign::Right) {
        textX = dstRect.x + dstRect.w - surface->w;
    }
    
    // Calculate vertical position
    int textY = dstRect.y;
    if (valign == TextVAlign::Middle) {
        textY = dstRect.y + (dstRect.h - surface->h) / 2;
    } else if (valign == TextVAlign::Bottom) {
        textY = dstRect.y + dstRect.h - surface->h;
    }
    
    // Set clip rect to prevent overflow
    SDL_RenderSetClipRect(m_sdlRenderer, &dstRect);
    
    SDL_Rect destRect = {textX, textY, surface->w, surface->h};
    SDL_RenderCopy(m_sdlRenderer, texture, nullptr, &destRect);
    
    // Clear clip rect
    SDL_RenderSetClipRect(m_sdlRenderer, nullptr);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

float Renderer::measureText(const std::string& text, int fontSize, FontWeight weight) {
    TTF_Font* font = getFont(fontSize, weight);
    if (!font) return 0.0f;
    
    int w, h;
    TTF_SizeUTF8(font, text.c_str(), &w, &h);
    
    return static_cast<float>(w) / m_scale;
}

// =============================================================================
// Image Rendering
// =============================================================================

void Renderer::drawTexture(SDL_Texture* texture, const Rect& destRect) {
    if (!texture) return;
    
    SDL_Rect dstRect = destRect.toSDL(m_scale);
    SDL_RenderCopy(m_sdlRenderer, texture, nullptr, &dstRect);
}

void Renderer::drawTexture(SDL_Texture* texture, const Rect& destRect, Uint8 alpha) {
    if (!texture) return;
    
    SDL_SetTextureAlphaMod(texture, alpha);
    drawTexture(texture, destRect);
    SDL_SetTextureAlphaMod(texture, 255);
}

void Renderer::drawTexture(SDL_Texture* texture, const Rect& srcRect, const Rect& destRect) {
    if (!texture) return;
    
    SDL_Rect src = srcRect.toSDL(1.0f);  // Source is not scaled
    SDL_Rect dst = destRect.toSDL(m_scale);
    SDL_RenderCopy(m_sdlRenderer, texture, &src, &dst);
}

// =============================================================================
// Clipping
// =============================================================================

void Renderer::setClipRect(const Rect& rect) {
    SDL_Rect clipRect = rect.toSDL(m_scale);
    SDL_RenderSetClipRect(m_sdlRenderer, &clipRect);
}

void Renderer::clearClipRect() {
    SDL_RenderSetClipRect(m_sdlRenderer, nullptr);
}

// =============================================================================
// Transform Stack
// =============================================================================

void Renderer::pushTransform(float offsetX, float offsetY) {
    m_transformStack.push_back({offsetX, offsetY});
    m_totalOffsetX += offsetX;
    m_totalOffsetY += offsetY;
}

void Renderer::popTransform() {
    if (!m_transformStack.empty()) {
        auto& last = m_transformStack.back();
        m_totalOffsetX -= last.first;
        m_totalOffsetY -= last.second;
        m_transformStack.pop_back();
    }
}

// =============================================================================
// Shadow Effects
// =============================================================================

void Renderer::drawShadow(const Rect& rect, int radius, int blur, 
                          int offsetX, int offsetY, const Color& color) {
    // Simple shadow implementation using multiple translucent layers
    // For better performance, consider pre-rendering shadow textures
    
    int layers = blur / 2;
    if (layers < 1) layers = 1;
    
    for (int i = layers; i >= 0; i--) {
        float expand = static_cast<float>(i * 2);
        Uint8 alpha = static_cast<Uint8>(color.a * (1.0f - (float)i / layers) * 0.3f);
        
        Rect shadowRect(
            rect.x + offsetX - expand / 2,
            rect.y + offsetY - expand / 2,
            rect.w + expand,
            rect.h + expand
        );
        
        Color shadowColor(color.r, color.g, color.b, alpha);
        drawRoundedRect(shadowRect, radius + i, shadowColor);
    }
}

// =============================================================================
// Font Management
// =============================================================================

TTF_Font* Renderer::getFont(int size, FontWeight weight) {
    // Scale the font size
    int scaledSize = static_cast<int>(size * m_scale);
    
    // Create cache key: combine size and weight
    int key = (scaledSize << 8) | static_cast<int>(weight);
    
    // Check cache
    auto it = m_fontCache.find(key);
    if (it != m_fontCache.end()) {
        return it->second;
    }
    
    // Determine font path based on weight
    const std::string& fontPath = (weight == FontWeight::Bold || 
                                    weight == FontWeight::Semibold) 
                                   ? m_boldFontPath : m_regularFontPath;
    
    // Try to load the font
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), scaledSize);
    
    // If custom font fails, try system font paths
    if (!font) {
        // Try common font paths on Switch
        // Nintendo Switch has fonts in various locations
        const char* fallbackPaths[] = {
            // Standard shared font (most reliable)
            "sdmc:/switch/fonts/NotoSansCJKsc-Regular.otf",
            "sdmc:/switch/appstore/fonts/NotoSansCJKsc-Regular.otf",
            // Atmosphere font override paths
            "/atmosphere/contents/fonts/00.ttf",
            "/atmosphere/contents/0100000000000811/romfs/nintendo_ext_003.bfttf",
            // Try common homebrew font locations
            "sdmc:/config/nx-hbmenu/fonts/font.ttf",
            // romfs fonts (if user adds them)
            "romfs:/fonts/NotoSansCJK-Regular.ttc",
            "romfs:/fonts/font.ttf",
            nullptr
        };
        
        for (int i = 0; fallbackPaths[i] != nullptr; i++) {
            font = TTF_OpenFont(fallbackPaths[i], scaledSize);
            if (font) break;
        }
    }
    
    // Cache and return
    m_fontCache[key] = font;
    return font;
}

bool Renderer::loadFont(const std::string& path, FontWeight weight) {
    if (weight == FontWeight::Bold || weight == FontWeight::Semibold) {
        m_boldFontPath = path;
    } else {
        m_regularFontPath = path;
    }
    return true;
}
