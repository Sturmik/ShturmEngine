#include "Game.h"

#include <iostream>

Game::Game() : _isRunning(false), _window(nullptr), _renderer(nullptr)
{
	std::cout << "Game constructor called!" << std::endl;
}

Game::~Game()
{
	std::cout << "Game destructor called!" << std::endl;
}

void Game::Initialize()
{
    if (!SDL_Init((SDL_INIT_VIDEO)))
    {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
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
        std::cerr <<  "Error creating SDL window: " << SDL_GetError() << std::endl;
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
        std::cerr << "Error creating SDL renderer: %s" << SDL_GetError() << std::endl;
        return;
    }

    _isRunning = true;
}

void Game::Run()
{
    while (_isRunning)
    {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
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
}

void Game::Update()
{
}

void Game::Render()
{
    SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
    SDL_RenderClear(_renderer);

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
