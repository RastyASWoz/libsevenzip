// example_basic_usage.c - Basic usage examples
// NOTE: Standalone compression (Compressor API) has been removed.
// This example now focuses on archive operations.
// For standalone compression, use standard libraries (see docs/Standalone_Compression_Guide.md)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sevenzip/sevenzip_capi.h"

void print_separator(void) {
    printf("\n%s\n", "========================================");
}

// Example 1: Get version information
void example_version_info(void) {
    print_separator();
    printf("Example 1: Version Information\n");
    print_separator();

    const char* version = sz_version_string();
    printf("Library version: %s\n", version);

    int major, minor, patch;
    sz_version_number(&major, &minor, &patch);
    printf("Version number: %d.%d.%d\n", major, minor, patch);

    printf("\nSupported archive formats:\n");
    const char* formats[] = {"7Z", "ZIP", "TAR"};
    sz_format format_codes[] = {SZ_FORMAT_7Z, SZ_FORMAT_ZIP, SZ_FORMAT_TAR};

    for (int i = 0; i < 3; i++) {
        int supported = sz_is_format_supported(format_codes[i]);
        printf("  %s: %s\n", formats[i], supported ? "Yes" : "No");
    }
}

// Example 2: Create archive
void example_create_archive(void) {
    print_separator();
    printf("Example 2: Create Archive\n");
    print_separator();

    printf("\nCreating a 7z archive...\n");
    printf("See convenience_demo.cpp for full archive creation examples.\n");
    printf("Use sz_create_archive() or sz_writer_create() functions.\n");
}

// Example 3: List archive contents
void example_list_archive(void) {
    print_separator();
    printf("Example 3: List Archive Contents\n");
    print_separator();

    printf("\nListing archive files...\n");
    printf("See convenience_demo.cpp for archive listing examples.\n");
    printf("Use sz_list_archive_files() function.\n");
}

// Example 4: Extract archive
void example_extract_archive(void) {
    print_separator();
    printf("Example 4: Extract Archive\n");
    print_separator();

    printf("\nExtracting archive...\n");
    printf("See convenience_demo.cpp for extraction examples.\n");
    printf("Use sz_extract_archive() function.\n");
}

// Example 5: Error handling
void example_error_handling(void) {
    print_separator();
    printf("Example 5: Error Handling\n");
    print_separator();

    printf("\nDemonstrating error handling:\n\n");

    // Try to open non-existent file
    sz_archive_handle archive;
    sz_result result = sz_archive_open("nonexistent.7z", &archive);

    if (result != SZ_OK) {
        printf("Expected error: %s\n", sz_error_to_string(result));
        const char* details = sz_get_last_error_message();
        if (details && *details) {
            printf("Error details: %s\n", details);
        }
    }

    // Try invalid arguments
    result = sz_archive_open(NULL, &archive);
    printf("\nNull argument error: %s\n", sz_error_to_string(result));

    // Clear error
    sz_clear_error();
    const char* msg = sz_get_last_error_message();
    printf("After clearing: \"%s\" (empty)\n", msg ? msg : "");
}

int main(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║  SevenZip C API - Basic Examples      ║\n");
    printf("║  (Archive Operations Only)             ║\n");
    printf("╚════════════════════════════════════════╝\n");

    example_version_info();
    example_create_archive();
    example_list_archive();
    example_extract_archive();
    example_error_handling();

    print_separator();
    printf("\n✓ All examples completed!\n");
    printf("\nNote: For working archive examples, see:\n");
    printf("  - convenience_demo.cpp (C++ API)\n");
    printf("  - examples/python/sevenzip.py (Python bindings)\n");
    printf("\nFor standalone compression (GZIP/BZIP2), see:\n");
    printf("  - docs/Standalone_Compression_Guide.md\n\n");

    return 0;
}
