# libsevenzip C API Reference

**Version**: 0.1.0 (Alpha)  
**Last Updated**: 2025-12-10  
**Language**: English | [中文](API_Reference.zh-CN.md)

This document provides a complete reference for the libsevenzip C FFI (Foreign Function Interface) API.

---

## Table of Contents

- [Overview](#overview)
- [Core Types](#core-types)
- [Error Handling](#error-handling)
- [Version Information](#version-information)
- [Archive Operations](#archive-operations)
- [Archive Writing](#archive-writing)
- [Convenience Functions](#convenience-functions)
- [Memory Management](#memory-management)
- [Code Examples](#code-examples)

---

## Overview

The libsevenzip FFI provides a C-compatible API for working with 7-Zip archives. All functions follow these conventions:

- **Prefix**: All functions start with `sz_`
- **Return type**: Most functions return `sz_result` (error code)
- **Naming**: Snake_case (e.g., `sz_archive_open`)
- **Thread safety**: Archive handles are NOT thread-safe; use separate handles per thread

### Header Files

```c
#include <sevenzip/sevenzip_capi.h>
```

This includes all necessary headers:
- `sz_types.h` - Type definitions
- `sz_error.h` - Error codes and handling
- `sz_archive.h` - Archive reading
- `sz_writer.h` - Archive writing
- `sz_convenience.h` - Convenience functions
- `sz_version.h` - Version information

---

## Core Types

### sz_result

```c
typedef int32_t sz_result;
```

Return type for all API functions. See [Error Handling](#error-handling) for error codes.

### sz_archive

```c
typedef struct sz_archive sz_archive;
```

Opaque handle to an open archive. Created by `sz_archive_open()`, freed by `sz_archive_close()`.

### sz_writer

```c
typedef struct sz_writer sz_writer;
```

Opaque handle to an archive writer. Created by `sz_writer_create()`, freed by `sz_writer_free()`.

### sz_format

```c
typedef enum {
    SZ_FORMAT_UNKNOWN = 0,
    SZ_FORMAT_7Z      = 1,    // 7-Zip format
    SZ_FORMAT_ZIP     = 2,    // Zip format
    SZ_FORMAT_TAR     = 3,    // Tar format
    SZ_FORMAT_GZIP    = 4,    // Gzip (.gz)
    SZ_FORMAT_BZIP2   = 5,    // Bzip2 (.bz2)
    SZ_FORMAT_XZ      = 6,    // Xz format
    SZ_FORMAT_RAR     = 7,    // RAR (read-only)
    SZ_FORMAT_RAR5    = 8     // RAR5 (read-only)
} sz_format;
```

### sz_item_info

```c
typedef struct {
    char name[512];              // Item name (UTF-8)
    uint64_t size;               // Uncompressed size in bytes
    uint64_t compressed_size;    // Compressed size in bytes
    uint64_t crc;                // CRC32 checksum
    int is_directory;            // 1 if directory, 0 if file
    uint64_t modified_time;      // Unix timestamp
    uint32_t attributes;         // File attributes
} sz_item_info;
```

---

## Error Handling

### Error Codes

```c
#define SZ_OK                        0    // Success
#define SZ_ERROR_FAIL                1    // General failure
#define SZ_ERROR_INVALID_PARAM       2    // Invalid parameter
#define SZ_ERROR_OUT_OF_MEMORY       3    // Memory allocation failed
#define SZ_ERROR_OPEN_ARCHIVE        4    // Cannot open archive
#define SZ_ERROR_READ_ARCHIVE        5    // Cannot read archive
#define SZ_ERROR_WRITE_FILE          6    // Cannot write file
#define SZ_ERROR_CREATE_DIRECTORY    7    // Cannot create directory
#define SZ_ERROR_UNSUPPORTED_FORMAT  8    // Unsupported format
#define SZ_ERROR_INVALID_ARCHIVE     9    // Invalid archive
#define SZ_ERROR_ITEM_NOT_FOUND     10    // Item not found
#define SZ_ERROR_EXTRACTION_FAILED  11    // Extraction failed
#define SZ_ERROR_COMPRESSION_FAILED 12    // Compression failed
#define SZ_ERROR_PASSWORD_REQUIRED  13    // Password required
#define SZ_ERROR_WRONG_PASSWORD     14    // Wrong password
#define SZ_ERROR_NOT_IMPLEMENTED    15    // Feature not implemented
```

### sz_get_error_message

```c
SZ_API const char* sz_get_error_message(sz_result error_code);
```

**Description**: Get a human-readable error message for an error code.

**Parameters**:
- `error_code`: The error code returned by an API function

**Returns**: Pointer to a static string describing the error. Do not free.

**Example**:
```c
sz_result result = sz_archive_open("test.7z", &archive);
if (result != SZ_OK) {
    printf("Error: %s\n", sz_get_error_message(result));
}
```

### sz_get_last_error

```c
SZ_API sz_result sz_get_last_error(char* buffer, size_t buffer_size);
```

**Description**: Get the last error message with additional context.

**Parameters**:
- `buffer`: Output buffer for error message
- `buffer_size`: Size of buffer in bytes

**Returns**: `SZ_OK` if successful

**Example**:
```c
char error_msg[256];
sz_get_last_error(error_msg, sizeof(error_msg));
printf("Last error: %s\n", error_msg);
```

---

## Version Information

### sz_get_version

```c
SZ_API sz_result sz_get_version(char* buffer, size_t buffer_size);
```

**Description**: Get the 7-Zip SDK version string.

**Parameters**:
- `buffer`: Output buffer for version string
- `buffer_size`: Size of buffer in bytes

**Returns**: `SZ_OK` if successful

**Example**:
```c
char version[64];
sz_get_version(version, sizeof(version));
printf("7-Zip SDK version: %s\n", version);  // "25.01"
```

---

## Archive Operations

### sz_archive_open

```c
SZ_API sz_result sz_archive_open(const char* path, sz_archive** archive);
```

**Description**: Open an archive for reading.

**Parameters**:
- `path`: Path to the archive file (UTF-8)
- `archive`: Output pointer to archive handle

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_OPEN_ARCHIVE` if file cannot be opened
- `SZ_ERROR_INVALID_ARCHIVE` if file is not a valid archive

**Example**:
```c
sz_archive* archive = NULL;
sz_result result = sz_archive_open("test.7z", &archive);
if (result == SZ_OK) {
    // Use archive
    sz_archive_close(archive);
}
```

**Supported Formats**: 7z, Zip, Tar, Gzip, Bzip2, Xz, Rar, Rar5

### sz_archive_close

```c
SZ_API void sz_archive_close(sz_archive* archive);
```

**Description**: Close an archive and free resources.

**Parameters**:
- `archive`: Archive handle (can be NULL)

**Example**:
```c
sz_archive_close(archive);
archive = NULL;  // Good practice
```

### sz_archive_get_item_count

```c
SZ_API sz_result sz_archive_get_item_count(sz_archive* archive, uint32_t* count);
```

**Description**: Get the number of items in the archive.

**Parameters**:
- `archive`: Archive handle
- `count`: Output pointer for item count

**Returns**: `SZ_OK` if successful

**Example**:
```c
uint32_t count;
if (sz_archive_get_item_count(archive, &count) == SZ_OK) {
    printf("Archive contains %u items\n", count);
}
```

### sz_archive_get_format

```c
SZ_API sz_result sz_archive_get_format(sz_archive* archive, char* buffer, size_t buffer_size);
```

**Description**: Get the archive format as a string.

**Parameters**:
- `archive`: Archive handle
- `buffer`: Output buffer for format name
- `buffer_size`: Size of buffer

**Returns**: `SZ_OK` if successful

**Example**:
```c
char format[32];
sz_archive_get_format(archive, format, sizeof(format));
printf("Format: %s\n", format);  // "7z", "zip", "tar", etc.
```

### sz_archive_get_item_info

```c
SZ_API sz_result sz_archive_get_item_info(
    sz_archive* archive, 
    uint32_t index, 
    sz_item_info* info
);
```

**Description**: Get information about an item in the archive.

**Parameters**:
- `archive`: Archive handle
- `index`: Zero-based item index
- `info`: Output structure for item information

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_INVALID_PARAM` if index is out of range

**Example**:
```c
sz_item_info info;
if (sz_archive_get_item_info(archive, 0, &info) == SZ_OK) {
    printf("Name: %s\n", info.name);
    printf("Size: %llu bytes\n", info.size);
    printf("Compressed: %llu bytes\n", info.compressed_size);
    printf("Directory: %s\n", info.is_directory ? "yes" : "no");
}
```

### sz_archive_extract

```c
SZ_API sz_result sz_archive_extract(
    sz_archive* archive, 
    const char* output_dir
);
```

**Description**: Extract all items from the archive.

**Parameters**:
- `archive`: Archive handle
- `output_dir`: Output directory path (created if doesn't exist)

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_CREATE_DIRECTORY` if output directory cannot be created
- `SZ_ERROR_EXTRACTION_FAILED` if extraction fails

**Example**:
```c
sz_result result = sz_archive_extract(archive, "output/");
if (result == SZ_OK) {
    printf("Extraction successful\n");
}
```

### sz_archive_extract_item

```c
SZ_API sz_result sz_archive_extract_item(
    sz_archive* archive, 
    uint32_t index, 
    const char* output_path
);
```

**Description**: Extract a single item from the archive.

**Parameters**:
- `archive`: Archive handle
- `index`: Zero-based item index
- `output_path`: Full path for extracted file

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_ITEM_NOT_FOUND` if index is invalid
- `SZ_ERROR_EXTRACTION_FAILED` if extraction fails

**Example**:
```c
sz_result result = sz_archive_extract_item(archive, 0, "output/file.txt");
if (result == SZ_OK) {
    printf("File extracted\n");
}
```

---

## Archive Writing

### sz_writer_create

```c
SZ_API sz_result sz_writer_create(sz_format format, sz_writer** writer);
```

**Description**: Create a new archive writer.

**Parameters**:
- `format`: Archive format (7z, zip, or tar)
- `writer`: Output pointer to writer handle

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_UNSUPPORTED_FORMAT` if format is not supported for writing

**Example**:
```c
sz_writer* writer = NULL;
sz_result result = sz_writer_create(SZ_FORMAT_7Z, &writer);
if (result == SZ_OK) {
    // Use writer
    sz_writer_free(writer);
}
```

**Supported Formats**: `SZ_FORMAT_7Z`, `SZ_FORMAT_ZIP`, `SZ_FORMAT_TAR`

### sz_writer_free

```c
SZ_API void sz_writer_free(sz_writer* writer);
```

**Description**: Free a writer handle.

**Parameters**:
- `writer`: Writer handle (can be NULL)

**Example**:
```c
sz_writer_free(writer);
writer = NULL;
```

### sz_writer_add_file

```c
SZ_API sz_result sz_writer_add_file(
    sz_writer* writer, 
    const char* file_path,
    const char* archive_path
);
```

**Description**: Add a file to the archive.

**Parameters**:
- `writer`: Writer handle
- `file_path`: Path to the file on disk
- `archive_path`: Path inside the archive (NULL = use filename only)

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_INVALID_PARAM` if file doesn't exist

**Example**:
```c
// Add file with custom archive path
sz_writer_add_file(writer, "data/file.txt", "files/file.txt");

// Add file with automatic naming
sz_writer_add_file(writer, "data/file.txt", NULL);  // Stored as "file.txt"
```

### sz_writer_add_directory

```c
SZ_API sz_result sz_writer_add_directory(
    sz_writer* writer, 
    const char* dir_path,
    const char* archive_path
);
```

**Description**: Add a directory and its contents recursively.

**Parameters**:
- `writer`: Writer handle
- `dir_path`: Path to the directory on disk
- `archive_path`: Path inside the archive (NULL = use directory name)

**Returns**: `SZ_OK` on success

**Example**:
```c
// Add entire directory
sz_writer_add_directory(writer, "my_folder", "backup/my_folder");

// Add directory with automatic naming
sz_writer_add_directory(writer, "my_folder", NULL);
```

### sz_writer_create_archive

```c
SZ_API sz_result sz_writer_create_archive(
    sz_writer* writer, 
    const char* archive_path
);
```

**Description**: Create the archive file with all added items.

**Parameters**:
- `writer`: Writer handle
- `archive_path`: Path for the output archive

**Returns**: 
- `SZ_OK` on success
- `SZ_ERROR_COMPRESSION_FAILED` if archive creation fails

**Example**:
```c
sz_writer* writer;
sz_writer_create(SZ_FORMAT_7Z, &writer);
sz_writer_add_file(writer, "file1.txt", NULL);
sz_writer_add_file(writer, "file2.txt", NULL);
sz_writer_create_archive(writer, "output.7z");
sz_writer_free(writer);
```

---

## Convenience Functions

### sz_extract_simple

```c
SZ_API sz_result sz_extract_simple(const char* archive_path, const char* output_dir);
```

**Description**: Extract an archive in one function call.

**Parameters**:
- `archive_path`: Path to the archive
- `output_dir`: Output directory

**Returns**: `SZ_OK` on success

**Example**:
```c
if (sz_extract_simple("archive.7z", "output/") == SZ_OK) {
    printf("Extraction successful\n");
}
```

### sz_compress_directory

```c
SZ_API sz_result sz_compress_directory(
    const char* dir_path,
    const char* archive_path,
    sz_format format
);
```

**Description**: Compress a directory to an archive in one function call.

**Parameters**:
- `dir_path`: Directory to compress
- `archive_path`: Output archive path
- `format`: Archive format

**Returns**: `SZ_OK` on success

**Example**:
```c
sz_result result = sz_compress_directory("my_folder", "backup.7z", SZ_FORMAT_7Z);
if (result == SZ_OK) {
    printf("Compression successful\n");
}
```

### sz_detect_format

```c
SZ_API sz_result sz_detect_format(const char* path, sz_format* format);
```

**Description**: Detect the format of an archive file.

**Parameters**:
- `path`: Path to the file
- `format`: Output format

**Returns**: `SZ_OK` if format detected

**Example**:
```c
sz_format format;
if (sz_detect_format("unknown.bin", &format) == SZ_OK) {
    printf("Format: %d\n", format);
}
```

### sz_format_to_string

```c
SZ_API const char* sz_format_to_string(sz_format format);
```

**Description**: Convert format enum to string.

**Parameters**:
- `format`: Format enum value

**Returns**: String representation ("7z", "zip", etc.)

**Example**:
```c
printf("Format: %s\n", sz_format_to_string(SZ_FORMAT_7Z));  // "7z"
```

---

## Memory Management

### sz_free_buffer

```c
SZ_API void sz_free_buffer(void* buffer);
```

**Description**: Free memory allocated by FFI functions.

**Parameters**:
- `buffer`: Buffer to free (can be NULL)

**Example**:
```c
// Some FFI functions may return allocated buffers
uint8_t* data = /* from FFI function */;
sz_free_buffer(data);
```

---

## Code Examples

### Extract Archive

```c
#include <sevenzip/sevenzip_capi.h>
#include <stdio.h>

int main() {
    sz_archive* archive = NULL;
    sz_result result;
    
    // Open archive
    result = sz_archive_open("test.7z", &archive);
    if (result != SZ_OK) {
        printf("Error opening archive: %s\n", sz_get_error_message(result));
        return 1;
    }
    
    // Get item count
    uint32_t count;
    sz_archive_get_item_count(archive, &count);
    printf("Archive contains %u items\n", count);
    
    // List all items
    for (uint32_t i = 0; i < count; i++) {
        sz_item_info info;
        sz_archive_get_item_info(archive, i, &info);
        printf("%s (%llu bytes)\n", info.name, info.size);
    }
    
    // Extract all
    result = sz_archive_extract(archive, "output/");
    if (result == SZ_OK) {
        printf("Extraction successful\n");
    }
    
    // Clean up
    sz_archive_close(archive);
    return 0;
}
```

### Create Archive

```c
#include <sevenzip/sevenzip_capi.h>
#include <stdio.h>

int main() {
    sz_writer* writer = NULL;
    sz_result result;
    
    // Create writer
    result = sz_writer_create(SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        printf("Error creating writer\n");
        return 1;
    }
    
    // Add files
    sz_writer_add_file(writer, "file1.txt", NULL);
    sz_writer_add_file(writer, "file2.txt", NULL);
    sz_writer_add_directory(writer, "my_folder", NULL);
    
    // Create archive
    result = sz_writer_create_archive(writer, "output.7z");
    if (result == SZ_OK) {
        printf("Archive created successfully\n");
    } else {
        printf("Error: %s\n", sz_get_error_message(result));
    }
    
    // Clean up
    sz_writer_free(writer);
    return 0;
}
```

---

## Platform Notes

### Windows-Only Limitation

This library currently only supports Windows due to:
- Windows SDK COM interface dependency
- MSVC-specific implementations

### Architecture Support

- **x64 (64-bit)**: Full support, recommended
- **x86 (32-bit)**: Supported, but with limitations:
  - Files larger than 4GB may have issues
  - Different calling conventions (__stdcall for internal COM)

### Thread Safety

- Archive and writer handles are NOT thread-safe
- Use separate handles per thread
- Error messages are stored in thread-local storage

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 0.1.0 | 2025-12-10 | Initial alpha release |

---

## See Also

- [中文 API 文档](API_Reference.zh-CN.md)
- [Contributing Guide](../CONTRIBUTING.md)
- [7-Zip SDK Documentation](https://www.7-zip.org/sdk.html)

---

**Note**: This is an alpha version. APIs may change before the 1.0 release.
