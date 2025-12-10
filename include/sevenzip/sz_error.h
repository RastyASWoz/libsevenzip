/**
 * @file sz_error.h
 * @brief 7-Zip C API - Error Handling
 * @version 1.0.0
 */

#ifndef SZ_ERROR_H
#define SZ_ERROR_H

#include "sz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert error code to human-readable string
 *
 * @param error Error code
 * @return Static string describing the error (never NULL)
 */
SZ_API const char* sz_error_to_string(sz_result error);

/**
 * @brief Get the last error message from the most recent API call
 *
 * @return Error message string, or empty string if no error
 *
 * @note This is thread-local storage. Each thread has its own error message.
 */
SZ_API const char* sz_get_last_error_message(void);

/**
 * @brief Clear the last error message
 */
SZ_API void sz_clear_error(void);

#ifdef __cplusplus
}
#endif

#endif /* SZ_ERROR_H */
