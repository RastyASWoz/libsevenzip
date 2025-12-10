# Examples for libsevenzip

# Convenience functions demo
add_executable(convenience_demo
    convenience_demo.cpp
)

target_link_libraries(convenience_demo
    PRIVATE
        sevenzip_api
        sevenzip_wrapper
        7zip_cpp
)

target_include_directories(convenience_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(convenience_demo PRIVATE cxx_std_17)

# C API basic usage example
add_executable(example_basic_usage
    example_basic_usage.c
)

target_link_libraries(example_basic_usage
    PRIVATE
        sevenzip  # Use static library instead to avoid DLL dependency issues
)

target_include_directories(example_basic_usage
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Install examples
install(TARGETS convenience_demo example_basic_usage
    RUNTIME DESTINATION bin/examples
)
