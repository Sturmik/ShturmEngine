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
#include "Systems/ScriptSystem.h"

LevelLoader::LevelLoader()
{
    LOG_INFO("LevelLoader constructor called!");
}

LevelLoader::~LevelLoader() 
{
    LOG_INFO("LevelLoader destructor called!");
}

void LevelLoader::LoadLevel(sol::state& luaState, Registry& registry, SDL_Window* window, SDL_Renderer* renderer, int levelNumber, float& outMapWidth, float& outMapHeight)
{
    sol::load_result script = luaState.load_file("./Assets/Scripts/Level" + std::to_string(levelNumber) + ".lua");
    // Checks the syntax of the script, but it does not execute the script
    if (!script.valid())
    {
        sol::error error = script;
        std::string errorMessage = error.what();

        LOG_ERROR("%s", errorMessage.c_str());
        return;
    }

    // Load the entities and components from according level script
    sol::protected_function_result result = script();
    if (!result.valid())
    {
        sol::error error = result;
        LOG_ERROR("%s", error.what());
        return;
    }

    //////////////////////// Read the big table for the current level
    sol::table level = luaState["level"];

    //////////////////////// Read the level assets
    sol::table assets = level["assets"];

    // Clear existing assets
    AssetStore::Get().ClearAssets();

    {
        int i = 0;
        while (true)
        {
            sol::optional<sol::table> hasAsset = assets[i];
            if (hasAsset == sol::nullopt)
            {
                break;
            }

            sol::table asset = assets[i];
        
            std::string assetType = asset["type"];
        
            if (assetType == "texture")
            {
                AssetStore::Get().AddTexture(renderer, asset["id"], asset["file"]);
            }
            else if (assetType == "font")
            {
                AssetStore::Get().AddFont(asset["id"], asset["file"], asset["font_size"]);
            }
            else if (assetType == "sound")
            {
                AssetStore::Get().AddSound(asset["id"], asset["file"]);
            }

            ++i;
        }
    }

    //////////////////////// Read tilemap
    sol::table map = level["tilemap"];
    std::string mapFilePath = map["map_file"];
    std::string mapTextureAssetId = map["texture_asset_id"];
    int mapNumRows = map["texture_num_rows"];
    int mapNumCols = map["texture_num_cols"];
    double mapScale = map["scale"];
    // Open tilemap data (grid of tile indices)
    std::ifstream file(mapFilePath);
    if (file.is_open())
    {
        // Query atlas (tileset) dimensions
        float atlasWidth = 0.0f;
        float atlasHeight = 0.0f;
        SDL_GetTextureSize(AssetStore::Get().GetTexture(mapTextureAssetId),
            &atlasWidth,
            &atlasHeight);

        // Size of a single tile in the atlas
        const int tileWidth = static_cast<int>(atlasWidth / mapNumCols);
        const int tileHeight = static_cast<int>(atlasHeight / mapNumRows);

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
                const int srcRow = tileIndex / mapNumCols;
                const int srcCol = tileIndex % mapNumCols;

                // Create tile entity
                Entity tile = registry.CreateEntity();
                tile.Group("tiles");

                // World position (grid-based placement)
                tile.AddComponent<TransformComponent>(
                    glm::vec2(col * tileWidth * mapScale, row * tileHeight * mapScale),
                    glm::vec2(mapScale, mapScale),
                    0.0f
                );

                // Source rectangle inside the atlas
                tile.AddComponent<SpriteComponent>(
                    mapTextureAssetId,
                    tileWidth,
                    tileHeight,
                    0,
                    srcCol * tileWidth,
                    srcRow * tileHeight
                );

                col++;
            }

            max_num_of_col = std::max(max_num_of_col, col);

            row++;
        }

        file.close();

        // Update map width and height variables
        outMapWidth = (max_num_of_col - 1) * tileWidth * mapScale + (tileWidth * mapScale);
        outMapHeight = (row - 1) * tileHeight * mapScale + (tileHeight * mapScale);
    }
    else
    {
        LOG_ERROR("Unable to open file: %s", mapFilePath);

        outMapWidth = std::numeric_limits<float>::max();
        outMapHeight = std::numeric_limits<float>::max();
    }

    //////////////////////// Read entities
    sol::table entities = level["entities"];
    
    {
        int i = 0;
        while (true) {
            sol::optional<sol::table> hasEntity = entities[i];
            if (hasEntity == sol::nullopt) {
                break;
            }

            sol::table entity = entities[i];

            Entity newEntity = registry.CreateEntity();

            // Tag
            sol::optional<std::string> tag = entity["tag"];
            if (tag != sol::nullopt) {
                newEntity.Tag(entity["tag"]);
            }

            // Group
            sol::optional<std::string> group = entity["group"];
            if (group != sol::nullopt) {
                newEntity.Group(entity["group"]);
            }

            // Components
            sol::optional<sol::table> hasComponents = entity["components"];
            if (hasComponents != sol::nullopt) {
                // Transform
                sol::optional<sol::table> transform = entity["components"]["transform"];
                if (transform != sol::nullopt) {
                    newEntity.AddComponent<TransformComponent>(
                        glm::vec2(
                            entity["components"]["transform"]["position"]["x"],
                            entity["components"]["transform"]["position"]["y"]
                        ),
                        glm::vec2(
                            entity["components"]["transform"]["scale"]["x"].get_or(1.0),
                            entity["components"]["transform"]["scale"]["y"].get_or(1.0)
                        ),
                        entity["components"]["transform"]["rotation"].get_or(0.0),
                        entity["components"]["transform"]["fixed"].get_or(false)
                    );
                }

                // RigidBody
                sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
                if (rigidbody != sol::nullopt) {
                    newEntity.AddComponent<RigidBodyComponent>(
                        glm::vec2(
                            entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
                            entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
                        )
                    );
                }

                // Sprite
                sol::optional<sol::table> sprite = entity["components"]["sprite"];
                if (sprite != sol::nullopt) {
                    // Check, if width or height present, if not. Apply automatic size calculation
                    sol::optional<int> spriteWidth = entity["components"]["sprite"]["width"];
                    sol::optional<int> spriteHeight = entity["components"]["sprite"]["height"];
                    if (spriteWidth == sol::nullopt || spriteHeight == sol::nullopt)
                    {
                        newEntity.AddComponent<SpriteComponent>(
                            AssetStore::Get(),
                            entity["components"]["sprite"]["texture_asset_id"],
                            entity["components"]["sprite"]["z_index"].get_or(1),
                            entity["components"]["sprite"]["src_rect_x"].get_or(0),
                            entity["components"]["sprite"]["src_rect_y"].get_or(0)
                        );
                    }
                    else
                    {
                        newEntity.AddComponent<SpriteComponent>(
                            entity["components"]["sprite"]["texture_asset_id"],
                            entity["components"]["sprite"]["width"],
                            entity["components"]["sprite"]["height"],
                            entity["components"]["sprite"]["z_index"].get_or(1),
                            entity["components"]["sprite"]["src_rect_x"].get_or(0),
                            entity["components"]["sprite"]["src_rect_y"].get_or(0)
                        );
                    }
                }

                // Animation
                sol::optional<sol::table> animation = entity["components"]["animation"];
                if (animation != sol::nullopt) {
                    newEntity.AddComponent<AnimationComponent>(
                        entity["components"]["animation"]["num_frames"].get_or(1),
                        entity["components"]["animation"]["speed_rate"].get_or(1)
                    );
                }

                // BoxCollider
                sol::optional<sol::table> collider = entity["components"]["boxcollider"];
                if (collider != sol::nullopt) {
                    newEntity.AddComponent<BoxColliderComponent>(
                        entity["components"]["boxcollider"]["width"],
                        entity["components"]["boxcollider"]["height"],
                        glm::vec2(
                            entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
                            entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
                        )
                    );
                }

                // Health
                sol::optional<sol::table> health = entity["components"]["health"];
                if (health != sol::nullopt) {
                    newEntity.AddComponent<HealthComponent>(
                        static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
                    );
                }

                // Health bar
                sol::optional<sol::table> healthBar = entity["components"]["health_bar"];
                if (healthBar != sol::nullopt) {
                    newEntity.AddComponent<HealthBarComponent>(
                        entity["components"]["health_bar"]["texture_asset_id"],
                        glm::vec2(
                            entity["components"]["health_bar"]["text_offset"]["x"].get_or(1.0),
                            entity["components"]["health_bar"]["text_offset"]["y"].get_or(1.0)
                        ),
                        glm::vec2(
                            entity["components"]["health_bar"]["health_bar_size"]["x"].get_or(1.0),
                            entity["components"]["health_bar"]["health_bar_size"]["y"].get_or(1.0)
                        ),
                        glm::vec2(
                            entity["components"]["health_bar"]["health_bar_offset"]["x"].get_or(1.0),
                            entity["components"]["health_bar"]["health_bar_offset"]["y"].get_or(1.0)
                        ));
                }

                // ProjectileEmitter
                sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
                if (projectileEmitter != sol::nullopt) {
                    newEntity.AddComponent<ProjectileEmitterComponent>(
                        glm::vec2(
                            entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
                            entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
                        ),
                        static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
                        static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
                        static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
                        entity["components"]["projectile_emitter"]["friendly"].get_or(false),
                        entity["components"]["projectile_emitter"]["texture_asset_id"].get_or(std::string{}),
                        entity["components"]["projectile_emitter"]["sound_asset_id"].get_or(std::string{})
                    );
                }

                // CameraFollow
                sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
                if (cameraFollow != sol::nullopt) {
                    newEntity.AddComponent<CameraFollowComponent>();
                }

                // KeyboardControlled
                sol::optional<sol::table> keyboardControlled = entity["components"]["keyboard_controller"];
                if (keyboardControlled != sol::nullopt) {
                    newEntity.AddComponent<KeyboardControlledComponent>(
                        glm::vec2(
                            entity["components"]["keyboard_controller"]["up_velocity"]["x"],
                            entity["components"]["keyboard_controller"]["up_velocity"]["y"]
                        ),
                        glm::vec2(
                            entity["components"]["keyboard_controller"]["right_velocity"]["x"],
                            entity["components"]["keyboard_controller"]["right_velocity"]["y"]
                        ),
                        glm::vec2(
                            entity["components"]["keyboard_controller"]["down_velocity"]["x"],
                            entity["components"]["keyboard_controller"]["down_velocity"]["y"]
                        ),
                        glm::vec2(
                            entity["components"]["keyboard_controller"]["left_velocity"]["x"],
                            entity["components"]["keyboard_controller"]["left_velocity"]["y"]
                        )
                    );
                }

                // Sound
                sol::optional<sol::table> sound = entity["components"]["sound"];
                if (sound != sol::nullopt) {
                    newEntity.AddComponent<SoundComponent>(
                        entity["components"]["sound"]["sound_asset_id"],
                        entity["components"]["sound"]["loop"].get_or(false),
                        entity["components"]["sound"]["volume"].get_or(0.5f)
                    );
                }

                // Script
                sol::optional<sol::table> script = entity["components"]["on_update_script"];
                if ( script != sol::nullopt )
                {
                    sol::function function = entity["components"]["on_update_script"][0];
                    newEntity.AddComponent<ScriptComponent>(function);
                }
            }
            i++;
        }
    }
}