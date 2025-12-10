/**
 * @file sz_version.h
 * @brief 7-Zip C API - Version Information
 * @version 1.0.0
 */

#ifndef SZ_VERSION_H
#define SZ_VERSION_H

#include "sz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get version string
 *
 * @return Version string (e.g., "1.0.0")
 */
SZ_API const char* sz_version_string(void);

/**
 * @brief Get version numbers
 *
 * @param major Pointer to receive major version (can be NULL)
 * @param minor Pointer to receive minor version (can be NULL)
 * @param patch Pointer to receive patch version (can be NULL)
 */
SZ_API void sz_version_number(int* major, int* minor, int* patch);

/**
 * @brief Check if a format is supported
 *
 * @param format Format to check
 * @return Non-zero if supported, zero otherwise
 */
SZ_API int sz_is_format_supported(sz_format format);

#ifdef __cplusplus
}
#endif

#endif /* SZ_VERSION_H */
