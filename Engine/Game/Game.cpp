#include "Game.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include <sstream>

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

void Game::LoadLevel(int level)
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
    
    // Audio system is a complex one and requires additional initialization
    _registry.AddSystem<SoundSystem>();
    _registry.GetSystem<SoundSystem>().Initialize(_audioDevice);

    // Perform the subscription of the events for all systems
    _registry.GetSystem<MovementSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<DamageSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<KeyboardControlSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<ProjectileEmitSystem>().SubscribeToEvents(_eventBus);
    _registry.GetSystem<SoundSystem>().SubscribeToEvents(_eventBus);

    // Add assets to the asset store:
    // Textures
    AssetStore::Get().AddTexture(_renderer, "tank-image", "./Assets/Images/tank-panther-right.png");
    AssetStore::Get().AddTexture(_renderer, "truck-image", "./Assets/Images/truck-ford-right.png");
    AssetStore::Get().AddTexture(_renderer, "tree-image", "./Assets/Images/tree.png");
    AssetStore::Get().AddTexture(_renderer, "chopper-image", "./Assets/Images/chopper-spritesheet.png");
    AssetStore::Get().AddTexture(_renderer, "radar-image", "./Assets/Images/radar.png");
    AssetStore::Get().AddTexture(_renderer, "bullet-image", "./Assets/Images/bullet.png");
    // Fonts
    AssetStore::Get().AddFont("charriot-font", "./Assets/Fonts/charriot.ttf", 16);
    AssetStore::Get().AddFont("arial-font", "./Assets/Fonts/arial.ttf", 16);
    AssetStore::Get().AddFont("pico-font-10", "./Assets/Fonts/pico8.ttf", 10);
    AssetStore::Get().AddFont("pico-font-12", "./Assets/Fonts/pico8.ttf", 12);
    // Music, Sounds
    AssetStore::Get().AddSound("helicopter-sound", "./Assets/Sounds/helicopter.wav");
    AssetStore::Get().AddSound("gunshot-sound", "./Assets/Sounds/gunshot.wav");
    AssetStore::Get().AddSound("SFU-music", "./Assets/Sounds/SFU_@bigsmuggs.wav");

    // Load tile atlas texture (tileset image)
    AssetStore::Get().AddTexture(_renderer, "jungle-tilemap-image", "./Assets/Tilemaps/jungle.png");
    // Open tilemap data (grid of tile indices)
    std::ifstream file("./Assets/Tilemaps/jungle.map");
    if (file.is_open()) 
    {
        // Query atlas (tileset) dimensions
        float atlasWidth = 0.0f;
        float atlasHeight = 0.0f;
        SDL_GetTextureSize( AssetStore::Get().GetTexture("jungle-tilemap-image"),
            &atlasWidth,
            &atlasHeight );

        // Tileset layout (number of tiles in atlas grid)
        const int TILE_ROWS = 3;
        const int TILE_COLUMNS = 10;

        // Size of a single tile in the atlas
        const int TILE_WIDTH = static_cast<int>(atlasWidth / TILE_COLUMNS);
        const int TILE_HEIGHT = static_cast<int>(atlasHeight / TILE_ROWS);

        const float TILE_SCALE = 3.0f;

        std::string line;
        int row = 0;
        int max_num_of_col = 0;

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
                tile.Group("tiles");

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
                    0,
                    srcCol * TILE_WIDTH,
                    srcRow * TILE_HEIGHT
                );

                col++;
            }

            max_num_of_col = std::max(max_num_of_col, col);

            row++;
        }

        file.close();

        // Update map width and height variables
        _mapWidth = (max_num_of_col - 1) * TILE_WIDTH * TILE_SCALE + (TILE_WIDTH * TILE_SCALE);
        _mapHeight = (row - 1) * TILE_HEIGHT * TILE_SCALE + (TILE_HEIGHT * TILE_SCALE);
    }
    else 
    {
        LOG_ERROR("Unable to open file: %s", "./Assets/Tilemaps/jungle.map");
    }

    // Create entities
    Entity radar = _registry.CreateEntity();
    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSizeInPixels(_window, &windowWidth, &windowHeight);
    radar.AddComponent<TransformComponent>(glm::vec2(windowWidth - 74, 10), glm::vec2(1.0, 1.0), 0.0, true);
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2);
    radar.AddComponent<AnimationComponent>(8, 5, true);

    Entity chopper = _registry.CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<TransformComponent>(glm::vec2(150, 150), glm::vec2(2.0, 2.0), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    chopper.AddComponent<SpriteComponent>( "chopper-image", 32, 32, 2);
    chopper.AddComponent<BoxColliderComponent>(chopper.GetComponent<SpriteComponent>().width, chopper.GetComponent<SpriteComponent>().height);
    chopper.AddComponent<AnimationComponent>(2, 15, true);
    const float chopperSpead = 200;
    chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0, -chopperSpead), glm::vec2(chopperSpead, 0), glm::vec2(0, chopperSpead), glm::vec2(-chopperSpead, 0));
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<HealthComponent>(100);
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(150.0, 150.0), 0, 5000, 10, true, "bullet-image", "gunshot-sound");
    chopper.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));
    chopper.AddComponent<SoundComponent>("helicopter-sound", true, 0.1f);

    Entity tank = _registry.CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(glm::vec2(700, 600), glm::vec2(2.0, 2.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    tank.AddComponent<SpriteComponent>(AssetStore::Get(), "tank-image", 2);
    tank.AddComponent<BoxColliderComponent>(tank.GetComponent<SpriteComponent>().width, tank.GetComponent<SpriteComponent>().height);
    tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 3000, 4000, 10, false, "bullet-image");
    tank.AddComponent<HealthComponent>(100);
    tank.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));

    Entity truck = _registry.CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(glm::vec2(300, 750), glm::vec2(2.0, 2.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(90, 0));
    truck.AddComponent<SpriteComponent>(AssetStore::Get(), "truck-image", 1);
    truck.AddComponent<BoxColliderComponent>(truck.GetComponent<SpriteComponent>().width, truck.GetComponent<SpriteComponent>().height);
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, -100.0), 200, 8000, 10, false, "bullet-image");
    truck.AddComponent<HealthComponent>(100);
    truck.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));

    Entity treeA = _registry.CreateEntity();
    treeA.Group("obstacles");
    treeA.AddComponent<TransformComponent>(glm::vec2(200, 700), glm::vec2(2.0, 2.0), 0.0);
    treeA.AddComponent<SpriteComponent>(AssetStore::Get(), "tree-image", 1);
    treeA.AddComponent<BoxColliderComponent>(treeA.GetComponent<SpriteComponent>().width, treeA.GetComponent<SpriteComponent>().height);

    Entity treeB = _registry.CreateEntity();
    treeB.Group("obstacles");
    treeB.AddComponent<TransformComponent>(glm::vec2(600, 700), glm::vec2(2.0, 2.0), 0.0);
    treeB.AddComponent<SpriteComponent>(AssetStore::Get(), "tree-image", 1);
    treeB.AddComponent<BoxColliderComponent>(treeB.GetComponent<SpriteComponent>().width, treeB.GetComponent<SpriteComponent>().height);

    Entity label = _registry.CreateEntity();
    label.AddComponent<TransformComponent>(glm::vec2(400, 650), glm::vec2(1.0, 1.0), 0.0);
    SDL_Color color = { 255, 0, 0};
    label.AddComponent<TextLabelComponent>( "THIS IS MY TRASH-CODE!!!!", "charriot-font", color);

    Entity levelSoundtrack = _registry.CreateEntity();
    levelSoundtrack.AddComponent<SoundComponent>("SFU-music", true, 0.4f);
}

void Game::Setup()
{
    LoadLevel(0);
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