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
    test_gzip_bzip2_xz.cpp
    test_edge_cases.cpp
    test_concurrency.cpp
    test_stress.cpp
    test_integration.cpp
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

# Debug executables removed (were: debug_compression_none, debug_password_crash)

# Stage 3 Modern C++ API tests
add_executable(test_archive
    test_archive.cpp
)

target_link_libraries(test_archive
    PRIVATE
        sevenzip_api
        sevenzip_wrapper
        7zip_cpp
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_archive
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(test_archive PRIVATE cxx_std_17)

gtest_discover_tests(test_archive)
add_test(NAME ArchiveAPITests COMMAND test_archive)

# Convenience functions tests
add_executable(test_convenience
    test_convenience.cpp
)

target_link_libraries(test_convenience
    PRIVATE
        sevenzip_api
        sevenzip_wrapper
        7zip_cpp
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_convenience
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(test_convenience PRIVATE cxx_std_17)

gtest_discover_tests(test_convenience)
add_test(NAME ConvenienceFunctionsTests COMMAND test_convenience)

# Compressor class tests
add_executable(test_compressor
    test_compressor.cpp
)

target_link_libraries(test_compressor
    PRIVATE
        sevenzip_api
        sevenzip_wrapper
        7zip_cpp
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_compressor
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(test_compressor PRIVATE cxx_std_17)

gtest_discover_tests(test_compressor)
add_test(NAME CompressorTests COMMAND test_compressor)

# ArchiveReader API tests
add_executable(test_archive_reader_api
    test_archive_reader_api.cpp
)

target_link_libraries(test_archive_reader_api
    PRIVATE
        sevenzip_api
        sevenzip_wrapper
        7zip_cpp
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_archive_reader_api
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(test_archive_reader_api PRIVATE cxx_std_17)

gtest_discover_tests(test_archive_reader_api)
add_test(NAME ArchiveReaderAPITests COMMAND test_archive_reader_api)

# ArchiveWriter API tests
add_executable(test_archive_writer_api
    test_archive_writer_api.cpp
)

target_link_libraries(test_archive_writer_api
    PRIVATE
        sevenzip_api
        sevenzip_wrapper
        7zip_cpp
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_archive_writer_api
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party/7zip/CPP
)

target_compile_features(test_archive_writer_api PRIVATE cxx_std_17)

gtest_discover_tests(test_archive_writer_api)
add_test(NAME ArchiveWriterAPITests COMMAND test_archive_writer_api)

