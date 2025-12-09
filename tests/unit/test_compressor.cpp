#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "sevenzip/compressor.hpp"

namespace fs = std::filesystem;

class CompressorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "sevenzip_compressor_test";
        fs::create_directories(testDir);

        testFile = testDir / "test.txt";
        std::ofstream ofs(testFile);
        ofs << "Hello, Compressor! This is a test file for compression.";
        ofs.close();
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
    fs::path testFile;
};

TEST_F(CompressorTest, CompressDecompressGZip) {
    sevenzip::Compressor compressor(sevenzip::Format::GZip);

    std::vector<uint8_t> testData(1000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 10);
    }

    auto compressed = compressor.compress(testData);
    EXPECT_GT(compressed.size(), 0u);

    auto decompressed = compressor.decompress(compressed);
    EXPECT_EQ(decompressed, testData);
}

TEST_F(CompressorTest, CompressDecompressBZip2) {
    sevenzip::Compressor compressor(sevenzip::Format::BZip2);

    std::vector<uint8_t> testData(1000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 10);
    }

    auto compressed = compressor.compress(testData);
    EXPECT_GT(compressed.size(), 0u);

    auto decompressed = compressor.decompress(compressed);
    EXPECT_EQ(decompressed, testData);
}

TEST_F(CompressorTest, CompressDecompressXz) {
    sevenzip::Compressor compressor(sevenzip::Format::Xz);

    std::vector<uint8_t> testData(1000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 10);
    }

    auto compressed = compressor.compress(testData);
    EXPECT_GT(compressed.size(), 0u);

    auto decompressed = compressor.decompress(compressed);
    EXPECT_EQ(decompressed, testData);
}

TEST_F(CompressorTest, CompressFileGZip) {
    auto gzPath = testDir / "test.txt.gz";
    auto outPath = testDir / "test_out.txt";

    sevenzip::Compressor compressor(sevenzip::Format::GZip);
    compressor.compressFile(testFile, gzPath);

    EXPECT_TRUE(fs::exists(gzPath));
    EXPECT_GT(fs::file_size(gzPath), 0u);

    compressor.decompressFile(gzPath, outPath);

    EXPECT_TRUE(fs::exists(outPath));

    std::ifstream ifs1(testFile);
    std::ifstream ifs2(outPath);
    std::string original((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>());
    std::string restored((std::istreambuf_iterator<char>(ifs2)), std::istreambuf_iterator<char>());

    EXPECT_EQ(original, restored);
}

TEST_F(CompressorTest, WithCompressionLevel) {
    sevenzip::Compressor fastCompressor(sevenzip::Format::GZip, sevenzip::CompressionLevel::Fast);
    sevenzip::Compressor maxCompressor(sevenzip::Format::GZip, sevenzip::CompressionLevel::Maximum);

    std::vector<uint8_t> testData(10000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 10);
    }

    auto fastCompressed = fastCompressor.compress(testData);
    auto maxCompressed = maxCompressor.compress(testData);

    EXPECT_GT(fastCompressed.size(), 0u);
    EXPECT_GT(maxCompressed.size(), 0u);
}

TEST_F(CompressorTest, ChainedConfiguration) {
    std::vector<uint8_t> testData(1000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 10);
    }

    auto compressed = sevenzip::Compressor(sevenzip::Format::GZip)
                          .withLevel(sevenzip::CompressionLevel::Maximum)
                          .compress(testData);

    EXPECT_GT(compressed.size(), 0u);

    auto decompressed = sevenzip::Compressor(sevenzip::Format::GZip).decompress(compressed);

    EXPECT_EQ(decompressed, testData);
}

TEST_F(CompressorTest, InvalidFormat) {
    EXPECT_THROW(
        { sevenzip::Compressor compressor(sevenzip::Format::SevenZip); }, std::runtime_error);
}

TEST_F(CompressorTest, EmptyData) {
    sevenzip::Compressor compressor(sevenzip::Format::GZip);

    std::vector<uint8_t> empty;
    auto compressed = compressor.compress(empty);
    EXPECT_GT(compressed.size(), 0u);  // 仍有头部信息
}

// 测试直接使用 createToMemory
TEST_F(CompressorTest, DirectMemoryGZip) {
    std::vector<uint8_t> testData(1000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 256);
    }

    std::vector<uint8_t> buffer;
    auto archive = sevenzip::Archive::createToMemory(buffer, sevenzip::Format::GZip);
    archive.addFromMemory(testData, "test.bin");
    archive.finalize();

    EXPECT_GT(buffer.size(), 0u);
}

// ============================================================================
// 新增测试：移动语义和边界条件
// ============================================================================

TEST_F(CompressorTest, MoveSemantics) {
    sevenzip::Compressor comp1(sevenzip::Format::GZip);

    std::vector<uint8_t> testData(100, 'A');
    auto compressed1 = comp1.compress(testData);

    sevenzip::Compressor comp2(std::move(comp1));
    auto decompressed = comp2.decompress(compressed1);

    EXPECT_EQ(decompressed, testData);
}

TEST_F(CompressorTest, LargeData) {
    sevenzip::Compressor compressor(sevenzip::Format::GZip);

    std::vector<uint8_t> largeData(1024 * 1024);  // 1MB
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<uint8_t>(i % 256);
    }

    auto compressed = compressor.compress(largeData);
    EXPECT_GT(compressed.size(), 0u);
    EXPECT_LT(compressed.size(), largeData.size());  // Should be smaller

    auto decompressed = compressor.decompress(compressed);
    EXPECT_EQ(decompressed, largeData);
}

TEST_F(CompressorTest, WithLevelChained) {
    std::vector<uint8_t> testData(1000, 'X');

    sevenzip::Compressor compressor(sevenzip::Format::BZip2);
    auto compressed = compressor.withLevel(sevenzip::CompressionLevel::Fast).compress(testData);

    EXPECT_GT(compressed.size(), 0u);
}
