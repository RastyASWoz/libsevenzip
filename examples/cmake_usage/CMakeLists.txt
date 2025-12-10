cmake_minimum_required(VERSION 3.20)
project(SevenZipExample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 SevenZip 库
# 如果 SevenZip 安装在非标准位置,使用:
#   cmake -B build -DSevenZip_DIR="C:/Program Files/SevenZip/lib/cmake/SevenZip"
# 或:
#   cmake -B build -DCMAKE_PREFIX_PATH="C:/Program Files/SevenZip"
find_package(SevenZip 0.1 REQUIRED)

# 创建示例程序
add_executable(cmake_example main.cpp)

# 链接 SevenZip 库
target_link_libraries(cmake_example PRIVATE SevenZip::sevenzip)

# 打印配置信息
message(STATUS "========================================")
message(STATUS "SevenZip Example Configuration")
message(STATUS "========================================")
message(STATUS "SevenZip Version: ${SevenZip_VERSION}")
message(STATUS "Include Dirs: ${SevenZip_INCLUDE_DIRS}")
message(STATUS "Library Dirs: ${SevenZip_LIBRARY_DIRS}")
message(STATUS "========================================")
