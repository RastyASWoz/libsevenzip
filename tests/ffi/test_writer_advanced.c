// test_writer_advanced.c - Advanced tests for archive writer functionality
// Tests edge cases, error handling, directory operations, and various scenarios

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

// Helper to create test file
void create_test_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(content, 1, strlen(content), f);
        fclose(f);
    }
}

// Helper to create test directory
void create_test_directory(const char* path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

void test_add_directory_recursive(void) {
    printf("\n=== Testing Add Directory Recursive ===\n");

    // Create test directory structure
    create_test_directory("test_dir");
    create_test_directory("test_dir/subdir");
    create_test_file("test_dir/file1.txt", "File 1 content");
    create_test_file("test_dir/file2.txt", "File 2 content");
    create_test_file("test_dir/subdir/file3.txt", "File 3 content");

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_recursive.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created successfully");

    if (result == SZ_OK) {
        // Add directory recursively
        result = sz_writer_add_directory(writer, "test_dir", 1);  // 1 = recursive
        TEST_ASSERT(result == SZ_OK, "Directory added recursively");

        result = sz_writer_finalize(writer);
        TEST_ASSERT(result == SZ_OK, "Archive finalized");

        sz_writer_cancel(writer);

        // Verify archive
        sz_archive_handle archive;
        result = sz_archive_open("test_recursive.7z", &archive);

        if (result == SZ_OK) {
            size_t count;
            sz_archive_get_item_count(archive, &count);
            TEST_ASSERT(count >= 3, "Archive contains at least 3 items");
            printf("  Archive contains %zu items\n", count);

            sz_archive_close(archive);
        }

        remove("test_recursive.7z");
    }

    // Cleanup
    remove("test_dir/subdir/file3.txt");
    remove("test_dir/file2.txt");
    remove("test_dir/file1.txt");
#ifdef _WIN32
    _rmdir("test_dir/subdir");
    _rmdir("test_dir");
#else
    rmdir("test_dir/subdir");
    rmdir("test_dir");
#endif
}

void test_add_directory_non_recursive(void) {
    printf("\n=== Testing Add Directory Non-Recursive ===\n");

    create_test_directory("test_dir_flat");
    create_test_file("test_dir_flat/file1.txt", "Content 1");
    create_test_file("test_dir_flat/file2.txt", "Content 2");

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_flat.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created");

    if (result == SZ_OK) {
        result = sz_writer_add_directory(writer, "test_dir_flat", 0);  // 0 = non-recursive
        TEST_ASSERT(result == SZ_OK, "Directory added non-recursively");

        sz_writer_finalize(writer);
        sz_writer_cancel(writer);
        remove("test_flat.7z");
    }

    remove("test_dir_flat/file1.txt");
    remove("test_dir_flat/file2.txt");
#ifdef _WIN32
    _rmdir("test_dir_flat");
#else
    rmdir("test_dir_flat");
#endif
}

void test_large_file_addition(void) {
    printf("\n=== Testing Large File Addition ===\n");

    // Create a "large" test file (1 MB)
    const size_t large_size = 1024 * 1024;
    char* large_data = (char*)malloc(large_size);
    if (!large_data) {
        printf("  Skipping test (memory allocation failed)\n");
        return;
    }

    // Fill with pattern
    for (size_t i = 0; i < large_size; i++) {
        large_data[i] = (char)(i % 256);
    }

    FILE* f = fopen("large_test.bin", "wb");
    if (f) {
        fwrite(large_data, 1, large_size, f);
        fclose(f);
    }

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_large.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created for large file");

    if (result == SZ_OK) {
        result = sz_writer_add_file(writer, "large_test.bin", "large.bin");
        TEST_ASSERT(result == SZ_OK, "Large file added successfully");

        result = sz_writer_finalize(writer);
        TEST_ASSERT(result == SZ_OK, "Archive with large file finalized");

        sz_writer_cancel(writer);

        // Verify
        sz_archive_handle archive;
        result = sz_archive_open("test_large.7z", &archive);

        if (result == SZ_OK) {
            sz_item_info info = {0};
            sz_archive_get_item_info(archive, 0, &info);

            TEST_ASSERT(info.size == large_size, "Large file size preserved");
            printf("  Original: %zu bytes, Stored: %llu bytes\n", large_size,
                   (unsigned long long)info.size);

            sz_item_info_free(&info);
            sz_archive_close(archive);
        }

        remove("test_large.7z");
    }

    remove("large_test.bin");
    free(large_data);
}

void test_empty_file_addition(void) {
    printf("\n=== Testing Empty File Addition ===\n");

    // Create empty file
    FILE* f = fopen("empty.txt", "wb");
    if (f) fclose(f);

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_empty_file.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created");

    if (result == SZ_OK) {
        result = sz_writer_add_file(writer, "empty.txt", "empty.txt");
        TEST_ASSERT(result == SZ_OK, "Empty file added");

        sz_writer_finalize(writer);
        sz_writer_cancel(writer);

        // Verify
        sz_archive_handle archive;
        result = sz_archive_open("test_empty_file.7z", &archive);

        if (result == SZ_OK) {
            sz_item_info info = {0};
            sz_archive_get_item_info(archive, 0, &info);
            TEST_ASSERT(info.size == 0, "Empty file has zero size");
            sz_item_info_free(&info);
            sz_archive_close(archive);
        }

        remove("test_empty_file.7z");
    }

    remove("empty.txt");
}

void test_special_characters_in_names(void) {
    printf("\n=== Testing Special Characters in Names ===\n");

    const char* special_names[] = {"file with spaces.txt", "file-with-dashes.txt",
                                   "file_with_underscores.txt", "file.multiple.dots.txt"};

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_special_chars.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created");

    if (result == SZ_OK) {
        for (size_t i = 0; i < sizeof(special_names) / sizeof(special_names[0]); i++) {
            create_test_file(special_names[i], "content");
            result = sz_writer_add_file(writer, special_names[i], special_names[i]);

            char msg[256];
            snprintf(msg, sizeof(msg), "Special filename added: %s", special_names[i]);
            TEST_ASSERT(result == SZ_OK, msg);

            remove(special_names[i]);
        }

        sz_writer_finalize(writer);
        sz_writer_cancel(writer);

        // Verify
        sz_archive_handle archive;
        result = sz_archive_open("test_special_chars.7z", &archive);

        if (result == SZ_OK) {
            size_t count;
            sz_archive_get_item_count(archive, &count);
            TEST_ASSERT(count == 4, "All special filename items stored");
            sz_archive_close(archive);
        }

        remove("test_special_chars.7z");
    }
}

void test_solid_mode_toggle(void) {
    printf("\n=== Testing Solid Mode Toggle ===\n");

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_solid.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created");

    if (result == SZ_OK) {
        // Enable solid mode
        result = sz_writer_set_solid_mode(writer, 1);
        TEST_ASSERT(result == SZ_OK, "Solid mode enabled");

        // Add multiple files
        for (int i = 0; i < 5; i++) {
            char filename[64];
            char content[128];
            snprintf(filename, sizeof(filename), "solid_file_%d.txt", i);
            snprintf(content, sizeof(content), "Content of solid file %d", i);

            create_test_file(filename, content);
            sz_writer_add_file(writer, filename, filename);
            remove(filename);
        }

        sz_writer_finalize(writer);
        sz_writer_cancel(writer);

        // Verify archive is solid
        sz_archive_handle archive;
        result = sz_archive_open("test_solid.7z", &archive);

        if (result == SZ_OK) {
            sz_archive_info info = {0};
            sz_archive_get_info(archive, &info);
            TEST_ASSERT(info.is_solid == 1, "Archive is solid");
            sz_archive_close(archive);
        }

        remove("test_solid.7z");
    }
}

void test_encrypted_headers(void) {
    printf("\n=== Testing Encrypted Headers ===\n");

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_encrypted_headers.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created");

    if (result == SZ_OK) {
        sz_writer_set_password(writer, "headerpass");
        sz_writer_set_encrypted_headers(writer, 1);

        const char* content = "Secret data with encrypted headers";
        sz_writer_add_memory(writer, content, strlen(content), "secret.txt");

        sz_writer_finalize(writer);
        sz_writer_cancel(writer);

        // Try to open without password - should fail or require password
        sz_archive_handle archive;
        result = sz_archive_open("test_encrypted_headers.7z", &archive);

        // With encrypted headers, even opening should require password
        if (result == SZ_OK) {
            sz_archive_info info = {0};
            sz_archive_get_info(archive, &info);
            TEST_ASSERT(info.has_encrypted_headers == 1, "Headers are encrypted");
            sz_archive_close(archive);
        }

        remove("test_encrypted_headers.7z");
    }
}

void test_writer_error_handling(void) {
    printf("\n=== Testing Writer Error Handling ===\n");

    sz_writer_handle writer;

    // Test invalid file path
    sz_result result = sz_writer_create(NULL, SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "NULL path rejected");

    // Note: Invalid format values are converted to default (7z) by FFI layer
    // This is a design choice for fault tolerance

    // Test adding non-existent file
    result = sz_writer_create("test_error.7z", SZ_FORMAT_7Z, &writer);
    if (result == SZ_OK) {
        result = sz_writer_add_file(writer, "nonexistent_file.txt", "file.txt");
        TEST_ASSERT(result != SZ_OK, "Non-existent file addition fails");
        sz_writer_cancel(writer);
        remove("test_error.7z");
    }

    // Test double finalize
    result = sz_writer_create("test_double.7z", SZ_FORMAT_7Z, &writer);
    if (result == SZ_OK) {
        sz_writer_finalize(writer);
        result = sz_writer_finalize(writer);
        TEST_ASSERT(result != SZ_OK, "Double finalize fails");
        sz_writer_cancel(writer);
        remove("test_double.7z");
    }
}

void test_memory_archive_variations(void) {
    printf("\n=== Testing Memory Archive Variations ===\n");

    // Test very small data
    sz_writer_handle writer;
    sz_result result = sz_writer_create_memory(SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Memory writer created");

    if (result == SZ_OK) {
        const char* tiny = "x";
        sz_writer_add_memory(writer, tiny, 1, "tiny.txt");
        sz_writer_finalize(writer);

        const void* data;
        size_t size;
        result = sz_writer_get_memory_data(writer, &data, &size);
        TEST_ASSERT(result == SZ_OK, "Memory archive retrieved");
        TEST_ASSERT(size > 0, "Memory archive has data");
        printf("  Tiny file (1 byte) -> archive: %zu bytes\n", size);

        sz_writer_cancel(writer);
    }

    // Test multiple memory additions
    result = sz_writer_create_memory(SZ_FORMAT_7Z, &writer);
    if (result == SZ_OK) {
        for (int i = 0; i < 10; i++) {
            char filename[32];
            char content[64];
            snprintf(filename, sizeof(filename), "mem_file_%d.txt", i);
            snprintf(content, sizeof(content), "Memory content %d", i);
            sz_writer_add_memory(writer, content, strlen(content), filename);
        }

        sz_writer_finalize(writer);

        const void* data;
        size_t size;
        sz_writer_get_memory_data(writer, &data, &size);
        printf("  10 memory files -> archive: %zu bytes\n", size);

        sz_writer_cancel(writer);
    }
}

void test_pending_count_tracking(void) {
    printf("\n=== Testing Pending Count Tracking ===\n");

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_pending.7z", SZ_FORMAT_7Z, &writer);
    TEST_ASSERT(result == SZ_OK, "Writer created");

    if (result == SZ_OK) {
        size_t count = 0;

        // Initial count should be 0
        sz_writer_get_pending_count(writer, &count);
        TEST_ASSERT(count == 0, "Initial pending count is 0");

        // Add some items
        const char* data = "test";
        sz_writer_add_memory(writer, data, 4, "file1.txt");
        sz_writer_add_memory(writer, data, 4, "file2.txt");
        sz_writer_add_memory(writer, data, 4, "file3.txt");

        sz_writer_get_pending_count(writer, &count);
        TEST_ASSERT(count >= 3, "Pending count reflects added items");
        printf("  Pending count after 3 additions: %zu\n", count);

        sz_writer_finalize(writer);
        sz_writer_cancel(writer);
        remove("test_pending.7z");
    }
}

int main(void) {
    printf("==============================================\n");
    printf(" Advanced Archive Writer Tests\n");
    printf("==============================================\n");

    // Run all tests
    test_add_directory_recursive();
    test_add_directory_non_recursive();
    test_large_file_addition();
    test_empty_file_addition();
    test_special_characters_in_names();
    test_solid_mode_toggle();
    test_encrypted_headers();
    test_writer_error_handling();
    test_memory_archive_variations();
    test_pending_count_tracking();

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
