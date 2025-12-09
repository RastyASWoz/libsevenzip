#include "wrapper_string.hpp"

#include <memory>

#include "wrapper_error.hpp"

#ifndef _WIN32
#include <codecvt>
#include <cstdlib>
#include <cstring>
#include <locale>
#endif

namespace sevenzip {
namespace detail {

// UTF-8 <-> UTF-16 转换
#ifdef _WIN32

std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    int size =
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    if (size == 0) {
        throw Exception(ErrorCode::InvalidArgument, "Invalid UTF-8 string");
    }

    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size);
    return result;
}

std::string wstring_to_utf8(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr,
                                   0, nullptr, nullptr);
    if (size == 0) {
        throw Exception(ErrorCode::InvalidArgument, "Invalid wide string");
    }

    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size,
                        nullptr, nullptr);
    return result;
}

#else

// 非 Windows 平台使用 codecvt
std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
        return converter.from_bytes(str);
    } catch (const std::range_error&) {
        throw Exception(ErrorCode::InvalidArgument, "Invalid UTF-8 string");
    }
}

std::string wstring_to_utf8(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
        return converter.to_bytes(str);
    } catch (const std::range_error&) {
        throw Exception(ErrorCode::InvalidArgument, "Invalid wide string");
    }
}

#endif

// 路径转换
std::wstring path_to_wstring(const std::filesystem::path& path) {
    return path.wstring();
}

std::filesystem::path wstring_to_path(const std::wstring& str) {
    return std::filesystem::path(str);
}

// BStrGuard 实现
#ifdef _WIN32

BStrGuard::BStrGuard() : bstr_(nullptr) {}

BStrGuard::BStrGuard(const wchar_t* str) : bstr_(nullptr) {
    if (str) {
        bstr_ = ::SysAllocString(str);
        if (!bstr_) {
            throw Exception(ErrorCode::OutOfMemory, "Failed to allocate BSTR");
        }
    }
}

BStrGuard::BStrGuard(const std::wstring& str) : bstr_(nullptr) {
    if (!str.empty()) {
        bstr_ = ::SysAllocString(str.c_str());
        if (!bstr_) {
            throw Exception(ErrorCode::OutOfMemory, "Failed to allocate BSTR");
        }
    }
}

BStrGuard::~BStrGuard() {
    if (bstr_) {
        ::SysFreeString(bstr_);
    }
}

BStrGuard::BStrGuard(BStrGuard&& other) noexcept : bstr_(other.bstr_) {
    other.bstr_ = nullptr;
}

BStrGuard& BStrGuard::operator=(BStrGuard&& other) noexcept {
    if (this == std::addressof(other)) {
        return *this;
    }
    if (bstr_) {
        ::SysFreeString(bstr_);
    }
    bstr_ = other.bstr_;
    other.bstr_ = nullptr;
    return *this;
}

BSTR BStrGuard::detach() {
    BSTR result = bstr_;
    bstr_ = nullptr;
    return result;
}

void BStrGuard::reset(const wchar_t* str) {
    if (bstr_) {
        ::SysFreeString(bstr_);
        bstr_ = nullptr;
    }

    if (str) {
        bstr_ = ::SysAllocString(str);
        if (!bstr_) {
            throw Exception(ErrorCode::OutOfMemory, "Failed to allocate BSTR");
        }
    }
}

#else

// 非 Windows 平台的简化实现
BStrGuard::BStrGuard() : bstr_(nullptr) {}

BStrGuard::BStrGuard(const wchar_t* str) : bstr_(nullptr) {
    if (str) {
        size_t len = wcslen(str);
        bstr_ = static_cast<BSTR>(malloc((len + 1) * sizeof(wchar_t)));
        if (bstr_) {
            wcscpy(bstr_, str);
        }
    }
}

BStrGuard::BStrGuard(const std::wstring& str) : bstr_(nullptr) {
    if (!str.empty()) {
        bstr_ = static_cast<BSTR>(malloc((str.size() + 1) * sizeof(wchar_t)));
        if (bstr_) {
            wcscpy(bstr_, str.c_str());
        }
    }
}

BStrGuard::~BStrGuard() {
    if (bstr_) {
        free(bstr_);
    }
}

BStrGuard::BStrGuard(BStrGuard&& other) noexcept : bstr_(other.bstr_) {
    other.bstr_ = nullptr;
}

BStrGuard& BStrGuard::operator=(BStrGuard&& other) noexcept {
    if (this != &other) {
        if (bstr_) {
            free(bstr_);
        }
        bstr_ = other.bstr_;
        other.bstr_ = nullptr;
    }
    return *this;
}

BSTR BStrGuard::detach() {
    BSTR result = bstr_;
    bstr_ = nullptr;
    return result;
}

void BStrGuard::reset(const wchar_t* str) {
    if (bstr_) {
        free(bstr_);
        bstr_ = nullptr;
    }

    if (str) {
        size_t len = wcslen(str);
        bstr_ = static_cast<BSTR>(malloc((len + 1) * sizeof(wchar_t)));
        if (bstr_) {
            wcscpy(bstr_, str);
        }
    }
}

#endif

}  // namespace detail
}  // namespace sevenzip
