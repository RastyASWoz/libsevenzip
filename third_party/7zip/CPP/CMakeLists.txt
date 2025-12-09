# 7-Zip C++ Layer

set(SEVENZIP_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Helper: append file to list only if it exists (avoid hard errors when upstream source layout varies)
macro(sevenzip_add_if_exists target_list file_path)
    if(EXISTS ${file_path})
        list(APPEND ${target_list} ${file_path})
        set(${target_list} "${${target_list}}" PARENT_SCOPE)
    else()
        message(VERBOSE "CPP: missing optional source: ${file_path}")
    endif()
endmacro()

# ============================================
# Common 模块
# ============================================

set(SEVENZIP_CPP_COMMON_SOURCES)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/CRC.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/CrcReg.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/DynLimBuf.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/IntToString.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/LzFindPrepare.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/MyString.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/MyVector.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/NewHandler.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/Sha256Prepare.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/StringConvert.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/StringToInt.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/UTFConvert.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/Common/Wildcard.cpp)

# ============================================
# Windows 兼容层
# ============================================

set(SEVENZIP_CPP_WINDOWS_SOURCES)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/windows_globals.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/FileDir.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/FileFind.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/FileIO.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/FileLink.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/FileName.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/PropVariant.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/PropVariantConv.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/PropVariantUtils.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/Synchronization.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/System.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_WINDOWS_SOURCES ${SEVENZIP_CPP_DIR}/Windows/TimeUtils.cpp)

# ============================================
# 7-Zip Common
# ============================================

set(SEVENZIP_CPP_7ZIP_COMMON_SOURCES)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/CreateCoder.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/CWrappers.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/FilePathAutoRename.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/FileStreams.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/FilterCoder.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/InBuffer.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/InOutTempBuffer.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/LimitedStreams.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/MemBlocks.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/MethodId.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/MethodProps.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/OffsetStream.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/OutBuffer.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/OutMemStream.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/ProgressMt.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/ProgressUtils.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/PropId.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/StreamBinder.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/StreamObjects.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/StreamUtils.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/UniqBlocks.cpp)
sevenzip_add_if_exists(SEVENZIP_CPP_7ZIP_COMMON_SOURCES ${SEVENZIP_CPP_DIR}/7zip/Common/VirtThread.cpp)

# ============================================
# 合并并创建库
# ============================================

set(SEVENZIP_CPP_ALL_SOURCES
    ${SEVENZIP_CPP_COMMON_SOURCES}
    ${SEVENZIP_CPP_WINDOWS_SOURCES}
    ${SEVENZIP_CPP_7ZIP_COMMON_SOURCES}
)

if(SEVENZIP_CPP_ALL_SOURCES)
    add_library(7zip_cpp STATIC ${SEVENZIP_CPP_ALL_SOURCES})

    target_include_directories(7zip_cpp
        PUBLIC
            ${SEVENZIP_CPP_DIR}
            ${SEVENZIP_CPP_DIR}/Common
            ${SEVENZIP_CPP_DIR}/Windows
            ${SEVENZIP_CPP_DIR}/7zip
            ${CMAKE_CURRENT_SOURCE_DIR}/../C
    )

    # 链接 C 层（如果存在）
    if(TARGET SevenZip::C)
        target_link_libraries(7zip_cpp PRIVATE SevenZip::C)
    endif()

    # 平台特定依赖
    if(WIN32)
        target_compile_definitions(7zip_cpp PRIVATE 
            _WIN32 
            WIN32
            _WIN32_WINNT=0x0601  # Windows 7+
            NOMINMAX             # 防止Windows.h定义min/max宏
            # 注意: 不定义WIN32_LEAN_AND_MEAN,因为7-Zip需要完整的COM支持(IUnknown等)
        )
        target_link_libraries(7zip_cpp PRIVATE oleaut32 ole32 uuid)
    else()
        target_compile_definitions(7zip_cpp PRIVATE _UNIX)
    endif()

    if(MSVC)
        target_compile_options(7zip_cpp PRIVATE 
            /wd4100    # 未引用的形参
            /wd4244    # 类型转换,可能丢失数据
            /wd4267    # size_t到int的转换
            /FS        # 强制同步PDB写入
        )
    else()
        target_compile_options(7zip_cpp PRIVATE -Wno-unused-parameter -Wno-sign-compare)
    endif()

    add_library(SevenZip::CPP ALIAS 7zip_cpp)
else()
    message(WARNING "No C++ sources collected for 7zip_cpp; creating INTERFACE placeholder.")
    add_library(7zip_cpp INTERFACE)
    target_include_directories(7zip_cpp INTERFACE ${SEVENZIP_CPP_DIR})
    add_library(SevenZip::CPP ALIAS 7zip_cpp)
endif()

# ============================================
# Archive handlers (格式处理器)
# ============================================
add_subdirectory(7zip/Archive)

# ============================================
# Compression codecs (压缩编解码器)
# ============================================
add_subdirectory(7zip/Compress)

# ============================================
# Cryptography codecs (加密编解码器)
# ============================================
add_subdirectory(7zip/Crypto)

