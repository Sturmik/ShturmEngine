-- Define a table with the values of the first level
level = {
    ----------------------------------------------------
    -- Table to define the list of assets
    ----------------------------------------------------
    assets = {
        [0] =
        { type = "texture", id = "tilemap-texture", file = "./Assets/Tilemaps/jungle.png" },
        { type = "texture", id = "chopper-texture", file = "./Assets/Images/chopper-spritesheet.png" },
        { type = "texture", id = "tank-texture",    file = "./Assets/Images/tank-tiger-up.png" },
        { type = "texture", id = "bullet-texture",  file = "./Assets/images/bullet.png" },
        { type = "font"   , id = "pico8-font-5",    file = "./Assets/Fonts/pico8.ttf", font_size = 5 },
        { type = "font"   , id = "pico8-font-10",   file = "./Assets/Fonts/pico8.ttf", font_size = 10 },
        { type = "font"   , id = "charriot-font",   file = "./Assets/Fonts/charriot.ttf", font_size = 16 },
        { type = "font"   , id = "arial-font",      file = "./Assets/Fonts/arial.ttf", font_size = 16 },
        { type = "sound"  , id = "helicopter-sound",file = "./Assets/Sounds/helicopter.wav", font_size = 16 },
        { type = "sound"  , id = "gunshot-sound",   file = "./Assets/Sounds/gunshot.wav", font_size = 16 },
        { type = "sound"  , id = "SFU-music",       file = "./Assets/Sounds/SFU_@bigsmuggs.wav", font_size = 16 }
    },

    ----------------------------------------------------
    -- table to define the map config variables
    ----------------------------------------------------
    tilemap = {
        map_file = "./assets/tilemaps/jungle.map",
        texture_asset_id = "tilemap-texture",
        num_rows = 3,
        num_cols = 10,
        scale = 2.0
    },

    ----------------------------------------------------
    -- table to define entities and their components
    ----------------------------------------------------
    entities = {
        [0] =
        {
            -- Player
            tag = "player",
            components = {
                transform = {
                    position = { x = 242, y = 110 },
                    scale = { x = 1.0, y = 1.0 },
                    rotation = 0.0, -- degrees
                    fixed = false
                },
                rigidbody = {
                    velocity = { x = 0.0, y = 0.0 }
                },
                sprite = {
                    texture_asset_id = "chopper-texture",
                    width = 32,
                    height = 32,
                    z_index = 4,
                    src_rect_x = 0,
                    src_rect_y = 0
                },
                animation = {
                    num_frames = 2,
                    speed_rate = 10 -- fps
                },
                boxcollider = {
                    width = 32,
                    height = 25,
                    offset = { x = 0, y = 5 }
                },
                health = {
                    health_percentage = 100
                },
                health_bar = {
                    texture_asset_id = "pico8-font-10",
                    text_offset = { x = 50, y = 0 },
                    health_bar_size = { x = 10, y = 5 },
                    health_bar_offset = { x = 50, y = 20 }
                },
                projectile_emitter = {
                    projectile_velocity = { x = 200, y = 200 },
                    projectile_duration = 10, -- seconds
                    repeat_frequency = 0, -- seconds
                    hit_percentage_damage = 10,
                    friendly = true,
                    texture_asset_id = "bullet-texture",
                    sound_asset_id = "gunshot-sound"
                },
                keyboard_controller = {
                    up_velocity = { x = 0, y = -50 },
                    right_velocity = { x = 50, y = 0 },
                    down_velocity = { x = 0, y = 50 },
                    left_velocity = { x = -50, y = 0 }
                },
                camera_follow = {
                    follow = true
                },
                sound = {
                    sound_asset_id =  "helicopter-sound", 
                    loop = true, 
                    volume = 0.1
                },
            }
        },
        {
            -- Tank
            group = "enemies",
            components = {
                transform = {
                    position = { x = 200, y = 497 },
                    scale = { x = 1.0, y = 1.0 },
                    rotation = 0.0, -- degrees
                },
                sprite = {
                    texture_asset_id = "tank-texture",
                    z_index = 2
                },
                boxcollider = {
                    width = 25,
                    height = 18,
                    offset = { x = 0, y = 7 }
                },
                health = {
                    health_percentage = 100
                },
                health_bar = {
                    texture_asset_id = "pico8-font-10",
                    text_offset = { x = 50, y = 0 },
                    health_bar_size = { x = 10, y = 5 },
                    health_bar_offset = { x = 50, y = 20 }
                },
                projectile_emitter = {
                    projectile_velocity = { x = 100, y = 0 },
                    projectile_duration = 2, -- seconds
                    repeat_frequency = 1, -- seconds
                    hit_percentage_damage = 20,
                    friendly = false,
                    texture_asset_id = "bullet-texture",
                    sound_asset_id = "gunshot-sound"
                }
            }
        },
        {
            -- Soundtrack
            components = {
                sound = {
                    sound_asset_id = "SFU-music", 
                    loop = true, 
                    volume = 0.4
                }
            }
        }
    }
}