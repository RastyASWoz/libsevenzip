// archive_stubs.cpp - Stub functions for optional archive handlers

#include "../Common/RegisterArc.h"
#include "StdAfx.h"

// Stub for ExtHandler检测函数
// 当我们不包含完整的ExtHandler时，提供一个简单的stub
namespace NArchive {
namespace NExt {

API_FUNC_IsArc IsArc_Ext(const Byte* p, size_t size) {
    (void)p;
    (void)size;
    // 简单的stub - 永远返回NO
    // 这样HandlerCont可以链接，但不会实际检测Ext文件系统
    return k_IsArc_Res_NO;
}

}  // namespace NExt
}  // namespace NArchive

// Stub for LzhCrc16Update (被ExtHandler使用)
UInt32 LzhCrc16Update(UInt32 crc, const void* data, size_t size) {
    (void)crc;
    (void)data;
    (void)size;
    return 0;  // Stub实现
}
