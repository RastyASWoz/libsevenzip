// test_archive_c.c - C language tests for archive operations
// Tests the C ABI to ensure it works correctly from pure C code

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

// Progress callback for testing
int progress_callback(uint64_t completed, uint64_t total, void* user_data) {
    int* call_count = (int*)user_data;
    (*call_count)++;
    return 1;  // Continue
}

void test_version_info(void) {
    printf("\n=== Testing Version Info ===\n");

    const char* version = sz_version_string();
    TEST_ASSERT(version != NULL, "sz_version_string returns non-NULL");
    TEST_ASSERT(strlen(version) > 0, "Version string is not empty");
    printf("  Version: %s\n", version);

    int major = 0, minor = 0, patch = 0;
    sz_version_number(&major, &minor, &patch);
    TEST_ASSERT(major > 0, "sz_version_number returns valid major version");
    printf("  Version number: %d.%d.%d\n", major, minor, patch);

    TEST_ASSERT(sz_is_format_supported(SZ_FORMAT_7Z), "7z format is supported");
    TEST_ASSERT(sz_is_format_supported(SZ_FORMAT_ZIP), "ZIP format is supported");
    TEST_ASSERT(sz_is_format_supported(SZ_FORMAT_TAR), "TAR format is supported");
}

void test_error_handling(void) {
    printf("\n=== Testing Error Handling ===\n");

    const char* error_msg = sz_error_to_string(SZ_OK);
    TEST_ASSERT(strcmp(error_msg, "Success") == 0, "SZ_OK translates to 'Success'");

    error_msg = sz_error_to_string(SZ_E_INVALID_ARGUMENT);
    TEST_ASSERT(strstr(error_msg, "Invalid") != NULL, "Invalid argument error message");

    error_msg = sz_error_to_string(SZ_E_FILE_NOT_FOUND);
    TEST_ASSERT(strstr(error_msg, "not found") != NULL, "File not found error message");

    // Test thread-local error storage
    sz_clear_error();
    const char* last_error = sz_get_last_error_message();
    TEST_ASSERT(last_error[0] == '\0', "Error cleared successfully");
}

void test_archive_open_failure(void) {
    printf("\n=== Testing Archive Open Failure ===\n");

    sz_archive_handle archive = NULL;
    sz_result result = sz_archive_open("nonexistent_file.7z", &archive);

    TEST_ASSERT(result != SZ_OK, "Opening nonexistent file fails");
    TEST_ASSERT(archive == NULL, "Handle remains NULL on failure");

    const char* error = sz_get_last_error_message();
    TEST_ASSERT(error != NULL && strlen(error) > 0, "Error message is set");
    printf("  Expected error: %s\n", error);
}

void test_invalid_arguments(void) {
    printf("\n=== Testing Invalid Arguments ===\n");

    // Test NULL pointers
    sz_result result = sz_archive_open(NULL, NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL path rejected");

    sz_archive_handle handle = NULL;
    result = sz_archive_open("test.7z", NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL handle pointer rejected");

    // Test writer with invalid format (API is stubbed)
    sz_writer_handle writer = NULL;
    result = sz_writer_create("test.7z", (sz_format)999, &writer);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT || result == SZ_E_NOT_IMPLEMENTED,
                "Invalid format rejected or not implemented");

    // Test compressor with invalid level
    sz_compressor_handle compressor = NULL;
    result = sz_compressor_create(SZ_FORMAT_7Z, (sz_compression_level)999, &compressor);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT || result == SZ_E_UNSUPPORTED_FORMAT,
                "Invalid level or format not supported for standalone compression");
}

void test_memory_operations(void) {
    printf("\n=== Testing Memory Operations ===\n");

    // Create a simple data buffer
    const char* test_data = "Hello, World! This is a test.";
    size_t test_size = strlen(test_data);

    // Test compression to memory
    sz_compressor_handle compressor = NULL;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);
    TEST_ASSERT(result == SZ_OK, "Compressor created successfully");

    if (result == SZ_OK) {
        void* compressed = NULL;
        size_t compressed_size = 0;

        result = sz_compress_data(compressor, test_data, test_size, &compressed, &compressed_size);
        TEST_ASSERT(result == SZ_OK, "Data compressed successfully");
        TEST_ASSERT(compressed != NULL, "Compressed data allocated");
        TEST_ASSERT(compressed_size > 0, "Compressed size is positive");

        if (result == SZ_OK) {
            printf("  Original size: %zu, Compressed size: %zu\n", test_size, compressed_size);

            // Test decompression
            void* decompressed = NULL;
            size_t decompressed_size = 0;

            result = sz_decompress_data(compressor, compressed, compressed_size, &decompressed,
                                        &decompressed_size);
            TEST_ASSERT(result == SZ_OK, "Data decompressed successfully");
            TEST_ASSERT(decompressed_size == test_size, "Decompressed size matches original");

            if (result == SZ_OK) {
                int match = memcmp(test_data, decompressed, test_size) == 0;
                TEST_ASSERT(match, "Decompressed data matches original");
                sz_memory_free(decompressed);
            }

            sz_memory_free(compressed);
        }

        sz_compressor_destroy(compressor);
    }
}

void test_item_info_lifecycle(void) {
    printf("\n=== Testing Item Info Lifecycle ===\n");

    // This test demonstrates proper memory management
    sz_item_info info = {0};
    info.path = strdup("test/path.txt");
    info.size = 12345;

    TEST_ASSERT(info.path != NULL, "Item info path allocated");

    // Free the info
    sz_item_info_free(&info);
    TEST_ASSERT(info.path == NULL, "Item info path freed");
}

void test_convenience_functions(void) {
    printf("\n=== Testing Convenience Functions ===\n");

    // Test that convenience functions handle invalid arguments properly
    sz_result result = sz_extract_simple(NULL, "output");
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "extract_simple rejects NULL archive");

    result = sz_extract_simple("test.7z", NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "extract_simple rejects NULL destination");

    result = sz_compress_simple(NULL, "output.7z", SZ_FORMAT_7Z);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT || result == SZ_E_NOT_IMPLEMENTED,
                "compress_simple rejects NULL source or not implemented");

    result = sz_compress_simple("source", NULL, SZ_FORMAT_7Z);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT || result == SZ_E_NOT_IMPLEMENTED,
                "compress_simple rejects NULL archive or not implemented");
}

void test_writer_lifecycle(void) {
    printf("\n=== Testing Writer Lifecycle (Stubbed) ===\n");

    sz_writer_handle writer = NULL;
    sz_result result = sz_writer_create("test_output.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_E_NOT_IMPLEMENTED, "Writer returns NOT_IMPLEMENTED as expected");

    // Writer is stubbed, no further operations needed
    printf("  (Writer API is stubbed for future implementation)\n");
}

void test_compression_levels(void) {
    printf("\n=== Testing Compression Levels ===\n");

    sz_compression_level levels[] = {SZ_LEVEL_NONE, SZ_LEVEL_FAST, SZ_LEVEL_NORMAL,
                                     SZ_LEVEL_MAXIMUM, SZ_LEVEL_ULTRA};

    const char* level_names[] = {"NONE", "FAST", "NORMAL", "MAXIMUM", "ULTRA"};

    for (size_t i = 0; i < sizeof(levels) / sizeof(levels[0]); i++) {
        sz_compressor_handle compressor = NULL;
        sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, levels[i], &compressor);

        char message[64];
        snprintf(message, sizeof(message), "Compression level %s accepted", level_names[i]);
        TEST_ASSERT(result == SZ_OK, message);

        if (result == SZ_OK) {
            sz_compressor_destroy(compressor);
        }
    }
}

int main(void) {
    printf("========================================\n");
    printf("SevenZip C API Test Suite\n");
    printf("========================================\n");

    // Run all tests
    test_version_info();
    test_error_handling();
    test_archive_open_failure();
    test_invalid_arguments();
    test_memory_operations();
    test_item_info_lifecycle();
    test_convenience_functions();
    test_writer_lifecycle();
    test_compression_levels();

    // Print summary
    printf("\n========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
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
