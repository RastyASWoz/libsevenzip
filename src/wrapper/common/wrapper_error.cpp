#include "wrapper_error.hpp"

#include <map>

namespace sevenzip {

// ErrorCategory 实现
const char* ErrorCategory::name() const noexcept {
    return "sevenzip";
}

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<ErrorCode>(ev)) {
        case ErrorCode::Success:
            return "Success";
        case ErrorCode::Unknown:
            return "Unknown error";
        case ErrorCode::NotImplemented:
            return "Not implemented";
        case ErrorCode::InvalidArgument:
            return "Invalid argument";
        case ErrorCode::OutOfMemory:
            return "Out of memory";
        case ErrorCode::FileNotFound:
            return "File not found";
        case ErrorCode::PathNotFound:
            return "Path not found";
        case ErrorCode::AccessDenied:
            return "Access denied";
        case ErrorCode::FileExists:
            return "File already exists";
        case ErrorCode::DiskFull:
            return "Disk is full";
        case ErrorCode::InvalidHandle:
            return "Invalid handle";
        case ErrorCode::InvalidArchive:
            return "Invalid archive";
        case ErrorCode::UnsupportedFormat:
            return "Unsupported archive format";
        case ErrorCode::CorruptedArchive:
            return "Archive is corrupted";
        case ErrorCode::HeaderError:
            return "Archive header error";
        case ErrorCode::DataError:
            return "Data error";
        case ErrorCode::CrcError:
            return "CRC check failed";
        case ErrorCode::UnexpectedEnd:
            return "Unexpected end of data";
        case ErrorCode::DataAfterEnd:
            return "Data after end of archive";
        case ErrorCode::WrongPassword:
            return "Wrong password";
        case ErrorCode::EncryptedHeader:
            return "Archive header is encrypted";
        case ErrorCode::OperationCancelled:
            return "Operation cancelled by user";
        case ErrorCode::UnsupportedMethod:
            return "Unsupported compression method";
        case ErrorCode::Unavailable:
            return "Resource unavailable";
        case ErrorCode::Aborted:
            return "Operation aborted";
        case ErrorCode::InvalidState:
            return "Invalid state";
        case ErrorCode::ArchiveWriteError:
            return "Archive write error";
        case ErrorCode::CannotOpenFile:
            return "Cannot open file";
        case ErrorCode::StreamReadError:
            return "Stream read error";
        case ErrorCode::StreamWriteError:
            return "Stream write error";
        case ErrorCode::StreamSeekError:
            return "Stream seek error";
        default:
            return "Unknown error code";
    }
}

const ErrorCategory& ErrorCategory::instance() {
    static ErrorCategory category;
    return category;
}

std::error_code make_error_code(ErrorCode e) {
    return {static_cast<int>(e), ErrorCategory::instance()};
}

// Exception 实现
Exception::Exception(ErrorCode code)
    : std::runtime_error(make_error_code(code).message()), code_(code) {}

Exception::Exception(ErrorCode code, const std::string& message)
    : std::runtime_error(message), code_(code) {}

Exception::Exception(ErrorCode code, const std::string& message, const std::string& context)
    : std::runtime_error(message + " [Context: " + context + "]"), code_(code), context_(context) {}

// HRESULT 映射
namespace detail {

struct HResultMapping {
    long hresult;
    ErrorCode error_code;
};

// HRESULT 常量定义
constexpr long S_OK = 0x00000000L;
constexpr long S_FALSE = 0x00000001L;
constexpr long E_NOTIMPL = 0x80004001L;
constexpr long E_INVALIDARG = 0x80070057L;
constexpr long E_OUTOFMEMORY = 0x8007000EL;
constexpr long E_POINTER = 0x80004003L;
constexpr long E_ABORT = 0x80004004L;
constexpr long E_FAIL = 0x80004005L;
constexpr long E_UNEXPECTED = 0x8000FFFFL;

// Win32 错误码 (通过 HRESULT_FROM_WIN32 转换)
constexpr long HRESULT_WIN32_FILE_NOT_FOUND = 0x80070002L;  // ERROR_FILE_NOT_FOUND (2)
constexpr long HRESULT_WIN32_PATH_NOT_FOUND = 0x80070003L;  // ERROR_PATH_NOT_FOUND (3)
constexpr long HRESULT_WIN32_ACCESS_DENIED = 0x80070005L;   // ERROR_ACCESS_DENIED (5)
constexpr long HRESULT_WIN32_INVALID_HANDLE = 0x80070006L;  // ERROR_INVALID_HANDLE (6)
constexpr long HRESULT_WIN32_FILE_EXISTS = 0x80070050L;     // ERROR_FILE_EXISTS (80)
constexpr long HRESULT_WIN32_DISK_FULL = 0x80070070L;       // ERROR_DISK_FULL (112)
constexpr long HRESULT_WIN32_NEGATIVE_SEEK = 0x80070083L;   // ERROR_NEGATIVE_SEEK (131)

constexpr HResultMapping hresult_mappings[] = {
    // 成功
    {S_OK, ErrorCode::Success},
    {S_FALSE, ErrorCode::Success},

    // COM 通用错误
    {E_NOTIMPL, ErrorCode::NotImplemented},
    {E_INVALIDARG, ErrorCode::InvalidArgument},
    {E_OUTOFMEMORY, ErrorCode::OutOfMemory},
    {E_POINTER, ErrorCode::InvalidArgument},
    {E_ABORT, ErrorCode::OperationCancelled},
    {E_FAIL, ErrorCode::Unknown},
    {E_UNEXPECTED, ErrorCode::Unknown},

    // Win32 文件系统错误
    {HRESULT_WIN32_FILE_NOT_FOUND, ErrorCode::FileNotFound},
    {HRESULT_WIN32_PATH_NOT_FOUND, ErrorCode::PathNotFound},
    {HRESULT_WIN32_ACCESS_DENIED, ErrorCode::AccessDenied},
    {HRESULT_WIN32_INVALID_HANDLE, ErrorCode::InvalidHandle},
    {HRESULT_WIN32_FILE_EXISTS, ErrorCode::FileExists},
    {HRESULT_WIN32_DISK_FULL, ErrorCode::DiskFull},
    {HRESULT_WIN32_NEGATIVE_SEEK, ErrorCode::StreamSeekError},
};

}  // namespace detail

ErrorCode hresult_to_error_code(long hr) {
    // 先查找映射表
    for (const auto& mapping : detail::hresult_mappings) {
        if (mapping.hresult == hr) {
            return mapping.error_code;
        }
    }

    // 如果是成功码
    if (hr >= 0) {
        return ErrorCode::Success;
    }

    // 未知错误
    return ErrorCode::Unknown;
}

std::error_code hresult_to_std_error(long hr) {
    return make_error_code(hresult_to_error_code(hr));
}

}  // namespace sevenzip
