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

# Install examples
install(TARGETS convenience_demo
    RUNTIME DESTINATION bin/examples
)
