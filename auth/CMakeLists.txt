# AshEmu Auth Server
# Builds ashemu_auth executable

add_executable(ashemu_auth
    src/main.c
    src/auth_server.c
    src/auth_session.c
)

target_include_directories(ashemu_auth PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link to common and database libraries
target_link_libraries(ashemu_auth PRIVATE common database)

# C17 standard
target_compile_features(ashemu_auth PRIVATE c_std_17)
