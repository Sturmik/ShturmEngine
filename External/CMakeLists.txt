# External/CMakeLists.txt

# We want consistent static linking in engine stacks
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---------------------------- Freetype ----------------------------
# (font rendering core dependency)

set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE) # we use our own harfbuzz build

add_subdirectory(freetype)

# Normalize target name (important for SDL_ttf compatibility)
if (TARGET freetype)
    add_library(Freetype::Freetype ALIAS freetype)
elseif(TARGET freetype_static)
    add_library(Freetype::Freetype ALIAS freetype_static)
endif()

set(FREETYPE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/freetype/include CACHE PATH "" FORCE)
set(FREETYPE_LIBRARY freetype CACHE FILEPATH "" FORCE)
set(FREETYPE_FOUND TRUE CACHE BOOL "" FORCE)

# ---------------------------- Harfbuzz ----------------------------
# (text shaping dependency for SDL_ttf)

set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)
set(HB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(HB_HAVE_FREETYPE ON CACHE BOOL "" FORCE)

add_subdirectory(harfbuzz)

# Normalize target name
if (TARGET harfbuzz)
    add_library(HarfBuzz::HarfBuzz ALIAS harfbuzz)
elseif(TARGET harfbuzz_static)
    add_library(HarfBuzz::HarfBuzz ALIAS harfbuzz_static)
endif()

# ---------------------------- SDL3 ----------------------------

add_subdirectory(SDL3)

# ---------------------------- SDL3 Image ----------------------------

# Disable formats that require external libs in SDL3_Image
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_VENDORED OFF)
set(SDLTTF_VENDORED OFF CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ ON CACHE BOOL "" FORCE)

add_subdirectory(SDL3_image)

# ---------------------------- SDL3 mixer ----------------------------

set(SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)

add_subdirectory(SDL3_mixer)

# ---------------------------- SDL3 ttf ----------------------------

# We explicitly disable vendored mode to match engine graph
set(SDLTTF_VENDORED OFF CACHE BOOL "" FORCE)

add_subdirectory(SDL3_ttf)

if (TARGET SDL3_ttf)
    target_link_libraries(SDL3_ttf PRIVATE
        Freetype::Freetype
        HarfBuzz::HarfBuzz
    )
endif()

# ---------------------------- glm ----------------------------

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE) 
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE) 
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE) 

add_subdirectory(glm)

set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lua)

set_source_files_properties(
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/linit.c
PROPERTIES LANGUAGE C)

add_library(lua STATIC
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/linit.c
)

target_include_directories(lua PUBLIC ${LUA_DIR})

# ---------------------------- sol2 ----------------------------

add_subdirectory(sol2)

# ---------------------------- ImGui ----------------------------

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
)

# ---------------- SDL3 backend for ImGui ----------------

add_library(imgui_sdl3 STATIC
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui_sdl3 PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui_sdl3 PUBLIC
    imgui
    SDL3::SDL3
)