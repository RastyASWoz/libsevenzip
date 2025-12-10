// sz_compress.cpp - Simplified compression implementation

#include "sevenzip/sz_compress.h"

#include <cstring>
#include <memory>

#include "sevenzip/compressor.hpp"
#include "sz_error_internal.h"

// Opaque handle structure
struct sz_compressor_s {
    std::unique_ptr<sevenzip::Compressor> compressor;
};

extern "C" {

SZ_API sz_result sz_compressor_create(sz_format format, sz_compression_level level,
                                      sz_compressor_handle* out_handle) {
    if (!out_handle) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    // Map C enum to C++ enum
    sevenzip::Format cpp_format;
    switch (format) {
        case SZ_FORMAT_GZIP:
            cpp_format = sevenzip::Format::GZip;
            break;
        case SZ_FORMAT_BZIP2:
            cpp_format = sevenzip::Format::BZip2;
            break;
        case SZ_FORMAT_XZ:
            cpp_format = sevenzip::Format::Xz;
            break;
        default:
            sz_set_last_error("Format not supported for standalone compression");
            return SZ_E_UNSUPPORTED_FORMAT;
    }

    sevenzip::CompressionLevel cpp_level;
    switch (level) {
        case SZ_LEVEL_NONE:
            cpp_level = sevenzip::CompressionLevel::None;
            break;
        case SZ_LEVEL_FAST:
            cpp_level = sevenzip::CompressionLevel::Fast;
            break;
        case SZ_LEVEL_NORMAL:
            cpp_level = sevenzip::CompressionLevel::Normal;
            break;
        case SZ_LEVEL_MAXIMUM:
            cpp_level = sevenzip::CompressionLevel::Maximum;
            break;
        case SZ_LEVEL_ULTRA:
            cpp_level = sevenzip::CompressionLevel::Ultra;
            break;
        default:
            sz_set_last_error("Invalid compression level");
            return SZ_E_INVALID_ARGUMENT;
    }

    auto handle = new sz_compressor_s();
    handle->compressor = std::make_unique<sevenzip::Compressor>(cpp_format, cpp_level);
    *out_handle = handle;

    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API void sz_compressor_destroy(sz_compressor_handle handle) {
    if (handle) {
        delete handle;
    }
}

SZ_API sz_result sz_compress_data(sz_compressor_handle handle, const void* input, size_t input_size,
                                  void** output, size_t* output_size) {
    if (!handle || !input || !output || !output_size) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    std::vector<uint8_t> input_buffer(static_cast<const uint8_t*>(input),
                                      static_cast<const uint8_t*>(input) + input_size);

    auto result = handle->compressor->compress(input_buffer);

    // Allocate and copy output
    void* buffer = malloc(result.size());
    if (!buffer) {
        sz_set_last_error("Failed to allocate output buffer");
        return SZ_E_OUT_OF_MEMORY;
    }

    std::memcpy(buffer, result.data(), result.size());
    *output = buffer;
    *output_size = result.size();

    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_decompress_data(sz_compressor_handle handle, const void* input,
                                    size_t input_size, void** output, size_t* output_size) {
    if (!handle || !input || !output || !output_size) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    std::vector<uint8_t> input_buffer(static_cast<const uint8_t*>(input),
                                      static_cast<const uint8_t*>(input) + input_size);

    auto result = handle->compressor->decompress(input_buffer);

    // Allocate and copy output
    void* buffer = malloc(result.size());
    if (!buffer) {
        sz_set_last_error("Failed to allocate output buffer");
        return SZ_E_OUT_OF_MEMORY;
    }

    std::memcpy(buffer, result.data(), result.size());
    *output = buffer;
    *output_size = result.size();

    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_compress_file(sz_compressor_handle handle, const char* input_path,
                                  const char* output_path) {
    if (!handle || !input_path || !output_path) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    handle->compressor->compressFile(input_path, output_path);
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

SZ_API sz_result sz_decompress_file(sz_compressor_handle handle, const char* input_path,
                                    const char* output_path) {
    if (!handle || !input_path || !output_path) {
        sz_set_last_error("Invalid argument: NULL pointer");
        return SZ_E_INVALID_ARGUMENT;
    }

    SZ_TRY_CATCH_BEGIN
    handle->compressor->decompressFile(input_path, output_path);
    sz_clear_error();
    return SZ_OK;
    SZ_TRY_CATCH_END(SZ_E_FAIL)
}

}  // extern "C"
