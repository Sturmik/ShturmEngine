#include "Game.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>

#include "ECS/ECS.h"
#include "Logger/LoggerMacro.h"

#include "Systems/MovementSystem.h"
#include "Systems/RenderSpriteSystem.h"
#include "Systems/AnimationSystem.h"
#include "Systems/CollisionSystem.h"
#include "Systems/RenderColliderSystem.h"
#include "Systems/DamageSystem.h"
#include "Systems/KeyboardControlSystem.h"
#include "Systems/CameraMovementSystem.h"
#include "Systems/ProjectileEmitSystem.h"
#include "Systems/LifecycleSystem.h"
#include "Systems/RenderTextSystem.h"
#include "Systems/RenderHealthBarSystem.h"
#include "Systems/RenderImGuiSystem.h"
#include "Systems/SoundSystem.h"
#include "Systems/ScriptSystem.h"

#include "LevelLoader.h"

Game::Game() : _isRunning(false), _isDebug(false), _window(nullptr), _renderer(nullptr), _camera(), _mapWidth(0), _mapHeight(0), _audioDevice(0)
{
	LOG_INFO("Game constructor called!");
}

Game::~Game()
{
    LOG_INFO("Game destructor called!");
}

void Game::Initialize()
{
    if (!SDL_Init((SDL_INIT_VIDEO | SDL_INIT_AUDIO)))
    {
        LOG_ERROR("Error initializing SDL: %s", SDL_GetError());
        return;
    }

    if (!TTF_Init())
    {
        LOG_ERROR("Error initializing TTF: %s", SDL_GetError());
        return;
    }

    const size_t WINDOW_WIDTH = 800;
    const size_t WINDOW_HEIGHT = 600;
    _window = SDL_CreateWindow(
        "ShturmEngine Window",
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    if (!_window)
    {
        LOG_ERROR("Error creating SDL window: %s", SDL_GetError());
        return;
    }

    // Create a property group
    SDL_PropertiesID props = SDL_CreateProperties();
    // Set the window where the renderer will draw
    SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, _window);
    // Set VSync
    SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1);
    // Create the renderer using properties
    _renderer = SDL_CreateRendererWithProperties(props);
    // Clean up the properties object
    SDL_DestroyProperties(props);

    if (!_renderer)
    {
        LOG_ERROR("Error creating SDL renderer: %s", SDL_GetError());
        return;
    }

    _isRunning = true;

    // Initialize the camera view with the entire screen area
    _camera.x = 0;
    _camera.y = 0;
    glm::vec2 windowSize = GetWindowSize();
    _camera.w = windowSize.x;
    _camera.h = windowSize.y;

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Optional settings
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();

    // Initialize platform/renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(_window, _renderer);
    ImGui_ImplSDLRenderer3_Init(_renderer);

    // Audio setup
    SDL_AudioSpec desired{};
    desired.freq = 44100;
    desired.format = SDL_AUDIO_S16;
    desired.channels = 2;

    _audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if (_audioDevice)
    {
        SDL_ResumeAudioDevice(_audioDevice);
        LOG_INFO("Audio device opened successfully");
    }
    else
    {
        LOG_ERROR("Failed to open audio device: %s", SDL_GetError());
    }
}

void Game::Setup()
{
    // Reset all event handlers for the current frame
    _eventBus.Reset();

    // Add the systems that need to be processed in our game
    _registry.AddSystem<MovementSystem>();
    _registry.AddSystem<RenderSpriteSystem>();
    _registry.AddSystem<AnimationSystem>();
    _registry.AddSystem<CollisionSystem>();
    _registry.AddSystem<RenderColliderSystem>();
    _registry.AddSystem<DamageSystem>();
    _registry.AddSystem<KeyboardControlSystem>();
    _registry.AddSystem<CameraMovementSystem>();
    _registry.AddSystem<ProjectileEmitSystem>();
    _registry.AddSystem<LifecycleSystem>();
    _registry.AddSystem<RenderTextSystem>();
    _registry.AddSystem<RenderHealthBarSystem>();
    _registry.AddSystem<RenderImGuiSystem>();
    _registry.AddSystem<ScriptSystem>();

    // Audio system is a complex one and requires additional initialization
    _registry.AddSystem<SoundSystem>();
    _registry.GetSystem<SoundSystem>().Initialize(_audioDevice);

    // Perform the subscription of the events for all systems
    _registry.GetSystem<MovementSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<DamageSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<KeyboardControlSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<ProjectileEmitSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<SoundSystem>().SubscribeToEvents(_eventBus);

    LevelLoader levelLoader;
    _luaState.open_libraries(sol::lib::base, sol::lib::os, sol::lib::math);
    levelLoader.LoadLevel(_luaState, _registry, _window, _renderer, 1, _mapWidth, _mapHeight);
}

void Game::ProcessInput(SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type)
    {
        case SDL_EVENT_WINDOW_RESIZED:
        {
            glm::vec2 windowSize = GetWindowSize();
            _camera.x = _camera.y = 0;
            _camera.w = windowSize.x;
            _camera.h = windowSize.y;
        }
        break;

        case SDL_EVENT_QUIT:
        {
            _isRunning = false;
        }
        break;

        case SDL_EVENT_KEY_DOWN:
        {
            _eventBus.EmitEvent<KeyPressedEvent>(event.key.scancode, &_eventBus);

            switch (event.key.scancode)
            {
                case SDL_SCANCODE_D:
                {
                    _isDebug = !_isDebug;                    
                }
                break;

                case SDL_SCANCODE_ESCAPE:
                {
                    _isRunning = false;
                }
                break;

                default:
                    break;
            }
        }
        break;

        default:
            break;
    }
}

void Game::Update()
{
    // Wait some time until we reach the milliseconds per frame
    static Uint64 millisecondsPreviousFrame = SDL_GetTicks();

    // Get current milliseconds
    Uint64 millisecondsCurrent = SDL_GetTicks();

    // Wait some time until the reach the target frame time in milliseconds
    Uint64 millisecondsFrameTime = millisecondsCurrent - millisecondsPreviousFrame;

    // Only delay execution if we are running too fast
    if (millisecondsFrameTime < MILLISECONDS_PER_FRAME)
    {
        SDL_Delay(static_cast<Uint32>(MILLISECONDS_PER_FRAME - millisecondsFrameTime));
        millisecondsCurrent = SDL_GetTicks();
        millisecondsFrameTime = millisecondsCurrent - millisecondsPreviousFrame;
    }

    // Delta time, convert milliseconds to seconds
    float deltaTime = static_cast<float>(millisecondsFrameTime) / 1000.0;

    // Update previous milliseconds per frame
    millisecondsPreviousFrame = millisecondsCurrent;

    // Update the registry to process the entities that are waiting to be created/deleted
    _registry.Update();

    // Update systems
    _registry.GetSystem<AnimationSystem>().Update();
    _registry.GetSystem<MovementSystem>().Update(deltaTime, GetMapSize());
    _registry.GetSystem<CollisionSystem>().Update(_eventBus);
    _registry.GetSystem<CameraMovementSystem>().Update(_camera, GetMapSize());
    _registry.GetSystem<ProjectileEmitSystem>().Update( _registry);
    _registry.GetSystem<LifecycleSystem>().Update();
    _registry.GetSystem<SoundSystem>().Update(_registry);
    _registry.GetSystem<ScriptSystem>().Update();
}

void Game::Render()
{
    SDL_SetRenderDrawColor(_renderer, 21, 21, 21, 255);
    SDL_RenderClear(_renderer);

    _registry.GetSystem<RenderSpriteSystem>().Update(*_renderer, AssetStore::Get(), _camera);
    _registry.GetSystem<RenderTextSystem>().Update(*_renderer, AssetStore::Get(), _camera);
    _registry.GetSystem<RenderHealthBarSystem>().Update(*_renderer, AssetStore::Get(), _camera);

    if (_isDebug)
    {
        _registry.GetSystem<RenderColliderSystem>().Update(*_renderer, _camera);
        _registry.GetSystem<RenderImGuiSystem>().Update(*_renderer, _registry, _camera);
    }

    SDL_RenderPresent(_renderer);
}

void Game::Destroy()
{
    _registry.ClearAll();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (_renderer)
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }

    if (_window)
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    SDL_Quit();
}

bool Game::IsGameRunning()
{
    return _isRunning;
}

glm::vec2 Game::GetWindowSize()
{
    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);

    return glm::vec2(windowWidth, windowHeight);
}

glm::vec2 Game::GetMapSize()
{
    return glm::vec2(_mapWidth, _mapHeight);
}