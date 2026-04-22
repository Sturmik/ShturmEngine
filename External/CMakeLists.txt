# External/CMakeLists.txt

# Disable formats that require external libs in SDL3_Image
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_VENDORED OFF)

# SDL3 (full build system included)
add_subdirectory(SDL3)
add_subdirectory(SDL3_Image)

# glm
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)