// =============================================================================
// Switch App Store - Animation System
// =============================================================================
// Provides tween animations with various easing functions
// Inspired by iOS UIKit animations and spring physics
// =============================================================================

#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <cmath>

// =============================================================================
// Easing Functions (matching iOS curves)
// =============================================================================
namespace Easing {

// Linear (no easing)
inline float linear(float t) { return t; }

// Quadratic
inline float easeInQuad(float t) { return t * t; }
inline float easeOutQuad(float t) { return t * (2 - t); }
inline float easeInOutQuad(float t) {
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

// Cubic (iOS default)
inline float easeInCubic(float t) { return t * t * t; }
inline float easeOutCubic(float t) {
    float f = t - 1;
    return f * f * f + 1;
}
inline float easeInOutCubic(float t) {
    return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

// Exponential
inline float easeOutExpo(float t) {
    return t == 1.0f ? 1.0f : 1 - std::pow(2, -10 * t);
}

// Back (overshoot like iOS spring)
inline float easeOutBack(float t) {
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1;
    return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
}

// Spring (iOS-like damped oscillation)
inline float spring(float t, float damping = 0.5f, float velocity = 0.0f) {
    float omega = 2 * 3.14159f * 3;  // ~3 oscillations
    float decay = std::exp(-damping * omega * t);
    return 1 - decay * std::cos(omega * std::sqrt(1 - damping * damping) * t);
}

} // namespace Easing

// =============================================================================
// Animation - Single property animation
// =============================================================================
class Animation {
public:
    using EasingFunc = float(*)(float);
    using UpdateCallback = std::function<void(float)>;
    using CompleteCallback = std::function<void()>;
    
    Animation(float from, float to, float duration, 
              EasingFunc easing = Easing::easeOutCubic);
    
    // Update the animation, returns true if still running
    bool update(float deltaTime);
    
    // Get current interpolated value
    float getValue() const { return m_currentValue; }
    
    // Set callbacks
    void onUpdate(UpdateCallback callback) { m_onUpdate = callback; }
    void onComplete(CompleteCallback callback) { m_onComplete = callback; }
    
    // Control
    void start() { m_running = true; m_elapsed = 0; }
    void pause() { m_running = false; }
    void resume() { m_running = true; }
    void stop() { m_running = false; m_elapsed = 0; }
    void reverse() { std::swap(m_from, m_to); m_elapsed = 0; }
    
    // State
    bool isRunning() const { return m_running; }
    bool isComplete() const { return m_elapsed >= m_duration; }
    float getProgress() const { return m_duration > 0 ? m_elapsed / m_duration : 1.0f; }
    
    // Delay before starting
    void setDelay(float delay) { m_delay = delay; }
    
private:
    float m_from;
    float m_to;
    float m_duration;
    float m_elapsed = 0.0f;
    float m_delay = 0.0f;
    float m_currentValue;
    bool m_running = false;
    EasingFunc m_easing;
    UpdateCallback m_onUpdate;
    CompleteCallback m_onComplete;
};

// =============================================================================
// AnimationGroup - Manage multiple animations
// =============================================================================
class AnimationGroup {
public:
    AnimationGroup() = default;
    
    // Add an animation
    void add(std::unique_ptr<Animation> anim);
    
    // Create and add a simple animation
    Animation* animate(float* target, float from, float to, float duration,
                       Animation::EasingFunc easing = Easing::easeOutCubic);
    
    // Update all animations
    void update(float deltaTime);
    
    // Control all animations
    void startAll();
    void stopAll();
    void clear();
    
    // Check if any animations running
    bool isAnyRunning() const;
    
    // Get number of running animations
    size_t getRunningCount() const;
    
private:
    std::vector<std::unique_ptr<Animation>> m_animations;
    std::vector<std::pair<float*, Animation*>> m_targetBindings;
};

// =============================================================================
// SpringAnimation - iOS-style spring physics
// =============================================================================
class SpringAnimation {
public:
    SpringAnimation(float& target, float destination);
    
    // Spring parameters
    void setDamping(float damping) { m_damping = damping; }  // 0.0-1.0, default 0.7
    void setStiffness(float stiffness) { m_stiffness = stiffness; }  // Higher = faster
    void setMass(float mass) { m_mass = mass; }  // Higher = slower
    
    // Update, returns true if still animating
    bool update(float deltaTime);
    
    // Check if settled
    bool isSettled() const { return m_settled; }
    
    // Change destination while running
    void setDestination(float dest) { m_destination = dest; m_settled = false; }
    
private:
    float& m_target;
    float m_destination;
    float m_velocity = 0.0f;
    float m_damping = 0.7f;
    float m_stiffness = 300.0f;
    float m_mass = 1.0f;
    bool m_settled = true;
    
    static constexpr float SETTLE_THRESHOLD = 0.01f;
};
