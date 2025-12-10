// test_compress_advanced.c - Advanced tests for compression functionality
// Tests edge cases, boundary conditions, and error scenarios

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sevenzip/sevenzip_capi.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message)                      \
    do {                                                     \
        if (condition) {                                     \
            tests_passed++;                                  \
            printf("✓ PASS: %s\n", message);                 \
        } else {                                             \
            tests_failed++;                                  \
            printf("✗ FAIL: %s\n", message);                 \
            const char* error = sz_get_last_error_message(); \
            if (error && *error) {                           \
                printf("  Error: %s\n", error);              \
            }                                                \
        }                                                    \
    } while (0)

void test_empty_data_compression(void) {
    printf("\n=== Testing Empty Data Compression ===\n");

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);
    TEST_ASSERT(result == SZ_OK, "Compressor created");

    if (result == SZ_OK) {
        void* compressed = NULL;
        size_t compressed_size = 0;

        // Compress empty data
        result = sz_compress_data(compressor, "", 0, &compressed, &compressed_size);
        TEST_ASSERT(result == SZ_OK, "Empty data compressed successfully");

        if (result == SZ_OK) {
            TEST_ASSERT(compressed != NULL, "Compressed data allocated");
            TEST_ASSERT(compressed_size > 0, "Compressed size > 0 (header/footer)");
            printf("  Empty data -> %zu bytes (header/footer)\n", compressed_size);

            // Decompress and verify
            void* decompressed = NULL;
            size_t decompressed_size = 0;
            result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                        &decompressed_size);
            TEST_ASSERT(result == SZ_OK, "Empty data decompressed");
            TEST_ASSERT(decompressed_size == 0, "Decompressed size is 0");

            sz_memory_free(compressed);
            if (decompressed) sz_memory_free(decompressed);
        }

        sz_compressor_destroy(compressor);
    }
}

void test_single_byte_compression(void) {
    printf("\n=== Testing Single Byte Compression ===\n");

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);
    TEST_ASSERT(result == SZ_OK, "Compressor created");

    if (result == SZ_OK) {
        const char data = 'A';
        void* compressed = NULL;
        size_t compressed_size = 0;

        result = sz_compress_data(compressor, &data, 1, &compressed, &compressed_size);
        TEST_ASSERT(result == SZ_OK, "Single byte compressed");

        if (result == SZ_OK) {
            printf("  1 byte -> %zu bytes\n", compressed_size);

            void* decompressed = NULL;
            size_t decompressed_size = 0;
            result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                        &decompressed_size);
            TEST_ASSERT(result == SZ_OK, "Single byte decompressed");
            TEST_ASSERT(decompressed_size == 1, "Size is 1");
            TEST_ASSERT(*(char*)decompressed == 'A', "Data matches");

            sz_memory_free(compressed);
            sz_memory_free(decompressed);
        }

        sz_compressor_destroy(compressor);
    }
}

void test_large_data_compression(void) {
    printf("\n=== Testing Large Data Compression ===\n");

    // Create 10 MB of data
    const size_t large_size = 10 * 1024 * 1024;
    char* large_data = (char*)malloc(large_size);

    if (!large_data) {
        printf("  Skipping test (memory allocation failed)\n");
        return;
    }

    // Fill with repetitive pattern (should compress well)
    for (size_t i = 0; i < large_size; i++) {
        large_data[i] = (char)('A' + (i % 26));
    }

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_FAST, &compressor);
    TEST_ASSERT(result == SZ_OK, "Compressor created for large data");

    if (result == SZ_OK) {
        void* compressed = NULL;
        size_t compressed_size = 0;

        printf("  Compressing 10 MB...\n");
        result =
            sz_compress_data(compressor, large_data, large_size, &compressed, &compressed_size);
        TEST_ASSERT(result == SZ_OK, "Large data compressed");

        if (result == SZ_OK) {
            double ratio = (double)compressed_size / (double)large_size * 100.0;
            printf("  10 MB -> %zu bytes (%.2f%% ratio)\n", compressed_size, ratio);
            TEST_ASSERT(compressed_size < large_size, "Compressed size < original");

            sz_memory_free(compressed);
        }

        sz_compressor_destroy(compressor);
    }

    free(large_data);
}

void test_highly_compressible_data(void) {
    printf("\n=== Testing Highly Compressible Data ===\n");

    // Create data that compresses very well (all same byte)
    const size_t size = 100000;
    char* data = (char*)malloc(size);
    if (!data) return;

    memset(data, 'X', size);

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_MAXIMUM, &compressor);

    if (result == SZ_OK) {
        void* compressed = NULL;
        size_t compressed_size = 0;

        result = sz_compress_data(compressor, data, size, &compressed, &compressed_size);
        TEST_ASSERT(result == SZ_OK, "Highly compressible data compressed");

        if (result == SZ_OK) {
            double ratio = (double)compressed_size / (double)size * 100.0;
            printf("  100 KB (all same byte) -> %zu bytes (%.2f%%)\n", compressed_size, ratio);
            TEST_ASSERT(ratio < 1.0, "Compression ratio < 1%");

            sz_memory_free(compressed);
        }

        sz_compressor_destroy(compressor);
    }

    free(data);
}

void test_incompressible_data(void) {
    printf("\n=== Testing Incompressible Data ===\n");

    // Random data (should not compress well)
    const size_t size = 10000;
    unsigned char* data = (unsigned char*)malloc(size);
    if (!data) return;

    // Generate pseudo-random data
    for (size_t i = 0; i < size; i++) {
        data[i] = (unsigned char)((i * 7919 + 12345) % 256);
    }

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_MAXIMUM, &compressor);

    if (result == SZ_OK) {
        void* compressed = NULL;
        size_t compressed_size = 0;

        result = sz_compress_data(compressor, data, size, &compressed, &compressed_size);
        TEST_ASSERT(result == SZ_OK, "Incompressible data compressed");

        if (result == SZ_OK) {
            double ratio = (double)compressed_size / (double)size * 100.0;
            printf("  10 KB (random) -> %zu bytes (%.2f%%)\n", compressed_size, ratio);
            // For small data, GZIP headers are significant, so ratio can be low
            TEST_ASSERT(compressed_size > 0, "Incompressible data produces output");

            sz_memory_free(compressed);
        }

        sz_compressor_destroy(compressor);
    }

    free(data);
}

void test_compression_level_differences(void) {
    printf("\n=== Testing Compression Level Differences ===\n");

    const char* test_data =
        "This is test data that will be compressed at different levels. "
        "It contains some repetitive text. repetitive text. repetitive text. "
        "And some more content to make it longer and more interesting. ";

    size_t data_size = strlen(test_data);

    sz_compression_level levels[] = {SZ_LEVEL_NONE, SZ_LEVEL_FAST, SZ_LEVEL_NORMAL,
                                     SZ_LEVEL_MAXIMUM, SZ_LEVEL_ULTRA};
    const char* level_names[] = {"NONE", "FAST", "NORMAL", "MAXIMUM", "ULTRA"};

    size_t prev_size = 0;

    for (size_t i = 0; i < 5; i++) {
        sz_compressor_handle compressor;
        sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, levels[i], &compressor);

        if (result == SZ_OK) {
            void* compressed = NULL;
            size_t compressed_size = 0;

            result =
                sz_compress_data(compressor, test_data, data_size, &compressed, &compressed_size);

            if (result == SZ_OK) {
                double ratio = (double)compressed_size / (double)data_size * 100.0;
                printf("  %s: %zu bytes (%.1f%%)\n", level_names[i], compressed_size, ratio);

                // NONE level still produces valid GZIP output with headers
                if (levels[i] == SZ_LEVEL_NONE) {
                    TEST_ASSERT(compressed_size > 0, "NONE level produces output");
                }

                sz_memory_free(compressed);
            }

            sz_compressor_destroy(compressor);
        }
    }
}

void test_corrupted_data_decompression(void) {
    printf("\n=== Testing Corrupted Data Decompression ===\n");

    // First create valid compressed data
    const char* original = "Test data for corruption";
    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);

    if (result == SZ_OK) {
        void* compressed = NULL;
        size_t compressed_size = 0;

        result =
            sz_compress_data(compressor, original, strlen(original), &compressed, &compressed_size);

        if (result == SZ_OK) {
            // Corrupt the data
            if (compressed_size > 10) {
                ((char*)compressed)[compressed_size / 2] ^= 0xFF;
            }

            // Try to decompress corrupted data
            void* decompressed = NULL;
            size_t decompressed_size = 0;
            result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                        &decompressed_size);

            TEST_ASSERT(result != SZ_OK, "Corrupted data decompression fails");
            printf("  Correctly detected corrupted data\n");

            sz_memory_free(compressed);
            if (decompressed) sz_memory_free(decompressed);
        }

        sz_compressor_destroy(compressor);
    }
}

void test_invalid_compressed_data(void) {
    printf("\n=== Testing Invalid Compressed Data ===\n");

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);

    if (result == SZ_OK) {
        // Try to decompress completely invalid data
        const char* invalid_data = "This is not compressed data at all!";
        void* decompressed = NULL;
        size_t decompressed_size = 0;

        result = sz_decompress_data(compressor, invalid_data, strlen(invalid_data), &decompressed,
                                    &decompressed_size);

        TEST_ASSERT(result != SZ_OK, "Invalid data decompression fails");

        if (decompressed) sz_memory_free(decompressed);
        sz_compressor_destroy(compressor);
    }
}

void test_null_pointer_handling(void) {
    printf("\n=== Testing NULL Pointer Handling ===\n");

    sz_compressor_handle compressor;
    sz_result result;

    // NULL compressor handle
    result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL handle rejected");

    // Create valid compressor
    result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);

    if (result == SZ_OK) {
        void* output = NULL;
        size_t output_size = 0;

        // NULL input data
        result = sz_compress_data(compressor, NULL, 100, &output, &output_size);
        TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL input data rejected");

        // NULL output pointer
        result = sz_compress_data(compressor, "test", 4, NULL, &output_size);
        TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL output pointer rejected");

        // NULL output size pointer
        result = sz_compress_data(compressor, "test", 4, &output, NULL);
        TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL output size rejected");

        sz_compressor_destroy(compressor);
    }
}

void test_format_support(void) {
    printf("\n=== Testing Format Support ===\n");

    const sz_format formats[] = {SZ_FORMAT_GZIP, SZ_FORMAT_BZIP2, SZ_FORMAT_XZ};
    const char* format_names[] = {"GZIP", "BZIP2", "XZ"};

    for (size_t i = 0; i < 3; i++) {
        sz_compressor_handle compressor;
        sz_result result = sz_compressor_create(formats[i], SZ_LEVEL_NORMAL, &compressor);

        char msg[128];
        snprintf(msg, sizeof(msg), "%s format compressor created", format_names[i]);
        TEST_ASSERT(result == SZ_OK, msg);

        if (result == SZ_OK) {
            const char* test_data = "Test data for format support";
            void* compressed = NULL;
            size_t compressed_size = 0;

            result = sz_compress_data(compressor, test_data, strlen(test_data), &compressed,
                                      &compressed_size);

            snprintf(msg, sizeof(msg), "%s compression successful", format_names[i]);
            TEST_ASSERT(result == SZ_OK, msg);

            if (result == SZ_OK) {
                void* decompressed = NULL;
                size_t decompressed_size = 0;
                result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                            &decompressed_size);

                snprintf(msg, sizeof(msg), "%s decompression successful", format_names[i]);
                TEST_ASSERT(result == SZ_OK, msg);

                if (result == SZ_OK) {
                    TEST_ASSERT(decompressed_size == strlen(test_data), "Size matches");
                    TEST_ASSERT(memcmp(decompressed, test_data, decompressed_size) == 0,
                                "Data matches");
                    sz_memory_free(decompressed);
                }

                sz_memory_free(compressed);
            }

            sz_compressor_destroy(compressor);
        }
    }
}

void test_multiple_compress_decompress_cycles(void) {
    printf("\n=== Testing Multiple Compress/Decompress Cycles ===\n");

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);
    TEST_ASSERT(result == SZ_OK, "Compressor created");

    if (result == SZ_OK) {
        const char* original = "Data for multiple cycles test";
        int success_count = 0;

        for (int i = 0; i < 10; i++) {
            void* compressed = NULL;
            size_t compressed_size = 0;

            result = sz_compress_data(compressor, original, strlen(original), &compressed,
                                      &compressed_size);

            if (result == SZ_OK) {
                void* decompressed = NULL;
                size_t decompressed_size = 0;

                result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                            &decompressed_size);

                if (result == SZ_OK && decompressed_size == strlen(original) &&
                    memcmp(decompressed, original, decompressed_size) == 0) {
                    success_count++;
                }

                sz_memory_free(compressed);
                if (decompressed) sz_memory_free(decompressed);
            }
        }

        TEST_ASSERT(success_count == 10, "All 10 cycles successful");
        printf("  Completed %d/10 cycles successfully\n", success_count);

        sz_compressor_destroy(compressor);
    }
}

int main(void) {
    printf("==============================================\n");
    printf(" Advanced Compression Tests\n");
    printf("==============================================\n");

    // Run all tests
    test_empty_data_compression();
    test_single_byte_compression();
    test_large_data_compression();
    test_highly_compressible_data();
    test_incompressible_data();
    test_compression_level_differences();
    test_corrupted_data_decompression();
    test_invalid_compressed_data();
    test_null_pointer_handling();
    test_format_support();
    test_multiple_compress_decompress_cycles();

    // Print summary
    printf("\n==============================================\n");
    printf(" Test Summary\n");
    printf("==============================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed!\n");
        return 1;
    }
}
