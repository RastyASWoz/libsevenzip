// sz_error_internal.h - Internal error handling helpers

#ifndef SZ_ERROR_INTERNAL_H
#define SZ_ERROR_INTERNAL_H

#include <exception>
#include <string>

#include "sevenzip/error.hpp"
#include "sevenzip/sz_error.h"
#include "sevenzip/sz_types.h"

// Internal helper functions (not exported)
void sz_set_last_error(const char* message);
void sz_set_last_error(const std::string& message);

// Convert C++ exception to error code
inline sz_result sz_exception_to_result(const sevenzip::Exception& e) {
    // Try to determine error type by exception class
    const std::string& msg = e.message();

    if (dynamic_cast<const sevenzip::FormatException*>(&e)) {
        return SZ_E_UNSUPPORTED_FORMAT;
    }
    if (dynamic_cast<const sevenzip::PasswordException*>(&e)) {
        return SZ_E_WRONG_PASSWORD;
    }
    if (dynamic_cast<const sevenzip::DataException*>(&e)) {
        return SZ_E_CORRUPTED_ARCHIVE;
    }
    if (dynamic_cast<const sevenzip::NotSupportedException*>(&e)) {
        return SZ_E_NOT_IMPLEMENTED;
    }
    if (dynamic_cast<const sevenzip::IoException*>(&e)) {
        // Try to determine specific IO error from message
        if (msg.find("not found") != std::string::npos ||
            msg.find("does not exist") != std::string::npos) {
            return SZ_E_FILE_NOT_FOUND;
        }
        if (msg.find("access denied") != std::string::npos ||
            msg.find("permission") != std::string::npos) {
            return SZ_E_ACCESS_DENIED;
        }
        if (msg.find("write") != std::string::npos) {
            return SZ_E_WRITE_ERROR;
        }
        if (msg.find("read") != std::string::npos) {
            return SZ_E_READ_ERROR;
        }
        if (msg.find("disk full") != std::string::npos ||
            msg.find("no space") != std::string::npos) {
            return SZ_E_DISK_FULL;
        }
        return SZ_E_FAIL;
    }

    // Check message for common error patterns
    if (msg.find("invalid argument") != std::string::npos ||
        msg.find("null pointer") != std::string::npos) {
        return SZ_E_INVALID_ARGUMENT;
    }
    if (msg.find("out of range") != std::string::npos || msg.find("index") != std::string::npos) {
        return SZ_E_INDEX_OUT_OF_RANGE;
    }

    return SZ_E_FAIL;
}

// Exception handler helper macro
#define SZ_TRY_CATCH_BEGIN try {
#define SZ_TRY_CATCH_END(default_error)     \
    }                                       \
    catch (const sevenzip::Exception& e) {  \
        sz_set_last_error(e.what());        \
        return sz_exception_to_result(e);   \
    }                                       \
    catch (const std::bad_alloc&) {         \
        sz_set_last_error("Out of memory"); \
        return SZ_E_OUT_OF_MEMORY;          \
    }                                       \
    catch (const std::exception& e) {       \
        sz_set_last_error(e.what());        \
        return default_error;               \
    }                                       \
    catch (...) {                           \
        sz_set_last_error("Unknown error"); \
        return default_error;               \
    }

#endif  // SZ_ERROR_INTERNAL_H
