#include "wrapper_propvariant.hpp"

#include "wrapper_error.hpp"
#include "wrapper_string.hpp"

#ifdef _WIN32
// 必须先包含 objbase.h 才能使用 COM 类型
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>  // IID, REFIID, GUID 基础定义
#include <oleauto.h>  // PROPVARIANT, BSTR
#include <propidl.h>
#else
// 非 Windows 平台需要定义 PROPVARIANT 结构
// 这里提供简化的兼容定义
typedef unsigned short VARTYPE;

#define VT_EMPTY 0
#define VT_I1 16
#define VT_I2 2
#define VT_I4 3
#define VT_I8 20
#define VT_UI1 17
#define VT_UI2 18
#define VT_UI4 19
#define VT_UI8 21
#define VT_BOOL 11
#define VT_BSTR 8
#define VT_LPWSTR 31
#define VT_FILETIME 64

#define VARIANT_TRUE ((short)-1)
#define VARIANT_FALSE ((short)0)

struct PROPVARIANT {
    VARTYPE vt;
    unsigned short wReserved1;
    unsigned short wReserved2;
    unsigned short wReserved3;
    union {
        char cVal;
        unsigned char bVal;
        short iVal;
        unsigned short uiVal;
        long lVal;
        unsigned long ulVal;
        int64_t hVal;
        uint64_t uhVal;
        short boolVal;
        BSTR bstrVal;
        wchar_t* pwszVal;
        FILETIME filetime;
    };
};

inline void PropVariantInit(PROPVARIANT* pvar) {
    memset(pvar, 0, sizeof(PROPVARIANT));
}

inline void PropVariantClear(PROPVARIANT* pvar) {
    if (pvar->vt == VT_BSTR && pvar->bstrVal) {
        free(pvar->bstrVal);
    }
    PropVariantInit(pvar);
}
#endif

namespace sevenzip {
namespace detail {

// PropVariantGuard 实现
PropVariantGuard::PropVariantGuard() {
    prop_ = new PROPVARIANT();
    PropVariantInit(prop_);
}

PropVariantGuard::~PropVariantGuard() {
    if (prop_) {
        PropVariantClear(prop_);
        delete prop_;
    }
}

PROPVARIANT* PropVariantGuard::get() {
    return prop_;
}

const PROPVARIANT* PropVariantGuard::get() const {
    return prop_;
}

void PropVariantGuard::clear() {
    if (prop_) {
        PropVariantClear(prop_);
        PropVariantInit(prop_);
    }
}

// PROPVARIANT 转换实现
std::optional<std::wstring> prop_to_wstring(const PROPVARIANT& prop) {
    switch (prop.vt) {
        case VT_BSTR:
            if (prop.bstrVal) {
                return std::wstring(prop.bstrVal);
            }
            break;
        case VT_LPWSTR:
            if (prop.pwszVal) {
                return std::wstring(prop.pwszVal);
            }
            break;
        case VT_EMPTY:
            return std::nullopt;
        default:
            break;
    }
    return std::nullopt;
}

std::optional<uint64_t> prop_to_uint64(const PROPVARIANT& prop) {
    switch (prop.vt) {
        case VT_UI8:
#ifdef _WIN32
            return prop.uhVal.QuadPart;
#else
            return prop.uhVal;
#endif
        case VT_UI4:
            return prop.ulVal;
        case VT_UI2:
            return prop.uiVal;
        case VT_UI1:
            return prop.bVal;
        case VT_I8:
#ifdef _WIN32
            return static_cast<uint64_t>(prop.hVal.QuadPart);
#else
            return static_cast<uint64_t>(prop.hVal);
#endif
        case VT_I4:
            return static_cast<uint64_t>(prop.lVal);
        case VT_I2:
            return static_cast<uint64_t>(prop.iVal);
        case VT_EMPTY:
            return std::nullopt;
        default:
            break;
    }
    return std::nullopt;
}

std::optional<int64_t> prop_to_int64(const PROPVARIANT& prop) {
    switch (prop.vt) {
        case VT_I8:
#ifdef _WIN32
            return prop.hVal.QuadPart;
#else
            return prop.hVal;
#endif
        case VT_I4:
            return prop.lVal;
        case VT_I2:
            return prop.iVal;
        case VT_I1:
            return prop.cVal;
        case VT_UI8:
#ifdef _WIN32
            return static_cast<int64_t>(prop.uhVal.QuadPart);
#else
            return static_cast<int64_t>(prop.uhVal);
#endif
        case VT_EMPTY:
            return std::nullopt;
        default:
            break;
    }
    return std::nullopt;
}

std::optional<uint32_t> prop_to_uint32(const PROPVARIANT& prop) {
    switch (prop.vt) {
        case VT_UI4:
            return prop.ulVal;
        case VT_UI2:
            return prop.uiVal;
        case VT_UI1:
            return prop.bVal;
        case VT_I4:
            return static_cast<uint32_t>(prop.lVal);
        case VT_I2:
            return static_cast<uint32_t>(prop.iVal);
        case VT_EMPTY:
            return std::nullopt;
        default:
            break;
    }
    return std::nullopt;
}

std::optional<bool> prop_to_bool(const PROPVARIANT& prop) {
    if (prop.vt == VT_BOOL) {
        return prop.boolVal != VARIANT_FALSE;
    }
    if (prop.vt == VT_EMPTY) {
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<FILETIME> prop_to_filetime(const PROPVARIANT& prop) {
    if (prop.vt == VT_FILETIME) {
        return prop.filetime;
    }
    return std::nullopt;
}

// 反向转换实现
void wstring_to_prop(const std::wstring& str, PROPVARIANT* prop) {
    PropVariantClear(prop);
    prop->vt = VT_BSTR;
#ifdef _WIN32
    prop->bstrVal = ::SysAllocString(str.c_str());
    if (!prop->bstrVal) {
        throw Exception(ErrorCode::OutOfMemory, "Failed to allocate BSTR");
    }
#else
    size_t len = str.length();
    prop->bstrVal = static_cast<BSTR>(malloc((len + 1) * sizeof(wchar_t)));
    if (prop->bstrVal) {
        wcscpy(prop->bstrVal, str.c_str());
    }
#endif
}

void uint64_to_prop(uint64_t value, PROPVARIANT* prop) {
    PropVariantClear(prop);
    prop->vt = VT_UI8;
#ifdef _WIN32
    prop->uhVal.QuadPart = value;
#else
    prop->uhVal = value;
#endif
}

void uint32_to_prop(uint32_t value, PROPVARIANT* prop) {
    PropVariantClear(prop);
    prop->vt = VT_UI4;
    prop->ulVal = value;
}

void bool_to_prop(bool value, PROPVARIANT* prop) {
    PropVariantClear(prop);
    prop->vt = VT_BOOL;
    prop->boolVal = value ? VARIANT_TRUE : VARIANT_FALSE;
}

void filetime_to_prop(const FILETIME& time, PROPVARIANT* prop) {
    PropVariantClear(prop);
    prop->vt = VT_FILETIME;
    prop->filetime = time;
}

// FILETIME 与 C++ chrono 转换
std::chrono::system_clock::time_point filetime_to_timepoint(const FILETIME& ft) {
    // FILETIME 是从 1601年1月1日 开始的 100纳秒间隔数
    // 转换为 Unix epoch (1970年1月1日)

    uint64_t ticks = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

    // 1601年1月1日 到 1970年1月1日 的 100纳秒间隔数
    constexpr uint64_t epoch_diff = 116444736000000000ULL;

    if (ticks < epoch_diff) {
        return std::chrono::system_clock::time_point();
    }

    ticks -= epoch_diff;

    // 转换为微秒
    uint64_t microseconds = ticks / 10;

    return std::chrono::system_clock::time_point(std::chrono::microseconds(microseconds));
}

FILETIME timepoint_to_filetime(const std::chrono::system_clock::time_point& tp) {
    auto microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();

    // 转换为 100纳秒间隔
    uint64_t ticks = static_cast<uint64_t>(microseconds) * 10;

    // 加上 1601年1月1日 到 1970年1月1日 的偏移
    constexpr uint64_t epoch_diff = 116444736000000000ULL;
    ticks += epoch_diff;

    FILETIME ft;
    ft.dwLowDateTime = static_cast<uint32_t>(ticks & 0xFFFFFFFF);
    ft.dwHighDateTime = static_cast<uint32_t>(ticks >> 32);

    return ft;
}

}  // namespace detail
}  // namespace sevenzip
