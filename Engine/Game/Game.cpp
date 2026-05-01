#include "Game.h"

#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <sstream>

#include "ECS/ECS.h"
#include "Logger/LoggerMacro.h"

#include "Systems/MovementSystem.h"
#include "Systems/RenderSystem.h"

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

void Game::LoadLevel(int level)
{
    // Add the systems that need to be processed in our game
    _registry.AddSystem<MovementSystem>();
    _registry.AddSystem<RenderSystem>();

    // Add assets to the asset store
    _assetStore.AddTexture(_renderer, "tank-image", "./Assets/Images/tank-panther-right.png");
    _assetStore.AddTexture(_renderer, "truck-image", "./Assets/Images/truck-ford-right.png");

    // Load tile atlas texture (tileset image)
    _assetStore.AddTexture(_renderer, "jungle-tilemap-image", "./Assets/Tilemaps/jungle.png");
    // Open tilemap data (grid of tile indices)
    std::ifstream file("./Assets/Tilemaps/jungle.map");
    if (file.is_open()) 
    {
        // Query atlas (tileset) dimensions
        float atlasWidth = 0.0f;
        float atlasHeight = 0.0f;
        SDL_GetTextureSize( _assetStore.GetTexture("jungle-tilemap-image"),
            &atlasWidth,
            &atlasHeight );

        // Tileset layout (number of tiles in atlas grid)
        const int TILE_ROWS = 3;
        const int TILE_COLUMNS = 10;

        // Size of a single tile in the atlas
        const int TILE_WIDTH = static_cast<int>(atlasWidth / TILE_COLUMNS);
        const int TILE_HEIGHT = static_cast<int>(atlasHeight / TILE_ROWS);

        const float TILE_SCALE = 4.0f;

        std::string line;
        int row = 0;

        // Read map row-by-row
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;
            int col = 0;

            // Parse comma-separated tile indices
            while (std::getline(ss, token, ','))
            {
                const int tileIndex = std::atoi(token.c_str());

                // Convert 1D tile index to 2D atlas coordinates
                const int srcRow = tileIndex / TILE_COLUMNS;
                const int srcCol = tileIndex % TILE_COLUMNS;

                // Create tile entity
                Entity tile = _registry.CreateEntity();

                // World position (grid-based placement)
                tile.AddComponent<TransformComponent>(
                    glm::vec2(col * TILE_WIDTH * TILE_SCALE, row * TILE_HEIGHT * TILE_SCALE),
                    glm::vec2(TILE_SCALE, TILE_SCALE),
                    0.0f
                );

                // Source rectangle inside the atlas
                tile.AddComponent<SpriteComponent>(
                    "jungle-tilemap-image",
                    TILE_WIDTH,
                    TILE_HEIGHT,
                    srcCol * TILE_WIDTH,
                    srcRow * TILE_HEIGHT
                );

                col++;
            }

            row++;
        }

        file.close();
    }
    else 
    {
        LOG_ERROR("Unable to open file: %s", "./Assets/Tilemaps/jungle.map");
    }

    // Create entities
    Entity tank = _registry.CreateEntity();
    tank.AddComponent<TransformComponent>(glm::vec2(10, 30), glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(50, 20));
    tank.AddComponent<SpriteComponent>(_assetStore, "tank-image");

    Entity truck = _registry.CreateEntity();
    truck.AddComponent<TransformComponent>(glm::vec2(40, 160), glm::vec2(1.0, 1.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(20, 20));
    truck.AddComponent<SpriteComponent>(_assetStore, "truck-image");
}

void Game::Setup()
{
    LoadLevel(0);
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

    // Update systems
    _registry.GetSystem<MovementSystem>().Update(deltaTime);

    // Update the registry to process the entities that are waiting to be created/deleted
    _registry.Update();
}

void Game::Render()
{
    SDL_SetRenderDrawColor(_renderer, 21, 21, 21, 255);
    SDL_RenderClear(_renderer);

    _registry.GetSystem<RenderSystem>().Update(*_renderer, _assetStore);

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
