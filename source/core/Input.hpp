// =============================================================================
// Switch App Store - Input Handler
// =============================================================================
// Handles controller and touch input for the Nintendo Switch
// =============================================================================

#pragma once

#include <switch.h>
#include <SDL2/SDL.h>

// =============================================================================
// Input - Handles all user input
// =============================================================================
class Input {
public:
    // -------------------------------------------------------------------------
    // Button enumeration (matching Switch controller)
    // -------------------------------------------------------------------------
    enum class Button {
        A,          // Confirm
        B,          // Back
        X,          // Search shortcut
        Y,          // Options/Refresh
        L,          // Previous tab
        R,          // Next tab
        ZL,         // Page up
        ZR,         // Page down
        Plus,       // Settings
        Minus,      // Downloads
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        LeftStick,  // L3
        RightStick, // R3
        Count
    };
    
    // -------------------------------------------------------------------------
    // Touch state - Enhanced for better UX
    // -------------------------------------------------------------------------
    struct TouchState {
        bool touching = false;      // Currently touching
        float x = 0.0f;             // Current X position
        float y = 0.0f;             // Current Y position
        float startX = 0.0f;        // Where touch started
        float startY = 0.0f;        // Where touch started
        float deltaX = 0.0f;        // Movement this frame
        float deltaY = 0.0f;        // Movement this frame
        float velocityX = 0.0f;     // Velocity (for momentum scrolling)
        float velocityY = 0.0f;     // Velocity (for momentum scrolling)
        bool justTouched = false;   // Touch started this frame
        bool justReleased = false;  // Touch ended this frame
        bool isTap = false;         // Was this a tap (quick touch without much movement)
        float totalMovement = 0.0f; // Total distance moved during this touch
        float duration = 0.0f;      // How long touch has been held
    };
    
    // -------------------------------------------------------------------------
    // Analog stick state
    // -------------------------------------------------------------------------
    struct StickState {
        float x = 0.0f;  // -1.0 to 1.0
        float y = 0.0f;  // -1.0 to 1.0
    };
    
    // -------------------------------------------------------------------------
    // Constructor & Destructor
    // -------------------------------------------------------------------------
    Input();
    ~Input();
    
    // Initialize input handling
    void init();
    
    // Update input state (call once per frame)
    void update();
    
    // -------------------------------------------------------------------------
    // Button state queries
    // -------------------------------------------------------------------------
    
    // Is button currently held down?
    bool isHeld(Button button) const;
    
    // Was button just pressed this frame?
    bool isPressed(Button button) const;
    
    // Was button just released this frame?
    bool isReleased(Button button) const;
    
    // -------------------------------------------------------------------------
    // Analog stick queries
    // -------------------------------------------------------------------------
    
    const StickState& getLeftStick() const { return m_leftStick; }
    const StickState& getRightStick() const { return m_rightStick; }
    
    // -------------------------------------------------------------------------
    // Touch state queries
    // -------------------------------------------------------------------------
    
    const TouchState& getTouch() const { return m_touch; }
    
private:
    // -------------------------------------------------------------------------
    // Private members
    // -------------------------------------------------------------------------
    
    // Button states
    u64 m_currentButtons = 0;
    u64 m_previousButtons = 0;
    
    // Analog sticks
    StickState m_leftStick;
    StickState m_rightStick;
    
    // Touch
    TouchState m_touch;
    
    // Controller handle
    PadState m_pad;
    
    // Deadzone for analog sticks
    static constexpr float STICK_DEADZONE = 0.15f;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Map libnx button to our Button enum
    u64 buttonToNxKey(Button button) const;
    
    // Apply deadzone to analog input
    float applyDeadzone(float value) const;
};
