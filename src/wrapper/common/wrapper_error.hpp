#pragma once

#include <stdexcept>
#include <string>
#include <system_error>

namespace sevenzip {

/**
 * @brief 错误码枚举
 */
enum class ErrorCode {
    Success = 0,

    // 通用错误
    Unknown = 1,
    NotImplemented,
    InvalidArgument,
    OutOfMemory,

    // 文件系统错误
    FileNotFound,
    PathNotFound,
    AccessDenied,
    FileExists,
    DiskFull,
    InvalidHandle,

    // 归档错误
    InvalidArchive,
    UnsupportedFormat,
    CorruptedArchive,
    HeaderError,

    // 数据错误
    DataError,
    CrcError,
    UnexpectedEnd,
    DataAfterEnd,

    // 认证错误
    WrongPassword,
    EncryptedHeader,

    // 操作错误
    OperationCancelled,
    UnsupportedMethod,
    Unavailable,
    Aborted,
    InvalidState,
    ArchiveWriteError,
    CannotOpenFile,

    // 流错误
    StreamReadError,
    StreamWriteError,
    StreamSeekError,
};

/**
 * @brief 错误分类
 */
class ErrorCategory : public std::error_category {
   public:
    const char* name() const noexcept override;
    std::string message(int ev) const override;

    static const ErrorCategory& instance();

   private:
    ErrorCategory() = default;
};

/**
 * @brief 创建 error_code
 */
std::error_code make_error_code(ErrorCode e);

/**
 * @brief 7-Zip 异常类
 */
class Exception : public std::runtime_error {
   public:
    explicit Exception(ErrorCode code);
    Exception(ErrorCode code, const std::string& message);
    Exception(ErrorCode code, const std::string& message, const std::string& context);

    ErrorCode code() const noexcept { return code_; }
    const std::string& context() const noexcept { return context_; }

   private:
    ErrorCode code_;
    std::string context_;
};

/**
 * @brief HRESULT 转换函数
 */
ErrorCode hresult_to_error_code(long hr);
std::error_code hresult_to_std_error(long hr);

/**
 * @brief HRESULT 检查宏
 */
#define SZ_CHECK_HR(hr)                                                          \
    do {                                                                         \
        long _hr = (hr);                                                         \
        if (_hr < 0) {                                                           \
            throw ::sevenzip::Exception(::sevenzip::hresult_to_error_code(_hr)); \
        }                                                                        \
    } while (0)

#define SZ_CHECK_HR_MSG(hr, msg)                                                        \
    do {                                                                                \
        long _hr = (hr);                                                                \
        if (_hr < 0) {                                                                  \
            throw ::sevenzip::Exception(::sevenzip::hresult_to_error_code(_hr), (msg)); \
        }                                                                               \
    } while (0)

}  // namespace sevenzip

// 注册到 std::error_code
namespace std {
template <>
struct is_error_code_enum<sevenzip::ErrorCode> : true_type {};
}  // namespace std
