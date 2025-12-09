# 7-Zip Archive Handlers Library

# 收集所有格式注册代码和处理器
set(SEVENZIP_ARCHIVE_SOURCES
    # Archive exports (contains CreateArchiver)
    ${SEVENZIP_CPP_DIR}/7zip/Archive/ArchiveExports.cpp
    # 注意: DllExports.cpp 和 DllExports2.cpp 是用于 DLL 导出的，不包含在静态库中
    # 注意: IArchive的GUID在IArchive.h中声明，因为IArchive.h包含了IStream.h，
    #      而IStream的GUID已在wrapper/stream/stream_guids.cpp中定义，所以不需要单独的archive_guids.cpp
    
    # Handler container
    ${SEVENZIP_CPP_DIR}/7zip/Archive/HandlerCont.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/archive_stubs.cpp
    
    # Common archive code
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/CoderMixer2.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/ItemNameUtils.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/OutStreamWithCRC.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/InStreamWithCRC.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/HandlerOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/ParseProperties.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/FindSignature.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/MultiStream.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/OutMemStream.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Common/DummyOutStream.cpp
    
    # 7z format
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zHandler.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zHandlerOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zIn.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zUpdate.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zExtract.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zProperties.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zSpecStream.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zFolderInStream.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zFolderOutStream.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zEncode.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zDecode.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/7z/7zHeader.cpp
    
    # Zip format
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipHandler.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipHandlerOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipIn.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipUpdate.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipAddCommon.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Zip/ZipItem.cpp
    
    # Tar format
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarHandler.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarHandlerOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarIn.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarOut.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarUpdate.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarRegister.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarItem.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Tar/TarHeader.cpp
    
    # GZip format
    ${SEVENZIP_CPP_DIR}/7zip/Archive/GzHandler.cpp
    
    # BZip2 format  
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Bz2Handler.cpp
    
    # XZ format
    ${SEVENZIP_CPP_DIR}/7zip/Archive/XzHandler.cpp
    
    # Rar format (read-only)
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Rar/RarHandler.cpp
    ${SEVENZIP_CPP_DIR}/7zip/Archive/Rar/Rar5Handler.cpp
)

# 过滤掉不存在的文件
set(SEVENZIP_ARCHIVE_SOURCES_EXIST)
foreach(src ${SEVENZIP_ARCHIVE_SOURCES})
    if(EXISTS ${src})
        list(APPEND SEVENZIP_ARCHIVE_SOURCES_EXIST ${src})
    else()
        message(VERBOSE "Archive: missing optional source: ${src}")
    endif()
endforeach()

if(SEVENZIP_ARCHIVE_SOURCES_EXIST)
    add_library(7zip_archive STATIC ${SEVENZIP_ARCHIVE_SOURCES_EXIST})
    
    target_include_directories(7zip_archive
        PUBLIC
            ${SEVENZIP_CPP_DIR}
            ${SEVENZIP_CPP_DIR}/7zip/Archive
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../C
    )
    
    # 链接依赖
    target_link_libraries(7zip_archive
        PUBLIC
            7zip_cpp
            7zip_codec
            7zip_crypto
        PRIVATE
            7zip_c
    )
    
    # 平台特定设置
    if(WIN32)
        target_compile_definitions(7zip_archive PRIVATE
            _WIN32
            WIN32
            _WIN32_WINNT=0x0601
            NOMINMAX
        )
    endif()
    
    if(MSVC)
        target_compile_options(7zip_archive PRIVATE
            /wd4100    # 未引用的形参
            /wd4244    # 类型转换
            /wd4267    # size_t到int
            /FS        # 同步PDB写入
        )
    endif()
    
    add_library(SevenZip::Archive ALIAS 7zip_archive)
    
    message(STATUS "7zip_archive: collected ${CMAKE_CURRENT_LIST_FILE} sources")
else()
    message(WARNING "No archive sources found; creating INTERFACE placeholder")
    add_library(7zip_archive INTERFACE)
    add_library(SevenZip::Archive ALIAS 7zip_archive)
endif()
