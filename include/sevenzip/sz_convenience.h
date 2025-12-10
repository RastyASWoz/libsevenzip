/**
 * @file sz_convenience.h
 * @brief 7-Zip C API - Convenience Functions
 * @version 1.0.0
 */

#ifndef SZ_CONVENIENCE_H
#define SZ_CONVENIENCE_H

#include "sz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Simple one-step extraction
 *
 * @param archive_path Archive file path (UTF-8)
 * @param dest_dir Destination directory (UTF-8)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_extract_simple(const char* archive_path, const char* dest_dir);

/**
 * @brief Simple one-step compression
 *
 * @param source_path Source file or directory (UTF-8)
 * @param archive_path Archive file path (UTF-8)
 * @param format Archive format
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_compress_simple(const char* source_path, const char* archive_path,
                                    sz_format format);

/**
 * @brief Extract with password
 *
 * @param archive_path Archive file path (UTF-8)
 * @param dest_dir Destination directory (UTF-8)
 * @param password Password (UTF-8)
 * @return SZ_OK on success, error code otherwise
 */
SZ_API sz_result sz_extract_with_password(const char* archive_path, const char* dest_dir,
                                          const char* password);

#ifdef __cplusplus
}
#endif

#endif /* SZ_CONVENIENCE_H */
