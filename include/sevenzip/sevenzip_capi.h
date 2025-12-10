/**
 * @file sevenzip_capi.h
 * @brief 7-Zip C API - Main Include File
 * @version 1.0.0
 *
 * This is the main header file for the 7-Zip C API.
 * Include this file to access all C API functionality.
 *
 * @example
 * @code
 * #include <sevenzip/sevenzip_capi.h>
 *
 * int main() {
 *     // Simple extraction
 *     sz_result result = sz_extract_simple("archive.7z", "output/");
 *     if (result != SZ_OK) {
 *         printf("Error: %s\n", sz_get_last_error_message());
 *         return 1;
 *     }
 *     return 0;
 * }
 * @endcode
 */

#ifndef SEVENZIP_CAPI_H
#define SEVENZIP_CAPI_H

/* Core types and definitions */
#include "sz_types.h"

/* Error handling */
#include "sz_error.h"

/* Archive reading */
#include "sz_archive.h"

/* Archive writing */
#include "sz_writer.h"

/* Standalone compression - NOT IMPLEMENTED */
/* Use standard libraries (zlib, bzip2, liblzma) for standalone compression */
/* #include "sz_compress.h" */

/* Convenience functions */
#include "sz_convenience.h"

/* Version information */
#include "sz_version.h"

#endif /* SEVENZIP_CAPI_H */
