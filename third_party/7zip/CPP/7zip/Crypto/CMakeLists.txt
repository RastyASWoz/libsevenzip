# 7-Zip Crypto Library

cmake_minimum_required(VERSION 3.20)

set(SEVENZIP_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Crypto codecs
set(SEVENZIP_CRYPTO_SOURCES
    # Initialize Password COM GUIDs
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/password_guids.cpp
    
    # Zip encryption
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/ZipCrypto.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/ZipStrong.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/WzAes.cpp
    
    # 7z encryption
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/7zAes.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/7zAesRegister.cpp
    
    # RAR encryption
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/Rar20Crypto.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/RarAes.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/Rar5Aes.cpp
    
    # Common crypto utilities
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/MyAes.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/MyAesReg.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/HmacSha1.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/HmacSha256.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/Pbkdf2HmacSha1.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Crypto/RandGen.cpp
)

# Filter out non-existent files
set(SEVENZIP_CRYPTO_SOURCES_EXIST)
foreach(SOURCE_FILE ${SEVENZIP_CRYPTO_SOURCES})
    if(EXISTS ${SOURCE_FILE})
        list(APPEND SEVENZIP_CRYPTO_SOURCES_EXIST ${SOURCE_FILE})
    else()
        message(STATUS "Skipping missing crypto file: ${SOURCE_FILE}")
    endif()
endforeach()

# Create the crypto library
add_library(7zip_crypto STATIC ${SEVENZIP_CRYPTO_SOURCES_EXIST})

# Include directories
target_include_directories(7zip_crypto PUBLIC
    ${SEVENZIP_CPP_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../C
)

# Link dependencies
target_link_libraries(7zip_crypto PUBLIC 7zip_cpp PRIVATE 7zip_c)

# Compiler settings
if(MSVC)
    target_compile_options(7zip_crypto PRIVATE
        /W3
        /wd4996  # Disable deprecated warnings
    )
    target_compile_definitions(7zip_crypto PUBLIC
        _UNICODE
        UNICODE
        WIN32
        _WINDOWS
    )
else()
    target_compile_options(7zip_crypto PRIVATE
        -Wall
        -Wno-deprecated
    )
endif()

message(STATUS "7zip_crypto library created with ${SEVENZIP_CRYPTO_SOURCES_EXIST} source files")
