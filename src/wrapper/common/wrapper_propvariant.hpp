#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

// 前向声明 PROPVARIANT 和 FILETIME
#ifdef _WIN32
struct tagPROPVARIANT;
typedef struct tagPROPVARIANT PROPVARIANT;

struct _FILETIME;
typedef struct _FILETIME FILETIME;
#else
// 非 Windows 平台的兼容定义
struct PROPVARIANT;
struct FILETIME {
    uint32_t dwLowDateTime;
    uint32_t dwHighDateTime;
};
#endif

namespace sevenzip {
namespace detail {

/**
 * @brief PROPVARIANT RAII 包装
 * 自动管理 PROPVARIANT 的初始化和清理
 */
class PropVariantGuard {
   public:
    PropVariantGuard();
    ~PropVariantGuard();

    // 禁止拷贝
    PropVariantGuard(const PropVariantGuard&) = delete;
    PropVariantGuard& operator=(const PropVariantGuard&) = delete;

    // 获取指针
    PROPVARIANT* get();
    const PROPVARIANT* get() const;

    PROPVARIANT* operator&() { return get(); }
    operator PROPVARIANT*() { return get(); }

    // 重置
    void clear();

   private:
    PROPVARIANT* prop_;
};

/**
 * @brief PROPVARIANT 转换函数
 */

// 转换为 wstring
std::optional<std::wstring> prop_to_wstring(const PROPVARIANT& prop);

// 转换为 uint64_t
std::optional<uint64_t> prop_to_uint64(const PROPVARIANT& prop);

// 转换为 int64_t
std::optional<int64_t> prop_to_int64(const PROPVARIANT& prop);

// 转换为 uint32_t
std::optional<uint32_t> prop_to_uint32(const PROPVARIANT& prop);

// 转换为 bool
std::optional<bool> prop_to_bool(const PROPVARIANT& prop);

// 转换为 FILETIME
std::optional<FILETIME> prop_to_filetime(const PROPVARIANT& prop);

/**
 * @brief 反向转换（设置 PROPVARIANT）
 */

void wstring_to_prop(const std::wstring& str, PROPVARIANT* prop);
void uint64_to_prop(uint64_t value, PROPVARIANT* prop);
void uint32_to_prop(uint32_t value, PROPVARIANT* prop);
void bool_to_prop(bool value, PROPVARIANT* prop);
void filetime_to_prop(const FILETIME& time, PROPVARIANT* prop);

/**
 * @brief FILETIME 与 C++ chrono 转换
 */
std::chrono::system_clock::time_point filetime_to_timepoint(const FILETIME& ft);
FILETIME timepoint_to_filetime(const std::chrono::system_clock::time_point& tp);

}  // namespace detail
}  // namespace sevenzip
