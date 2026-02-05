# AshEmu Combined Launcher
# Builds ashemu executable

add_executable(ashemu
    src/main.c
    # Include auth server sources
    ${CMAKE_SOURCE_DIR}/auth/src/auth_server.c
    ${CMAKE_SOURCE_DIR}/auth/src/auth_session.c
    # Include world server sources
    ${CMAKE_SOURCE_DIR}/world/src/world_server.c
    ${CMAKE_SOURCE_DIR}/world/src/world_session.c
    ${CMAKE_SOURCE_DIR}/world/src/player.c
    ${CMAKE_SOURCE_DIR}/world/src/update.c
    ${CMAKE_SOURCE_DIR}/world/src/positions.c
)

target_include_directories(ashemu PRIVATE
    ${CMAKE_SOURCE_DIR}/auth/include
    ${CMAKE_SOURCE_DIR}/world/include
)

# Link to common and database libraries
target_link_libraries(ashemu PRIVATE common database)

# Platform-specific threading library
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(ashemu PRIVATE Threads::Threads)
endif()

# C17 standard
target_compile_features(ashemu PRIVATE c_std_17)
