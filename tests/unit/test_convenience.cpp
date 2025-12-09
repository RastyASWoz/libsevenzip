#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>

#include "sevenzip/convenience.hpp"
#include "sevenzip/error.hpp"

namespace fs = std::filesystem;

class ConvenienceTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "sevenzip_convenience_test";
        fs::create_directories(testDir);

        // 创建测试文件
        testFile = testDir / "test.txt";
        std::ofstream ofs(testFile);
        ofs << "Hello, convenience functions!";
        ofs.close();

        // 创建测试目录
        testSubDir = testDir / "subdir";
        fs::create_directories(testSubDir);

        auto subFile = testSubDir / "subfile.txt";
        std::ofstream ofs2(subFile);
        ofs2 << "File in subdirectory";
        ofs2.close();

        // 归档路径
        archivePath = testDir / "test.7z";
        extractDir = testDir / "extracted";
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
    fs::path testFile;
    fs::path testSubDir;
    fs::path archivePath;
    fs::path extractDir;
};

TEST_F(ConvenienceTest, CompressAndExtractFile) {
    // 压缩单个文件
    sevenzip::compress(testFile, archivePath);

    EXPECT_TRUE(fs::exists(archivePath));
    EXPECT_GT(fs::file_size(archivePath), 0u);

    // 解压
    sevenzip::extract(archivePath, extractDir);

    auto extractedFile = extractDir / "test.txt";
    EXPECT_TRUE(fs::exists(extractedFile));

    // 验证内容
    std::ifstream ifs(extractedFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "Hello, convenience functions!");
}

TEST_F(ConvenienceTest, CompressAndExtractDirectory) {
    // 压缩目录
    sevenzip::compress(testSubDir, archivePath);

    EXPECT_TRUE(fs::exists(archivePath));

    // 解压
    sevenzip::extract(archivePath, extractDir);

    // 文件应该在 extractDir/subdir/subfile.txt (保留目录结构)
    auto extractedFile = extractDir / "subdir" / "subfile.txt";
    EXPECT_TRUE(fs::exists(extractedFile));

    std::ifstream ifs(extractedFile);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "File in subdirectory");
}

TEST_F(ConvenienceTest, CompressWithFormat) {
    // 测试不同格式
    auto zipPath = testDir / "test.zip";
    sevenzip::compress(testFile, zipPath, sevenzip::Format::Zip);
    EXPECT_TRUE(fs::exists(zipPath));

    auto tarPath = testDir / "test.tar";
    sevenzip::compress(testFile, tarPath, sevenzip::Format::Tar);
    EXPECT_TRUE(fs::exists(tarPath));
}

TEST_F(ConvenienceTest, CompressWithLevel) {
    // Fast compression
    auto fastPath = testDir / "fast.7z";
    sevenzip::compress(testFile, fastPath, sevenzip::Format::SevenZip,
                       sevenzip::CompressionLevel::Fast);
    EXPECT_TRUE(fs::exists(fastPath));

    // Maximum compression
    auto maxPath = testDir / "max.7z";
    sevenzip::compress(testFile, maxPath, sevenzip::Format::SevenZip,
                       sevenzip::CompressionLevel::Maximum);
    EXPECT_TRUE(fs::exists(maxPath));

    // Fast应该稍大或相�?(小文件可能差不多)
    auto fastSize = fs::file_size(fastPath);
    auto maxSize = fs::file_size(maxPath);
    EXPECT_GT(fastSize, 0u);
    EXPECT_GT(maxSize, 0u);
}

TEST_F(ConvenienceTest, CompressWithPassword) {
    // 压缩带密码
    sevenzip::compress(testFile, archivePath, sevenzip::Format::SevenZip,
                       sevenzip::CompressionLevel::Normal, "secret123");

    EXPECT_TRUE(fs::exists(archivePath));

    // 不带密码解压应该失败
    EXPECT_THROW(sevenzip::extract(archivePath, extractDir), std::exception);

    // 带密码解压应该成�?
    sevenzip::extract(archivePath, extractDir, "secret123");

    auto extractedFile = extractDir / "test.txt";
    EXPECT_TRUE(fs::exists(extractedFile));
}

TEST_F(ConvenienceTest, CompressAndExtractData) {
    // 创建可压缩的测试数据 (重复模式)
    std::vector<uint8_t> testData(1000);
    for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] = static_cast<uint8_t>(i % 10);  // 重复0-9
    }

    // 压缩数据
    auto compressed = sevenzip::compressData(testData);

    EXPECT_GT(compressed.size(), 0u);
    // 不验证压缩比,因为7z归档开销可能较大

    // 写入文件并解�?
    std::ofstream ofs(archivePath, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    ofs.close();

    auto extracted = sevenzip::extractSingleFile(archivePath);
    EXPECT_EQ(extracted, testData);
}

TEST_F(ConvenienceTest, ExtractSingleFile) {
    // 创建包含单个文件的归�?
    sevenzip::compress(testFile, archivePath);

    // 解压单个文件到内�?
    auto data = sevenzip::extractSingleFile(archivePath);

    std::string content(data.begin(), data.end());
    EXPECT_EQ(content, "Hello, convenience functions!");
}

TEST_F(ConvenienceTest, ExtractSingleFileWithIndex) {
    // 创建包含多个文件的归�?
    auto archive = sevenzip::Archive::create(archivePath);
    archive.addFile(testFile, "file1.txt");
    archive.addFile(testFile, "file2.txt");
    archive.finalize();

    // 解压指定索引的文�?
    auto data0 = sevenzip::extractSingleFile(archivePath, 0);
    auto data1 = sevenzip::extractSingleFile(archivePath, 1);

    std::string content0(data0.begin(), data0.end());
    std::string content1(data1.begin(), data1.end());

    EXPECT_EQ(content0, "Hello, convenience functions!");
    EXPECT_EQ(content1, "Hello, convenience functions!");
}

TEST_F(ConvenienceTest, List) {
    // 创建归档
    auto archive = sevenzip::Archive::create(archivePath);
    archive.addFile(testFile, "test.txt");
    archive.addDirectory(testSubDir, false);
    archive.finalize();

    // 列出内容
    auto items = sevenzip::list(archivePath);

    EXPECT_GE(items.size(), 1u);  // 至少有一个文�?

    bool foundTestFile = false;
    for (const auto& item : items) {
        if (item.path == "test.txt") {
            foundTestFile = true;
            EXPECT_FALSE(item.isDirectory);
        }
    }
    EXPECT_TRUE(foundTestFile);
}

TEST_F(ConvenienceTest, ListWithPassword) {
    // 创建加密归档(不加密文件头,这样可以列出但不能提取)
    auto archive = sevenzip::Archive::create(archivePath);
    archive.withPassword("secret");
    // 不加密文件头,可以列出文件但无法提取内容
    archive.addFile(testFile);
    archive.finalize();

    // 不带密码也可以列出文件名(因为文件头未加密)
    auto items = sevenzip::list(archivePath);
    EXPECT_GE(items.size(), 1u);

    // 带密码列出也应该成功
    auto itemsWithPwd = sevenzip::list(archivePath, "secret");
    EXPECT_GE(itemsWithPwd.size(), 1u);
}

TEST_F(ConvenienceTest, TestArchive) {
    // 创建有效归档
    sevenzip::compress(testFile, archivePath);

    // 测试完整�?
    EXPECT_TRUE(sevenzip::testArchive(archivePath));

    // 损坏归档
    {
        std::fstream fs(archivePath, std::ios::in | std::ios::out | std::ios::binary);
        fs.seekp(100);
        char garbage[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        fs.write(garbage, 10);
    }

    // 测试损坏的归�?
    EXPECT_FALSE(sevenzip::testArchive(archivePath));
}

TEST_F(ConvenienceTest, GetArchiveInfo) {
    // 创建归档
    auto archive = sevenzip::Archive::create(archivePath);
    archive.addFile(testFile);
    archive.finalize();

    // 获取信息
    auto info = sevenzip::getArchiveInfo(archivePath);

    EXPECT_EQ(info.format, sevenzip::Format::SevenZip);
    EXPECT_GT(info.itemCount, 0u);
}

TEST_F(ConvenienceTest, IsArchive) {
    // 创建归档
    sevenzip::compress(testFile, archivePath);

    // 测试是否为归�?
    EXPECT_TRUE(sevenzip::isArchive(archivePath));

    // 测试普通文�?
    EXPECT_FALSE(sevenzip::isArchive(testFile));

    // 测试不存在的文件
    EXPECT_FALSE(sevenzip::isArchive(testDir / "nonexistent.7z"));
}

TEST_F(ConvenienceTest, ExtractNonExistentArchive) {
    EXPECT_THROW(sevenzip::extract(testDir / "nonexistent.7z", extractDir), std::exception);
}

TEST_F(ConvenienceTest, CompressNonExistentFile) {
    EXPECT_THROW(sevenzip::compress(testDir / "nonexistent.txt", archivePath), std::exception);
}

TEST_F(ConvenienceTest, ExtractSingleFileFromEmpty) {
    // 创建空归档（实际上7z不支持完全空归档，但我们可以测试没有文件的情况）
    auto emptyArchive = sevenzip::Archive::create(archivePath);
    emptyArchive.finalize();

    EXPECT_THROW(sevenzip::extractSingleFile(archivePath), std::exception);
}

// ============================================================================
// 新增测试：增强便利函数覆盖
// ============================================================================

TEST_F(ConvenienceTest, CompressDataBasic) {
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o'};

    auto compressed = sevenzip::compressData(data);
    EXPECT_GT(compressed.size(), 0u);
}

TEST_F(ConvenienceTest, CompressDataWithFormat) {
    std::vector<uint8_t> data(1000, 'X');

    auto zip = sevenzip::compressData(data, sevenzip::Format::Zip);
    auto bz2 = sevenzip::compressData(data, sevenzip::Format::BZip2);

    EXPECT_GT(zip.size(), 0u);
    EXPECT_GT(bz2.size(), 0u);
}

TEST_F(ConvenienceTest, CompressDataWithLevel) {
    std::vector<uint8_t> data(10000, 'A');

    auto fast =
        sevenzip::compressData(data, sevenzip::Format::SevenZip, sevenzip::CompressionLevel::Fast);
    auto max = sevenzip::compressData(data, sevenzip::Format::SevenZip,
                                      sevenzip::CompressionLevel::Maximum);

    EXPECT_GT(fast.size(), 0u);
    EXPECT_GT(max.size(), 0u);
}

TEST_F(ConvenienceTest, ExtractWithPassword) {
    // Create archive with password
    sevenzip::compress(testFile, archivePath, sevenzip::Format::SevenZip,
                       sevenzip::CompressionLevel::Normal, "password123");

    EXPECT_TRUE(fs::exists(archivePath));

    // Extract with password
    sevenzip::extract(archivePath, extractDir, "password123");

    auto extractedFile = extractDir / "test.txt";
    EXPECT_TRUE(fs::exists(extractedFile));
}

TEST_F(ConvenienceTest, ExtractWithFormatAndPassword) {
    // Create zip with password
    auto zipPath = testDir / "test.zip";
    sevenzip::compress(testFile, zipPath, sevenzip::Format::Zip, sevenzip::CompressionLevel::Normal,
                       "secret");

    // Extract with format specification
    sevenzip::extract(zipPath, extractDir, sevenzip::Format::Zip, "secret");

    EXPECT_TRUE(fs::exists(extractDir / "test.txt"));
}
