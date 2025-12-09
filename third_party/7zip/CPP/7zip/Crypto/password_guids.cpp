// password_guids.cpp - Initialize COM GUIDs for Password/Crypto interfaces

// Define the GUIDs for password interfaces
#define INITGUID
#include <initguid.h>

// This will cause all the Z7_DEFINE_GUID macros in IPassword.h to actually define the GUIDs
#include "../IPassword.h"

// That's all we need - IPassword.h使用Z7_IFACE_CONSTR_PASSWORD宏声明了所有的密码接口GUID
// 当定义了INITGUID时，这些宏会实际定义这些GUID的全局变量
