// sz_version.cpp - Version information implementation

#include "sevenzip/sz_version.h"

#include <sstream>

#include "sz_error_internal.h"

// Version constants
#define SZ_VERSION_MAJOR 1
#define SZ_VERSION_MINOR 0
#define SZ_VERSION_PATCH 0
#define SZ_VERSION_STRING "1.0.0-alpha"

extern "C" {

const char* sz_version_string(void) {
    return SZ_VERSION_STRING;
}

void sz_version_number(int* major, int* minor, int* patch) {
    if (major) *major = SZ_VERSION_MAJOR;
    if (minor) *minor = SZ_VERSION_MINOR;
    if (patch) *patch = SZ_VERSION_PATCH;
}

int sz_is_format_supported(sz_format format) {
    switch (format) {
        case SZ_FORMAT_7Z:
        case SZ_FORMAT_ZIP:
        case SZ_FORMAT_TAR:
        case SZ_FORMAT_GZIP:
        case SZ_FORMAT_BZIP2:
        case SZ_FORMAT_XZ:
            return 1;
        case SZ_FORMAT_AUTO:
            // AUTO is supported for reading
            return 1;
        default:
            return 0;
    }
}

}  // extern "C"
