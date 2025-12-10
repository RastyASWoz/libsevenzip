/**
 * @file sz_writer.h
 * @brief 7-Zip C API - Archive Writing Operations
 * @version 1.0.0
 */

#ifndef SZ_WRITER_H
#define SZ_WRITER_H

#include "sz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Writer Creation/Destruction
 * ========================================================================= */

/**
 * @brief Create a new archive writer
 *
 * @param path Archive file path (UTF-8)
 * @param format Archive format
 * @param out_handle Pointer to receive writer handle
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_writer_finalize() or sz_writer_cancel() when done
 */
SZ_API sz_result sz_writer_create(const char* path, sz_format format, sz_writer_handle* out_handle);

/**
 * @brief Create an archive writer that writes to memory
 *
 * @param format Archive format
 * @param out_handle Pointer to receive writer handle
 * @return SZ_OK on success, error code otherwise
 *
 * @note Use sz_writer_get_memory_data() to retrieve the buffer
 */
SZ_API sz_result sz_writer_create_memory(sz_format format, sz_writer_handle* out_handle);

/**
 * @brief Finalize and close the archive writer
 *
 * @param handle Writer handle
 * @return SZ_OK on success, error code otherwise
 *
 * @note This completes the archive and flushes all data
 */
SZ_API sz_result sz_writer_finalize(sz_writer_handle handle);

/**
 * @brief Cancel archive creation and free resources
 *
 * @param handle Writer handle (NULL is allowed)
 *
 * @note This discards the incomplete archive
 */
SZ_API void sz_writer_cancel(sz_writer_handle handle);

/* ============================================================================
 * Configuration
 * ========================================================================= */

/**
 * @brief Set compression level
 *
 * @param handle Writer handle
 * @param level Compression level
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_set_compression_level(sz_writer_handle handle,
                                                 sz_compression_level level);

/**
 * @brief Set password for encryption
 *
 * @param handle Writer handle
 * @param password Password string (UTF-8, NULL to disable)
 * @return SZ_OK on success, error code otherwise
 *
 * @note Password protection may not be supported for all formats
 */
SZ_API sz_result sz_writer_set_password(sz_writer_handle handle, const char* password);

/**
 * @brief Enable/disable header encryption
 *
 * @param handle Writer handle
 * @param encrypt Non-zero to encrypt headers
 * @return SZ_OK on success, error code otherwise
 *
 * @note Only supported for 7z format
 */
SZ_API sz_result sz_writer_set_encrypted_headers(sz_writer_handle handle, int encrypt);

/**
 * @brief Enable/disable solid mode
 *
 * @param handle Writer handle
 * @param solid Non-zero to enable solid compression
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_set_solid_mode(sz_writer_handle handle, int solid);

/**
 * @brief Set progress callback
 *
 * @param handle Writer handle
 * @param callback Progress callback function (NULL to disable)
 * @param user_data User data passed to callback
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_set_progress_callback(sz_writer_handle handle,
                                                 sz_progress_callback callback, void* user_data);

/* ============================================================================
 * Adding Items
 * ========================================================================= */

/**
 * @brief Add a file to the archive
 *
 * @param handle Writer handle
 * @param file_path Source file path (UTF-8)
 * @param archive_path Path in archive (UTF-8, NULL to use file name)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_add_file(sz_writer_handle handle, const char* file_path,
                                    const char* archive_path);

/**
 * @brief Add a directory to the archive
 *
 * @param handle Writer handle
 * @param dir_path Source directory path (UTF-8)
 * @param recursive Non-zero to include subdirectories
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_add_directory(sz_writer_handle handle, const char* dir_path,
                                         int recursive);

/**
 * @brief Add data from memory to the archive
 *
 * @param handle Writer handle
 * @param data Pointer to data
 * @param size Data size in bytes
 * @param archive_path Path in archive (UTF-8)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_add_memory(sz_writer_handle handle, const void* data, size_t size,
                                      const char* archive_path);

/**
 * @brief Get number of pending items
 *
 * @param handle Writer handle
 * @param out_count Pointer to receive count
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_writer_get_pending_count(sz_writer_handle handle, size_t* out_count);

/* ============================================================================
 * Memory Archive Access
 * ========================================================================= */

/**
 * @brief Get the memory buffer for a memory-based archive
 *
 * @param handle Writer handle (must be created with sz_writer_create_memory)
 * @param out_data Pointer to receive data pointer
 * @param out_size Pointer to receive data size
 * @return SZ_OK on success, error code otherwise
 *
 * @note Buffer is valid until sz_writer_finalize() or sz_writer_cancel()
 */
SZ_API sz_result sz_writer_get_memory_data(sz_writer_handle handle, const void** out_data,
                                           size_t* out_size);

#ifdef __cplusplus
}
#endif

#endif /* SZ_WRITER_H */
