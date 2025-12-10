// example_basic_usage.c - Basic usage examples
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

    printf("\nSupported formats:\n");
    const char* formats[] = {"7Z", "ZIP", "TAR", "GZIP", "BZIP2", "XZ"};
    sz_format format_codes[] = {SZ_FORMAT_7Z,   SZ_FORMAT_ZIP,   SZ_FORMAT_TAR,
                                SZ_FORMAT_GZIP, SZ_FORMAT_BZIP2, SZ_FORMAT_XZ};

    for (int i = 0; i < 6; i++) {
        int supported = sz_is_format_supported(format_codes[i]);
        printf("  %s: %s\n", formats[i], supported ? "✓" : "✗");
    }
}

// Example 2: Compress data in memory
void example_memory_compression(void) {
    print_separator();
    printf("Example 2: Memory Compression\n");
    print_separator();

    const char* text = "Hello, World! This is a test of memory compression.";
    size_t text_len = strlen(text);

    printf("\nOriginal text (%zu bytes):\n\"%s\"\n", text_len, text);

    // Create compressor
    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_FAST, &compressor);
    if (result != SZ_OK) {
        printf("Error creating compressor: %s\n", sz_error_to_string(result));
        return;
    }

    // Compress
    void* compressed = NULL;
    size_t compressed_size = 0;
    result = sz_compress_data(compressor, text, text_len, &compressed, &compressed_size);
    if (result != SZ_OK) {
        printf("Error compressing: %s\n", sz_error_to_string(result));
        sz_compressor_destroy(compressor);
        return;
    }

    printf("\nCompressed to %zu bytes (%.1f%% of original)\n", compressed_size,
           100.0 * compressed_size / text_len);

    // Decompress
    void* decompressed = NULL;
    size_t decompressed_size = 0;
    result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                &decompressed_size);
    if (result != SZ_OK) {
        printf("Error decompressing: %s\n", sz_error_to_string(result));
        sz_memory_free(compressed);
        sz_compressor_destroy(compressor);
        return;
    }

    printf("Decompressed to %zu bytes\n", decompressed_size);

    // Verify
    if (decompressed_size == text_len && memcmp(decompressed, text, text_len) == 0) {
        printf("✓ Data verified successfully!\n");
    } else {
        printf("✗ Data verification failed!\n");
    }

    // Cleanup
    sz_memory_free(compressed);
    sz_memory_free(decompressed);
    sz_compressor_destroy(compressor);
}

// Example 3: Compress a file
void example_file_compression(void) {
    print_separator();
    printf("Example 3: File Compression\n");
    print_separator();

    const char* input = "tests/data/sample/test.txt";
    const char* output = "tests/data/example_output.txt.bz2";

    printf("\nCompressing: %s\n", input);
    printf("Output: %s\n", output);

    // Create compressor with BZip2 format
    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_BZIP2, SZ_LEVEL_MAXIMUM, &compressor);
    if (result != SZ_OK) {
        printf("Error creating compressor: %s\n", sz_error_to_string(result));
        return;
    }

    // Compress file
    result = sz_compress_file(compressor, input, output);
    if (result != SZ_OK) {
        printf("Error compressing file: %s\n", sz_error_to_string(result));
        const char* error = sz_get_last_error_message();
        if (error && *error) {
            printf("Details: %s\n", error);
        }
        sz_compressor_destroy(compressor);
        return;
    }

    printf("✓ File compressed successfully!\n");

    sz_compressor_destroy(compressor);
}

// Example 4: Compression levels comparison
void example_compression_levels(void) {
    print_separator();
    printf("Example 4: Compression Levels\n");
    print_separator();

    // Create test data
    const char* text =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog.";
    size_t text_len = strlen(text);

    printf("\nOriginal data: %zu bytes\n", text_len);
    printf("Comparing compression levels with GZIP:\n\n");

    sz_compression_level levels[] = {SZ_LEVEL_FAST, SZ_LEVEL_NORMAL, SZ_LEVEL_MAXIMUM,
                                     SZ_LEVEL_ULTRA};
    const char* level_names[] = {"FAST", "NORMAL", "MAXIMUM", "ULTRA"};

    for (int i = 0; i < 4; i++) {
        sz_compressor_handle compressor;
        sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, levels[i], &compressor);
        if (result != SZ_OK) continue;

        void* compressed = NULL;
        size_t compressed_size = 0;
        result = sz_compress_data(compressor, text, text_len, &compressed, &compressed_size);

        if (result == SZ_OK) {
            printf("  %s: %zu bytes (%.1f%%)\n", level_names[i], compressed_size,
                   100.0 * compressed_size / text_len);
            sz_memory_free(compressed);
        }

        sz_compressor_destroy(compressor);
    }
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
    printf("After clearing: \"%s\" (empty)\n", msg);
}

int main(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║  SevenZip C API - Usage Examples      ║\n");
    printf("╚════════════════════════════════════════╝\n");

    example_version_info();
    example_memory_compression();
    example_file_compression();
    example_compression_levels();
    example_error_handling();

    print_separator();
    printf("\n✓ All examples completed!\n\n");

    return 0;
}
