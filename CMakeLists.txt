cmake_minimum_required(VERSION 3.20)

project(ShturmEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(External)
add_subdirectory(Engine)
add_subdirectory(Game)