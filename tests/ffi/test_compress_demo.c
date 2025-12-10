// test_compress_demo.c - Demo of compression/decompression functionality
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sevenzip/sevenzip_capi.h"

int main(void) {
    printf("=== SevenZip FFI Compression Demo ===\n\n");

    // Test data
    const char* original_text =
        "The quick brown fox jumps over the lazy dog. "
        "This is a test of the SevenZip compression library. "
        "Repeat: The quick brown fox jumps over the lazy dog.";
    size_t original_size = strlen(original_text);

    printf("Original text (%zu bytes):\n%s\n\n", original_size, original_text);

    // Create compressor
    sz_compressor_handle compressor = NULL;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_MAXIMUM, &compressor);

    if (result != SZ_OK) {
        printf("Failed to create compressor: %s\n", sz_error_to_string(result));
        return 1;
    }
    printf("✓ Compressor created (GZIP, MAXIMUM level)\n");

    // Compress data
    void* compressed_data = NULL;
    size_t compressed_size = 0;
    result = sz_compress_data(compressor, original_text, original_size, &compressed_data,
                              &compressed_size);

    if (result != SZ_OK) {
        printf("Failed to compress: %s\n", sz_error_to_string(result));
        const char* error = sz_get_last_error_message();
        if (error && *error) {
            printf("  Details: %s\n", error);
        }
        sz_compressor_destroy(compressor);
        return 1;
    }
    printf("✓ Data compressed: %zu bytes -> %zu bytes (%.1f%% ratio)\n", original_size,
           compressed_size, 100.0 * compressed_size / original_size);

    // Decompress data
    void* decompressed_data = NULL;
    size_t decompressed_size = 0;
    result = sz_decompress_data(compressor, compressed_data, compressed_size, &decompressed_data,
                                &decompressed_size);

    if (result != SZ_OK) {
        printf("Failed to decompress: %s\n", sz_error_to_string(result));
        sz_memory_free(compressed_data);
        sz_compressor_destroy(compressor);
        return 1;
    }
    printf("✓ Data decompressed: %zu bytes\n", decompressed_size);

    // Verify result
    if (decompressed_size == original_size &&
        memcmp(decompressed_data, original_text, original_size) == 0) {
        printf("✓ Decompressed data matches original!\n\n");
        printf("Decompressed text:\n%.*s\n", (int)decompressed_size, (char*)decompressed_data);
    } else {
        printf("✗ Decompressed data does NOT match original!\n");
        sz_memory_free(decompressed_data);
        sz_memory_free(compressed_data);
        sz_compressor_destroy(compressor);
        return 1;
    }

    // Cleanup
    sz_memory_free(decompressed_data);
    sz_memory_free(compressed_data);
    sz_compressor_destroy(compressor);

    printf("\n=== Success! ===\n");
    return 0;
}
