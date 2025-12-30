// =============================================================================
// Switch App Store - App Implementation
// =============================================================================
// Main application class implementation
// =============================================================================

#include "app.hpp"
#include "core/Renderer.hpp"
#include "core/Input.hpp"
#include "ui/Router.hpp"
#include "ui/Theme.hpp"
// TodayScreen removed
#include "ui/screens/GamesScreen.hpp"
#include "ui/screens/ToolsScreen.hpp"
#include "ui/screens/EmulatorsScreen.hpp"
#include "ui/screens/SearchScreen.hpp"
#include "store/StoreManager.hpp"
#include "network/HttpClient.hpp"

#include <switch.h>

// =============================================================================
// Constants
// =============================================================================

// Target frame rate and frame time
constexpr int TARGET_FPS = 60;
constexpr float TARGET_FRAME_TIME = 1000.0f / TARGET_FPS;

// Resolution constants
constexpr int HANDHELD_WIDTH = 1280;
constexpr int HANDHELD_HEIGHT = 720;
constexpr int DOCKED_WIDTH = 1920;
constexpr int DOCKED_HEIGHT = 1080;

// =============================================================================
// Constructor & Destructor
// =============================================================================

App::App() = default;

App::~App() {
    // -------------------------------------------------------------------------
    // Cleanup in reverse order of creation
    // -------------------------------------------------------------------------
    
    // Subsystems are cleaned up automatically by unique_ptr
    m_router.reset();
    m_theme.reset();
    m_input.reset();
    m_renderer.reset();
    
    // Destroy SDL resources
    if (m_sdlRenderer) {
        SDL_DestroyRenderer(m_sdlRenderer);
        m_sdlRenderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    // Cleanup global curl state
    HttpClient::cleanup();
}

// =============================================================================
// Initialization
// =============================================================================

bool App::init() {
    // -------------------------------------------------------------------------
    // Initialize networking
    // -------------------------------------------------------------------------
    if (!HttpClient::init()) {
        return false;
    }
    
    // Initialize store manager and fetch catalog
    StoreManager::getInstance().init("sdmc:/switch/appstore/config.json");
    StoreManager::getInstance().refresh();  // Fetch store data
    
    // -------------------------------------------------------------------------
    // Check current display mode (docked vs handheld)
    // -------------------------------------------------------------------------
    AppletOperationMode opMode = appletGetOperationMode();
    m_isDocked = (opMode == AppletOperationMode_Console);
    
    // Set resolution based on mode
    if (m_isDocked) {
        m_windowWidth = DOCKED_WIDTH;
        m_windowHeight = DOCKED_HEIGHT;
        m_scale = 1.5f;
    } else {
        m_windowWidth = HANDHELD_WIDTH;
        m_windowHeight = HANDHELD_HEIGHT;
        m_scale = 1.0f;
    }
    
    // -------------------------------------------------------------------------
    // Create SDL window
    // -------------------------------------------------------------------------
    m_window = SDL_CreateWindow(
        "Switch App Store",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_SHOWN
    );
    
    if (!m_window) {
        return false;
    }
    
    // -------------------------------------------------------------------------
    // Create SDL renderer with hardware acceleration
    // -------------------------------------------------------------------------
    m_sdlRenderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!m_sdlRenderer) {
        return false;
    }
    
    // Enable alpha blending for transparency effects
    SDL_SetRenderDrawBlendMode(m_sdlRenderer, SDL_BLENDMODE_BLEND);
    
    // -------------------------------------------------------------------------
    // Initialize subsystems
    // -------------------------------------------------------------------------
    
    // Create renderer wrapper
    m_renderer = std::make_unique<Renderer>(m_sdlRenderer, m_scale);
    if (!m_renderer->init()) {
        return false;
    }
    
    // Create input handler
    m_input = std::make_unique<Input>();
    m_input->init();
    
    // Create theme manager
    m_theme = std::make_unique<Theme>();
    m_theme->loadTheme("light");  // Start with light theme (Apple style)
    
    // Create router and set up tab screens (like iOS App Store)
    m_router = std::make_unique<Router>();
    
    // Initialize router (creates TabBar component)
    m_router->init(this);
    
    // Add all 4 tab screens (Today removed)
    // Tab 0: Games - Game catalog
    m_router->addTabScreen(std::make_unique<GamesScreen>(this));
    // Tab 1: Tools - Homebrew utilities
    m_router->addTabScreen(std::make_unique<ToolsScreen>(this));
    // Tab 2: Emulators - Retro gaming
    m_router->addTabScreen(std::make_unique<EmulatorsScreen>(this));
    // Tab 3: Search
    m_router->addTabScreen(std::make_unique<SearchScreen>(this));
    
    // -------------------------------------------------------------------------
    // Initialize timing
    // -------------------------------------------------------------------------
    m_lastFrameTime = SDL_GetPerformanceCounter();
    
    return true;
}

// =============================================================================
// Main Loop
// =============================================================================

void App::run() {
    m_running = true;
    
    while (m_running) {
        // Calculate delta time
        Uint64 currentTime = SDL_GetPerformanceCounter();
        float deltaTime = (currentTime - m_lastFrameTime) / 
                          (float)SDL_GetPerformanceFrequency();
        m_lastFrameTime = currentTime;
        
        // Cap delta time to prevent huge jumps after pauses
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }
        
        // Check for resolution changes (dock/undock events)
        checkResolutionChange();
        
        // Process input events
        handleEvents();
        
        // Update logic
        update(deltaTime);
        
        // Render frame
        render();
        
        // Handle Switch-specific applet events
        if (!appletMainLoop()) {
            m_running = false;
        }
    }
}

// =============================================================================
// Event Handling
// =============================================================================

void App::handleEvents() {
    // Update input state (reads controller input)
    m_input->update();
    
    // Check for quit request (B button from home screen, or + to exit)
    if (m_input->isPressed(Input::Button::Plus)) {
        // TODO: Show exit confirmation or settings menu
        m_running = false;
    }
    
    // Pass input to the current screen via router
    if (m_router) {
        m_router->handleInput(*m_input);
    }
}

// =============================================================================
// Update
// =============================================================================

void App::update(float deltaTime) {
    // Update the router (handles screen transitions, etc.)
    if (m_router) {
        m_router->update(deltaTime);
    }
}

// =============================================================================
// Rendering
// =============================================================================

void App::render() {
    // Clear screen with background color
    const auto& bgColor = m_theme->getColor("background");
    SDL_SetRenderDrawColor(m_sdlRenderer, 
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(m_sdlRenderer);
    
    // Render the current screen via router
    if (m_router) {
        m_router->render(*m_renderer);
    }
    
    // Present the frame
    SDL_RenderPresent(m_sdlRenderer);
}

// =============================================================================
// Resolution Change Detection
// =============================================================================

void App::checkResolutionChange() {
    // Check current operation mode
    AppletOperationMode opMode = appletGetOperationMode();
    bool nowDocked = (opMode == AppletOperationMode_Console);
    
    // If mode changed, update resolution
    if (nowDocked != m_isDocked) {
        m_isDocked = nowDocked;
        
        if (m_isDocked) {
            m_windowWidth = DOCKED_WIDTH;
            m_windowHeight = DOCKED_HEIGHT;
            m_scale = 1.5f;
        } else {
            m_windowWidth = HANDHELD_WIDTH;
            m_windowHeight = HANDHELD_HEIGHT;
            m_scale = 1.0f;
        }
        
        // Update window size
        SDL_SetWindowSize(m_window, m_windowWidth, m_windowHeight);
        
        // Update renderer scale
        if (m_renderer) {
            m_renderer->setScale(m_scale);
        }
        
        // Notify router of resolution change (to update layouts)
        if (m_router) {
            m_router->onResolutionChanged(m_windowWidth, m_windowHeight, m_scale);
        }
    }
}
