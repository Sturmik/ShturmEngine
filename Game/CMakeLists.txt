# Game/CMakeLists.txt

add_executable(GameExe
    main.cpp
)

target_link_libraries(GameExe PRIVATE Engine)

# Copy assets to the output
add_custom_command(TARGET GameExe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:GameExe>/Assets
)