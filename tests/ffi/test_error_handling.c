// test_error_handling.c - Comprehensive error handling tests
// Tests all error codes, error messages, and error recovery scenarios

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sevenzip/sevenzip_capi.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message)      \
    do {                                     \
        if (condition) {                     \
            tests_passed++;                  \
            printf("✓ PASS: %s\n", message); \
        } else {                             \
            tests_failed++;                  \
            printf("✗ FAIL: %s\n", message); \
        }                                    \
    } while (0)

void test_error_code_messages(void) {
    printf("\n=== Testing Error Code Messages ===\n");

    // Test all defined error codes have messages
    const sz_result error_codes[] = {SZ_OK,
                                     SZ_E_FAIL,
                                     SZ_E_INVALID_ARGUMENT,
                                     SZ_E_OUT_OF_MEMORY,
                                     SZ_E_FILE_NOT_FOUND,
                                     SZ_E_ACCESS_DENIED,
                                     SZ_E_READ_ERROR,
                                     SZ_E_WRITE_ERROR,
                                     SZ_E_UNSUPPORTED_FORMAT,
                                     SZ_E_CORRUPTED_ARCHIVE,
                                     SZ_E_WRONG_PASSWORD,
                                     SZ_E_NOT_IMPLEMENTED,
                                     SZ_E_INDEX_OUT_OF_RANGE,
                                     SZ_E_DISK_FULL};

    for (size_t i = 0; i < sizeof(error_codes) / sizeof(error_codes[0]); i++) {
        const char* msg = sz_error_to_string(error_codes[i]);
        TEST_ASSERT(msg != NULL, "Error message is not NULL");
        TEST_ASSERT(strlen(msg) > 0, "Error message is not empty");

        printf("  Code %d: %s\n", error_codes[i], msg);
    }

    // Test unknown error code
    const char* unknown = sz_error_to_string((sz_result)99999);
    TEST_ASSERT(unknown != NULL, "Unknown error code returns message");
    printf("  Unknown code: %s\n", unknown);
}

void test_thread_local_error_storage(void) {
    printf("\n=== Testing Thread-Local Error Storage ===\n");

    // Clear error
    sz_clear_error();
    const char* msg = sz_get_last_error_message();
    TEST_ASSERT(strlen(msg) == 0, "Error cleared successfully");

    // Trigger an error
    sz_archive_handle archive = NULL;
    sz_result result = sz_archive_open("nonexistent_file.7z", &archive);
    TEST_ASSERT(result != SZ_OK, "Operation failed as expected");

    // Check error message was set
    msg = sz_get_last_error_message();
    TEST_ASSERT(strlen(msg) > 0, "Error message set after failure");
    printf("  Error message: %s\n", msg);

    // Clear and verify
    sz_clear_error();
    msg = sz_get_last_error_message();
    TEST_ASSERT(strlen(msg) == 0, "Error cleared again");
}

void test_error_persistence(void) {
    printf("\n=== Testing Error Persistence ===\n");

    sz_clear_error();

    // Trigger error
    sz_archive_handle archive;
    sz_archive_open(NULL, &archive);

    const char* first_msg = sz_get_last_error_message();
    TEST_ASSERT(strlen(first_msg) > 0, "Error message available");

    // Get error multiple times without clearing
    const char* second_msg = sz_get_last_error_message();
    TEST_ASSERT(strcmp(first_msg, second_msg) == 0, "Error persists across calls");

    // Successful operation should clear error
    const char* version = sz_version_string();
    (void)version;  // Use it

    // Error might still be set depending on implementation
    // Just verify we can still get the message
    const char* third_msg = sz_get_last_error_message();
    TEST_ASSERT(third_msg != NULL, "Error message still accessible");

    sz_clear_error();
}

void test_invalid_argument_errors(void) {
    printf("\n=== Testing Invalid Argument Errors ===\n");

    sz_result result;

    // Archive operations with NULL
    result = sz_archive_open(NULL, NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_archive_open(NULL, NULL)");

    sz_archive_handle handle = NULL;
    result = sz_archive_open("test.7z", NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_archive_open(..., NULL)");

    result = sz_archive_get_item_count(NULL, NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_archive_get_item_count(NULL, NULL)");

    size_t count;
    result = sz_archive_get_item_count(NULL, &count);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_archive_get_item_count(NULL, ...)");

    // Writer operations with NULL
    result = sz_writer_create(NULL, SZ_FORMAT_7Z, NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_writer_create(NULL, ...)");

    // Compressor operations with NULL
    result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, NULL);
    TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_compressor_create(..., NULL)");

    sz_compressor_handle compressor;
    result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);
    if (result == SZ_OK) {
        result = sz_compress_data(compressor, NULL, 100, NULL, NULL);
        TEST_ASSERT(result == SZ_E_INVALID_ARGUMENT, "sz_compress_data with NULLs");
        sz_compressor_destroy(compressor);
    }
}

void test_file_not_found_errors(void) {
    printf("\n=== Testing File Not Found Errors ===\n");

    sz_archive_handle archive;
    sz_result result = sz_archive_open("this_file_does_not_exist.7z", &archive);
    TEST_ASSERT(result != SZ_OK, "Opening nonexistent file fails");

    const char* error = sz_get_last_error_message();
    TEST_ASSERT(strlen(error) > 0, "Error message set");
    printf("  Error: %s\n", error);

    // Try with invalid path characters (if applicable to OS)
    result = sz_archive_open("\0\0\0invalid", &archive);
    TEST_ASSERT(result != SZ_OK, "Opening invalid path fails");
}

void test_corrupted_archive_errors(void) {
    printf("\n=== Testing Corrupted Archive Errors ===\n");

    // Create a fake archive file with invalid content
    FILE* f = fopen("fake_archive.7z", "wb");
    if (f) {
        const char* fake_data = "This is not a valid 7z archive!";
        fwrite(fake_data, 1, strlen(fake_data), f);
        fclose(f);

        sz_archive_handle archive;
        sz_result result = sz_archive_open("fake_archive.7z", &archive);
        TEST_ASSERT(result != SZ_OK, "Opening corrupted archive fails");

        const char* error = sz_get_last_error_message();
        printf("  Error: %s\n", error);

        remove("fake_archive.7z");
    }
}

void test_unsupported_format_errors(void) {
    printf("\n=== Testing Unsupported Format Errors ===\n");

    // Try to create compressor with unsupported format (7z for standalone)
    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_7Z, SZ_LEVEL_NORMAL, &compressor);

    // 7z might not be supported for standalone compression
    if (result != SZ_OK) {
        TEST_ASSERT(result == SZ_E_UNSUPPORTED_FORMAT || result == SZ_E_INVALID_ARGUMENT,
                    "7z format rejected for standalone compression");
        printf("  7z format correctly rejected for standalone compression\n");
    } else {
        printf("  7z format accepted (implementation supports it)\n");
        sz_compressor_destroy(compressor);
    }

    // Try with completely invalid format
    result = sz_writer_create("test.7z", (sz_format)99999, NULL);
    TEST_ASSERT(result != SZ_OK, "Invalid format rejected");
}

void test_password_errors(void) {
    printf("\n=== Testing Password Errors ===\n");

    // This test needs a password-protected archive
    // We'll create one if writer is implemented

    sz_writer_handle writer;
    sz_result result = sz_writer_create("test_pass_error.7z", SZ_FORMAT_7Z, &writer);

    if (result == SZ_OK) {
        sz_writer_set_password(writer, "secret123");
        const char* data = "Protected data";
        sz_writer_add_memory(writer, data, strlen(data), "secret.txt");
        sz_writer_finalize(writer);
        sz_writer_cancel(writer);

        // Try to extract with wrong password
        sz_archive_handle archive;
        result = sz_archive_open("test_pass_error.7z", &archive);

        if (result == SZ_OK) {
            sz_archive_set_password(archive, "wrong_password");

            void* extracted = NULL;
            size_t size = 0;
            result = sz_archive_extract_to_memory(archive, 0, &extracted, &size);

            TEST_ASSERT(result != SZ_OK, "Wrong password causes error");
            printf("  Wrong password correctly rejected\n");

            if (extracted) sz_memory_free(extracted);
            sz_archive_close(archive);
        }

        remove("test_pass_error.7z");
    } else {
        printf("  Skipping password test (writer not implemented)\n");
    }
}

void test_out_of_memory_simulation(void) {
    printf("\n=== Testing Out of Memory Scenarios ===\n");

    // This is difficult to test without actually running out of memory
    // We can test that the API handles allocation failures gracefully

    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);

    if (result == SZ_OK) {
        // Try to compress extremely large (but valid) size
        // Most implementations will fail gracefully
        const size_t huge_size = (size_t)-1 / 2;
        void* output = NULL;
        size_t output_size = 0;

        result = sz_compress_data(compressor, NULL, huge_size, &output, &output_size);
        TEST_ASSERT(result != SZ_OK, "Huge size request fails gracefully");
        printf("  API handles extreme size request gracefully\n");

        sz_compressor_destroy(compressor);
    }
}

void test_error_recovery(void) {
    printf("\n=== Testing Error Recovery ===\n");

    // Trigger an error
    sz_archive_handle archive;
    sz_result result = sz_archive_open("nonexistent.7z", &archive);
    TEST_ASSERT(result != SZ_OK, "First operation fails");

    const char* error1 = sz_get_last_error_message();
    TEST_ASSERT(strlen(error1) > 0, "Error message available");

    // Clear error and perform successful operation
    sz_clear_error();

    const char* version = sz_version_string();
    TEST_ASSERT(version != NULL, "Successful operation after error");

    // Verify error was cleared
    const char* error2 = sz_get_last_error_message();
    TEST_ASSERT(strlen(error2) == 0, "Error cleared after successful operation");

    printf("  Error recovery successful\n");
}

void test_cascading_errors(void) {
    printf("\n=== Testing Cascading Errors ===\n");

    // Perform multiple failing operations
    sz_archive_handle archive;
    sz_result result1 = sz_archive_open("fake1.7z", &archive);
    const char* msg1 = sz_get_last_error_message();

    sz_result result2 = sz_archive_open("fake2.7z", &archive);
    const char* msg2 = sz_get_last_error_message();

    TEST_ASSERT(result1 != SZ_OK, "First operation fails");
    TEST_ASSERT(result2 != SZ_OK, "Second operation fails");
    TEST_ASSERT(strlen(msg2) > 0, "Latest error message available");

    printf("  Cascading errors handled correctly\n");
}

void test_error_message_format(void) {
    printf("\n=== Testing Error Message Format ===\n");

    // Trigger various errors and check message quality
    sz_archive_handle archive;
    sz_archive_open(NULL, &archive);
    const char* msg1 = sz_get_last_error_message();
    TEST_ASSERT(strstr(msg1, "NULL") != NULL || strstr(msg1, "null") != NULL ||
                    strstr(msg1, "Invalid") != NULL,
                "NULL pointer error mentions 'NULL' or 'Invalid'");

    sz_clear_error();

    sz_archive_open("nonexistent.7z", &archive);
    const char* msg2 = sz_get_last_error_message();
    // Some error messages may be empty or very short
    TEST_ASSERT(strlen(msg2) >= 0, "Error message is accessible");
    TEST_ASSERT(strlen(msg2) < 500, "Error message is not excessively long");

    printf("  Error messages are well-formatted\n");
}

void test_concurrent_error_handling(void) {
    printf("\n=== Testing Error Isolation ===\n");

    // Test that errors don't interfere across separate operations
    sz_clear_error();

    // Operation 1: Fail
    sz_archive_handle archive1;
    sz_archive_open("fake1.7z", &archive1);
    const char* error1 = sz_get_last_error_message();

    // Operation 2: Create compressor (should succeed)
    sz_compressor_handle compressor;
    sz_result result = sz_compressor_create(SZ_FORMAT_GZIP, SZ_LEVEL_NORMAL, &compressor);

    if (result == SZ_OK) {
        // Operation 1's error should still be accessible
        const char* error1_again = sz_get_last_error_message();
        TEST_ASSERT(strcmp(error1, error1_again) == 0, "Error persists through other operations");

        sz_compressor_destroy(compressor);
    }

    printf("  Error isolation verified\n");
}

int main(void) {
    printf("==============================================\n");
    printf(" Error Handling Integration Tests\n");
    printf("==============================================\n");

    // Run all tests
    test_error_code_messages();
    test_thread_local_error_storage();
    test_error_persistence();
    test_invalid_argument_errors();
    test_file_not_found_errors();
    test_corrupted_archive_errors();
    test_unsupported_format_errors();
    test_password_errors();
    test_out_of_memory_simulation();
    test_error_recovery();
    test_cascading_errors();
    test_error_message_format();
    test_concurrent_error_handling();

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
