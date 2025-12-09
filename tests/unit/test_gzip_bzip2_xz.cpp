// test_gzip_bzip2_xz.cpp - Tests for single-file compression formats
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

class SingleFileCompressionTest : public ::testing::Test {
   protected:
    void SetUp() override {
        tempDir_ = fs::temp_directory_path() / "libsevenzip_singlefile_tests";

        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
        fs::create_directories(tempDir_);

        testFilesDir_ = tempDir_ / "test_files";
        fs::create_directories(testFilesDir_);

        createTestFile(testFilesDir_ / "test1.txt", "Hello, World!");
        createTestFile(testFilesDir_ / "test2.txt", "This is a test file.");
        createTestFile(testFilesDir_ / "binary.bin", std::string(1024, '\xFF'));
    }

    void TearDown() override {
        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
    }

    void createTestFile(const fs::path& path, const std::string& content) {
        std::ofstream file(path, std::ios::binary);
        file << content;
    }

    std::string readFile(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }

    fs::path tempDir_;
    fs::path testFilesDir_;
};

// ============================================================================
// GZIP Tests
// ============================================================================

TEST_F(SingleFileCompressionTest, CreateGzipArchive) {
    auto gzPath = tempDir_ / "test.gz";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    writer.create(gzPath.wstring(), ArchiveFormat::GZip);
    writer.addFile(testFile.wstring(), L"test1.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(gzPath));
    ASSERT_GT(fs::file_size(gzPath), 0);

    // Verify by reading back
    ArchiveReader reader;
    reader.open(gzPath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto extractDir = tempDir_ / "gzip_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "test1.txt";
    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}

TEST_F(SingleFileCompressionTest, GzipWithCompressionLevel) {
    auto gzPath = tempDir_ / "test_compressed.gz";

    // Create large test data for better compression ratio comparison
    std::string largeData(10 * 1024, 'A');  // 10KB of 'A'
    auto testFile = tempDir_ / "large_repeated.txt";
    createTestFile(testFile, largeData);

    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Maximum;

    writer.create(gzPath.wstring(), ArchiveFormat::GZip);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"large_repeated.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(gzPath));

    // With maximum compression, the compressed file should be much smaller
    EXPECT_LT(fs::file_size(gzPath), 500);

    // Verify extraction
    ArchiveReader reader;
    reader.open(gzPath.wstring());

    auto extractDir = tempDir_ / "gzip_max_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "large_repeated.txt";
    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}

TEST_F(SingleFileCompressionTest, GzipFromMemory) {
    std::string testData = "Compress this data to GZIP!";
    std::vector<uint8_t> data(testData.begin(), testData.end());

    auto gzPath = tempDir_ / "from_memory.gz";

    ArchiveWriter writer;
    writer.create(gzPath.wstring(), ArchiveFormat::GZip);
    writer.addFileFromMemory(data, L"data.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(gzPath));

    // Extract and verify
    ArchiveReader reader;
    reader.open(gzPath.wstring());

    auto extractDir = tempDir_ / "gzip_mem_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "data.txt";
    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(testData, readFile(extractedFile));

    reader.close();
}

// ============================================================================
// BZIP2 Tests
// ============================================================================

TEST_F(SingleFileCompressionTest, CreateBzip2Archive) {
    auto bz2Path = tempDir_ / "test.bz2";
    auto testFile = testFilesDir_ / "test2.txt";

    ArchiveWriter writer;
    writer.create(bz2Path.wstring(), ArchiveFormat::BZip2);
    writer.addFile(testFile.wstring(), L"test2.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(bz2Path));
    ASSERT_GT(fs::file_size(bz2Path), 0);

    // Verify by reading back
    ArchiveReader reader;
    reader.open(bz2Path.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto extractDir = tempDir_ / "bzip2_extracted";
    reader.extractAll(extractDir.wstring());

    // List files in extract directory for debugging
    std::cout << "Extracted files in " << extractDir << ":\n";
    for (const auto& entry : fs::directory_iterator(extractDir)) {
        std::cout << "  - " << entry.path().filename() << "\n";
    }

    // For single-file compression formats, the extracted file might have a different name
    // Try both the expected name and the actual archive name
    auto extractedFile = extractDir / "test2.txt";
    if (!fs::exists(extractedFile)) {
        // Try without extension (common for bzip2)
        extractedFile = extractDir / "test";
        if (!fs::exists(extractedFile) && fs::exists(extractDir) && !fs::is_empty(extractDir)) {
            // Just get the first file
            extractedFile = fs::directory_iterator(extractDir)->path();
        }
    }

    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}

TEST_F(SingleFileCompressionTest, Bzip2LargeFile) {
    auto bz2Path = tempDir_ / "large.bz2";

    // Create 1MB test file
    std::string largeData(1024 * 1024, '\0');
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<char>((i % 256));
    }
    auto testFile = tempDir_ / "large_test.bin";
    createTestFile(testFile, largeData);

    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Fast;

    writer.create(bz2Path.wstring(), ArchiveFormat::BZip2);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"large_test.bin");
    writer.finalize();

    ASSERT_TRUE(fs::exists(bz2Path));
    ASSERT_GT(fs::file_size(bz2Path), 0);

    // Verify extraction
    ArchiveReader reader;
    reader.open(bz2Path.wstring());

    auto extractDir = tempDir_ / "bzip2_large_extracted";
    reader.extractAll(extractDir.wstring());

    // For single-file compression formats, get the actual extracted file
    auto extractedFile = extractDir / "large_test.bin";
    if (!fs::exists(extractedFile) && fs::exists(extractDir) && !fs::is_empty(extractDir)) {
        extractedFile = fs::directory_iterator(extractDir)->path();
    }

    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(fs::file_size(extractedFile), largeData.size());
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}

// ============================================================================
// XZ Tests
// ============================================================================

TEST_F(SingleFileCompressionTest, CreateXzArchive) {
    auto xzPath = tempDir_ / "test.xz";
    auto testFile = testFilesDir_ / "binary.bin";

    ArchiveWriter writer;
    writer.create(xzPath.wstring(), ArchiveFormat::Xz);
    writer.addFile(testFile.wstring(), L"binary.bin");
    writer.finalize();

    ASSERT_TRUE(fs::exists(xzPath));
    ASSERT_GT(fs::file_size(xzPath), 0);

    // Verify by reading back
    ArchiveReader reader;
    reader.open(xzPath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto extractDir = tempDir_ / "xz_extracted";
    reader.extractAll(extractDir.wstring());

    // For single-file compression formats, get the actual extracted file
    auto extractedFile = extractDir / "binary.bin";
    if (!fs::exists(extractedFile) && fs::exists(extractDir) && !fs::is_empty(extractDir)) {
        extractedFile = fs::directory_iterator(extractDir)->path();
    }

    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}

TEST_F(SingleFileCompressionTest, XzWithMaximumCompression) {
    auto xzPath = tempDir_ / "test_max.xz";

    // Create repetitive data that compresses well
    std::string repeatData;
    for (int i = 0; i < 1000; ++i) {
        repeatData += "This is a repeated line of text for compression testing.\n";
    }
    auto testFile = tempDir_ / "repeat.txt";
    createTestFile(testFile, repeatData);

    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Maximum;

    writer.create(xzPath.wstring(), ArchiveFormat::Xz);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"repeat.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(xzPath));

    // XZ should achieve good compression ratio
    EXPECT_LT(fs::file_size(xzPath), repeatData.size() / 10);

    // Verify extraction
    ArchiveReader reader;
    reader.open(xzPath.wstring());

    auto extractDir = tempDir_ / "xz_max_extracted";
    reader.extractAll(extractDir.wstring());

    // For single-file compression formats, get the actual extracted file
    auto extractedFile = extractDir / "repeat.txt";
    if (!fs::exists(extractedFile) && fs::exists(extractDir) && !fs::is_empty(extractDir)) {
        extractedFile = fs::directory_iterator(extractDir)->path();
    }

    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}

TEST_F(SingleFileCompressionTest, XzFastCompression) {
    auto xzPath = tempDir_ / "test_fast.xz";
    auto testFile = testFilesDir_ / "test1.txt";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Fast;

    writer.create(xzPath.wstring(), ArchiveFormat::Xz);
    writer.setProperties(props);
    writer.addFile(testFile.wstring(), L"test1.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(xzPath));

    // Verify extraction
    ArchiveReader reader;
    reader.open(xzPath.wstring());

    auto extractDir = tempDir_ / "xz_fast_extracted";
    reader.extractAll(extractDir.wstring());

    // For single-file compression formats, get the actual extracted file
    auto extractedFile = extractDir / "test1.txt";
    if (!fs::exists(extractedFile) && fs::exists(extractDir) && !fs::is_empty(extractDir)) {
        extractedFile = fs::directory_iterator(extractDir)->path();
    }

    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(testFile), readFile(extractedFile));

    reader.close();
}
