cmake_minimum_required(VERSION 3.20)

# ============================================
# 项目信息
# ============================================
project(libsevenzip VERSION 0.1.0 LANGUAGES C CXX)

message(STATUS "========================================")
message(STATUS "  libsevenzip - Modern FFI for 7-Zip")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "========================================")

# ============================================
# 平台检测
# ============================================
if(NOT WIN32)
    message(FATAL_ERROR "This project currently only supports Windows platform")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "This project currently only supports MSVC compiler")
endif()

# 检测架构
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SEVENZIP_ARCH "x64")
    message(STATUS "Target Architecture: x64")
else()
    set(SEVENZIP_ARCH "x86")
    message(STATUS "Target Architecture: x86 (32-bit)")
endif()

# ============================================
# Windows SDK 自动检测
# ============================================
set(PREFERRED_SDK_VERSIONS 
    "10.0.19041.0"
    "10.0.22000.0"
    "10.0.22621.0"
    "10.0.26100.0"
)

if(DEFINED ENV{WindowsSdkDir})
    set(SDK_ROOT "$ENV{WindowsSdkDir}")
else()
    set(SDK_ROOT "C:/Program Files (x86)/Windows Kits/10")
    if(NOT EXISTS "${SDK_ROOT}")
        set(SDK_ROOT "D:/Windows Kits/10")
    endif()
endif()

set(SDK_FOUND FALSE)
foreach(SDK_VERSION IN LISTS PREFERRED_SDK_VERSIONS)
    set(SDK_INCLUDE_PATH "${SDK_ROOT}/Include/${SDK_VERSION}")
    if(EXISTS "${SDK_INCLUDE_PATH}/um/windows.h")
        set(CMAKE_SYSTEM_VERSION "${SDK_VERSION}" CACHE STRING "Windows SDK version" FORCE)
        set(SDK_FOUND TRUE)
        message(STATUS "Found Windows SDK: ${CMAKE_SYSTEM_VERSION}")
        break()
    endif()
endforeach()

if(NOT SDK_FOUND)
    message(WARNING "Preferred Windows SDK not found, using default")
endif()

# ============================================
# 项目根目录
# ============================================
set(SEVENZIP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# ============================================
# 构建选项
# ============================================
option(SEVENZIP_BUILD_FFI "Build FFI library (primary target)" ON)
option(SEVENZIP_BUILD_CPP_API "Build modern C++ API (optional)" OFF)
option(SEVENZIP_BUILD_EXAMPLES "Build examples" OFF)
option(SEVENZIP_BUILD_TESTS "Build tests" OFF)
option(SEVENZIP_ENABLE_INSTALL "Enable CMake install config (for C++ ecosystem)" OFF)

message(STATUS "Build Options:")
message(STATUS "  FFI Library: ${SEVENZIP_BUILD_FFI}")
message(STATUS "  C++ API: ${SEVENZIP_BUILD_CPP_API}")
message(STATUS "  Examples: ${SEVENZIP_BUILD_EXAMPLES}")
message(STATUS "  Tests: ${SEVENZIP_BUILD_TESTS}")
message(STATUS "  CMake Install: ${SEVENZIP_ENABLE_INSTALL}")

# ============================================
# 全局编译设置
# ============================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 全局定义
add_definitions(-D_WIN32 -DWIN32 -DNOMINMAX)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-D_WIN64)
endif()

# MSVC 设置
if(MSVC)
    add_compile_options(
        /W3
        /MP           # 多进程编译
        /permissive-  # 严格标准
        /Zc:__cplusplus  # 正确的 __cplusplus 宏
        /utf-8        # UTF-8 编码
    )
    
    # 运行时库
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/MDd)
    else()
        add_compile_options(/MD)
    endif()
endif()

# ============================================
# 第三方库: 7-Zip
# ============================================
message(STATUS "Building 7-Zip libraries...")
add_subdirectory(third_party/7zip/C)
add_subdirectory(third_party/7zip/CPP)

# ============================================
# Wrapper 层 (COM 接口封装)
# ============================================
message(STATUS "Building wrapper layer...")
add_subdirectory(src/wrapper)

# ============================================
# C++ API (内部依赖或可选公开)
# ============================================
# FFI 层需要 C++ API，所以始终构建但默认不公开
if(SEVENZIP_BUILD_FFI OR SEVENZIP_BUILD_CPP_API)
    if(NOT SEVENZIP_BUILD_CPP_API)
        message(STATUS "Building C++ API (internal for FFI)...")
    else()
        message(STATUS "Building C++ API (public)...")
    endif()
    add_subdirectory(src/api)
    
    # 静态库
    add_library(sevenzip STATIC
        $<TARGET_OBJECTS:sevenzip_api>
        $<TARGET_OBJECTS:sevenzip_wrapper>
        $<TARGET_OBJECTS:7zip_cpp>
        $<TARGET_OBJECTS:7zip_c>
    )
    
    target_include_directories(sevenzip PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    
    target_link_libraries(sevenzip PUBLIC
        7zip_archive
        7zip_codec
        7zip_crypto
    )
endif()

# ============================================
# FFI 层 (主要目标)
# ============================================
if(SEVENZIP_BUILD_FFI)
    message(STATUS "Building FFI library (primary target)...")
    add_subdirectory(src/ffi)
endif()

# ============================================
# 示例程序
# ============================================
if(SEVENZIP_BUILD_EXAMPLES)
    message(STATUS "Building examples...")
    add_subdirectory(examples)
endif()

# ============================================
# 单元测试
# ============================================
if(SEVENZIP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================
# 安装配置 (仅用于 C++ 生态系统集成)
# ============================================
include(GNUInstallDirs)

# 始终安装头文件和 FFI 库
install(DIRECTORY include/sevenzip
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

if(SEVENZIP_BUILD_FFI)
    install(TARGETS sevenzip_ffi
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# 完整 CMake 配置 (仅在启用时)
if(SEVENZIP_ENABLE_INSTALL AND SEVENZIP_BUILD_CPP_API)
    include(CMakePackageConfigHelpers)
    
    install(TARGETS sevenzip
        EXPORT SevenZipTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    if(SEVENZIP_BUILD_FFI)
        install(TARGETS sevenzip_ffi
            EXPORT SevenZipTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
    
    set(SEVENZIP_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/SevenZip)
    
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    
    install(EXPORT SevenZipTargets
        FILE SevenZipTargets.cmake
        NAMESPACE SevenZip::
        DESTINATION ${SEVENZIP_CMAKE_DIR}
    )
    
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SevenZipConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfig.cmake"
        INSTALL_DESTINATION ${SEVENZIP_CMAKE_DIR}
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
    )
    
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfigVersion.cmake"
        DESTINATION ${SEVENZIP_CMAKE_DIR}
    )
endif()

# ============================================
# 构建总结
# ============================================
message(STATUS "========================================")
message(STATUS "  Build Configuration Summary")
message(STATUS "========================================")
message(STATUS "Architecture: ${SEVENZIP_ARCH}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "FFI Library: ${SEVENZIP_BUILD_FFI}")
message(STATUS "C++ API: ${SEVENZIP_BUILD_CPP_API}")
message(STATUS "Install: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")
