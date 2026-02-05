# AshEmu World Server
# Builds ashemu_world executable

add_executable(ashemu_world
    src/main.c
    src/world_server.c
    src/world_session.c
    src/player.c
    src/update.c
    src/positions.c
)

target_include_directories(ashemu_world PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link to common and database libraries
target_link_libraries(ashemu_world PRIVATE common database)

# C17 standard
target_compile_features(ashemu_world PRIVATE c_std_17)
