// sz_convenience.cpp - Simplified convenience functions

#include "sevenzip/sz_convenience.h"

#include "sevenzip/sz_archive.h"
#include "sz_error_internal.h"

extern "C" {

SZ_API sz_result sz_extract_simple(const char* archive_path, const char* dest_dir) {
    if (!archive_path || !dest_dir) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    sz_archive_handle archive = nullptr;
    sz_result result = sz_archive_open(archive_path, &archive);
    if (result != SZ_OK) {
        return result;
    }

    result = sz_archive_extract_all(archive, dest_dir, nullptr, nullptr);
    sz_archive_close(archive);

    return result;
}

SZ_API sz_result sz_compress_simple(const char* source_path, const char* archive_path,
                                    sz_format format) {
    // Not implemented - would require writer
    sz_set_last_error("sz_compress_simple not yet implemented");
    return SZ_E_NOT_IMPLEMENTED;
}

SZ_API sz_result sz_extract_with_password(const char* archive_path, const char* dest_dir,
                                          const char* password) {
    if (!archive_path || !dest_dir) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    sz_archive_handle archive = nullptr;
    sz_result result = sz_archive_open(archive_path, &archive);
    if (result != SZ_OK) {
        return result;
    }

    if (password) {
        result = sz_archive_set_password(archive, password);
        if (result != SZ_OK) {
            sz_archive_close(archive);
            return result;
        }
    }

    result = sz_archive_extract_all(archive, dest_dir, nullptr, nullptr);
    sz_archive_close(archive);

    return result;
}

}  // extern "C"
