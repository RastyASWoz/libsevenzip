cmake_minimum_required(VERSION 3.20)

# ============================================
# Windows SDK 版本控制 (必须在 project() 之前)
# ============================================
if(WIN32)
    # 强制使用 Windows SDK 10.0.19041.0 (Windows 10 SDK)
    # 原因: SDK 10.0.26100.0 (Windows 11) 与 7-Zip COM 接口不兼容
    set(CMAKE_SYSTEM_VERSION "10.0.19041.0" CACHE STRING "Windows SDK version for 7-Zip compatibility" FORCE)
    message(STATUS "Forcing Windows SDK: ${CMAKE_SYSTEM_VERSION}")
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

set(CMAKE_CXX_STANDARD 17)
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
# 平台检测
# ============================================

if(WIN32)
    add_definitions(-D_WIN32 -DWIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions(-D_WIN64)
    endif()
elseif(APPLE)
    add_definitions(-DAPPLE -D_DARWIN)
elseif(UNIX)
    add_definitions(-DUNIX -D_UNIX)
endif()

# ============================================
# 编译器设置
# ============================================

if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra)
endif()

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

install(DIRECTORY include/sevenzip
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
