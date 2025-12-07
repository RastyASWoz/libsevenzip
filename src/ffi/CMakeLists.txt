add_library(sevenzip_ffi OBJECT
    placeholder.c
)

target_include_directories(sevenzip_ffi
    PRIVATE
        ${SEVENZIP_ROOT}/include
)
