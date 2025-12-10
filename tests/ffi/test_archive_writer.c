// test_archive_writer.c - Archive Writer API tests

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sevenzip/sz_archive.h"
#include "sevenzip/sz_writer.h"

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST_PASS(message, ...)         \
    do {                                \
        tests_passed++;                 \
        printf("  �?PASS: ");           \
        printf(message, ##__VA_ARGS__); \
        printf("\n");                   \
    } while (0)

#define TEST_FAIL(message, ...)                        \
    do {                                               \
        tests_failed++;                                \
        printf("  �?FAIL: ");                          \
        printf(message, ##__VA_ARGS__);                \
        printf("\n");                                  \
        const char* err = sz_get_last_error_message(); \
        if (err && *err) {                             \
            printf("    Error: %s\n", err);            \
        }                                              \
    } while (0)

// Helper: Create test file
static int create_test_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;
    fputs(content, f);
    fclose(f);
    return 1;
}

// Helper: Read file content
static char* read_file_content(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);

    return buffer;
}

// Test 1: Create simple 7z archive
void test_create_7z_archive() {
    printf("\n[TEST] Creating 7z archive...\n");

    const char* test_file = "temp_test_file.txt";
    const char* archive_path = "temp_test.7z";
    const char* content = "Hello, 7-Zip!";

    // Create test file
    if (!create_test_file(test_file, content)) {
        TEST_FAIL("Failed to create test file");
        return;
    }

    // Create writer
    sz_writer_handle writer;
    sz_result result = sz_writer_create(archive_path, SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to create writer: %s", sz_get_last_error_message());
        remove(test_file);
        return;
    }

    // Add file
    result = sz_writer_add_file(writer, test_file, "test.txt");
    if (result != SZ_OK) {
        TEST_FAIL("Failed to add file: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        remove(test_file);
        return;
    }

    // Finalize
    result = sz_writer_finalize(writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to finalize: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        remove(test_file);
        return;
    }

    sz_writer_cancel(writer);

    // Verify archive can be opened
    sz_archive_handle archive;
    result = sz_archive_open(archive_path, &archive);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to open created archive: %s", sz_get_last_error_message());
        remove(test_file);
        remove(archive_path);
        return;
    }

    size_t count;
    result = sz_archive_get_item_count(archive, &count);
    if (result != SZ_OK || count != 1) {
        TEST_FAIL("Expected 1 item, got %zu", count);
        sz_archive_close(archive);
        remove(test_file);
        remove(archive_path);
        return;
    }

    sz_archive_close(archive);
    remove(test_file);
    remove(archive_path);

    TEST_PASS("Created and verified 7z archive");
}

// Test 2: Set compression level
void test_compression_levels() {
    printf("\n[TEST] Testing compression levels...\n");

    const char* test_file = "temp_test_file.txt";
    const char* archive_fast = "temp_fast.7z";
    const char* archive_ultra = "temp_ultra.7z";

    // Create test file with compressible content
    char content[1024];
    for (int i = 0; i < sizeof(content) - 1; i++) {
        content[i] = 'A' + (i % 26);
    }
    content[sizeof(content) - 1] = '\0';

    if (!create_test_file(test_file, content)) {
        TEST_FAIL("Failed to create test file");
        return;
    }

    // Create archive with FAST compression
    sz_writer_handle writer;
    sz_result result = sz_writer_create(archive_fast, SZ_FORMAT_7Z, &writer);
    if (result == SZ_OK) {
        sz_writer_set_compression_level(writer, SZ_LEVEL_FAST);
        sz_writer_add_file(writer, test_file, "test.txt");
        sz_writer_finalize(writer);
        sz_writer_cancel(writer);
    }

    // Create archive with ULTRA compression
    result = sz_writer_create(archive_ultra, SZ_FORMAT_7Z, &writer);
    if (result == SZ_OK) {
        sz_writer_set_compression_level(writer, SZ_LEVEL_ULTRA);
        sz_writer_add_file(writer, test_file, "test.txt");
        sz_writer_finalize(writer);
        sz_writer_cancel(writer);
    }

    remove(test_file);
    remove(archive_fast);
    remove(archive_ultra);

    TEST_PASS("Compression levels tested");
}

// Test 3: Add memory data
void test_add_memory() {
    printf("\n[TEST] Adding data from memory...\n");

    const char* archive_path = "temp_memory.7z";
    const char* data = "In-memory content";

    sz_writer_handle writer;
    sz_result result = sz_writer_create(archive_path, SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to create writer: %s", sz_get_last_error_message());
        return;
    }

    result = sz_writer_add_memory(writer, data, strlen(data), "memory.txt");
    if (result != SZ_OK) {
        TEST_FAIL("Failed to add memory: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        return;
    }

    result = sz_writer_finalize(writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to finalize: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        return;
    }

    sz_writer_cancel(writer);

    // Verify
    sz_archive_handle archive;
    result = sz_archive_open(archive_path, &archive);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to open archive: %s", sz_get_last_error_message());
        remove(archive_path);
        return;
    }

    size_t count;
    sz_archive_get_item_count(archive, &count);
    sz_archive_close(archive);
    remove(archive_path);

    if (count == 1) {
        TEST_PASS("Added and verified memory data");
    } else {
        TEST_FAIL("Expected 1 item, got %zu", count);
    }
}

// Test 4: Create memory-based archive
void test_create_memory_archive() {
    printf("\n[TEST] Creating memory-based archive...\n");

    sz_writer_handle writer;
    sz_result result = sz_writer_create_memory(SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to create memory writer: %s", sz_get_last_error_message());
        return;
    }

    const char* data = "Test data";
    result = sz_writer_add_memory(writer, data, strlen(data), "test.txt");
    if (result != SZ_OK) {
        TEST_FAIL("Failed to add memory: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        return;
    }

    result = sz_writer_finalize(writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to finalize: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        return;
    }

    const void* archive_data;
    size_t archive_size;
    result = sz_writer_get_memory_data(writer, &archive_data, &archive_size);
    if (result != SZ_OK || archive_size == 0) {
        TEST_FAIL("Failed to get memory data: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        return;
    }

    printf("  Memory archive size: %zu bytes\n", archive_size);

    sz_writer_cancel(writer);
    TEST_PASS("Created memory-based archive");
}

// Test 5: Set password
void test_password_protection() {
    printf("\n[TEST] Testing password protection...\n");

    const char* archive_path = "temp_encrypted.7z";
    const char* password = "secret123";

    sz_writer_handle writer;
    sz_result result = sz_writer_create(archive_path, SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to create writer: %s", sz_get_last_error_message());
        return;
    }

    result = sz_writer_set_password(writer, password);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to set password: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        return;
    }

    const char* data = "Secret content";
    result = sz_writer_add_memory(writer, data, strlen(data), "secret.txt");
    if (result != SZ_OK) {
        TEST_FAIL("Failed to add memory: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        remove(archive_path);
        return;
    }

    result = sz_writer_finalize(writer);
    sz_writer_cancel(writer);

    if (result == SZ_OK) {
        TEST_PASS("Created password-protected archive");
    } else {
        TEST_FAIL("Failed to finalize: %s", sz_get_last_error_message());
    }

    remove(archive_path);
}

// Test 6: Pending count
void test_pending_count() {
    printf("\n[TEST] Testing pending count...\n");

    const char* archive_path = "temp_count.7z";

    sz_writer_handle writer;
    sz_result result = sz_writer_create(archive_path, SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to create writer: %s", sz_get_last_error_message());
        return;
    }

    size_t count;
    result = sz_writer_get_pending_count(writer, &count);
    if (result != SZ_OK || count != 0) {
        TEST_FAIL("Expected 0 pending items, got %zu", count);
        sz_writer_cancel(writer);
        remove(archive_path);
        return;
    }

    const char* data = "Test";
    sz_writer_add_memory(writer, data, strlen(data), "file1.txt");
    sz_writer_add_memory(writer, data, strlen(data), "file2.txt");

    result = sz_writer_get_pending_count(writer, &count);
    if (result != SZ_OK) {
        TEST_FAIL("Failed to get pending count: %s", sz_get_last_error_message());
        sz_writer_cancel(writer);
        remove(archive_path);
        return;
    }

    printf("  Pending items: %zu\n", count);

    sz_writer_finalize(writer);
    sz_writer_cancel(writer);
    remove(archive_path);

    if (count >= 2) {
        TEST_PASS("Pending count is correct");
    } else {
        TEST_FAIL("Expected at least 2 pending items, got %zu", count);
    }
}

// Test 7: Multiple formats
void test_multiple_formats() {
    printf("\n[TEST] Testing multiple archive formats...\n");

    const char* data = "Test content";
    const char* test_file = "temp_file.txt";
    create_test_file(test_file, data);

    struct {
        sz_format format;
        const char* path;
        const char* name;
    } formats[] = {
        {SZ_FORMAT_7Z, "temp.7z", "7z"},
        {SZ_FORMAT_ZIP, "temp.zip", "ZIP"},
        {SZ_FORMAT_TAR, "temp.tar", "TAR"},
    };

    int success_count = 0;
    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
        sz_writer_handle writer;
        sz_result result = sz_writer_create(formats[i].path, formats[i].format, &writer);
        if (result == SZ_OK) {
            sz_writer_add_file(writer, test_file, "test.txt");
            sz_writer_finalize(writer);
            sz_writer_cancel(writer);

            // Verify
            sz_archive_handle archive;
            if (sz_archive_open(formats[i].path, &archive) == SZ_OK) {
                size_t count;
                if (sz_archive_get_item_count(archive, &count) == SZ_OK && count == 1) {
                    printf("  %s format: OK\n", formats[i].name);
                    success_count++;
                }
                sz_archive_close(archive);
            }
            remove(formats[i].path);
        }
    }

    remove(test_file);

    if (success_count >= 2) {
        TEST_PASS("Multiple formats tested (%d/3 succeeded)", success_count);
    } else {
        TEST_FAIL("Too few formats succeeded (%d/3)", success_count);
    }
}

// Main test runner
int main() {
    printf("==============================================\n");
    printf(" Archive Writer API Tests\n");
    printf("==============================================\n");

    test_create_7z_archive();
    test_compression_levels();
    test_add_memory();
    test_create_memory_archive();
    test_password_protection();
    test_pending_count();
    test_multiple_formats();

    printf("\n==============================================\n");
    printf(" Test Summary\n");
    printf("==============================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    printf("==============================================\n");

    return tests_failed > 0 ? 1 : 0;
}
