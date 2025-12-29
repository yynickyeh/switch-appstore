// =============================================================================
// Switch App Store - Animation Implementation
// =============================================================================

#include "Animation.hpp"
#include <algorithm>

// =============================================================================
// Animation
// =============================================================================

Animation::Animation(float from, float to, float duration, EasingFunc easing)
    : m_from(from)
    , m_to(to)
    , m_duration(duration)
    , m_currentValue(from)
    , m_easing(easing)
{
}

bool Animation::update(float deltaTime) {
    if (!m_running) return false;
    
    // Handle delay
    if (m_delay > 0) {
        m_delay -= deltaTime;
        if (m_delay > 0) return true;
        deltaTime = -m_delay;  // Use overflow from delay
        m_delay = 0;
    }
    
    // Update elapsed time
    m_elapsed += deltaTime;
    
    // Calculate progress with easing
    float progress = std::min(1.0f, m_elapsed / m_duration);
    float easedProgress = m_easing ? m_easing(progress) : progress;
    
    // Interpolate value
    m_currentValue = m_from + (m_to - m_from) * easedProgress;
    
    // Fire update callback
    if (m_onUpdate) {
        m_onUpdate(m_currentValue);
    }
    
    // Check if complete
    if (progress >= 1.0f) {
        m_running = false;
        m_currentValue = m_to;  // Ensure exact final value
        
        if (m_onComplete) {
            m_onComplete();
        }
        return false;
    }
    
    return true;
}

// =============================================================================
// AnimationGroup
// =============================================================================

void AnimationGroup::add(std::unique_ptr<Animation> anim) {
    m_animations.push_back(std::move(anim));
}

Animation* AnimationGroup::animate(float* target, float from, float to, 
                                    float duration, Animation::EasingFunc easing) {
    auto anim = std::make_unique<Animation>(from, to, duration, easing);
    
    // Bind target to animation
    anim->onUpdate([target](float value) {
        *target = value;
    });
    
    Animation* ptr = anim.get();
    anim->start();
    m_animations.push_back(std::move(anim));
    
    return ptr;
}

void AnimationGroup::update(float deltaTime) {
    // Update all animations, remove completed ones
    auto it = m_animations.begin();
    while (it != m_animations.end()) {
        bool running = (*it)->update(deltaTime);
        if (!running && (*it)->isComplete()) {
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }
}

void AnimationGroup::startAll() {
    for (auto& anim : m_animations) {
        anim->start();
    }
}

void AnimationGroup::stopAll() {
    for (auto& anim : m_animations) {
        anim->stop();
    }
}

void AnimationGroup::clear() {
    m_animations.clear();
}

bool AnimationGroup::isAnyRunning() const {
    for (const auto& anim : m_animations) {
        if (anim->isRunning()) return true;
    }
    return false;
}

size_t AnimationGroup::getRunningCount() const {
    size_t count = 0;
    for (const auto& anim : m_animations) {
        if (anim->isRunning()) count++;
    }
    return count;
}

// =============================================================================
// SpringAnimation
// =============================================================================

SpringAnimation::SpringAnimation(float& target, float destination)
    : m_target(target)
    , m_destination(destination)
{
    m_settled = (std::abs(target - destination) < SETTLE_THRESHOLD);
}

bool SpringAnimation::update(float deltaTime) {
    if (m_settled) return false;
    
    // Spring physics simulation
    // F = -k * x - d * v
    // a = F / m
    
    float displacement = m_target - m_destination;
    float springForce = -m_stiffness * displacement;
    float dampingForce = -m_damping * m_velocity * 2 * std::sqrt(m_stiffness * m_mass);
    float acceleration = (springForce + dampingForce) / m_mass;
    
    m_velocity += acceleration * deltaTime;
    m_target += m_velocity * deltaTime;
    
    // Check if settled
    if (std::abs(displacement) < SETTLE_THRESHOLD && 
        std::abs(m_velocity) < SETTLE_THRESHOLD) {
        m_target = m_destination;
        m_velocity = 0;
        m_settled = true;
        return false;
    }
    
    return true;
}
