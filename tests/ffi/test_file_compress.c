// test_file_compress.c - Test file compression/decompression
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sevenzip/sevenzip_capi.h"

int main(int argc, char* argv[]) {
    printf("=== File Compression/Decompression Test ===\n\n");

    // Test files
    const char* test_input = "tests/data/sample/test.txt";
    const char* compressed_file = "tests/data/test.txt.gz";
    const char* decompressed_file = "tests/data/test_decompressed.txt";

    // 1. Compress file
    printf("Step 1: Compressing file...\n");
    sz_compressor_handle compressor = NULL;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);

    if (result != SZ_OK) {
        printf("✗ Failed to create compressor: %s\n", sz_error_to_string(result));
        return 1;
    }
    printf("✓ Compressor created\n");

    result = sz_compress_file(compressor, test_input, compressed_file);
    if (result != SZ_OK) {
        printf("✗ Failed to compress file: %s\n", sz_error_to_string(result));
        const char* error = sz_get_last_error_message();
        if (error && *error) {
            printf("  Details: %s\n", error);
        }
        sz_compressor_destroy(compressor);
        return 1;
    }
    printf("✓ File compressed: %s -> %s\n\n", test_input, compressed_file);

    // 2. Decompress file
    printf("Step 2: Decompressing file...\n");
    result = sz_decompress_file(compressor, compressed_file, decompressed_file);
    if (result != SZ_OK) {
        printf("✗ Failed to decompress file: %s\n", sz_error_to_string(result));
        const char* error = sz_get_last_error_message();
        if (error && *error) {
            printf("  Details: %s\n", error);
        }
        sz_compressor_destroy(compressor);
        return 1;
    }
    printf("✓ File decompressed: %s -> %s\n\n", compressed_file, decompressed_file);

    // 3. Verify files match
    printf("Step 3: Verifying file content...\n");

    FILE* f1 = fopen(test_input, "rb");
    FILE* f2 = fopen(decompressed_file, "rb");

    if (!f1 || !f2) {
        printf("✗ Failed to open files for comparison\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        sz_compressor_destroy(compressor);
        return 1;
    }

    // Read both files
    fseek(f1, 0, SEEK_END);
    long size1 = ftell(f1);
    fseek(f1, 0, SEEK_SET);

    fseek(f2, 0, SEEK_END);
    long size2 = ftell(f2);
    fseek(f2, 0, SEEK_SET);

    if (size1 != size2) {
        printf("✗ File sizes don't match: %ld vs %ld\n", size1, size2);
        fclose(f1);
        fclose(f2);
        sz_compressor_destroy(compressor);
        return 1;
    }

    char* buf1 = (char*)malloc(size1);
    char* buf2 = (char*)malloc(size2);

    fread(buf1, 1, size1, f1);
    fread(buf2, 1, size2, f2);

    fclose(f1);
    fclose(f2);

    if (memcmp(buf1, buf2, size1) == 0) {
        printf("✓ Files match! (%ld bytes)\n", size1);
        free(buf1);
        free(buf2);
    } else {
        printf("✗ File contents don't match!\n");
        free(buf1);
        free(buf2);
        sz_compressor_destroy(compressor);
        return 1;
    }

    sz_compressor_destroy(compressor);

    printf("\n=== All tests passed! ===\n");
    return 0;
}
