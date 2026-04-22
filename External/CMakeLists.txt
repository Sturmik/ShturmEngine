# External/CMakeLists.txt

set(EXTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR})

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