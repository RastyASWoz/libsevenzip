# libsevenzip 源码

add_subdirectory(wrapper)
add_subdirectory(api)
add_subdirectory(ffi)

# 主库（合并所有组件）
add_library(sevenzip
    $<TARGET_OBJECTS:sevenzip_wrapper>
    $<TARGET_OBJECTS:sevenzip_api>
)

target_link_libraries(sevenzip
    PUBLIC
        SevenZip::CPP
        SevenZip::C
)

target_include_directories(sevenzip
    PUBLIC
        $<BUILD_INTERFACE:${SEVENZIP_ROOT}/include>
        $<INSTALL_INTERFACE:include>
)

# 共享库版本
if(SEVENZIP_BUILD_SHARED)
    add_library(sevenzip_shared SHARED
        $<TARGET_OBJECTS:sevenzip_wrapper>
        $<TARGET_OBJECTS:sevenzip_api>
        $<TARGET_OBJECTS:sevenzip_ffi>
    )
    
    target_link_libraries(sevenzip_shared
        PRIVATE
            SevenZip::CPP
            SevenZip::C
            7zip_cpp
            7zip_archive
            7zip_codec
            7zip_crypto
    )
    
    set_target_properties(sevenzip_shared PROPERTIES
        OUTPUT_NAME sevenzip
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()
