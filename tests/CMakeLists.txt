# Tests for libsevenzip

# Unit tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/CMakeLists.txt)
    add_subdirectory(unit)
endif()

# Integration tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/CMakeLists.txt)
    add_subdirectory(integration)
endif()
