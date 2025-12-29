// =============================================================================
// Switch App Store - Page Transition System
// =============================================================================
// Manages animated transitions between screens (push, pop, fade, etc.)
// Inspired by iOS UINavigationController transitions
// =============================================================================

#pragma once

#include "Animation.hpp"
#include "screens/Screen.hpp"
#include "core/Renderer.hpp"
#include <memory>
#include <functional>

// Forward declarations
class Screen;
class Theme;

// =============================================================================
// Transition Types
// =============================================================================
enum class TransitionType {
    None,           // Instant switch, no animation
    Push,           // Slide from right (iOS navigation push)
    Pop,            // Slide to right (iOS navigation pop)
    SlideUp,        // Modal presentation (slide from bottom)
    SlideDown,      // Modal dismissal (slide to bottom)
    Fade,           // Cross-fade between screens
    CrossDissolve,  // Faster cross-fade
    Scale,          // Scale + fade (like opening an app)
};

// =============================================================================
// Transition - Manages a single screen transition
// =============================================================================
class Transition {
public:
    Transition(TransitionType type, float duration = 0.3f);
    
    // Set the screens involved
    void setScreens(Screen* from, Screen* to);
    
    // Start the transition
    void start();
    
    // Update transition, returns true if still running
    bool update(float deltaTime);
    
    // Render both screens with transition effect
    void render(Renderer& renderer, Theme& theme);
    
    // Check state
    bool isComplete() const { return m_progress >= 1.0f; }
    bool isRunning() const { return m_running; }
    float getProgress() const { return m_progress; }
    
    // Get target screen
    Screen* getToScreen() const { return m_toScreen; }
    Screen* getFromScreen() const { return m_fromScreen; }
    
    // Completion callback
    void onComplete(std::function<void()> callback) { m_onComplete = callback; }
    
private:
    TransitionType m_type;
    float m_duration;
    float m_progress = 0.0f;
    bool m_running = false;
    
    Screen* m_fromScreen = nullptr;
    Screen* m_toScreen = nullptr;
    
    // Screen dimensions (for offset calculations)
    float m_screenWidth = 1280.0f;
    float m_screenHeight = 720.0f;
    
    // Easing function
    Animation::EasingFunc m_easing;
    
    // Completion callback
    std::function<void()> m_onComplete;
    
    // Helper methods
    void renderPush(Renderer& renderer, Theme& theme);
    void renderPop(Renderer& renderer, Theme& theme);
    void renderSlideUp(Renderer& renderer, Theme& theme);
    void renderSlideDown(Renderer& renderer, Theme& theme);
    void renderFade(Renderer& renderer, Theme& theme);
    void renderScale(Renderer& renderer, Theme& theme);
};
