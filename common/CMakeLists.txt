# AshEmu Common Library
# Builds libcommon.a

add_library(common STATIC
    src/common.c
    src/packet.c
    src/crypto.c
    src/worldcrypt.c
    src/network.c
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(common PUBLIC OpenSSL::Crypto)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(common PUBLIC ws2_32)
endif()

# C17 standard
target_compile_features(common PUBLIC c_std_17)
