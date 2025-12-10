# 7-Zip C Layer

set(SEVENZIP_C_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================
# 核心算法库
# ============================================

set(SEVENZIP_C_CORE_SOURCES
    # 基础
    ${SEVENZIP_C_DIR}/7zCrc.c
    ${SEVENZIP_C_DIR}/7zCrcOpt.c
    ${SEVENZIP_C_DIR}/CpuArch.c
    ${SEVENZIP_C_DIR}/Sort.c
    
    # 内存管理
    ${SEVENZIP_C_DIR}/Alloc.c
    ${SEVENZIP_C_DIR}/7zAlloc.c
    
    # 流
    ${SEVENZIP_C_DIR}/7zStream.c
    ${SEVENZIP_C_DIR}/7zFile.c
    ${SEVENZIP_C_DIR}/7zBuf.c
    ${SEVENZIP_C_DIR}/7zBuf2.c
)

# ============================================
# LZMA
# ============================================

set(SEVENZIP_C_LZMA_SOURCES
    ${SEVENZIP_C_DIR}/LzFind.c
    ${SEVENZIP_C_DIR}/LzFindMt.c
    ${SEVENZIP_C_DIR}/LzFindOpt.c
    ${SEVENZIP_C_DIR}/LzmaDec.c
    ${SEVENZIP_C_DIR}/LzmaEnc.c
    ${SEVENZIP_C_DIR}/Lzma2Dec.c
    ${SEVENZIP_C_DIR}/Lzma2DecMt.c
    ${SEVENZIP_C_DIR}/Lzma2Enc.c
    ${SEVENZIP_C_DIR}/Lzma86Dec.c
    ${SEVENZIP_C_DIR}/Lzma86Enc.c
    ${SEVENZIP_C_DIR}/LzmaLib.c
)

# ============================================
# 其他压缩算法
# ============================================

set(SEVENZIP_C_COMPRESS_SOURCES
    # BCJ 过滤器
    ${SEVENZIP_C_DIR}/Bra.c
    ${SEVENZIP_C_DIR}/Bra86.c
    ${SEVENZIP_C_DIR}/BraIA64.c
    ${SEVENZIP_C_DIR}/Bcj2.c
    ${SEVENZIP_C_DIR}/Bcj2Enc.c
    
    # Delta
    ${SEVENZIP_C_DIR}/Delta.c
    
    # PPMd
    ${SEVENZIP_C_DIR}/Ppmd7.c
    ${SEVENZIP_C_DIR}/Ppmd7aDec.c
    ${SEVENZIP_C_DIR}/Ppmd7Dec.c
    ${SEVENZIP_C_DIR}/Ppmd7Enc.c
    ${SEVENZIP_C_DIR}/Ppmd8.c
    ${SEVENZIP_C_DIR}/Ppmd8Dec.c
    ${SEVENZIP_C_DIR}/Ppmd8Enc.c
    
    # BWT
    ${SEVENZIP_C_DIR}/BwtSort.c
    ${SEVENZIP_C_DIR}/HuffEnc.c
    
    # Zstd
    ${SEVENZIP_C_DIR}/ZstdDec.c
)

# ============================================
# 加密
# ============================================

set(SEVENZIP_C_CRYPTO_SOURCES
    ${SEVENZIP_C_DIR}/Aes.c
    ${SEVENZIP_C_DIR}/AesOpt.c
    ${SEVENZIP_C_DIR}/Sha1.c
    ${SEVENZIP_C_DIR}/Sha1Opt.c
    ${SEVENZIP_C_DIR}/Sha256.c
    ${SEVENZIP_C_DIR}/Sha256Opt.c
    ${SEVENZIP_C_DIR}/Sha512.c
    ${SEVENZIP_C_DIR}/Sha512Opt.c
    ${SEVENZIP_C_DIR}/Sha3.c
    ${SEVENZIP_C_DIR}/Md5.c
    ${SEVENZIP_C_DIR}/Blake2s.c
)

# ============================================
# 哈希
# ============================================

set(SEVENZIP_C_HASH_SOURCES
    ${SEVENZIP_C_DIR}/Xxh64.c
    ${SEVENZIP_C_DIR}/XzCrc64.c
    ${SEVENZIP_C_DIR}/XzCrc64Opt.c
)

# ============================================
# 7z 格式
# ============================================

set(SEVENZIP_C_7Z_SOURCES
    ${SEVENZIP_C_DIR}/7zArcIn.c
    ${SEVENZIP_C_DIR}/7zDec.c
)

# ============================================
# XZ 格式
# ============================================

set(SEVENZIP_C_XZ_SOURCES
    ${SEVENZIP_C_DIR}/Xz.c
    ${SEVENZIP_C_DIR}/XzDec.c
    ${SEVENZIP_C_DIR}/XzEnc.c
    ${SEVENZIP_C_DIR}/XzIn.c
)

# ============================================
# 多线程
# ============================================

set(SEVENZIP_C_MT_SOURCES
    ${SEVENZIP_C_DIR}/Threads.c
    ${SEVENZIP_C_DIR}/MtCoder.c
    ${SEVENZIP_C_DIR}/MtDec.c
)

# ============================================
# 字节交换
# ============================================

set(SEVENZIP_C_MISC_SOURCES
    ${SEVENZIP_C_DIR}/SwapBytes.c
)

# ============================================
# 合并所有源文件
# ============================================

set(SEVENZIP_C_ALL_SOURCES
    ${SEVENZIP_C_CORE_SOURCES}
    ${SEVENZIP_C_LZMA_SOURCES}
    ${SEVENZIP_C_COMPRESS_SOURCES}
    ${SEVENZIP_C_CRYPTO_SOURCES}
    ${SEVENZIP_C_HASH_SOURCES}
    ${SEVENZIP_C_7Z_SOURCES}
    ${SEVENZIP_C_XZ_SOURCES}
    ${SEVENZIP_C_MT_SOURCES}
    ${SEVENZIP_C_MISC_SOURCES}
)

# ============================================
# 创建静态库
# ============================================

add_library(7zip_c STATIC ${SEVENZIP_C_ALL_SOURCES})

target_include_directories(7zip_c
    PUBLIC
        ${SEVENZIP_C_DIR}
)

# 平台特定配置
if(WIN32)
    target_compile_definitions(7zip_c PRIVATE _WIN32)
else()
    target_compile_definitions(7zip_c PRIVATE _UNIX)
    target_link_libraries(7zip_c PRIVATE pthread)
endif()

# 禁用某些警告
if(MSVC)
    target_compile_options(7zip_c PRIVATE /wd4100 /wd4244 /wd4267)
    
    # 修复 x86 调用约定: 32位Windows需要使用 __stdcall
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_compile_definitions(7zip_c PUBLIC _7ZIP_ST)
    endif()
else()
    target_compile_options(7zip_c PRIVATE 
        -Wno-unused-parameter 
        -Wno-sign-compare
    )
endif()

# 设置别名
add_library(SevenZip::C ALIAS 7zip_c)
