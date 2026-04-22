# Engine/CMakeLists.txt

add_library(Engine STATIC
    Logger/Logger.cpp
    Game/Game.cpp
)

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Engine
    PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        freetype
        harfbuzz
        SDL3_ttf::SDL3_ttf
        glm
        sol2::sol2
        lua
        imgui
        imgui_sdl3
)