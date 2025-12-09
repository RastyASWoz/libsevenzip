# Stream wrapper module

add_library(sevenzip_wrapper_stream OBJECT
    stream_file.cpp
)

target_include_directories(sevenzip_wrapper_stream
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/7zip
)

target_link_libraries(sevenzip_wrapper_stream
    PRIVATE
        sevenzip
)

# 设置编译选项
if(MSVC)
    target_compile_options(sevenzip_wrapper_stream PRIVATE /W4)
else()
    target_compile_options(sevenzip_wrapper_stream PRIVATE -Wall -Wextra)
endif()
