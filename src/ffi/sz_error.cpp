// sz_error.cpp - Error handling implementation

#include "sevenzip/sz_error.h"

#include <mutex>
#include <string>

namespace {
// Thread-local error message storage
thread_local std::string g_last_error;
}  // namespace

extern "C" {

const char* sz_error_to_string(sz_result error) {
    switch (error) {
        case SZ_OK:
            return "Success";
        case SZ_E_FAIL:
            return "General failure";
        case SZ_E_OUT_OF_MEMORY:
            return "Out of memory";
        case SZ_E_FILE_NOT_FOUND:
            return "File not found";
        case SZ_E_ACCESS_DENIED:
            return "Access denied";
        case SZ_E_INVALID_ARGUMENT:
            return "Invalid argument";
        case SZ_E_UNSUPPORTED_FORMAT:
            return "Unsupported format";
        case SZ_E_CORRUPTED_ARCHIVE:
            return "Corrupted archive";
        case SZ_E_WRONG_PASSWORD:
            return "Wrong password";
        case SZ_E_CANCELLED:
            return "Operation cancelled";
        case SZ_E_INDEX_OUT_OF_RANGE:
            return "Index out of range";
        case SZ_E_ALREADY_OPEN:
            return "Archive already open";
        case SZ_E_NOT_OPEN:
            return "Archive not open";
        case SZ_E_WRITE_ERROR:
            return "Write error";
        case SZ_E_READ_ERROR:
            return "Read error";
        case SZ_E_NOT_IMPLEMENTED:
            return "Feature not implemented";
        case SZ_E_DISK_FULL:
            return "Disk full";
        default:
            return "Unknown error";
    }
}

const char* sz_get_last_error_message() {
    return g_last_error.c_str();
}

void sz_clear_error() {
    g_last_error.clear();
}

}  // extern "C"

// Internal helper function for setting error messages
void sz_set_last_error(const char* message) {
    if (message) {
        g_last_error = message;
    } else {
        g_last_error.clear();
    }
}

void sz_set_last_error(const std::string& message) {
    g_last_error = message;
}
