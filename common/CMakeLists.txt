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

# Find OpenSSL - try find_package first, fall back to manual paths
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    target_link_libraries(common PUBLIC OpenSSL::Crypto)
else()
    # Manual OpenSSL discovery (for zig/clang on Windows with MSVC-built OpenSSL)
    if(DEFINED ENV{OPENSSL_ROOT_DIR})
        set(_OPENSSL_ROOT "$ENV{OPENSSL_ROOT_DIR}")
    elseif(OPENSSL_ROOT_DIR)
        set(_OPENSSL_ROOT "${OPENSSL_ROOT_DIR}")
    elseif(WIN32)
        set(_OPENSSL_ROOT "C:/Program Files/OpenSSL-Win64")
    endif()

    if(EXISTS "${_OPENSSL_ROOT}/include/openssl/ssl.h")
        message(STATUS "Using manual OpenSSL from: ${_OPENSSL_ROOT}")
        target_include_directories(common PUBLIC "${_OPENSSL_ROOT}/include")
        target_link_libraries(common PUBLIC "${_OPENSSL_ROOT}/lib/VC/x64/MD/libcrypto.lib")
    else()
        message(FATAL_ERROR "Could not find OpenSSL. Set OPENSSL_ROOT_DIR.")
    endif()
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(common PUBLIC ws2_32)
endif()

# C17 standard
target_compile_features(common PUBLIC c_std_17)
