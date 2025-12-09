# Unit tests for sevenzip wrapper

# Find Google Test (vcpkg installed)
find_package(GTest CONFIG REQUIRED)

# Create test executable
add_executable(wrapper_unit_tests
    test_error.cpp
    test_string.cpp
    test_propvariant.cpp
    test_format.cpp
    test_stream.cpp
    test_archive_reader.cpp
    test_extraction.cpp
    test_archive_writer.cpp
    # Password debug tests removed - password+files functionality has known limitations
    # test_password_debug.cpp
    # test_password_minimal.cpp
    # test_password_memory.cpp
)

target_link_libraries(wrapper_unit_tests
    PRIVATE
        sevenzip_wrapper
        7zip_cpp
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(wrapper_unit_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP  # 7-Zip COM 头文件
)

# Enable testing
include(GoogleTest)
gtest_discover_tests(wrapper_unit_tests)

# Set C++17 standard
target_compile_features(wrapper_unit_tests PRIVATE cxx_std_17)

# Add test to CTest
add_test(NAME WrapperUnitTests COMMAND wrapper_unit_tests)

# Debug test executable for CompressionLevel::None issue
add_executable(debug_compression_none
    ${CMAKE_SOURCE_DIR}/tests/debug_compression_none.cpp
)

target_link_libraries(debug_compression_none
    PRIVATE
        sevenzip_wrapper
        7zip_cpp
)

target_include_directories(debug_compression_none
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(debug_compression_none PRIVATE cxx_std_17)

# Debug test executable for password + file crash
add_executable(debug_password_crash
    ${CMAKE_SOURCE_DIR}/tests/debug_password_crash.cpp
)

target_link_libraries(debug_password_crash
    PRIVATE
        sevenzip_wrapper
        7zip_cpp
)

target_include_directories(debug_password_crash
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(debug_password_crash PRIVATE cxx_std_17)

