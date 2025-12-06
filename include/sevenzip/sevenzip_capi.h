#ifndef SEVENZIP_CAPI_H
#define SEVENZIP_CAPI_H

/**
 * @file sevenzip_capi.h
 * @brief C API for libsevenzip - FFI friendly interface
 */

#ifdef __cplusplus
extern "C" {
#endif

#define SEVENZIP_API_VERSION 1

/* Error codes will be defined here */
typedef int sz_result_t;

#define SZ_OK 0
#define SZ_ERROR_UNKNOWN -1

/* Opaque handles will be defined here */

#ifdef __cplusplus
}
#endif

#endif /* SEVENZIP_CAPI_H */
