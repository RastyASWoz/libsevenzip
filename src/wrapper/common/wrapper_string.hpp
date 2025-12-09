#pragma once

#include <filesystem>
#include <string>

// 前向声明 BSTR(Windows 类型)
#ifdef _WIN32
// Windows: 使用系统定义的 BSTR
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>  // COM 基础类型
#include <oleauto.h>  // BSTR 定义
#else
// 非 Windows: 自定义 BSTR 类型
typedef wchar_t* BSTR;
#endif

namespace sevenzip {
namespace detail {

/**
 * @brief UTF-8 到 UTF-16 转换
 */
std::wstring utf8_to_wstring(const std::string& str);

/**
 * @brief UTF-16 到 UTF-8 转换
 */
std::string wstring_to_utf8(const std::wstring& str);

/**
 * @brief 路径到 wstring 转换
 */
std::wstring path_to_wstring(const std::filesystem::path& path);

/**
 * @brief wstring 到路径转换
 */
std::filesystem::path wstring_to_path(const std::wstring& str);

/**
 * @brief BSTR 辅助类（RAII）
 * 自动管理 BSTR 的分配和释放
 */
class BStrGuard {
   public:
    BStrGuard();
    explicit BStrGuard(const wchar_t* str);
    explicit BStrGuard(const std::wstring& str);
    ~BStrGuard();

    // 禁止拷贝
    BStrGuard(const BStrGuard&) = delete;
    BStrGuard& operator=(const BStrGuard&) = delete;

    // 允许移动
    BStrGuard(BStrGuard&& other) noexcept;
    BStrGuard& operator=(BStrGuard&& other) noexcept;

    // 获取 BSTR
    BSTR get() const { return bstr_; }
    BSTR* operator&() { return &bstr_; }
    operator BSTR() const { return bstr_; }

    // 释放所有权
    BSTR detach();

    // 重置
    void reset(const wchar_t* str = nullptr);

   private:
    BSTR bstr_;
};

}  // namespace detail
}  // namespace sevenzip
