// sz_writer.cpp - Archive writing implementation

#include "sevenzip/sz_writer.h"

#include <memory>
#include <string>
#include <vector>

#include "sevenzip/archive_writer.hpp"
#include "sz_error_internal.h"

// Internal writer structure
struct SzWriter {
    std::unique_ptr<sevenzip::ArchiveWriter> writer;
    sevenzip::Format format;
    std::vector<uint8_t>* memory_buffer;  // For memory-based archives
    bool finalized;
    bool is_memory;

    sz_progress_callback progress_callback;
    void* progress_user_data;

    SzWriter()
        : format(sevenzip::Format::SevenZip),
          memory_buffer(nullptr),
          finalized(false),
          is_memory(false),
          progress_callback(nullptr),
          progress_user_data(nullptr) {}

    ~SzWriter() {
        if (memory_buffer) {
            delete memory_buffer;
            memory_buffer = nullptr;
        }
    }
};

extern "C" {

// Helper: Convert sz_format to Format
static sevenzip::Format convert_format(sz_format fmt) {
    switch (fmt) {
        case SZ_FORMAT_7Z:
            return sevenzip::Format::SevenZip;
        case SZ_FORMAT_ZIP:
            return sevenzip::Format::Zip;
        case SZ_FORMAT_TAR:
            return sevenzip::Format::Tar;
        case SZ_FORMAT_GZIP:
            return sevenzip::Format::GZip;
        case SZ_FORMAT_BZIP2:
            return sevenzip::Format::BZip2;
        case SZ_FORMAT_XZ:
            return sevenzip::Format::Xz;
        default:
            return sevenzip::Format::SevenZip;
    }
}

// Helper: Convert sz_compression_level to CompressionLevel
static sevenzip::CompressionLevel convert_compression_level(sz_compression_level level) {
    switch (level) {
        case SZ_LEVEL_NONE:
            return sevenzip::CompressionLevel::None;
        case SZ_LEVEL_FAST:
            return sevenzip::CompressionLevel::Fast;
        case SZ_LEVEL_NORMAL:
            return sevenzip::CompressionLevel::Normal;
        case SZ_LEVEL_MAXIMUM:
            return sevenzip::CompressionLevel::Maximum;
        case SZ_LEVEL_ULTRA:
            return sevenzip::CompressionLevel::Ultra;
        default:
            return sevenzip::CompressionLevel::Normal;
    }
}

/* ============================================================================
 * Writer Creation/Destruction
 * ========================================================================= */

SZ_API sz_result sz_writer_create(const char* path, sz_format format,
                                  sz_writer_handle* out_handle) {
    if (!path || !out_handle) {
        sz_set_last_error("Invalid argument: path and out_handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer_obj = std::make_unique<SzWriter>();
    writer_obj->format = convert_format(format);
    writer_obj->is_memory = false;

    // Create ArchiveWriter using static factory method
    auto writer = sevenzip::ArchiveWriter::create(path, writer_obj->format);
    writer_obj->writer = std::make_unique<sevenzip::ArchiveWriter>(std::move(writer));

    *out_handle = reinterpret_cast<sz_writer_handle>(writer_obj.release());
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_create_memory(sz_format format, sz_writer_handle* out_handle) {
    if (!out_handle) {
        sz_set_last_error("Invalid argument: out_handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer_obj = std::make_unique<SzWriter>();
    writer_obj->format = convert_format(format);
    writer_obj->is_memory = true;
    writer_obj->memory_buffer = new std::vector<uint8_t>();

    // Create ArchiveWriter to memory using static factory method
    auto writer =
        sevenzip::ArchiveWriter::createToMemory(*writer_obj->memory_buffer, writer_obj->format);
    writer_obj->writer = std::make_unique<sevenzip::ArchiveWriter>(std::move(writer));

    *out_handle = reinterpret_cast<sz_writer_handle>(writer_obj.release());
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_finalize(sz_writer_handle handle) {
    if (!handle) {
        sz_set_last_error("Invalid argument: handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Archive already finalized");
        return SZ_E_FAIL;
    }

    // Finalize the archive
    writer->writer->finalize();
    writer->finalized = true;

    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API void sz_writer_cancel(sz_writer_handle handle) {
    if (!handle) return;

    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->memory_buffer) {
        delete writer->memory_buffer;
        writer->memory_buffer = nullptr;
    }

    delete writer;
}

/* ============================================================================
 * Configuration
 * ========================================================================= */

SZ_API sz_result sz_writer_set_compression_level(sz_writer_handle handle,
                                                 sz_compression_level level) {
    if (!handle) {
        sz_set_last_error("Invalid argument: handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot modify finalized archive");
        return SZ_E_FAIL;
    }

    writer->writer->withLevel(convert_compression_level(level));
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_set_password(sz_writer_handle handle, const char* password) {
    if (!handle) {
        sz_set_last_error("Invalid argument: handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot modify finalized archive");
        return SZ_E_FAIL;
    }

    if (password && *password) {
        writer->writer->withPassword(password);
    }

    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_set_encrypted_headers(sz_writer_handle handle, int enabled) {
    if (!handle) {
        sz_set_last_error("Invalid argument: handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot modify finalized archive");
        return SZ_E_FAIL;
    }

    // Only 7z format supports encrypted headers
    if (writer->format != sevenzip::Format::SevenZip && enabled) {
        sz_set_last_error("Encrypted headers only supported for 7z format");
        return SZ_E_UNSUPPORTED_FORMAT;
    }

    writer->writer->withEncryptedHeaders(enabled != 0);
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_set_solid_mode(sz_writer_handle handle, int enabled) {
    if (!handle) {
        sz_set_last_error("Invalid argument: handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot modify finalized archive");
        return SZ_E_FAIL;
    }

    // Only 7z format supports solid mode
    if (writer->format != sevenzip::Format::SevenZip && enabled) {
        sz_set_last_error("Solid mode only supported for 7z format");
        return SZ_E_UNSUPPORTED_FORMAT;
    }

    writer->writer->withSolidMode(enabled != 0);
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_set_progress_callback(sz_writer_handle handle,
                                                 sz_progress_callback progress, void* user_data) {
    if (!handle) {
        sz_set_last_error("Invalid argument: handle must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);
    writer->progress_callback = progress;
    writer->progress_user_data = user_data;

    // Set progress callback on ArchiveWriter
    if (progress) {
        writer->writer->withProgress([writer](uint64_t completed, uint64_t total) -> bool {
            if (writer->progress_callback) {
                return writer->progress_callback(completed, total, writer->progress_user_data) != 0;
            }
            return true;
        });
    }

    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

/* ============================================================================
 * Adding Items
 * ========================================================================= */

SZ_API sz_result sz_writer_add_file(sz_writer_handle handle, const char* file_path,
                                    const char* archive_path) {
    if (!handle || !file_path) {
        sz_set_last_error("Invalid argument: handle and file_path must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot add files to finalized archive");
        return SZ_E_FAIL;
    }

    if (archive_path) {
        writer->writer->addFile(file_path, archive_path);
    } else {
        writer->writer->addFile(file_path);
    }
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_add_directory(sz_writer_handle handle, const char* dir_path,
                                         int recursive) {
    if (!handle || !dir_path) {
        sz_set_last_error("Invalid argument: handle and dir_path must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot add directories to finalized archive");
        return SZ_E_FAIL;
    }

    writer->writer->addDirectory(dir_path, recursive != 0);
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_add_memory(sz_writer_handle handle, const void* data, size_t size,
                                      const char* archive_path) {
    if (!handle || !data || !archive_path) {
        sz_set_last_error("Invalid argument: handle, data, and archive_path must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (writer->finalized) {
        sz_set_last_error("Cannot add files to finalized archive");
        return SZ_E_FAIL;
    }

    std::vector<uint8_t> buffer(static_cast<const uint8_t*>(data),
                                static_cast<const uint8_t*>(data) + size);

    writer->writer->addFromMemory(buffer, archive_path);
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

/* ============================================================================
 * Information
 * ========================================================================= */

SZ_API sz_result sz_writer_get_pending_count(sz_writer_handle handle, size_t* out_count) {
    if (!handle || !out_count) {
        sz_set_last_error("Invalid argument: handle and out_count must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    auto writer = reinterpret_cast<SzWriter*>(handle);
    *out_count = writer->writer->pendingCount();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_writer_get_memory_data(sz_writer_handle handle, const void** out_data,
                                           size_t* out_size) {
    if (!handle || !out_data || !out_size) {
        sz_set_last_error("Invalid argument: handle, out_data, and out_size must not be NULL");
        return SZ_E_INVALID_ARGUMENT;
    }

    auto writer = reinterpret_cast<SzWriter*>(handle);

    if (!writer->is_memory) {
        sz_set_last_error("Not a memory-based archive");
        return SZ_E_FAIL;
    }

    if (!writer->finalized) {
        sz_set_last_error("Archive not yet finalized");
        return SZ_E_FAIL;
    }

    if (!writer->memory_buffer) {
        sz_set_last_error("Memory buffer not available");
        return SZ_E_FAIL;
    }

    *out_data = writer->memory_buffer->data();
    *out_size = writer->memory_buffer->size();
    return SZ_OK;
}

}  // extern "C"
