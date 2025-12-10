// sz_archive.cpp - Archive reading implementation

#include "sevenzip/sz_archive.h"

#include <cstring>
#include <memory>

#include "sevenzip/archive_reader.hpp"
#include "sz_error_internal.h"

// Opaque handle structure
struct sz_archive_s {
    std::unique_ptr<sevenzip::ArchiveReader> reader;
};

extern "C" {

sz_result sz_archive_open(const char* path, sz_archive_handle* out_handle) {
    if (!path || !out_handle) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto handle = new sz_archive_s();
    handle->reader = std::make_unique<sevenzip::ArchiveReader>(path);
    *out_handle = handle;
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

sz_result sz_archive_open_memory(const void* data, size_t size, sz_format format,
                                 sz_archive_handle* out_handle) {
    if (!data || !out_handle) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    std::vector<uint8_t> buffer(static_cast<const uint8_t*>(data),
                                static_cast<const uint8_t*>(data) + size);

    auto handle = new sz_archive_s();
    handle->reader = std::make_unique<sevenzip::ArchiveReader>(buffer);
    *out_handle = handle;
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

void sz_archive_close(sz_archive_handle handle) {
    if (handle) {
        delete handle;
    }
}

sz_result sz_archive_get_info(sz_archive_handle handle, sz_archive_info* out_info) {
    if (!handle || !out_info) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto info = handle->reader->info();

    // Map C++ enum to C enum
    switch (info.format) {
        case sevenzip::Format::SevenZip:
            out_info->format = SZ_FORMAT_7Z;
            break;
        case sevenzip::Format::Zip:
            out_info->format = SZ_FORMAT_ZIP;
            break;
        case sevenzip::Format::Tar:
            out_info->format = SZ_FORMAT_TAR;
            break;
        case sevenzip::Format::GZip:
            out_info->format = SZ_FORMAT_GZIP;
            break;
        case sevenzip::Format::BZip2:
            out_info->format = SZ_FORMAT_BZIP2;
            break;
        case sevenzip::Format::Xz:
            out_info->format = SZ_FORMAT_XZ;
            break;
        default:
            out_info->format = SZ_FORMAT_AUTO;
            break;
    }

    out_info->item_count = info.itemCount;
    out_info->total_size = info.totalSize;
    out_info->packed_size = info.packedSize;
    out_info->is_solid = info.isSolid ? 1 : 0;
    out_info->is_multi_volume = info.isMultiVolume ? 1 : 0;
    out_info->has_encrypted_headers = info.hasEncryptedHeaders ? 1 : 0;

    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

sz_result sz_archive_get_item_count(sz_archive_handle handle, size_t* out_count) {
    if (!handle || !out_count) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    *out_count = handle->reader->itemCount();
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

sz_result sz_archive_get_item_info(sz_archive_handle handle, size_t index, sz_item_info* out_info) {
    if (!handle || !out_info) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto info = handle->reader->itemInfo(index);

    out_info->index = info.index;
    out_info->path = _strdup(info.path.string().c_str());  // Allocate copy
    out_info->size = info.size;
    out_info->packed_size = info.packedSize;
    out_info->crc = info.crc.value_or(0);
    out_info->has_crc = info.crc.has_value() ? 1 : 0;

    // Convert time_point to Unix timestamp
    auto to_unix_time = [](const std::chrono::system_clock::time_point& tp) {
        return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    };

    out_info->creation_time = to_unix_time(info.creationTime);
    out_info->modification_time = to_unix_time(info.modificationTime);
    out_info->is_directory = info.isDirectory ? 1 : 0;
    out_info->is_encrypted = info.isEncrypted ? 1 : 0;

    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

void sz_item_info_free(sz_item_info* info) {
    if (info && info->path) {
        free(info->path);
        info->path = nullptr;
    }
}

sz_result sz_archive_extract_all(sz_archive_handle handle, const char* dest_dir,
                                 sz_progress_callback progress, void* user_data) {
    if (!handle || !dest_dir) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    // Set up progress callback if provided
    if (progress) {
        handle->reader->withProgress([progress, user_data](uint64_t completed, uint64_t total) {
            return progress(completed, total, user_data) != 0;
        });
    }

    handle->reader->extractAll(dest_dir);
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

sz_result sz_archive_extract_item(sz_archive_handle handle, size_t index, const char* dest_path) {
    if (!handle || !dest_path) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    handle->reader->extractTo(index, dest_path);
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

sz_result sz_archive_extract_to_memory(sz_archive_handle handle, size_t index, void** out_data,
                                       size_t* out_size) {
    if (!handle || !out_data || !out_size) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto data = handle->reader->extract(index);

    // Allocate memory and copy
    void* buffer = malloc(data.size());
    if (!buffer) {
        sz_set_last_error("Failed to allocate memory");
        return SZ_E_OUT_OF_MEMORY;
    }

    std::memcpy(buffer, data.data(), data.size());
    *out_data = buffer;
    *out_size = data.size();

    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

void sz_memory_free(void* data) {
    if (data) {
        free(data);
    }
}

sz_result sz_archive_set_password(sz_archive_handle handle, const char* password) {
    if (!handle) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    if (password) {
        handle->reader->withPassword(password);
    } else {
        handle->reader->withPassword("");
    }
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

sz_result sz_archive_test(sz_archive_handle handle) {
    if (!handle) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    bool result = handle->reader->test();
    if (!result) {
        sz_set_last_error("Archive test failed");
        return SZ_E_CORRUPTED_ARCHIVE;
    }
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

}  // extern "C"
