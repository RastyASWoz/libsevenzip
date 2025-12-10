/**
 * @file test_archive_extract.c
 * @brief Archive extraction test for SevenZip FFI C API
 *
 * Tests complete archive reading workflow:
 * - Open archives (7z, zip, tar.gz)
 * - List archive contents
 * - Extract files to directory
 * - Verify extracted file contents
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sevenzip/sevenzip_capi.h"

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define PATH_SEP "/"
#endif

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message)                        \
    do {                                                       \
        if (!(condition)) {                                    \
            printf("  ✗ FAIL: %s\n", message);                 \
            const char* err_msg = sz_get_last_error_message(); \
            if (err_msg && *err_msg) {                         \
                printf("    Error: %s\n", err_msg);            \
            }                                                  \
            tests_failed++;                                    \
            return -1;                                         \
        } else {                                               \
            printf("  ✓ %s\n", message);                       \
            tests_passed++;                                    \
        }                                                      \
    } while (0)

/**
 * Read file contents into buffer
 */
static char* read_file_contents(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(buffer, 1, size, f);
    fclose(f);

    buffer[read] = '\0';
    if (out_size) *out_size = read;
    return buffer;
}

/**
 * Test opening and reading archive information
 */
static int test_open_archive(const char* archive_path) {
    printf("\n=== Testing Archive: %s ===\n", archive_path);

    sz_archive_handle archive = NULL;
    sz_result result = sz_archive_open(archive_path, &archive);
    TEST_ASSERT(result == SZ_OK, "Archive opened successfully");
    TEST_ASSERT(archive != NULL, "Archive handle is valid");

    // Get item count
    size_t item_count = 0;
    result = sz_archive_get_item_count(archive, &item_count);
    TEST_ASSERT(result == SZ_OK, "Got item count");
    printf("  Archive contains %zu items\n", item_count);
    TEST_ASSERT(item_count > 0, "Archive has items");

    // List all items
    printf("  Listing archive contents:\n");
    for (size_t i = 0; i < item_count; i++) {
        sz_item_info info = {0};
        result = sz_archive_get_item_info(archive, i, &info);

        if (result == SZ_OK) {
            printf("    [%zu] %s %s (%llu bytes)\n", i, info.is_directory ? "DIR " : "FILE",
                   info.path ? info.path : "(null)", (unsigned long long)info.size);
            sz_item_info_free(&info);
        }
    }

    // Clean up
    sz_archive_close(archive);
    printf("  ✓ Archive closed\n");

    return 0;
}

/**
 * Test extracting archive to directory
 */
static int test_extract_archive(const char* archive_path, const char* extract_dir) {
    printf("\n=== Testing Extraction: %s ===\n", archive_path);

    // Create extraction directory
    mkdir(extract_dir, 0755);

    sz_archive_handle archive = NULL;
    sz_result result = sz_archive_open(archive_path, &archive);
    TEST_ASSERT(result == SZ_OK, "Archive opened for extraction");

    // Extract to directory (NULL progress callback)
    result = sz_archive_extract_all(archive, extract_dir, NULL, NULL);
    TEST_ASSERT(result == SZ_OK, "Extraction completed successfully");

    sz_archive_close(archive);

    return 0;
}

/**
 * Test extracting specific item by index
 */
static int test_extract_item(const char* archive_path, size_t item_index, const char* output_path) {
    printf("\n=== Testing Item Extraction: Item %zu ===\n", item_index);

    sz_archive_handle archive = NULL;
    sz_result result = sz_archive_open(archive_path, &archive);
    TEST_ASSERT(result == SZ_OK, "Archive opened");

    // Get item info
    sz_item_info info = {0};
    result = sz_archive_get_item_info(archive, item_index, &info);
    TEST_ASSERT(result == SZ_OK, "Item info retrieved");

    printf("  Extracting: %s (%llu bytes)\n", info.path ? info.path : "(null)",
           (unsigned long long)info.size);

    // Extract to memory
    void* data = NULL;
    size_t data_size = 0;
    result = sz_archive_extract_to_memory(archive, item_index, &data, &data_size);
    TEST_ASSERT(result == SZ_OK, "Item extracted to memory");
    TEST_ASSERT(data != NULL, "Extracted data is valid");
    TEST_ASSERT(data_size == info.size, "Extracted size matches");

    printf("  Extracted %zu bytes to memory\n", data_size);

    // Write to file for verification
    FILE* f = fopen(output_path, "wb");
    if (f) {
        fwrite(data, 1, data_size, f);
        fclose(f);
        printf("  ✓ Saved to %s\n", output_path);
    }

    // Clean up
    sz_memory_free(data);
    sz_item_info_free(&info);
    sz_archive_close(archive);

    return 0;
}

/**
 * Test verifying extracted file contents
 */
static int test_verify_extracted_files(const char* extract_dir) {
    printf("\n=== Verifying Extracted Files ===\n");

    // Expected files and contents (UTF-8 with BOM from PowerShell)
    struct {
        const char* path;
        const char* expected_content;
    } expected_files[] = {{"file1.txt",
                           "\xEF\xBB"
                           "\xBF"
                           "File 1 content - Hello World!"},
                          {"file2.txt",
                           "\xEF\xBB"
                           "\xBF"
                           "File 2 content - Testing 7z"},
                          {"subdir" PATH_SEP "file3.txt",
                           "\xEF\xBB"
                           "\xBF"
                           "Subdirectory file content"}};

    for (size_t i = 0; i < sizeof(expected_files) / sizeof(expected_files[0]); i++) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s%s%s", extract_dir, PATH_SEP,
                 expected_files[i].path);

        size_t size;
        char* content = read_file_contents(full_path, &size);

        if (content) {
            int matches = strcmp(content, expected_files[i].expected_content) == 0;
            if (matches) {
                printf("  ✓ %s content verified (%zu bytes)\n", expected_files[i].path, size);
                tests_passed++;
            } else {
                // Try without BOM
                const char* content_ptr = content;
                if (size >= 3 && (unsigned char)content[0] == 0xEF &&
                    (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF) {
                    content_ptr = content + 3;  // Skip BOM
                }

                const char* expected_ptr = expected_files[i].expected_content;
                if (expected_ptr[0] == '\xEF' && expected_ptr[1] == '\xBB' &&
                    expected_ptr[2] == '\xBF') {
                    expected_ptr += 3;  // Skip BOM in expected too
                }

                if (strcmp(content_ptr, expected_ptr) == 0) {
                    printf("  ✓ %s content verified (%zu bytes, UTF-8 BOM)\n",
                           expected_files[i].path, size);
                    tests_passed++;
                } else {
                    printf("  ✗ %s content mismatch\n", expected_files[i].path);
                    printf("    Expected: \"%s\"\n", expected_ptr);
                    printf("    Got:      \"%s\"\n", content_ptr);
                    tests_failed++;
                }
            }
            free(content);
        } else {
            printf("  ✗ Failed to read %s\n", expected_files[i].path);
            tests_failed++;
        }
    }

    return 0;
}

/**
 * Test error handling with invalid archive
 */
static int test_error_handling() {
    printf("\n=== Testing Error Handling ===\n");

    sz_archive_handle archive = NULL;
    sz_result result;

    // Test opening non-existent file
    result = sz_archive_open("nonexistent_file.7z", &archive);
    TEST_ASSERT(result != SZ_OK, "Opening non-existent file fails");
    TEST_ASSERT(archive == NULL, "Archive handle is NULL on failure");

    const char* error_msg = sz_get_last_error_message();
    printf("  Error message: %s\n", error_msg ? error_msg : "(none)");

    sz_clear_error();

    // Test with NULL parameters
    result = sz_archive_open(NULL, &archive);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL path rejected");

    result = sz_archive_open("test.7z", NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL handle pointer rejected");

    return 0;
}

int main(void) {
    printf("═══════════════════════════════════════════════════\n");
    printf("SevenZip FFI C API - Archive Extraction Tests\n");
    printf("═══════════════════════════════════════════════════\n");

    // Test opening and reading archives
    if (test_open_archive("tests/data/archives/test.7z") != 0) {
        printf("\n✗ 7z archive test failed\n");
        return 1;
    }

    if (test_open_archive("tests/data/archives/test.zip") != 0) {
        printf("\n✗ ZIP archive test failed\n");
        return 1;
    }

    if (test_open_archive("tests/data/archives/test.tar.gz") != 0) {
        printf("\n✗ TAR.GZ archive test failed\n");
        return 1;
    }

    // Test extraction
    if (test_extract_archive("tests/data/archives/test.7z", "tests/data/extracted_7z") != 0) {
        printf("\n✗ Extraction test failed\n");
        return 1;
    }

    // Verify extracted files
    if (test_verify_extracted_files("tests/data/extracted_7z") != 0) {
        printf("\n✗ Verification test failed\n");
        return 1;
    }

    // Test extracting specific item
    if (test_extract_item("tests/data/archives/test.zip", 0, "tests/data/extracted_item.txt") !=
        0) {
        printf("\n✗ Item extraction test failed\n");
        return 1;
    }

    // Test error handling
    if (test_error_handling() != 0) {
        printf("\n✗ Error handling test failed\n");
        return 1;
    }

    // Summary
    printf("\n═══════════════════════════════════════════════════\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("═══════════════════════════════════════════════════\n");

    if (tests_failed == 0) {
        printf("✓ All archive extraction tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
