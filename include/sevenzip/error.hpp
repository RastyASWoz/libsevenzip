#pragma once

/// @file error.hpp
/// @brief Exception classes for error handling

#include <exception>
#include <string>
#include <system_error>

namespace sevenzip {

/// Base exception class for all sevenzip errors
class Exception : public std::exception {
   public:
    /// Construct with error message
    /// @param message Error message
    explicit Exception(const std::string& message) : message_(message) {}

    /// Construct with error message and source location
    /// @param message Error message
    /// @param file Source file name
    /// @param line Source line number
    Exception(const std::string& message, const char* file, int line)
        : message_(message + " (at " + file + ":" + std::to_string(line) + ")") {}

    /// Get error message
    /// @return Error message as C string
    const char* what() const noexcept override { return message_.c_str(); }

    /// Get error message as string
    /// @return Error message
    const std::string& message() const noexcept { return message_; }

   private:
    std::string message_;
};

/// IO exception for file/stream errors
class IoException : public Exception {
   public:
    /// Construct with error message
    /// @param message Error message
    explicit IoException(const std::string& message) : Exception(message), error_code_(0) {}

    /// Construct with error message and system error code
    /// @param message Error message
    /// @param error_code System error code (HRESULT or errno)
    IoException(const std::string& message, int error_code)
        : Exception(message + " (error code: 0x" + to_hex(error_code) + ")"),
          error_code_(error_code) {}

    /// Get system error code
    /// @return Error code (0 if not set)
    int error_code() const noexcept { return error_code_; }

   private:
    int error_code_;

    static std::string to_hex(int value) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%08X", static_cast<unsigned>(value));
        return buf;
    }
};

/// Format exception for archive format errors
class FormatException : public Exception {
   public:
    using Exception::Exception;
};

/// Password exception for encryption/decryption errors
class PasswordException : public Exception {
   public:
    using Exception::Exception;
};

/// Data exception for corrupted/invalid archive data
class DataException : public Exception {
   public:
    /// Construct with error message
    /// @param message Error message
    explicit DataException(const std::string& message) : Exception(message), is_crc_error_(false) {}

    /// Construct with error message and CRC error flag
    /// @param message Error message
    /// @param is_crc_error true if this is a CRC mismatch error
    DataException(const std::string& message, bool is_crc_error)
        : Exception(message), is_crc_error_(is_crc_error) {}

    /// Check if this is a CRC error
    /// @return true if data failed CRC check
    bool is_crc_error() const noexcept { return is_crc_error_; }

   private:
    bool is_crc_error_;
};

/// Not supported exception for unsupported operations
class NotSupportedException : public Exception {
   public:
    using Exception::Exception;
};

// Helper macros for throwing with source location
#define SEVENZIP_THROW(ExceptionType, message) throw ExceptionType(message, __FILE__, __LINE__)

#define SEVENZIP_THROW_IO(message, code) \
    throw IoException(                   \
        std::string(message) + " (at " + __FILE__ + ":" + std::to_string(__LINE__) + ")", code)

}  // namespace sevenzip
