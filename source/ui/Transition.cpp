// =============================================================================
// Switch App Store - Page Transition Implementation
// =============================================================================

#include "Transition.hpp"
#include "Theme.hpp"

// =============================================================================
// Constructor
// =============================================================================

Transition::Transition(TransitionType type, float duration)
    : m_type(type)
    , m_duration(duration)
{
    // Set easing based on transition type
    switch (type) {
        case TransitionType::Push:
        case TransitionType::Pop:
            m_easing = Easing::easeOutCubic;  // iOS navigation default
            break;
        case TransitionType::SlideUp:
            m_easing = Easing::easeOutExpo;   // Snappy modal
            break;
        case TransitionType::SlideDown:
            m_easing = Easing::easeInCubic;   // Accelerating dismiss
            break;
        case TransitionType::Scale:
            m_easing = Easing::easeOutBack;   // Slight overshoot
            break;
        default:
            m_easing = Easing::easeOutQuad;
            break;
    }
}

// =============================================================================
// Setup and Control
// =============================================================================

void Transition::setScreens(Screen* from, Screen* to) {
    m_fromScreen = from;
    m_toScreen = to;
}

void Transition::start() {
    m_running = true;
    m_progress = 0.0f;
    
    // Notify screens
    if (m_toScreen) {
        m_toScreen->onEnter();
    }
}

bool Transition::update(float deltaTime) {
    if (!m_running) return false;
    
    // Instant transition
    if (m_type == TransitionType::None) {
        m_progress = 1.0f;
        m_running = false;
        if (m_onComplete) m_onComplete();
        return false;
    }
    
    // Update progress
    m_progress += deltaTime / m_duration;
    
    if (m_progress >= 1.0f) {
        m_progress = 1.0f;
        m_running = false;
        
        // Notify from screen it's exiting
        if (m_fromScreen) {
            m_fromScreen->onExit();
        }
        
        if (m_onComplete) {
            m_onComplete();
        }
        return false;
    }
    
    return true;
}

// =============================================================================
// Rendering
// =============================================================================

void Transition::render(Renderer& renderer, Theme& theme) {
    if (!m_running && m_progress >= 1.0f) {
        // Transition complete, just render target screen
        if (m_toScreen) {
            m_toScreen->render(renderer);
        }
        return;
    }
    
    switch (m_type) {
        case TransitionType::Push:
            renderPush(renderer, theme);
            break;
        case TransitionType::Pop:
            renderPop(renderer, theme);
            break;
        case TransitionType::SlideUp:
            renderSlideUp(renderer, theme);
            break;
        case TransitionType::SlideDown:
            renderSlideDown(renderer, theme);
            break;
        case TransitionType::Fade:
        case TransitionType::CrossDissolve:
            renderFade(renderer, theme);
            break;
        case TransitionType::Scale:
            renderScale(renderer, theme);
            break;
        default:
            // Just render target
            if (m_toScreen) m_toScreen->render(renderer);
            break;
    }
}

// =============================================================================
// Push Transition (iOS Navigation Push)
// =============================================================================
// From screen slides left (25%), to screen slides in from right
// =============================================================================

void Transition::renderPush(Renderer& renderer, Theme& theme) {
    float easedT = m_easing(m_progress);
    
    // From screen slides left (only 25% of width for depth effect)
    if (m_fromScreen) {
        float fromOffset = -m_screenWidth * 0.25f * easedT;
        
        // Apply offset via clip/translate (simplified)
        renderer.pushTransform(fromOffset, 0);
        m_fromScreen->render(renderer);
        renderer.popTransform();
        
        // Darken the from screen as it goes back
        Color overlay(0, 0, 0, static_cast<Uint8>(60 * easedT));
        renderer.drawRect(
            Rect(fromOffset, 0, m_screenWidth, m_screenHeight), 
            overlay
        );
    }
    
    // To screen slides in from right
    if (m_toScreen) {
        float toOffset = m_screenWidth * (1.0f - easedT);
        
        renderer.pushTransform(toOffset, 0);
        m_toScreen->render(renderer);
        renderer.popTransform();
        
        // Shadow on left edge of incoming screen
        if (toOffset > 0) {
            Color shadowColor(0, 0, 0, static_cast<Uint8>(40 * (1 - easedT)));
            renderer.drawGradientRect(
                Rect(toOffset - 20, 0, 20, m_screenHeight),
                Color(0, 0, 0, 0), shadowColor
            );
        }
    }
}

// =============================================================================
// Pop Transition (iOS Navigation Pop)
// =============================================================================
// Reverse of push: to screen slides back from -25%, from screen slides right
// =============================================================================

void Transition::renderPop(Renderer& renderer, Theme& theme) {
    float easedT = m_easing(m_progress);
    
    // To screen (was behind) comes forward from -25%
    if (m_toScreen) {
        float toOffset = -m_screenWidth * 0.25f * (1.0f - easedT);
        
        renderer.pushTransform(toOffset, 0);
        m_toScreen->render(renderer);
        renderer.popTransform();
        
        // Fade out the darkening overlay
        Color overlay(0, 0, 0, static_cast<Uint8>(60 * (1 - easedT)));
        renderer.drawRect(
            Rect(toOffset, 0, m_screenWidth, m_screenHeight),
            overlay
        );
    }
    
    // From screen slides off to the right
    if (m_fromScreen) {
        float fromOffset = m_screenWidth * easedT;
        
        renderer.pushTransform(fromOffset, 0);
        m_fromScreen->render(renderer);
        renderer.popTransform();
    }
}

// =============================================================================
// Slide Up (Modal Presentation)
// =============================================================================

void Transition::renderSlideUp(Renderer& renderer, Theme& theme) {
    float easedT = m_easing(m_progress);
    
    // From screen stays in place but gets dimmed
    if (m_fromScreen) {
        m_fromScreen->render(renderer);
        Color overlay(0, 0, 0, static_cast<Uint8>(100 * easedT));
        renderer.drawRect(Rect(0, 0, m_screenWidth, m_screenHeight), overlay);
    }
    
    // To screen slides up from bottom
    if (m_toScreen) {
        float toOffset = m_screenHeight * (1.0f - easedT);
        
        renderer.pushTransform(0, toOffset);
        m_toScreen->render(renderer);
        renderer.popTransform();
    }
}

// =============================================================================
// Slide Down (Modal Dismissal)
// =============================================================================

void Transition::renderSlideDown(Renderer& renderer, Theme& theme) {
    float easedT = m_easing(m_progress);
    
    // To screen (behind) lightens up
    if (m_toScreen) {
        m_toScreen->render(renderer);
        Color overlay(0, 0, 0, static_cast<Uint8>(100 * (1 - easedT)));
        renderer.drawRect(Rect(0, 0, m_screenWidth, m_screenHeight), overlay);
    }
    
    // From screen slides down
    if (m_fromScreen) {
        float fromOffset = m_screenHeight * easedT;
        
        renderer.pushTransform(0, fromOffset);
        m_fromScreen->render(renderer);
        renderer.popTransform();
    }
}

// =============================================================================
// Fade / Cross-Dissolve
// =============================================================================

void Transition::renderFade(Renderer& renderer, Theme& theme) {
    float easedT = m_easing(m_progress);
    
    // Render from screen fading out
    if (m_fromScreen) {
        // Note: proper implementation would use render targets for opacity
        // Simplified: draw at full opacity, easedT controls blend
        m_fromScreen->render(renderer);
        
        // Overlay to simulate fade-out based on easedT
        if (easedT > 0.0f) {
            Color fadeOverlay(0, 0, 0, static_cast<Uint8>(easedT * 100));
            renderer.drawRect(Rect(0, 0, m_screenWidth, m_screenHeight), fadeOverlay);
        }
    }
    
    // Render to screen fading in on top
    if (m_toScreen) {
        // Draw with increasing opacity (simplified overlay approach)
        m_toScreen->render(renderer);
    }
}

// =============================================================================
// Scale (App Icon Launch Effect)
// =============================================================================

void Transition::renderScale(Renderer& renderer, Theme& theme) {
    float easedT = m_easing(m_progress);
    
    // From screen fades and scales down slightly
    if (m_fromScreen && easedT < 0.5f) {
        float scale = 1.0f - 0.05f * (easedT / 0.5f);
        float offsetX = m_screenWidth * (1 - scale) / 2;
        float offsetY = m_screenHeight * (1 - scale) / 2;
        
        renderer.pushTransform(offsetX, offsetY);
        m_fromScreen->render(renderer);
        renderer.popTransform();
    }
    
    // To screen scales up from center
    if (m_toScreen) {
        float scale = 0.9f + 0.1f * easedT;
        float offsetX = m_screenWidth * (1 - scale) / 2;
        float offsetY = m_screenHeight * (1 - scale) / 2;
        
        renderer.pushTransform(offsetX, offsetY);
        m_toScreen->render(renderer);
        renderer.popTransform();
    }
}
