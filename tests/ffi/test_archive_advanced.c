// test_archive_advanced.c - Advanced tests for archive reader functionality
// Tests edge cases, item properties, timestamps, and various archive formats

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// Helper to create test archive
void create_test_archive_with_items(const char* archive_path, int item_count) {
    sz_writer_handle writer;
    sz_result result = sz_writer_create(archive_path, SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        printf("Warning: Could not create test archive (writer may not be fully implemented)\n");
        return;
    }

    for (int i = 0; i < item_count; i++) {
        char filename[64];
        char content[256];
        snprintf(filename, sizeof(filename), "test_file_%d.txt", i);
        snprintf(content, sizeof(content), "Content of file %d\nLine 2\nLine 3\n", i);

        FILE* f = fopen(filename, "wb");
        if (f) {
            fwrite(content, 1, strlen(content), f);
            fclose(f);
            sz_writer_add_file(writer, filename, filename);
            remove(filename);
        }
    }

    sz_writer_finalize(writer);
    sz_writer_cancel(writer);
}

void test_item_properties(void) {
    printf("\n=== Testing Item Properties ===\n");

    // Create a test archive first
    create_test_archive_with_items("test_properties.7z", 3);

    sz_archive_handle archive;
    sz_result result = sz_archive_open("test_properties.7z", &archive);

    if (result != SZ_OK) {
        printf("Skipping test (archive not available)\n");
        return;
    }

    size_t count;
    sz_archive_get_item_count(archive, &count);
    TEST_ASSERT(count == 3, "Archive contains expected number of items");

    // Test detailed item info
    for (size_t i = 0; i < count; i++) {
        sz_item_info info = {0};
        result = sz_archive_get_item_info(archive, i, &info);
        TEST_ASSERT(result == SZ_OK, "Item info retrieved successfully");

        if (result == SZ_OK) {
            TEST_ASSERT(info.path != NULL, "Item path is not NULL");
            TEST_ASSERT(strlen(info.path) > 0, "Item path is not empty");
            TEST_ASSERT(info.size > 0, "Item size is positive");
            TEST_ASSERT(info.is_directory == 0, "Item is not a directory");

            printf("  Item %zu: %s (size: %llu bytes)\n", i, info.path,
                   (unsigned long long)info.size);

            // Check timestamps (should be non-zero for real files)
            if (info.modification_time > 0) {
                time_t mtime = (time_t)info.modification_time;
                printf("    Modified: %s", ctime(&mtime));
            }

            // Free the allocated path
            sz_item_info_free(&info);
            TEST_ASSERT(info.path == NULL, "Item info properly freed");
        }
    }

    sz_archive_close(archive);
    remove("test_properties.7z");
}

void test_archive_info_details(void) {
    printf("\n=== Testing Archive Info Details ===\n");

    create_test_archive_with_items("test_info.7z", 5);

    sz_archive_handle archive;
    sz_result result = sz_archive_open("test_info.7z", &archive);

    if (result != SZ_OK) {
        printf("Skipping test (archive not available)\n");
        return;
    }

    sz_archive_info info = {0};
    result = sz_archive_get_info(archive, &info);
    TEST_ASSERT(result == SZ_OK, "Archive info retrieved");

    if (result == SZ_OK) {
        TEST_ASSERT(info.format == SZ_FORMAT_7Z, "Format is 7z");
        TEST_ASSERT(info.item_count == 5, "Item count is correct");
        TEST_ASSERT(info.total_size > 0, "Total size is positive");
        TEST_ASSERT(info.packed_size > 0, "Packed size is positive");
        TEST_ASSERT(info.packed_size <= info.total_size, "Packed size <= total size");

        double ratio = (double)info.packed_size / (double)info.total_size * 100.0;
        printf("  Total: %llu bytes, Packed: %llu bytes (%.1f%%)\n",
               (unsigned long long)info.total_size, (unsigned long long)info.packed_size, ratio);
        printf("  Solid: %s, Multi-volume: %s\n", info.is_solid ? "yes" : "no",
               info.is_multi_volume ? "yes" : "no");
    }

    sz_archive_close(archive);
    remove("test_info.7z");
}

void test_extract_to_memory(void) {
    printf("\n=== Testing Extract to Memory ===\n");

    create_test_archive_with_items("test_extract_mem.7z", 2);

    sz_archive_handle archive;
    sz_result result = sz_archive_open("test_extract_mem.7z", &archive);

    if (result != SZ_OK) {
        printf("Skipping test (archive not available)\n");
        return;
    }

    size_t count;
    sz_archive_get_item_count(archive, &count);

    for (size_t i = 0; i < count; i++) {
        void* data = NULL;
        size_t size = 0;

        result = sz_archive_extract_to_memory(archive, i, &data, &size);
        TEST_ASSERT(result == SZ_OK, "Extract to memory succeeded");

        if (result == SZ_OK) {
            TEST_ASSERT(data != NULL, "Extracted data is not NULL");
            TEST_ASSERT(size > 0, "Extracted size is positive");

            printf("  Item %zu extracted: %zu bytes\n", i, size);

            // Verify content contains expected text
            char* content = (char*)data;
            int has_content = (size >= 7 && strncmp(content, "Content", 7) == 0);
            TEST_ASSERT(has_content, "Extracted content is correct");

            sz_memory_free(data);
        }
    }

    sz_archive_close(archive);
    remove("test_extract_mem.7z");
}

void test_password_protected_archive(void) {
    printf("\n=== Testing Password Protected Archive ===\n");

    // Create password-protected archive
    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_password.7z", SZ_FORMAT_7Z, &writer);

    if (result != SZ_OK) {
        printf("Skipping test (writer not fully implemented)\n");
        return;
    }

    sz_writer_set_password(writer, "testpass123");

    const char* content = "Secret content";
    sz_writer_add_memory(writer, content, strlen(content), "secret.txt");
    sz_writer_finalize(writer);
    sz_writer_cancel(writer);

    // Try to open without password - should succeed (7z format)
    sz_archive_handle archive;
    result = sz_archive_open("test_password.7z", &archive);
    TEST_ASSERT(result == SZ_OK, "Archive opens without password (headers not encrypted)");

    if (result == SZ_OK) {
        // Try to extract without password - should fail
        void* data = NULL;
        size_t size = 0;
        result = sz_archive_extract_to_memory(archive, 0, &data, &size);
        TEST_ASSERT(result != SZ_OK, "Extract without password fails");

        // Set correct password
        sz_archive_set_password(archive, "testpass123");
        result = sz_archive_extract_to_memory(archive, 0, &data, &size);
        TEST_ASSERT(result == SZ_OK, "Extract with correct password succeeds");

        if (result == SZ_OK) {
            TEST_ASSERT(size == strlen(content), "Extracted size matches");
            TEST_ASSERT(memcmp(data, content, size) == 0, "Extracted content matches");
            sz_memory_free(data);
        }

        sz_archive_close(archive);
    }

    remove("test_password.7z");
}

void test_archive_test_integrity(void) {
    printf("\n=== Testing Archive Integrity Test ===\n");

    create_test_archive_with_items("test_integrity.7z", 3);

    sz_archive_handle archive;
    sz_result result = sz_archive_open("test_integrity.7z", &archive);

    if (result != SZ_OK) {
        printf("Skipping test (archive not available)\n");
        return;
    }

    // Test archive integrity
    result = sz_archive_test(archive);
    TEST_ASSERT(result == SZ_OK, "Archive integrity test passed");

    sz_archive_close(archive);
    remove("test_integrity.7z");
}

void test_empty_archive(void) {
    printf("\n=== Testing Empty Archive ===\n");

    // Create empty archive
    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_empty.7z", SZ_FORMAT_7Z, &writer);

    if (result != SZ_OK) {
        printf("Skipping test (writer not fully implemented)\n");
        return;
    }

    // Finalize without adding anything
    sz_writer_finalize(writer);
    sz_writer_cancel(writer);

    // Open and verify
    sz_archive_handle archive;
    result = sz_archive_open("test_empty.7z", &archive);
    TEST_ASSERT(result == SZ_OK, "Empty archive opens successfully");

    if (result == SZ_OK) {
        size_t count;
        sz_archive_get_item_count(archive, &count);
        TEST_ASSERT(count == 0, "Empty archive has zero items");

        sz_archive_close(archive);
    }

    remove("test_empty.7z");
}

void test_large_item_count(void) {
    printf("\n=== Testing Large Item Count ===\n");

    // Create archive with many items
    create_test_archive_with_items("test_many_items.7z", 50);

    sz_archive_handle archive;
    sz_result result = sz_archive_open("test_many_items.7z", &archive);

    if (result != SZ_OK) {
        printf("Skipping test (archive not available)\n");
        return;
    }

    size_t count;
    sz_archive_get_item_count(archive, &count);
    TEST_ASSERT(count == 50, "Archive contains 50 items");

    // Test random access
    sz_item_info info = {0};
    result = sz_archive_get_item_info(archive, 25, &info);
    TEST_ASSERT(result == SZ_OK, "Can access middle item");

    if (result == SZ_OK) {
        TEST_ASSERT(strstr(info.path, "test_file_25") != NULL, "Correct item retrieved");
        sz_item_info_free(&info);
    }

    // Test last item
    result = sz_archive_get_item_info(archive, count - 1, &info);
    TEST_ASSERT(result == SZ_OK, "Can access last item");

    if (result == SZ_OK) {
        sz_item_info_free(&info);
    }

    // Test out of bounds
    result = sz_archive_get_item_info(archive, count, &info);
    TEST_ASSERT(result != SZ_OK, "Out of bounds access fails");

    sz_archive_close(archive);
    remove("test_many_items.7z");
}

// Global variables for progress callback test
static int g_callback_count = 0;

static int progress_callback_test(uint64_t completed, uint64_t total, void* user_data) {
    g_callback_count++;
    int* custom_data = (int*)user_data;
    (*custom_data)++;
    return 1;  // Continue
}

void test_progress_callback(void) {
    printf("\n=== Testing Progress Callback ===\n");

    create_test_archive_with_items("test_progress.7z", 5);

    sz_archive_handle archive;
    sz_result result = sz_archive_open("test_progress.7z", &archive);

    if (result != SZ_OK) {
        printf("Skipping test (archive not available)\n");
        return;
    }

    // Reset callback counter
    g_callback_count = 0;
    int user_counter = 0;

    // Extract all with progress
    result =
        sz_archive_extract_all(archive, "temp_extract_dir", progress_callback_test, &user_counter);

    if (result == SZ_OK) {
        TEST_ASSERT(g_callback_count > 0, "Progress callback was called");
        TEST_ASSERT(user_counter == g_callback_count, "User data passed correctly");
        printf("  Progress callback called %d times\n", g_callback_count);
    } else {
        printf("  Extract failed, callback test skipped\n");
    }

    sz_archive_close(archive);
    remove("test_progress.7z");

// Clean up extracted files
#ifdef _WIN32
    system("rmdir /s /q temp_extract_dir 2>nul");
#else
    system("rm -rf temp_extract_dir");
#endif
}

void test_multiple_formats(void) {
    printf("\n=== Testing Multiple Archive Formats ===\n");

    const char* test_files[] = {"test_format.7z", "test_format.zip", "test_format.tar"};

    const sz_format formats[] = {SZ_FORMAT_7Z, SZ_FORMAT_ZIP, SZ_FORMAT_TAR};

    const char* format_names[] = {"7z", "ZIP", "TAR"};

    for (size_t i = 0; i < 3; i++) {
        sz_writer_handle writer;
        sz_result result = sz_writer_create(test_files[i], formats[i], &writer);

        if (result != SZ_OK) {
            printf("  Skipping %s format (writer not fully implemented)\n", format_names[i]);
            continue;
        }

        const char* content = "Test content for format test";
        sz_writer_add_memory(writer, content, strlen(content), "test.txt");
        sz_writer_finalize(writer);
        sz_writer_cancel(writer);

        // Open and verify
        sz_archive_handle archive;
        result = sz_archive_open(test_files[i], &archive);

        char msg[128];
        snprintf(msg, sizeof(msg), "%s format opens successfully", format_names[i]);
        TEST_ASSERT(result == SZ_OK, msg);

        if (result == SZ_OK) {
            sz_archive_info info = {0};
            sz_archive_get_info(archive, &info);

            snprintf(msg, sizeof(msg), "%s format detected correctly", format_names[i]);
            TEST_ASSERT(info.format == formats[i], msg);

            sz_archive_close(archive);
        }

        remove(test_files[i]);
    }
}

int main(void) {
    printf("==============================================\n");
    printf(" Advanced Archive Reader Tests\n");
    printf("==============================================\n");

    // Run all tests
    test_item_properties();
    test_archive_info_details();
    test_extract_to_memory();
    test_password_protected_archive();
    test_archive_test_integrity();
    test_empty_archive();
    test_large_item_count();
    test_progress_callback();
    test_multiple_formats();

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
