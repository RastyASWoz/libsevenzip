add_library(sevenzip_api OBJECT
    placeholder.cpp
    modern/format.cpp
    modern/compression.cpp
)

target_include_directories(sevenzip_api
    PUBLIC
        ${SEVENZIP_ROOT}/include
    PRIVATE
        ${SEVENZIP_ROOT}/src
)
