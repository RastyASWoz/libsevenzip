add_library(sevenzip_wrapper OBJECT
    placeholder.cpp  # 临时占位
)

target_include_directories(sevenzip_wrapper
    PRIVATE
        ${SEVENZIP_ROOT}/include
        ${SEVENZIP_7ZIP_ROOT}/CPP
)

target_link_libraries(sevenzip_wrapper
    PRIVATE
        SevenZip::CPP
)
