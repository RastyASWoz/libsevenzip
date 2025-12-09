add_library(sevenzip_api OBJECT
    archive.cpp
    convenience.cpp
    compressor.cpp
    archive_reader.cpp
    archive_writer.cpp
)

target_include_directories(sevenzip_api
    PUBLIC
        ${SEVENZIP_ROOT}/include
    PRIVATE
        ${SEVENZIP_ROOT}/src
        ${SEVENZIP_ROOT}/third_party/7zip/CPP
)
