/**
 * @file sz_compress.h
 * @brief 7-Zip C API - Standalone Compression Operations
 * @version 1.0.0
 */

#ifndef SZ_COMPRESS_H
#define SZ_COMPRESS_H

#include "sz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Compressor Management
 * ========================================================================= */

/**
 * @brief Create a standalone compressor
 *
 * @param format Compression format (GZIP, BZIP2, or XZ)
 * @param level Compression level
 * @param out_handle Pointer to receive compressor handle
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_compressor_destroy() when done
 */
SZ_API sz_result sz_compressor_create(sz_format format, sz_compression_level level,
                                      sz_compressor_handle* out_handle);

/**
 * @brief Destroy a compressor and free resources
 *
 * @param handle Compressor handle (NULL is allowed)
 */
SZ_API void sz_compressor_destroy(sz_compressor_handle handle);

/* ============================================================================
 * Data Compression/Decompression
 * ========================================================================= */

/**
 * @brief Compress data in memory
 *
 * @param handle Compressor handle
 * @param input Input data pointer
 * @param input_size Input data size
 * @param out_data Pointer to receive compressed data
 * @param out_size Pointer to receive compressed size
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_memory_free() to free out_data when done
 */
SZ_API sz_result sz_compress_data(sz_compressor_handle handle, const void* input, size_t input_size,
                                  void** out_data, size_t* out_size);

/**
 * @brief Decompress data in memory
 *
 * @param handle Compressor handle
 * @param input Compressed data pointer
 * @param input_size Compressed data size
 * @param out_data Pointer to receive decompressed data
 * @param out_size Pointer to receive decompressed size
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_memory_free() to free out_data when done
 */
SZ_API sz_result sz_decompress_data(sz_compressor_handle handle, const void* input,
                                    size_t input_size, void** out_data, size_t* out_size);

/* ============================================================================
 * File Compression/Decompression
 * ========================================================================= */

/**
 * @brief Compress a file
 *
 * @param handle Compressor handle
 * @param input_path Input file path (UTF-8)
 * @param output_path Output file path (UTF-8)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_compress_file(sz_compressor_handle handle, const char* input_path,
                                  const char* output_path);

/**
 * @brief Decompress a file
 *
 * @param handle Compressor handle
 * @param input_path Input file path (UTF-8)
 * @param output_path Output file path (UTF-8)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_decompress_file(sz_compressor_handle handle, const char* input_path,
                                    const char* output_path);

#ifdef __cplusplus
}
#endif

#endif /* SZ_COMPRESS_H */
