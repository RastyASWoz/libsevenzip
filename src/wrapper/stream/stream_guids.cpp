// stream_guids.cpp - 7-Zip COM 接口 GUID 定义
// 通过定义 INITGUID 并包含 IStream.h 来定义所有需要的 IID

#define INITGUID  // 使 Z7_DEFINE_GUID 定义符号而非声明
#include "7zip/IStream.h"

// 此文件的唯一目的是提供 IID_IInStream, IID_IOutStream, IID_IStreamGetSize 的定义
// 7-Zip 的头文件 IStream.h 通过 Z7_DEFINE_GUID 宏自动生成这些定义
