#include "LevelLoader.h"

#include <fstream>
#include <sstream>

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

LevelLoader::LevelLoader()
{
    LOG_INFO("LevelLoader constructor called!");
}

LevelLoader::~LevelLoader() 
{
    LOG_INFO("LevelLoader destructor called!");
}

void LevelLoader::LoadLevel(Registry& registry, SDL_Window* window, SDL_Renderer* renderer, int level, float& outMapWidth, float& outMapHeight)
{
    // Add assets to the asset store:
    // Textures
    AssetStore::Get().AddTexture(renderer, "tank-image", "./Assets/Images/tank-panther-right.png");
    AssetStore::Get().AddTexture(renderer, "truck-image", "./Assets/Images/truck-ford-right.png");
    AssetStore::Get().AddTexture(renderer, "tree-image", "./Assets/Images/tree.png");
    AssetStore::Get().AddTexture(renderer, "chopper-image", "./Assets/Images/chopper-spritesheet.png");
    AssetStore::Get().AddTexture(renderer, "radar-image", "./Assets/Images/radar.png");
    AssetStore::Get().AddTexture(renderer, "bullet-image", "./Assets/Images/bullet.png");
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
    AssetStore::Get().AddTexture(renderer, "jungle-tilemap-image", "./Assets/Tilemaps/jungle.png");
    // Open tilemap data (grid of tile indices)
    std::ifstream file("./Assets/Tilemaps/jungle.map");
    if (file.is_open())
    {
        // Query atlas (tileset) dimensions
        float atlasWidth = 0.0f;
        float atlasHeight = 0.0f;
        SDL_GetTextureSize(AssetStore::Get().GetTexture("jungle-tilemap-image"),
            &atlasWidth,
            &atlasHeight);

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
                Entity tile = registry.CreateEntity();
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
        outMapWidth = (max_num_of_col - 1) * TILE_WIDTH * TILE_SCALE + (TILE_WIDTH * TILE_SCALE);
        outMapHeight = (row - 1) * TILE_HEIGHT * TILE_SCALE + (TILE_HEIGHT * TILE_SCALE);
    }
    else
    {
        LOG_ERROR("Unable to open file: %s", "./Assets/Tilemaps/jungle.map");
    }

    // Create entities
    Entity radar = registry.CreateEntity();
    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);
    radar.AddComponent<TransformComponent>(glm::vec2(windowWidth - 74, 10), glm::vec2(1.0, 1.0), 0.0, true);
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2);
    radar.AddComponent<AnimationComponent>(8, 5, true);

    Entity chopper = registry.CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<TransformComponent>(glm::vec2(150, 150), glm::vec2(2.0, 2.0), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 2);
    chopper.AddComponent<BoxColliderComponent>(chopper.GetComponent<SpriteComponent>().width, chopper.GetComponent<SpriteComponent>().height);
    chopper.AddComponent<AnimationComponent>(2, 15, true);
    const float chopperSpead = 200;
    chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0, -chopperSpead), glm::vec2(chopperSpead, 0), glm::vec2(0, chopperSpead), glm::vec2(-chopperSpead, 0));
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<HealthComponent>(100);
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(150.0, 150.0), 0, 5000, 10, true, "bullet-image", "gunshot-sound");
    chopper.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));
    chopper.AddComponent<SoundComponent>("helicopter-sound", true, 0.1f);

    Entity tank = registry.CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(glm::vec2(700, 600), glm::vec2(2.0, 2.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    tank.AddComponent<SpriteComponent>(AssetStore::Get(), "tank-image", 2);
    tank.AddComponent<BoxColliderComponent>(tank.GetComponent<SpriteComponent>().width, tank.GetComponent<SpriteComponent>().height);
    tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 3000, 4000, 10, false, "bullet-image");
    tank.AddComponent<HealthComponent>(100);
    tank.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));

    Entity truck = registry.CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(glm::vec2(300, 750), glm::vec2(2.0, 2.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(90, 0));
    truck.AddComponent<SpriteComponent>(AssetStore::Get(), "truck-image", 1);
    truck.AddComponent<BoxColliderComponent>(truck.GetComponent<SpriteComponent>().width, truck.GetComponent<SpriteComponent>().height);
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, -100.0), 200, 8000, 10, false, "bullet-image");
    truck.AddComponent<HealthComponent>(100);
    truck.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));

    Entity treeA = registry.CreateEntity();
    treeA.Group("obstacles");
    treeA.AddComponent<TransformComponent>(glm::vec2(200, 700), glm::vec2(2.0, 2.0), 0.0);
    treeA.AddComponent<SpriteComponent>(AssetStore::Get(), "tree-image", 1);
    treeA.AddComponent<BoxColliderComponent>(treeA.GetComponent<SpriteComponent>().width, treeA.GetComponent<SpriteComponent>().height);

    Entity treeB = registry.CreateEntity();
    treeB.Group("obstacles");
    treeB.AddComponent<TransformComponent>(glm::vec2(600, 700), glm::vec2(2.0, 2.0), 0.0);
    treeB.AddComponent<SpriteComponent>(AssetStore::Get(), "tree-image", 1);
    treeB.AddComponent<BoxColliderComponent>(treeB.GetComponent<SpriteComponent>().width, treeB.GetComponent<SpriteComponent>().height);

    Entity label = registry.CreateEntity();
    label.AddComponent<TransformComponent>(glm::vec2(400, 650), glm::vec2(1.0, 1.0), 0.0);
    SDL_Color color = { 255, 0, 0 };
    label.AddComponent<TextLabelComponent>("THIS IS MY TRASH-CODE!!!!", "charriot-font", color);

    Entity levelSoundtrack = registry.CreateEntity();
    levelSoundtrack.AddComponent<SoundComponent>("SFU-music", true, 0.4f);
}