# Tests for libsevenzip

# Enable testing
enable_testing()

# FFI C API tests
add_executable(test_archive_c ffi/test_archive_c.c)
target_link_libraries(test_archive_c PRIVATE sevenzip_ffi)
target_include_directories(test_archive_c PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_archive_c PRIVATE SEVENZIP_STATIC)
add_test(NAME test_archive_c COMMAND test_archive_c)

# FFI compression demo
add_executable(test_compress_demo ffi/test_compress_demo.c)
target_link_libraries(test_compress_demo PRIVATE sevenzip_ffi)
target_include_directories(test_compress_demo PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_compress_demo PRIVATE SEVENZIP_STATIC)

# FFI file compression test
add_executable(test_file_compress ffi/test_file_compress.c)
target_link_libraries(test_file_compress PRIVATE sevenzip_ffi)
target_include_directories(test_file_compress PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_file_compress PRIVATE SEVENZIP_STATIC)
add_test(NAME test_file_compress 
         COMMAND test_file_compress 
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# FFI archive extraction test
add_executable(test_archive_extract ffi/test_archive_extract.c)
target_link_libraries(test_archive_extract PRIVATE sevenzip_ffi)
target_include_directories(test_archive_extract PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_archive_extract PRIVATE SEVENZIP_STATIC)
add_test(NAME test_archive_extract 
         COMMAND test_archive_extract 
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# FFI archive writer test
add_executable(test_archive_writer ffi/test_archive_writer.c)
target_link_libraries(test_archive_writer PRIVATE sevenzip_ffi)
target_include_directories(test_archive_writer PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
)
target_compile_definitions(test_archive_writer PRIVATE SEVENZIP_STATIC)
add_test(NAME test_archive_writer 
         COMMAND test_archive_writer 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# ===== Advanced FFI Tests =====

# Advanced archive reader tests
add_executable(test_archive_advanced ffi/test_archive_advanced.c)
target_link_libraries(test_archive_advanced PRIVATE sevenzip_ffi)
target_include_directories(test_archive_advanced PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_archive_advanced PRIVATE SEVENZIP_STATIC)
add_test(NAME test_archive_advanced 
         COMMAND test_archive_advanced 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Advanced writer tests
add_executable(test_writer_advanced ffi/test_writer_advanced.c)
target_link_libraries(test_writer_advanced PRIVATE sevenzip_ffi)
target_include_directories(test_writer_advanced PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_writer_advanced PRIVATE SEVENZIP_STATIC)
add_test(NAME test_writer_advanced 
         COMMAND test_writer_advanced 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Advanced compression tests
add_executable(test_compress_advanced ffi/test_compress_advanced.c)
target_link_libraries(test_compress_advanced PRIVATE sevenzip_ffi)
target_include_directories(test_compress_advanced PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_compress_advanced PRIVATE SEVENZIP_STATIC)
add_test(NAME test_compress_advanced 
         COMMAND test_compress_advanced 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Error handling integration tests
add_executable(test_error_handling ffi/test_error_handling.c)
target_link_libraries(test_error_handling PRIVATE sevenzip_ffi)
target_include_directories(test_error_handling PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_definitions(test_error_handling PRIVATE SEVENZIP_STATIC)
add_test(NAME test_error_handling 
         COMMAND test_error_handling 
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Unit tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/CMakeLists.txt)
    add_subdirectory(unit)
endif()

# Integration tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/CMakeLists.txt)
    add_subdirectory(integration)
endif()
