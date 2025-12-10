/**
 * @file sz_types.h
 * @brief 7-Zip C API - Basic Type Definitions
 * @version 1.0.0
 *
 * This file defines all basic types, enums, and structures used in the C API.
 */

#ifndef SZ_TYPES_H
#define SZ_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Version Information
 * ========================================================================= */

#define SZ_VERSION_MAJOR 1
#define SZ_VERSION_MINOR 0
#define SZ_VERSION_PATCH 0

/* ============================================================================
 * Export/Import Macros
 * ========================================================================= */

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef SEVENZIP_STATIC
#define SZ_API
#elif defined(SEVENZIP_EXPORTS)
#define SZ_API __declspec(dllexport)
#else
#define SZ_API __declspec(dllimport)
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define SZ_API __attribute__((visibility("default")))
#else
#define SZ_API
#endif
#endif

/* ============================================================================
 * Opaque Handle Types
 * ========================================================================= */

/** Opaque handle to an archive reader */
typedef struct sz_archive_s* sz_archive_handle;

/** Opaque handle to an archive writer */
typedef struct sz_writer_s* sz_writer_handle;

/** Opaque handle to a compressor */
typedef struct sz_compressor_s* sz_compressor_handle;

/* ============================================================================
 * Result Code
 * ========================================================================= */

/**
 * @brief Result codes for all API functions
 */
typedef enum sz_result {
    SZ_OK = 0,                    /**< Success */
    SZ_E_FAIL = 1,                /**< General failure */
    SZ_E_OUT_OF_MEMORY = 2,       /**< Out of memory */
    SZ_E_FILE_NOT_FOUND = 3,      /**< File not found */
    SZ_E_ACCESS_DENIED = 4,       /**< Access denied */
    SZ_E_INVALID_ARGUMENT = 5,    /**< Invalid argument (NULL pointer, etc.) */
    SZ_E_UNSUPPORTED_FORMAT = 6,  /**< Unsupported archive format */
    SZ_E_CORRUPTED_ARCHIVE = 7,   /**< Archive is corrupted */
    SZ_E_WRONG_PASSWORD = 8,      /**< Wrong password */
    SZ_E_CANCELLED = 9,           /**< Operation cancelled by user */
    SZ_E_INDEX_OUT_OF_RANGE = 10, /**< Index out of range */
    SZ_E_ALREADY_OPEN = 11,       /**< Archive already open */
    SZ_E_NOT_OPEN = 12,           /**< Archive not open */
    SZ_E_WRITE_ERROR = 13,        /**< Write error */
    SZ_E_READ_ERROR = 14,         /**< Read error */
    SZ_E_NOT_IMPLEMENTED = 15,    /**< Feature not implemented */
    SZ_E_DISK_FULL = 16           /**< Disk full */
} sz_result;

/* ============================================================================
 * Archive Format
 * ========================================================================= */

/**
 * @brief Archive format types
 */
typedef enum sz_format {
    SZ_FORMAT_AUTO = 0,  /**< Auto-detect (open only) */
    SZ_FORMAT_7Z = 1,    /**< 7z format */
    SZ_FORMAT_ZIP = 2,   /**< ZIP format */
    SZ_FORMAT_TAR = 3,   /**< TAR format */
    SZ_FORMAT_GZIP = 4,  /**< GZIP format */
    SZ_FORMAT_BZIP2 = 5, /**< BZIP2 format */
    SZ_FORMAT_XZ = 6     /**< XZ format */
} sz_format;

/* ============================================================================
 * Compression Level
 * ========================================================================= */

/**
 * @brief Compression level
 */
typedef enum sz_compression_level {
    SZ_LEVEL_NONE = 0,    /**< No compression (store only) */
    SZ_LEVEL_FAST = 1,    /**< Fast compression */
    SZ_LEVEL_NORMAL = 5,  /**< Normal compression */
    SZ_LEVEL_MAXIMUM = 7, /**< Maximum compression */
    SZ_LEVEL_ULTRA = 9    /**< Ultra compression */
} sz_compression_level;

/* ============================================================================
 * Archive Information
 * ========================================================================= */

/**
 * @brief Archive metadata information
 */
typedef struct sz_archive_info {
    sz_format format;          /**< Archive format */
    size_t item_count;         /**< Number of items in archive */
    uint64_t total_size;       /**< Total uncompressed size (bytes) */
    uint64_t packed_size;      /**< Total compressed size (bytes) */
    int is_solid;              /**< Non-zero if solid archive */
    int is_multi_volume;       /**< Non-zero if multi-volume archive */
    int has_encrypted_headers; /**< Non-zero if headers are encrypted */
} sz_archive_info;

/* ============================================================================
 * Item Information
 * ========================================================================= */

/**
 * @brief Information about a single item in an archive
 *
 * @note Strings are allocated by the library and must be freed with
 *       sz_string_free() when no longer needed.
 */
typedef struct sz_item_info {
    size_t index;              /**< Item index in archive */
    char* path;                /**< Item path (UTF-8, needs sz_string_free) */
    uint64_t size;             /**< Uncompressed size (bytes) */
    uint64_t packed_size;      /**< Compressed size (bytes) */
    uint32_t crc;              /**< CRC32 checksum */
    int has_crc;               /**< Non-zero if CRC is valid */
    int64_t creation_time;     /**< Creation time (Unix timestamp) */
    int64_t modification_time; /**< Modification time (Unix timestamp) */
    int is_directory;          /**< Non-zero if this is a directory */
    int is_encrypted;          /**< Non-zero if this item is encrypted */
} sz_item_info;

/* ============================================================================
 * Callback Types
 * ========================================================================= */

/**
 * @brief Progress callback function type
 *
 * @param completed Number of bytes processed so far
 * @param total Total number of bytes to process
 * @param user_data User-provided context pointer
 * @return Non-zero to continue, zero to cancel operation
 */
typedef int (*sz_progress_callback)(uint64_t completed, uint64_t total, void* user_data);

/**
 * @brief Password callback function type
 *
 * @param user_data User-provided context pointer
 * @return Password string (UTF-8), or NULL to cancel
 *
 * @note The returned string must remain valid until the next API call
 */
typedef const char* (*sz_password_callback)(void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* SZ_TYPES_H */
