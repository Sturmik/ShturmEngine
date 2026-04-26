#include "Game.h"

#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include <iostream>

#include "ECS/ECS.h"
#include "Logger/LoggerMacro.h"

#include "Components/TransformComponent.h"
#include "Components/RigidBodyComponent.h"

Game::Game() : _isRunning(false), _window(nullptr), _renderer(nullptr)
{
	LOG_INFO("Game constructor called!");
}

Game::~Game()
{
    LOG_INFO("Game destructor called!");
}

void Game::Initialize()
{
    if (!SDL_Init((SDL_INIT_VIDEO)))
    {
        LOG_ERROR("Error initializing SDL: %s", SDL_GetError());
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
}

void Game::Setup()
{
    // Create an entity
    Entity tank = _registry.CreateEntity();
    
    // Add some components to that entity
    tank.AddComponent<TransformComponent>( glm::vec2(10, 30), glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>( glm::vec2(50, 20));

    tank.RemoveComponent<TransformComponent>();
}

void Game::ProcessInput(SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_EVENT_QUIT:
        {
            _isRunning = false;
        }
        break;

        case SDL_EVENT_KEY_DOWN:
        {
            switch (event.key.scancode)
            {
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

    // TODO:
    // MovementSystem.Update();
    // CollisionSystem.Update();
    // DamageSystem.Update();
}

void Game::Render()
{
    SDL_SetRenderDrawColor(_renderer, 21, 21, 21, 255);
    SDL_RenderClear(_renderer);

    // TODO: Render game objects... 

    SDL_RenderPresent(_renderer);
}

void Game::Destroy()
{
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
