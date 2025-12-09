# 7-Zip Compress/Codec Library
# This library provides all compression/decompression codecs used by archive handlers

cmake_minimum_required(VERSION 3.20)

set(SEVENZIP_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Essential codecs - these are required by almost all archive formats
set(SEVENZIP_CODEC_CORE_SOURCES
    # ICoder interface GUIDs (manually defined to avoid IStream conflicts)
    ${SEVENZIP_CPP_DIR}/7zip/Compress/codec_guids_manual.cpp
    
    # Copy codec (uncompressed data) - REQUIRED by all handlers
    ${SEVENZIP_CPP_DIR}/7zip/Compress/CopyCoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/CopyRegister.cpp
    
    # Branch filters for executable compression
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BcjCoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BcjRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Bcj2Coder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Bcj2Register.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BranchMisc.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BranchRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ByteSwap.cpp
    
    # Delta filter
    ${SEVENZIP_CPP_DIR}/7zip/Compress/DeltaFilter.cpp
    
    # LZMA codec - used by 7z, xz
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzmaDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzmaEncoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzmaRegister.cpp
    
    # LZMA2 codec - used by 7z, xz
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Lzma2Decoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Lzma2Encoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Lzma2Register.cpp
    
    # Deflate codec - used by ZIP, gzip
    ${SEVENZIP_CPP_DIR}/7zip/Compress/DeflateDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/DeflateEncoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/DeflateRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Deflate64Register.cpp
    
    # BZip2 codec - used by BZ2, tar.bz2
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BZip2Decoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BZip2Encoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BZip2Register.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BZip2Crc.cpp
    
    # XZ codec
    ${SEVENZIP_CPP_DIR}/7zip/Compress/XzDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/XzEncoder.cpp
    
    # Zlib (deflate with zlib wrapper)
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ZlibDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ZlibEncoder.cpp
    
    # PPMd codec
    ${SEVENZIP_CPP_DIR}/7zip/Compress/PpmdDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/PpmdEncoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/PpmdRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/PpmdZip.cpp
    
    # LzOut window
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzOutWindow.cpp
    
    # Codec exports and registration
    ${SEVENZIP_CPP_DIR}/7zip/Compress/CodecExports.cpp
)

# RAR codecs (read-only support)
set(SEVENZIP_CODEC_RAR_SOURCES
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Rar1Decoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Rar2Decoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Rar3Decoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Rar3Vm.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/Rar5Decoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/RarCodecsRegister.cpp
)

# Additional decompression codecs
set(SEVENZIP_CODEC_EXTRA_SOURCES
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ShrinkDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ImplodeDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ImplodeHuffmanDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ZDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/QuantumDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzhDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzxDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzmsDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/XpressDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/LzfseDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/ZstdDecoder.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Compress/BitlDecoder.cpp
)

# Combine all sources
set(SEVENZIP_CODEC_ALL_SOURCES
    ${SEVENZIP_CODEC_CORE_SOURCES}
    ${SEVENZIP_CODEC_RAR_SOURCES}
    ${SEVENZIP_CODEC_EXTRA_SOURCES}
)

# Filter out non-existent files
set(SEVENZIP_CODEC_SOURCES_EXIST)
foreach(SOURCE_FILE ${SEVENZIP_CODEC_ALL_SOURCES})
    if(EXISTS ${SOURCE_FILE})
        list(APPEND SEVENZIP_CODEC_SOURCES_EXIST ${SOURCE_FILE})
    else()
        message(STATUS "Skipping missing file: ${SOURCE_FILE}")
    endif()
endforeach()

# Create the codec library
add_library(7zip_codec STATIC ${SEVENZIP_CODEC_SOURCES_EXIST})

# Include directories
target_include_directories(7zip_codec PUBLIC
    ${SEVENZIP_CPP_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../C
)

# Link dependencies
target_link_libraries(7zip_codec PUBLIC 7zip_cpp PRIVATE 7zip_c)

# Compiler settings
if(MSVC)
    target_compile_options(7zip_codec PRIVATE
        /W3
        /wd4996  # Disable deprecated warnings
        /D_7ZIP_LARGE_PAGES
        /D_7ZIP_ST  # Single-threaded for now
    )
    target_compile_definitions(7zip_codec PUBLIC
        _UNICODE
        UNICODE
        WIN32
        _WINDOWS
    )
else()
    target_compile_options(7zip_codec PRIVATE
        -Wall
        -Wno-deprecated
    )
endif()

message(STATUS "7zip_codec library created with ${SEVENZIP_CODEC_SOURCES_EXIST} source files")
