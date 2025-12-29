// =============================================================================
// Switch App Store - Router Implementation
// =============================================================================

#include "Router.hpp"
#include "screens/Screen.hpp"
#include "core/Renderer.hpp"
#include "core/Input.hpp"

// =============================================================================
// Constructor & Destructor
// =============================================================================

Router::Router() = default;

Router::~Router() = default;

// =============================================================================
// Screen Management
// =============================================================================

void Router::setRootScreen(std::unique_ptr<Screen> screen) {
    // Clear all existing screens
    m_screens.clear();
    
    // Set the new root screen
    if (screen) {
        screen->onEnter();
        m_screens.push_back(std::move(screen));
    }
}

void Router::push(std::unique_ptr<Screen> screen) {
    if (!screen) return;
    
    // Start transition animation
    m_transitioning = true;
    m_transitionProgress = 0.0f;
    m_transitionIsPush = true;
    m_pendingScreen = std::move(screen);
}

void Router::pop() {
    if (m_screens.size() <= 1) return;  // Don't pop the root screen
    
    // Start transition animation
    m_transitioning = true;
    m_transitionProgress = 0.0f;
    m_transitionIsPush = false;
}

void Router::popToRoot() {
    while (m_screens.size() > 1) {
        m_screens.back()->onExit();
        m_screens.pop_back();
    }
}

Screen* Router::currentScreen() const {
    if (m_screens.empty()) return nullptr;
    return m_screens.back().get();
}

// =============================================================================
// Tab Navigation
// =============================================================================

void Router::addTabScreen(std::unique_ptr<Screen> screen) {
    if (screen) {
        // First tab screen gets entered immediately
        if (m_tabScreens.empty()) {
            screen->onEnter();
        }
        m_tabScreens.push_back(std::move(screen));
    }
}

void Router::switchTab(int tabIndex) {
    if (tabIndex == m_currentTab) return;
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_tabScreens.size())) return;
    
    // Exit current tab screen if any
    if (m_currentTab >= 0 && m_currentTab < static_cast<int>(m_tabScreens.size())) {
        if (m_tabScreens[m_currentTab]) {
            m_tabScreens[m_currentTab]->onExit();
        }
    }
    
    // Switch to new tab
    m_currentTab = tabIndex;
    
    // Enter new tab screen
    if (m_tabScreens[m_currentTab]) {
        m_tabScreens[m_currentTab]->onEnter();
    }
    
    // Update the main screen stack to point to the new tab
    if (!m_tabScreens.empty() && m_tabScreens[m_currentTab]) {
        m_screens.clear();
        // Create a copy reference - we don't move, just use raw pointer for display
        // The tab screens are owned by m_tabScreens
    }
}

// =============================================================================
// Update & Render
// =============================================================================

void Router::handleInput(const Input& input) {
    // Don't process input during transitions
    if (m_transitioning) return;
    
    // Handle back button (B) for screen stack
    if (input.isPressed(Input::Button::B)) {
        if (m_screens.size() > 1) {
            pop();
            return;
        }
    }
    
    // Pass input to current tab screen if using tabs
    if (!m_tabScreens.empty()) {
        if (m_currentTab >= 0 && m_currentTab < static_cast<int>(m_tabScreens.size())) {
            if (m_tabScreens[m_currentTab]) {
                m_tabScreens[m_currentTab]->handleInput(input);
            }
        }
        return;
    }
    
    // Pass input to current screen in stack
    Screen* screen = currentScreen();
    if (screen) {
        screen->handleInput(input);
    }
}

void Router::update(float deltaTime) {
    // Update transition animation
    if (m_transitioning) {
        m_transitionProgress += deltaTime / m_transitionDuration;
        
        if (m_transitionProgress >= 1.0f) {
            // Transition complete
            m_transitionProgress = 1.0f;
            m_transitioning = false;
            
            if (m_transitionIsPush && m_pendingScreen) {
                // Finish push: add new screen
                m_pendingScreen->onEnter();
                m_screens.push_back(std::move(m_pendingScreen));
            } else if (!m_transitionIsPush) {
                // Finish pop: remove current screen
                if (!m_screens.empty()) {
                    m_screens.back()->onExit();
                    m_screens.pop_back();
                }
            }
        }
    }
    
    // Update current tab screen if using tabs
    if (!m_tabScreens.empty()) {
        if (m_currentTab >= 0 && m_currentTab < static_cast<int>(m_tabScreens.size())) {
            if (m_tabScreens[m_currentTab]) {
                m_tabScreens[m_currentTab]->update(deltaTime);
            }
        }
        return;
    }
    
    // Update current screen in stack
    Screen* screen = currentScreen();
    if (screen) {
        screen->update(deltaTime);
    }
}

void Router::render(Renderer& renderer) {
    // If we have tab screens, render the current tab
    if (!m_tabScreens.empty()) {
        if (m_currentTab >= 0 && m_currentTab < static_cast<int>(m_tabScreens.size())) {
            if (m_tabScreens[m_currentTab]) {
                m_tabScreens[m_currentTab]->render(renderer);
            }
        }
        return;
    }
    
    // Otherwise use the screen stack
    if (m_screens.empty()) return;
    
    // During transitions, render both screens with appropriate offsets
    if (m_transitioning) {
        // Calculate eased progress (ease-out cubic)
        float t = m_transitionProgress;
        float easedT = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
        
        if (m_transitionIsPush) {
            // Push: current screen slides left, new screen slides in from right
            // For now, just render the current screen (full animation in future)
            m_screens.back()->render(renderer);
        } else {
            // Pop: current screen slides right, previous screen revealed
            m_screens.back()->render(renderer);
        }
    } else {
        // Normal rendering: just render the top screen
        m_screens.back()->render(renderer);
    }
}

// =============================================================================
// Resolution Change
// =============================================================================

void Router::onResolutionChanged(int width, int height, float scale) {
    // Notify all screens of resolution change
    for (auto& screen : m_screens) {
        screen->onResolutionChanged(width, height, scale);
    }
}
