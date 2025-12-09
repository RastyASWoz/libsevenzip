add_library(sevenzip_wrapper OBJECT
    # Common infrastructure
    common/wrapper_error.cpp
    common/wrapper_string.cpp
    common/wrapper_propvariant.cpp
    
    # Archive format detection
    archive/archive_format.cpp
    
    # Archive reader/writer
    archive/archive_reader.cpp
    archive/archive_writer.cpp
    archive/archive_factory.cpp
    archive/archive_init.cpp
    archive/archive_guids.cpp
    archive/extract_callback.cpp
    archive/update_callback.cpp
    
    # Stream implementations
    stream/stream_guids.cpp
    stream/stream_file.cpp
)

# Windows platform-specific stream implementation (isolated compilation unit)
if(WIN32)
    target_sources(sevenzip_wrapper PRIVATE
        stream/stream_file_win32.cpp  # Windows API wrapper (isolated)
    )
endif()

# TODO: Add more source files as they are implemented

target_include_directories(sevenzip_wrapper
    PUBLIC
        ${SEVENZIP_ROOT}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SEVENZIP_7ZIP_ROOT}/CPP
        ${SEVENZIP_7ZIP_ROOT}/C
)

target_link_libraries(sevenzip_wrapper
    PRIVATE
        SevenZip::CPP
)

# Set C++17 standard
target_compile_features(sevenzip_wrapper PUBLIC cxx_std_17)

# Link to 7-Zip libraries
target_link_libraries(sevenzip_wrapper
    PUBLIC
        7zip_cpp        # C++ utilities
        7zip_c          # C core
)

# 链接整个 7zip_archive, 7zip_codec 和 7zip_crypto 库（包含所有格式、编解码器和加密注册代码）
# 使用 WHOLE_ARCHIVE 确保所有格式处理器、编解码器和加密模块都被链接，即使没有直接引用
if(MSVC)
    target_link_options(sevenzip_wrapper PUBLIC 
        "/WHOLEARCHIVE:7zip_archive"
        "/WHOLEARCHIVE:7zip_codec"
        "/WHOLEARCHIVE:7zip_crypto"
    )
    target_link_libraries(sevenzip_wrapper PUBLIC 7zip_archive 7zip_codec 7zip_crypto)
else()
    target_link_libraries(sevenzip_wrapper PUBLIC 
        -Wl,--whole-archive 7zip_archive 7zip_codec 7zip_crypto -Wl,--no-whole-archive
    )
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(sevenzip_wrapper
        PRIVATE
            ole32
            oleaut32
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(sevenzip_wrapper PRIVATE
        _WIN32_WINNT=0x0601  # Windows 7+
        NOMINMAX             # 防止Windows.h定义min/max宏
        # 注意: 不定义WIN32_LEAN_AND_MEAN,因为7-Zip需要完整的COM支持(IUnknown等)
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(sevenzip_wrapper PRIVATE
        /W4                 # Warning level 4
        /permissive-        # Standards conformance
        /Zc:__cplusplus     # Enable correct __cplusplus macro
        /utf-8              # 使用UTF-8编码
        /FS                 # 强制同步 PDB 写入
    )
else()
    target_compile_options(sevenzip_wrapper PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Special handling for stream_file_win32.cpp: exclude 7-Zip headers to avoid conflicts
if(WIN32)
    set_source_files_properties(
        stream/stream_file_win32.cpp
        PROPERTIES
        COMPILE_DEFINITIONS ""  # Clear inherited definitions if needed
    )
endif()
