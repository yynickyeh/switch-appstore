// =============================================================================
// Switch App Store - Renderer
// =============================================================================
// High-level rendering wrapper around SDL2
// Provides convenient drawing methods for UI elements with automatic scaling
// =============================================================================

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

// =============================================================================
// Color structure for easy color management
// =============================================================================
struct Color {
    Uint8 r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(255) {}
    Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) : r(r), g(g), b(b), a(a) {}
    
    // Create color from hex value (e.g., 0x007AFF for iOS blue)
    static Color fromHex(uint32_t hex, Uint8 alpha = 255) {
        return Color(
            (hex >> 16) & 0xFF,
            (hex >> 8) & 0xFF,
            hex & 0xFF,
            alpha
        );
    }
};

// =============================================================================
// Rectangle structure for convenience
// =============================================================================
struct Rect {
    float x, y, w, h;
    
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
    
    // Convert to SDL_Rect (with scaling)
    SDL_Rect toSDL(float scale = 1.0f) const {
        return SDL_Rect{
            static_cast<int>(x * scale),
            static_cast<int>(y * scale),
            static_cast<int>(w * scale),
            static_cast<int>(h * scale)
        };
    }
    
    // Check if a point is inside this rect
    bool contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

// =============================================================================
// Text alignment options
// =============================================================================
enum class TextAlign {
    Left,
    Center,
    Right
};

enum class TextVAlign {
    Top,
    Middle,
    Bottom
};

// =============================================================================
// Font weight options (matching Apple's style)
// =============================================================================
enum class FontWeight {
    Regular,
    Medium,
    Semibold,
    Bold
};

// =============================================================================
// Renderer - Main rendering class
// =============================================================================
class Renderer {
public:
    // -------------------------------------------------------------------------
    // Constructor & Destructor
    // -------------------------------------------------------------------------
    Renderer(SDL_Renderer* sdlRenderer, float scale);
    ~Renderer();
    
    // Initialize renderer (load fonts, etc.)
    bool init();
    
    // -------------------------------------------------------------------------
    // Scale management
    // -------------------------------------------------------------------------
    void setScale(float scale) { m_scale = scale; }
    float getScale() const { return m_scale; }
    
    // Access underlying SDL renderer (for texture creation, etc.)
    SDL_Renderer* getSDLRenderer() const { return m_sdlRenderer; }
    
    // -------------------------------------------------------------------------
    // Basic drawing operations
    // -------------------------------------------------------------------------
    
    // Draw a filled rectangle
    void drawRect(const Rect& rect, const Color& color);
    
    // Draw a rectangle outline
    void drawRectOutline(const Rect& rect, const Color& color, int thickness = 1);
    
    // Draw a rounded rectangle (iOS card style)
    void drawRoundedRect(const Rect& rect, int radius, const Color& color);
    
    // Draw a rounded rectangle outline
    void drawRoundedRectOutline(const Rect& rect, int radius, 
                                 const Color& color, int thickness = 1);
    
    // Draw a circle
    void drawCircle(float x, float y, float radius, const Color& color);
    
    // Draw a circle outline
    void drawCircleOutline(float x, float y, float radius, 
                           const Color& color, int thickness = 1);
    
    // Draw a line
    void drawLine(float x1, float y1, float x2, float y2, 
                  const Color& color, int thickness = 1);
    
    // -------------------------------------------------------------------------
    // Gradient drawing (for cards and backgrounds)
    // -------------------------------------------------------------------------
    
    // Draw a vertical gradient rectangle
    void drawGradientRect(const Rect& rect, 
                          const Color& topColor, const Color& bottomColor);
    
    // Draw a rounded rect with gradient
    void drawGradientRoundedRect(const Rect& rect, int radius,
                                  const Color& topColor, const Color& bottomColor);
    
    // -------------------------------------------------------------------------
    // Text rendering
    // -------------------------------------------------------------------------
    
    // Draw text at a position
    void drawText(const std::string& text, float x, float y, 
                  int fontSize, const Color& color,
                  FontWeight weight = FontWeight::Regular,
                  TextAlign align = TextAlign::Left);
    
    // Draw text within a rectangle (with clipping)
    void drawTextInRect(const std::string& text, const Rect& rect,
                        int fontSize, const Color& color,
                        FontWeight weight = FontWeight::Regular,
                        TextAlign align = TextAlign::Left,
                        TextVAlign valign = TextVAlign::Top);
    
    // Measure text size (returns width)
    float measureText(const std::string& text, int fontSize, 
                      FontWeight weight = FontWeight::Regular);
    
    // -------------------------------------------------------------------------
    // Image rendering
    // -------------------------------------------------------------------------
    
    // Draw a texture
    void drawTexture(SDL_Texture* texture, const Rect& destRect);
    
    // Draw a texture with alpha
    void drawTexture(SDL_Texture* texture, const Rect& destRect, Uint8 alpha);
    
    // Draw a texture with source rect (for sprite sheets)
    void drawTexture(SDL_Texture* texture, const Rect& srcRect, const Rect& destRect);
    
    // -------------------------------------------------------------------------
    // Clipping
    // -------------------------------------------------------------------------
    
    // Set clipping rectangle (for ScrollView, etc.)
    void setClipRect(const Rect& rect);
    
    // Clear clipping rectangle
    void clearClipRect();
    
    // -------------------------------------------------------------------------
    // Transform stack (for transitions and animations)
    // -------------------------------------------------------------------------
    
    // Push a translation offset
    void pushTransform(float offsetX, float offsetY);
    
    // Pop the last transform
    void popTransform();
    
    // Get current total offset
    float getOffsetX() const { return m_totalOffsetX; }
    float getOffsetY() const { return m_totalOffsetY; }
    
    // -------------------------------------------------------------------------
    // Shadow effects (for cards)
    // -------------------------------------------------------------------------
    
    // Draw a shadow under a rectangle
    void drawShadow(const Rect& rect, int radius, int blur, 
                    int offsetX, int offsetY, const Color& color);
    
private:
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Get or create font of specified size and weight
    TTF_Font* getFont(int size, FontWeight weight);
    
    // Load font from romfs
    bool loadFont(const std::string& path, FontWeight weight);
    
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    SDL_Renderer* m_sdlRenderer;
    float m_scale;
    
    // Font cache: maps (size << 8 | weight) to font
    std::unordered_map<int, TTF_Font*> m_fontCache;
    
    // Base font paths
    std::string m_regularFontPath;
    std::string m_boldFontPath;
    
    // Transform stack for transitions
    std::vector<std::pair<float, float>> m_transformStack;
    float m_totalOffsetX = 0.0f;
    float m_totalOffsetY = 0.0f;
};
