add_library(sevenzip_api OBJECT
    placeholder.cpp
)

target_include_directories(sevenzip_api
    PRIVATE
        ${SEVENZIP_ROOT}/include
)
