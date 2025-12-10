cmake_minimum_required(VERSION 3.20)

# ============================================
# Windows SDK 版本控制 (必须在 project() 之前)
# ============================================
if(NOT WIN32)
    message(FATAL_ERROR "This project currently only supports Windows platform")
endif()

# 弹性 SDK 版本控制: 按优先级查找兼容的 Windows SDK
# 优先级从高到低,确保向后兼容性
set(PREFERRED_SDK_VERSIONS 
    "10.0.19041.0"  # Windows 10 2004 (已验证兼容 7-Zip COM)
    "10.0.22000.0"  # Windows 11 21H2 (向后兼容)
    "10.0.22621.0"  # Windows 11 22H2 (向后兼容)
    "10.0.26100.0"  # Windows 11 24H2 (可能有兼容性问题,最后尝试)
)

# 检测 Windows Kits 安装路径
if(DEFINED ENV{WindowsSdkDir})
    set(SDK_ROOT "$ENV{WindowsSdkDir}")
else()
    # 默认安装路径 (处理 Program Files (x86) 的括号)
    set(SDK_ROOT "C:/Program Files (x86)/Windows Kits/10")
    if(NOT EXISTS "${SDK_ROOT}")
        set(SDK_ROOT "D:/Windows Kits/10")  # 备用路径
    endif()
endif()

set(SDK_FOUND FALSE)
foreach(SDK_VERSION IN LISTS PREFERRED_SDK_VERSIONS)
    set(SDK_INCLUDE_PATH "${SDK_ROOT}/Include/${SDK_VERSION}")
    if(EXISTS "${SDK_INCLUDE_PATH}/um/windows.h")
        set(CMAKE_SYSTEM_VERSION "${SDK_VERSION}" CACHE STRING 
            "Windows SDK version for 7-Zip COM compatibility" FORCE)
        set(SDK_FOUND TRUE)
        message(STATUS "Found compatible Windows SDK: ${CMAKE_SYSTEM_VERSION}")
        message(STATUS "  Location: ${SDK_INCLUDE_PATH}")
        break()
    endif()
endforeach()

if(NOT SDK_FOUND)
    message(WARNING 
        "None of the preferred Windows SDK versions found!\n"
        "  Preferred versions: ${PREFERRED_SDK_VERSIONS}\n"
        "  SDK root: ${SDK_ROOT}\n"
        "Using default SDK. 7-Zip COM compatibility not guaranteed.\n"
        "Recommended: Install Windows SDK 10.0.19041.0 from\n"
        "  https://developer.microsoft.com/windows/downloads/sdk-archive/")
endif()

project(libsevenzip
    VERSION 0.1.0
    DESCRIPTION "Modern C++ wrapper for 7-Zip"
    LANGUAGES C CXX
)

# ============================================
# vcpkg 配置
# ============================================

if(EXISTS "D:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# ============================================
# 全局设置
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

# ============================================
# 选项
# ============================================

option(SEVENZIP_BUILD_SHARED "Build shared library" ON)
option(SEVENZIP_BUILD_STATIC "Build static library" ON)
option(SEVENZIP_BUILD_TESTS "Build tests" ON)
option(SEVENZIP_BUILD_EXAMPLES "Build examples" ON)
option(SEVENZIP_ENABLE_ASM "Enable assembly optimizations" OFF)

# ============================================
# Windows 平台设置
# ============================================

add_definitions(-D_WIN32 -DWIN32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-D_WIN64)
endif()

# ============================================
# MSVC 编译器设置
# ============================================

if(NOT MSVC)
    message(FATAL_ERROR "This project currently only supports MSVC compiler on Windows")
endif()

add_compile_options(/W4 /utf-8)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# ============================================
# 路径定义
# ============================================

set(SEVENZIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SEVENZIP_THIRD_PARTY ${SEVENZIP_ROOT}/third_party)
set(SEVENZIP_7ZIP_ROOT ${SEVENZIP_THIRD_PARTY}/7zip)

# ============================================
# 子目录
# ============================================

# 7-Zip 原始代码
if(EXISTS ${SEVENZIP_THIRD_PARTY}/7zip/CMakeLists.txt)
    add_subdirectory(third_party/7zip)
endif()

# 包装库
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
    add_subdirectory(src)
endif()

# 测试
if(SEVENZIP_BUILD_TESTS)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
        add_subdirectory(tests)
    endif()
endif()

# 示例
if(SEVENZIP_BUILD_EXAMPLES)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
        add_subdirectory(examples)
    endif()
endif()

# ============================================
# 安装配置
# ============================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 安装头文件
install(DIRECTORY include/sevenzip
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 安装库文件
install(TARGETS sevenzip
    EXPORT SevenZipTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(SEVENZIP_BUILD_SHARED)
    install(TARGETS sevenzip_shared sevenzip_ffi
        EXPORT SevenZipTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# 生成并安装 CMake 配置文件
set(SEVENZIP_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/SevenZip)

# 生成版本文件
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 导出目标
install(EXPORT SevenZipTargets
    FILE SevenZipTargets.cmake
    NAMESPACE SevenZip::
    DESTINATION ${SEVENZIP_CMAKE_DIR}
)

# 创建配置文件
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SevenZipConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfig.cmake"
    INSTALL_DESTINATION ${SEVENZIP_CMAKE_DIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SevenZipConfigVersion.cmake"
    DESTINATION ${SEVENZIP_CMAKE_DIR}
)

# 生成 pkg-config 文件 (Linux/macOS)
if(UNIX)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sevenzip.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/sevenzip.pc"
        @ONLY
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/sevenzip.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()
