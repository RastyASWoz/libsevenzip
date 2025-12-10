# FFI (Foreign Function Interface) C ABI Layer
# Provides a stable C API for language bindings

# Collect all FFI implementation files
set(FFI_SOURCES
    sz_error.cpp
    sz_archive.cpp
    sz_writer.cpp
    sz_compress.cpp
    sz_convenience.cpp
    sz_version.cpp
)

set(FFI_HEADERS
    sz_error_internal.h
)

# Create FFI library as shared library (DLL on Windows)
add_library(sevenzip_ffi SHARED ${FFI_SOURCES} ${FFI_HEADERS})

# Include directories
target_include_directories(sevenzip_ffi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Link with C++ API (required), wrapper and 7-Zip libraries
target_link_libraries(sevenzip_ffi PUBLIC
    sevenzip_api
    sevenzip_wrapper
    7zip_archive
    7zip_codec
    7zip_crypto
    7zip_cpp
    7zip_c
)

# Set C++ standard
target_compile_features(sevenzip_ffi PRIVATE cxx_std_20)

# Platform-specific settings
if(WIN32)
    # Windows DLL export/import
    target_compile_definitions(sevenzip_ffi PRIVATE SEVENZIP_EXPORTS)
else()
    # Linux/Unix shared library visibility
    target_compile_options(sevenzip_ffi PRIVATE -fvisibility=hidden)
endif()

# Optional: Create shared library variant
option(BUILD_SHARED_FFI "Build shared library for FFI layer" OFF)
if(BUILD_SHARED_FFI)
    add_library(sevenzip_ffi_shared SHARED ${FFI_SOURCES} ${FFI_HEADERS})
    target_include_directories(sevenzip_ffi_shared PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(sevenzip_ffi_shared PUBLIC
        sevenzip_api
        sevenzip_wrapper
    )
    target_compile_features(sevenzip_ffi_shared PRIVATE cxx_std_20)
    
    if(WIN32)
        target_compile_definitions(sevenzip_ffi_shared PRIVATE SZ_BUILD_DLL)
    else()
        target_compile_options(sevenzip_ffi_shared PRIVATE -fvisibility=hidden)
    endif()
    
    # Set output name
    set_target_properties(sevenzip_ffi_shared PROPERTIES
        OUTPUT_NAME sevenzip_c
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()
