// =============================================================================
// Switch App Store - Input Implementation
// =============================================================================

#include "Input.hpp"
#include <cmath>

// =============================================================================
// Constructor & Destructor
// =============================================================================

Input::Input() = default;

Input::~Input() = default;

void Input::init() {
    // Initialize the pad (controller)
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&m_pad);
    
    // Initialize touch screen
    hidInitializeTouchScreen();
}

// =============================================================================
// Update
// =============================================================================

void Input::update() {
    // Store previous button state
    m_previousButtons = m_currentButtons;
    
    // Update pad state
    padUpdate(&m_pad);
    
    // Get current button state
    m_currentButtons = padGetButtons(&m_pad);
    
    // -------------------------------------------------------------------------
    // Update analog sticks
    // -------------------------------------------------------------------------
    HidAnalogStickState leftStick = padGetStickPos(&m_pad, 0);
    HidAnalogStickState rightStick = padGetStickPos(&m_pad, 1);
    
    // Normalize to -1.0 to 1.0 range and apply deadzone
    m_leftStick.x = applyDeadzone(static_cast<float>(leftStick.x) / JOYSTICK_MAX);
    m_leftStick.y = applyDeadzone(static_cast<float>(leftStick.y) / JOYSTICK_MAX);
    m_rightStick.x = applyDeadzone(static_cast<float>(rightStick.x) / JOYSTICK_MAX);
    m_rightStick.y = applyDeadzone(static_cast<float>(rightStick.y) / JOYSTICK_MAX);
    
    // -------------------------------------------------------------------------
    // Update touch state using SDL events (more responsive than polling)
    // This approach is used by borealis library for better touch handling
    // -------------------------------------------------------------------------
    
    // Save previous state
    bool wasTouching = m_touch.touching;
    float prevX = m_touch.x;
    float prevY = m_touch.y;
    
    // Reset per-frame flags
    m_touch.justTouched = false;
    m_touch.justReleased = false;
    m_touch.isTap = false;
    m_touch.deltaX = 0.0f;
    m_touch.deltaY = 0.0f;
    
    // Process SDL events for touch input
    SDL_Event event;
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FINGERDOWN, SDL_FINGERMOTION) > 0) {
        // Get screen dimensions for coordinate conversion
        float screenW = 1280.0f;
        float screenH = 720.0f;
        
        switch (event.type) {
            case SDL_FINGERDOWN: {
                // Convert normalized coordinates (0-1) to screen coordinates
                float newX = event.tfinger.x * screenW;
                float newY = event.tfinger.y * screenH;
                
                m_touch.touching = true;
                m_touch.justTouched = true;
                m_touch.startX = newX;
                m_touch.startY = newY;
                m_touch.x = newX;
                m_touch.y = newY;
                m_touch.totalMovement = 0.0f;
                m_touch.duration = 0.0f;
                m_touch.velocityX = 0.0f;
                m_touch.velocityY = 0.0f;
                break;
            }
            
            case SDL_FINGERUP: {
                float newX = event.tfinger.x * screenW;
                float newY = event.tfinger.y * screenH;
                
                m_touch.x = newX;
                m_touch.y = newY;
                m_touch.touching = false;
                m_touch.justReleased = true;
                
                // Determine if this was a tap
                constexpr float TAP_MAX_MOVEMENT = 50.0f;
                constexpr float TAP_MAX_DURATION = 0.5f;
                m_touch.isTap = (m_touch.totalMovement < TAP_MAX_MOVEMENT && 
                                 m_touch.duration < TAP_MAX_DURATION);
                break;
            }
            
            case SDL_FINGERMOTION: {
                float newX = event.tfinger.x * screenW;
                float newY = event.tfinger.y * screenH;
                
                // Calculate delta (SDL provides dx/dy but we can also calculate)
                m_touch.deltaX = newX - m_touch.x;
                m_touch.deltaY = newY - m_touch.y;
                
                // Track total movement
                float frameDist = std::sqrt(m_touch.deltaX * m_touch.deltaX + 
                                           m_touch.deltaY * m_touch.deltaY);
                m_touch.totalMovement += frameDist;
                
                // Smoothed velocity for momentum scrolling
                constexpr float VELOCITY_SMOOTHING = 0.4f;
                m_touch.velocityX = VELOCITY_SMOOTHING * m_touch.deltaX + 
                                    (1.0f - VELOCITY_SMOOTHING) * m_touch.velocityX;
                m_touch.velocityY = VELOCITY_SMOOTHING * m_touch.deltaY + 
                                    (1.0f - VELOCITY_SMOOTHING) * m_touch.velocityY;
                
                m_touch.x = newX;
                m_touch.y = newY;
                break;
            }
        }
    }
    
    // Update duration if touching
    if (m_touch.touching) {
        m_touch.duration += 0.0167f;  // ~60fps
    }
}

// =============================================================================
// Button Queries
// =============================================================================

bool Input::isHeld(Button button) const {
    return (m_currentButtons & buttonToNxKey(button)) != 0;
}

bool Input::isPressed(Button button) const {
    u64 key = buttonToNxKey(button);
    return (m_currentButtons & key) != 0 && (m_previousButtons & key) == 0;
}

bool Input::isReleased(Button button) const {
    u64 key = buttonToNxKey(button);
    return (m_currentButtons & key) == 0 && (m_previousButtons & key) != 0;
}

// =============================================================================
// Button Mapping
// =============================================================================

u64 Input::buttonToNxKey(Button button) const {
    switch (button) {
        case Button::A:         return HidNpadButton_A;
        case Button::B:         return HidNpadButton_B;
        case Button::X:         return HidNpadButton_X;
        case Button::Y:         return HidNpadButton_Y;
        case Button::L:         return HidNpadButton_L;
        case Button::R:         return HidNpadButton_R;
        case Button::ZL:        return HidNpadButton_ZL;
        case Button::ZR:        return HidNpadButton_ZR;
        case Button::Plus:      return HidNpadButton_Plus;
        case Button::Minus:     return HidNpadButton_Minus;
        case Button::DPadUp:    return HidNpadButton_Up;
        case Button::DPadDown:  return HidNpadButton_Down;
        case Button::DPadLeft:  return HidNpadButton_Left;
        case Button::DPadRight: return HidNpadButton_Right;
        case Button::LeftStick: return HidNpadButton_StickL;
        case Button::RightStick:return HidNpadButton_StickR;
        default:                return 0;
    }
}

// =============================================================================
// Deadzone
// =============================================================================

float Input::applyDeadzone(float value) const {
    if (std::abs(value) < STICK_DEADZONE) {
        return 0.0f;
    }
    
    // Rescale the value to remove the deadzone range
    float sign = value > 0.0f ? 1.0f : -1.0f;
    float absValue = std::abs(value);
    return sign * (absValue - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
}
