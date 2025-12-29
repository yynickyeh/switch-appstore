// =============================================================================
// Switch App Store - Component Base Class
// =============================================================================
// Base class for all UI components (buttons, cards, labels, etc.)
// Provides common functionality: bounds, visibility, focus, animations
// =============================================================================

#pragma once

#include "core/Renderer.hpp"
#include "core/Input.hpp"
#include <functional>
#include <memory>
#include <vector>

// Forward declarations
class App;
class Theme;

// =============================================================================
// Component - Base class for all UI components
// =============================================================================
// All UI elements inherit from Component. Components can:
// - Have position and size (bounds)
// - Be visible or hidden
// - Be enabled or disabled
// - Be focused (for controller navigation)
// - Have children (for container components)
// - Animate their properties
// =============================================================================
class Component {
public:
    // -------------------------------------------------------------------------
    // Constructor & Destructor
    // -------------------------------------------------------------------------
    Component();
    virtual ~Component() = default;
    
    // -------------------------------------------------------------------------
    // Core lifecycle methods
    // -------------------------------------------------------------------------
    
    // Handle input events (override in subclasses)
    virtual void handleInput(const Input& input);
    
    // Update component state (animations, etc.)
    virtual void update(float deltaTime);
    
    // Render the component
    virtual void render(Renderer& renderer, Theme& theme) = 0;
    
    // -------------------------------------------------------------------------
    // Bounds management
    // -------------------------------------------------------------------------
    
    void setPosition(float x, float y);
    void setSize(float width, float height);
    void setBounds(float x, float y, float width, float height);
    void setBounds(const Rect& bounds);
    
    const Rect& getBounds() const { return m_bounds; }
    float getX() const { return m_bounds.x; }
    float getY() const { return m_bounds.y; }
    float getWidth() const { return m_bounds.w; }
    float getHeight() const { return m_bounds.h; }
    
    // -------------------------------------------------------------------------
    // Visibility and state
    // -------------------------------------------------------------------------
    
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    
    void setFocused(bool focused);
    bool isFocused() const { return m_focused; }
    
    // Can this component receive focus?
    virtual bool canFocus() const { return m_enabled && m_visible; }
    
    // -------------------------------------------------------------------------
    // Touch/click detection
    // -------------------------------------------------------------------------
    
    // Check if a point is within this component's bounds
    bool containsPoint(float x, float y) const;
    
    // Check if a touch/click hit this component
    bool hitTest(float x, float y) const {
        return m_visible && m_enabled && containsPoint(x, y);
    }
    
    // -------------------------------------------------------------------------
    // Parent/child relationships (for containers)
    // -------------------------------------------------------------------------
    
    void setParent(Component* parent) { m_parent = parent; }
    Component* getParent() const { return m_parent; }
    
    // Add a child component
    void addChild(std::unique_ptr<Component> child);
    
    // Remove a child component
    void removeChild(Component* child);
    
    // Get children
    const std::vector<std::unique_ptr<Component>>& getChildren() const { 
        return m_children; 
    }
    
    // -------------------------------------------------------------------------
    // Event callbacks
    // -------------------------------------------------------------------------
    
    using Callback = std::function<void()>;
    
    // Called when component is tapped/clicked
    void setOnTap(Callback callback) { m_onTap = callback; }
    
    // Called when component gains focus
    void setOnFocus(Callback callback) { m_onFocus = callback; }
    
    // Called when component loses focus
    void setOnBlur(Callback callback) { m_onBlur = callback; }
    
    // -------------------------------------------------------------------------
    // Animation helpers
    // -------------------------------------------------------------------------
    
    // Get current scale (for press animations)
    float getScale() const { return m_scale; }
    void setScale(float scale) { m_scale = scale; }
    
    // Get current opacity (0.0 to 1.0)
    float getOpacity() const { return m_opacity; }
    void setOpacity(float opacity) { m_opacity = opacity; }
    
protected:
    // -------------------------------------------------------------------------
    // Protected helper methods
    // -------------------------------------------------------------------------
    
    // Convert local coordinates to screen coordinates
    Rect getScreenBounds() const;
    
    // Fire the tap callback
    void fireTap();
    
    // Update children
    void updateChildren(float deltaTime);
    
    // Render children
    void renderChildren(Renderer& renderer, Theme& theme);
    
    // -------------------------------------------------------------------------
    // Protected members
    // -------------------------------------------------------------------------
    
    Rect m_bounds{0, 0, 100, 50};   // Position and size
    bool m_visible = true;          // Is component visible?
    bool m_enabled = true;          // Is component enabled (interactive)?
    bool m_focused = false;         // Does component have focus?
    float m_scale = 1.0f;           // Scale for animations
    float m_opacity = 1.0f;         // Opacity (0.0 to 1.0)
    
    // Parent/children
    Component* m_parent = nullptr;
    std::vector<std::unique_ptr<Component>> m_children;
    
    // Callbacks
    Callback m_onTap;
    Callback m_onFocus;
    Callback m_onBlur;
    
    // Animation state
    bool m_isPressed = false;       // Is component being pressed?
    float m_pressAnimProgress = 0.0f;  // Press animation progress
};
