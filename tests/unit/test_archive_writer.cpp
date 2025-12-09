// test_archive_writer.cpp - Unit tests for ArchiveWriter
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "../../src/wrapper/archive/archive_reader.hpp"
#include "../../src/wrapper/archive/archive_writer.hpp"
#include "../../src/wrapper/common/wrapper_error.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

namespace fs = std::filesystem;

// ============================================================================
// Test Fixture
// ============================================================================

class ArchiveWriterTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDataDir_ = fs::path(__FILE__).parent_path() / ".." / "data" / "archives";
        tempDir_ = fs::temp_directory_path() / "libsevenzip_writer_tests";

        // Clean up from previous runs
        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
        fs::create_directories(tempDir_);

        // Create test files for archiving
        testFilesDir_ = tempDir_ / "test_files";
        fs::create_directories(testFilesDir_);
        createTestFile(testFilesDir_ / "test1.txt", "Hello, World!");
        createTestFile(testFilesDir_ / "test2.txt", "This is a test file.");
        createTestFile(testFilesDir_ / "binary.bin", std::string(1024, '\xFF'));  // Binary data

        // Create subdirectory with files
        fs::create_directories(testFilesDir_ / "subdir");
        createTestFile(testFilesDir_ / "subdir" / "nested.txt", "Nested file content");
    }

    void TearDown() override {
        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
    }

    void createTestFile(const fs::path& path, const std::string& content) {
        std::ofstream file(path, std::ios::binary);
        file.write(content.data(), content.size());
    }

    std::string readFile(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }

    fs::path testDataDir_;
    fs::path tempDir_;
    fs::path testFilesDir_;
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(ArchiveWriterTest, ConstructorDestructor) {
    ArchiveWriter writer;
    // Should not throw
}

TEST_F(ArchiveWriterTest, CreateEmpty7z) {
    auto archivePath = tempDir_ / "empty.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));
    EXPECT_GT(fs::file_size(archivePath), 0);

    // Verify with reader
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 0);
    reader.close();
}

TEST_F(ArchiveWriterTest, AddSingleFile) {
    auto archivePath = tempDir_ / "single.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test1.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // Verify with reader
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto info = reader.getItemInfo(0);
    EXPECT_EQ(info.path, L"test1.txt");
    EXPECT_FALSE(info.isDirectory);
    EXPECT_EQ(info.size, 13);  // "Hello, World!" length

    reader.close();
}

TEST_F(ArchiveWriterTest, AddMultipleFiles) {
    auto archivePath = tempDir_ / "multiple.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile((testFilesDir_ / "test1.txt").wstring(), L"file1.txt");
    writer.addFile((testFilesDir_ / "test2.txt").wstring(), L"file2.txt");
    writer.addFile((testFilesDir_ / "binary.bin").wstring(), L"data.bin");
    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 3);
    reader.close();
}

TEST_F(ArchiveWriterTest, AddFileFromMemory) {
    auto archivePath = tempDir_ / "memory.7z";

    std::string content = "Content from memory buffer";
    std::vector<uint8_t> data(content.begin(), content.end());

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFileFromMemory(data, L"memory.txt");
    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto info = reader.getItemInfo(0);
    EXPECT_EQ(info.path, L"memory.txt");
    EXPECT_EQ(info.size, data.size());

    reader.close();
}

TEST_F(ArchiveWriterTest, AddEmptyDirectory) {
    auto archivePath = tempDir_ / "withdir.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addEmptyDirectory(L"emptydir");
    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto info = reader.getItemInfo(0);
    EXPECT_EQ(info.path, L"emptydir");
    EXPECT_TRUE(info.isDirectory);

    reader.close();
}

TEST_F(ArchiveWriterTest, AddDirectory) {
    auto archivePath = tempDir_ / "directory.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addDirectory(testFilesDir_.wstring(), L"root");
    writer.finalize();

    // Verify - should have directory + files
    ArchiveReader reader;
    reader.open(archivePath.wstring());

    // Should contain: root/, root/test1.txt, root/test2.txt, root/binary.bin,
    // root/subdir/, root/subdir/nested.txt
    EXPECT_GE(reader.getItemCount(), 4);  // At least the files

    reader.close();
}

TEST_F(ArchiveWriterTest, RoundTripVerification) {
    auto archivePath = tempDir_ / "roundtrip.7z";
    auto extractDir = tempDir_ / "extracted";

    // Create archive
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile((testFilesDir_ / "test1.txt").wstring(), L"test1.txt");
    writer.finalize();

    // Extract and verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    reader.extractAll(extractDir.wstring());
    reader.close();

    auto extractedFile = extractDir / "test1.txt";
    ASSERT_TRUE(fs::exists(extractedFile));

    std::string originalContent = "Hello, World!";
    std::string extractedContent = readFile(extractedFile);
    EXPECT_EQ(originalContent, extractedContent);
}

// ============================================================================
// Compression Parameter Tests
// ============================================================================

TEST_F(ArchiveWriterTest, CompressionLevelNone) {
    // Use ZIP format for stored (uncompressed) files - 7z format Copy codec has issues
    auto archivePath = tempDir_ / "no_compression.zip";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::None;
    props.method = CompressionMethod::Copy;  // Explicitly use Copy method for no compression

    writer.create(archivePath.wstring(), ArchiveFormat::Zip);  // Use ZIP format
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // File should be stored without compression, archive size close to original
    auto originalSize = fs::file_size(testFile);
    auto archiveSize = fs::file_size(archivePath);
    EXPECT_GT(archiveSize, originalSize * 0.8);  // Archive overhead, but not much compression
}

TEST_F(ArchiveWriterTest, CompressionLevelUltra) {
    auto archivePath = tempDir_ / "ultra_compression.7z";

    // Create larger test file for meaningful compression
    auto largeFile = testFilesDir_ / "large.txt";
    std::string content(10000, 'A');  // Highly compressible
    createTestFile(largeFile, content);

    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Ultra;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(largeFile.wstring(), L"large.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // Should achieve good compression on repetitive data
    auto originalSize = fs::file_size(largeFile);
    auto archiveSize = fs::file_size(archivePath);
    EXPECT_LT(archiveSize, originalSize * 0.1);  // Should compress to <10%
}

TEST_F(ArchiveWriterTest, CompressionMethodLZMA2) {
    auto archivePath = tempDir_ / "lzma2.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.method = CompressionMethod::LZMA2;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // Verify can be read back
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);
    reader.close();
}

TEST_F(ArchiveWriterTest, SolidCompression) {
    auto archivePath = tempDir_ / "solid.7z";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.solid = true;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile((testFilesDir_ / "test1.txt").wstring(), L"test1.txt");
    writer.addFile((testFilesDir_ / "test2.txt").wstring(), L"test2.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, NonSolidCompression) {
    auto archivePath = tempDir_ / "nonsolid.7z";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.solid = false;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile((testFilesDir_ / "test1.txt").wstring(), L"test1.txt");
    writer.addFile((testFilesDir_ / "test2.txt").wstring(), L"test2.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, MultiThreadedCompression) {
    auto archivePath = tempDir_ / "multithreaded.7z";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.numThreads = 2;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile((testFilesDir_ / "test1.txt").wstring(), L"test1.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));
}

// ============================================================================
// Multi-Format Tests
// ============================================================================

TEST_F(ArchiveWriterTest, CreateZipArchive) {
    auto archivePath = tempDir_ / "test.zip";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::Zip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // Verify with reader
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);
    reader.close();
}

TEST_F(ArchiveWriterTest, CreateTarArchive) {
    auto archivePath = tempDir_ / "test.tar";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::Tar);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // Verify with reader
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);
    reader.close();
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(ArchiveWriterTest, InvalidOutputPath) {
    ArchiveWriter writer;

    // Path with invalid characters or non-existent directory
    auto invalidPath = L"Z:\\nonexistent\\path\\archive.7z";

    EXPECT_THROW(writer.create(invalidPath, ArchiveFormat::SevenZip), Exception);
}

TEST_F(ArchiveWriterTest, AddNonExistentFile) {
    auto archivePath = tempDir_ / "error.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

    EXPECT_THROW(writer.addFile(L"nonexistent.txt", L"file.txt"), Exception);
}

TEST_F(ArchiveWriterTest, FinalizeWithoutCreate) {
    ArchiveWriter writer;
    EXPECT_THROW(writer.finalize(), Exception);
}

TEST_F(ArchiveWriterTest, DoubleFinalize) {
    auto archivePath = tempDir_ / "double.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.finalize();

    // Second finalize should throw
    EXPECT_THROW(writer.finalize(), Exception);
}

TEST_F(ArchiveWriterTest, AddFileAfterFinalize) {
    auto archivePath = tempDir_ / "afterfinalize.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.finalize();

    // Adding file after finalize should throw
    EXPECT_THROW(writer.addFile(testFile.wstring(), L"test.txt"), Exception);
}

// ============================================================================
// Progress Callback Tests
// ============================================================================

TEST_F(ArchiveWriterTest, ProgressCallback) {
    auto archivePath = tempDir_ / "progress.7z";

    // Create larger file for meaningful progress
    auto largeFile = testFilesDir_ / "large.txt";
    std::string content(100000, 'X');
    createTestFile(largeFile, content);

    bool progressCalled = false;
    uint64_t lastCompleted = 0;
    uint64_t totalSize = 0;

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProgressCallback([&](uint64_t completed, uint64_t total) {
        progressCalled = true;
        lastCompleted = completed;
        totalSize = total;
        return true;  // Continue
    });

    writer.addFile(largeFile.wstring(), L"large.txt");
    writer.finalize();

    EXPECT_TRUE(progressCalled);
    EXPECT_GT(totalSize, 0);
    EXPECT_LE(lastCompleted, totalSize);
}

TEST_F(ArchiveWriterTest, ProgressCallbackCancellation) {
    auto archivePath = tempDir_ / "cancelled.7z";

    // Create larger file
    auto largeFile = testFilesDir_ / "large.txt";
    std::string content(100000, 'Y');
    createTestFile(largeFile, content);

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProgressCallback([](uint64_t completed, uint64_t total) {
        return false;  // Cancel immediately
    });

    writer.addFile(largeFile.wstring(), L"large.txt");

    // Finalize should throw due to cancellation
    EXPECT_THROW(writer.finalize(), Exception);
}

// ============================================================================
// Encryption Tests
// ============================================================================

TEST_F(ArchiveWriterTest, PasswordProtectedArchive) {
    // Testing with IStreamGetProps implemented
    // GTEST_SKIP() << "Password + files causes crash in 7-Zip encoder - under investigation";
    // return;  // Must return after GTEST_SKIP

    auto archivePath = tempDir_ / "encrypted.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.password = L"testpassword123";

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // Verify with reader using password
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    reader.setPasswordCallback([]() { return L"testpassword123"; });

    auto extractDir = tempDir_ / "encrypted_extracted";
    EXPECT_NO_THROW(reader.extractAll(extractDir.wstring()));

    EXPECT_TRUE(fs::exists(extractDir / "test.txt"));
    reader.close();
}

TEST_F(ArchiveWriterTest, EncryptedHeaders) {
    auto archivePath = tempDir_ / "encrypted_headers.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.password = L"secret";
    props.encryptHeaders = true;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();
    writer.close();  // Explicitly close writer to ensure file is flushed

    ASSERT_TRUE(fs::exists(archivePath));

    // Reader should require password even to list files
    ArchiveReader reader;
    reader.setPasswordCallback([]() { return L"secret"; });  // Set password BEFORE opening
    reader.open(archivePath.wstring());

    EXPECT_NO_THROW(reader.getItemCount());
    reader.close();
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ArchiveWriterTest, EmptyFileContent) {
    auto archivePath = tempDir_ / "empty_content.7z";
    auto emptyFile = testFilesDir_ / "empty.txt";
    createTestFile(emptyFile, "");

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(emptyFile.wstring(), L"empty.txt");
    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto info = reader.getItemInfo(0);
    EXPECT_EQ(info.size, 0);

    reader.close();
}

TEST_F(ArchiveWriterTest, LargeFilePath) {
    auto archivePath = tempDir_ / "longpath.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    // Create long path name
    std::wstring longPath = L"a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z/";
    longPath += L"verylongfilename_with_many_characters_to_test_path_limits.txt";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), longPath);
    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto info = reader.getItemInfo(0);
    // 7-Zip normalizes path separators to backslash on Windows
    std::wstring expectedPath = longPath;
    std::replace(expectedPath.begin(), expectedPath.end(), L'/', L'\\');
    EXPECT_EQ(info.path, expectedPath);

    reader.close();
}

TEST_F(ArchiveWriterTest, UnicodeFileName) {
    auto archivePath = tempDir_ / "unicode.7z";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"测试文件_テスト_тест_🎉.txt");
    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto info = reader.getItemInfo(0);
    EXPECT_EQ(info.path, L"测试文件_テスト_тест_🎉.txt");

    reader.close();
}

TEST_F(ArchiveWriterTest, MixedFilesAndDirectories) {
    auto archivePath = tempDir_ / "mixed.7z";

    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

    // Add files and directories in mixed order
    writer.addEmptyDirectory(L"dir1");
    writer.addFile((testFilesDir_ / "test1.txt").wstring(), L"file1.txt");
    writer.addEmptyDirectory(L"dir2");
    writer.addFile((testFilesDir_ / "test2.txt").wstring(), L"dir1/file2.txt");
    writer.addEmptyDirectory(L"dir1/subdir");

    writer.finalize();

    // Verify
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 5);
    reader.close();
}
