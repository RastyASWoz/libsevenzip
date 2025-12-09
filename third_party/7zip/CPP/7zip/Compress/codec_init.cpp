// codec_init.cpp - Initialize COM GUIDs for codec interfaces
// 注意: IStream相关的GUID已经在wrapper/stream/stream_guids.cpp中定义了
// 这里只定义ICoder特有的接口GUID

#include "../../Common/Common0.h"
#include "../../Common/MyUnknown.h"

// 手动定义ICoder相关的GUID，避免与IStream.h重复定义
#include <guiddef.h>

// 注意：Z7_IFACE_CONSTR_CODER宏在ICoder.h中使用了groupId=4
// GUID格式: {23170F69-40C1-278A-0000-0004-00XX-0000}

// 这些GUID实际上会在codecs使用时由头文件中的Z7_DEFINE_GUID声明
// 我们这里提供一个extern声明，实际定义由链接器从其他编译单元找到
// 实际上不需要这个文件 - 删除它可能更好，因为GUID会在头文件包含时自动处理
