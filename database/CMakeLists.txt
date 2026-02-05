# AshEmu Database Library
# Builds libdatabase.a

# Build SQLite from amalgamation
add_library(sqlite3 STATIC
    ${CMAKE_SOURCE_DIR}/third_party/sqlite/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/sqlite
)
# Disable SQLite shell and other optional features
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_OMIT_LOAD_EXTENSION
)
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /W0)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()

add_library(database STATIC
    src/models.c
    src/database.c
)

target_include_directories(database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/sqlite
)

# Link to common library and sqlite3
target_link_libraries(database PUBLIC common sqlite3)

# C17 standard
target_compile_features(database PUBLIC c_std_17)
