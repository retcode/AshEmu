# AshEmu - WoW 1.12.1 Server Emulator
# Root CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(AshEmu C)

# C17 standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories in dependency order
add_subdirectory(common)
add_subdirectory(database)
add_subdirectory(auth)
add_subdirectory(world)
add_subdirectory(launcher)

# Print configuration summary
message(STATUS "")
message(STATUS "===========================================")
message(STATUS "  AshEmu Build Configuration")
message(STATUS "===========================================")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "")
message(STATUS "  Targets:")
message(STATUS "    - libcommon.a (common library)")
message(STATUS "    - libdatabase.a (database library)")
message(STATUS "    - ashemu_auth (standalone auth server)")
message(STATUS "    - ashemu_world (standalone world server)")
message(STATUS "    - ashemu (combined launcher)")
message(STATUS "===========================================")
message(STATUS "")
