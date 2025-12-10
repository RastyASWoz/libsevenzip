/**
 * @file sz_archive.h
 * @brief 7-Zip C API - Archive Reading Operations
 * @version 1.0.0
 */

#ifndef SZ_ARCHIVE_H
#define SZ_ARCHIVE_H

#include "sz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Archive Opening/Closing
 * ========================================================================= */

/**
 * @brief Open an archive file for reading
 *
 * @param path Path to archive file (UTF-8)
 * @param out_handle Pointer to receive archive handle
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_archive_close() when done
 */
SZ_API sz_result sz_archive_open(const char* path, sz_archive_handle* out_handle);

/**
 * @brief Open an archive from memory buffer
 *
 * @param data Pointer to archive data
 * @param size Size of data in bytes
 * @param format Archive format (use SZ_FORMAT_AUTO to auto-detect)
 * @param out_handle Pointer to receive archive handle
 * @return SZ_OK on success, error code otherwise
 *
 * @note The data buffer must remain valid until sz_archive_close() is called
 */
SZ_API sz_result sz_archive_open_memory(const void* data, size_t size, sz_format format,
                                        sz_archive_handle* out_handle);

/**
 * @brief Close an archive and free resources
 *
 * @param handle Archive handle (NULL is allowed)
 */
SZ_API void sz_archive_close(sz_archive_handle handle);

/* ============================================================================
 * Archive Information
 * ========================================================================= */

/**
 * @brief Get archive metadata information
 *
 * @param handle Archive handle
 * @param out_info Pointer to receive archive info
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_archive_get_info(sz_archive_handle handle, sz_archive_info* out_info);

/**
 * @brief Get number of items in archive
 *
 * @param handle Archive handle
 * @param out_count Pointer to receive item count
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_archive_get_item_count(sz_archive_handle handle, size_t* out_count);

/**
 * @brief Get information about a specific item
 *
 * @param handle Archive handle
 * @param index Item index (0-based)
 * @param out_info Pointer to receive item info
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_item_info_free() to free the path string when done
 */
SZ_API sz_result sz_archive_get_item_info(sz_archive_handle handle, size_t index,
                                          sz_item_info* out_info);

/**
 * @brief Free strings allocated in sz_item_info
 *
 * @param info Pointer to item info structure
 */
SZ_API void sz_item_info_free(sz_item_info* info);

/* ============================================================================
 * Extraction Operations
 * ========================================================================= */

/**
 * @brief Extract all items to a directory
 *
 * @param handle Archive handle
 * @param dest_dir Destination directory (UTF-8)
 * @param progress Progress callback (optional, can be NULL)
 * @param user_data User data passed to callback
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_archive_extract_all(sz_archive_handle handle, const char* dest_dir,
                                        sz_progress_callback progress, void* user_data);

/**
 * @brief Extract a single item to a file
 *
 * @param handle Archive handle
 * @param index Item index to extract
 * @param dest_path Destination file path (UTF-8)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_archive_extract_item(sz_archive_handle handle, size_t index,
                                         const char* dest_path);

/**
 * @brief Extract an item to memory
 *
 * @param handle Archive handle
 * @param index Item index to extract
 * @param out_data Pointer to receive allocated data buffer
 * @param out_size Pointer to receive data size
 * @return SZ_OK on success, error code otherwise
 *
 * @note Call sz_memory_free() to free the data buffer when done
 */
SZ_API sz_result sz_archive_extract_to_memory(sz_archive_handle handle, size_t index,
                                              void** out_data, size_t* out_size);

/**
 * @brief Free memory allocated by the library
 *
 * @param data Pointer to memory (NULL is allowed)
 */
SZ_API void sz_memory_free(void* data);

/* ============================================================================
 * Configuration
 * ========================================================================= */

/**
 * @brief Set password for encrypted archives
 *
 * @param handle Archive handle
 * @param password Password string (UTF-8, NULL to clear)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_archive_set_password(sz_archive_handle handle, const char* password);

/**
 * @brief Test archive integrity
 *
 * @param handle Archive handle
 * @return SZ_OK if archive is valid, error code otherwise
 */
SZ_API sz_result sz_archive_test(sz_archive_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* SZ_ARCHIVE_H */
