// test_extraction.cpp - Tests for archive extraction functionality
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "wrapper/archive/archive_reader.hpp"
#include "wrapper/common/wrapper_error.hpp"

using namespace sevenzip::detail;
namespace fs = std::filesystem;

class ExtractionTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDataDir_ = fs::path(__FILE__).parent_path().parent_path() / "data" / "archives";
    }

    fs::path testDataDir_;
};

TEST_F(ExtractionTest, ExtractToMemoryBasic) {
    // 打开测试压缩包
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 提取第一个文件到内存
    ASSERT_GT(reader.getItemCount(), 0u);

    auto data = reader.extractToMemory(0);

    // 验证数据非空
    EXPECT_FALSE(data.empty());

    // 验证数据内容（test.7z中第一个文件内容，可能带UTF-8 BOM）
    std::string content(data.begin(), data.end());
    // Windows平台文件使用\r\n换行
    std::string expected =
        "This is test file 1\r\nWith multiple lines\r\nFor archive testing\r\r\n";

    // 如果有UTF-8 BOM (EF BB BF)，去掉它
    if (content.size() >= 3 && static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        EXPECT_EQ(content.substr(3), expected);
    } else {
        EXPECT_EQ(content, expected);
    }
}

TEST_F(ExtractionTest, ExtractToMemoryInvalidIndex) {
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 尝试提取不存在的索引
    uint32_t count = reader.getItemCount();
    EXPECT_THROW(reader.extractToMemory(count), std::exception);
}

TEST_F(ExtractionTest, ExtractToFileBasic) {
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 提取到临时文件
    auto tempFile = fs::temp_directory_path() / "extracted_test.txt";

    // 清理可能存在的旧文件
    if (fs::exists(tempFile)) {
        fs::remove(tempFile);
    }

    reader.extractToFile(0, tempFile.wstring());

    // 验证文件存在
    ASSERT_TRUE(fs::exists(tempFile));

    // 验证文件内容
    {
        std::ifstream file(tempFile, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        // Windows平台文件使用\r\n换行
        std::string expected =
            "This is test file 1\r\nWith multiple lines\r\nFor archive testing\r\r\n";

        // 如果有UTF-8 BOM (EF BB BF)，去掉它
        if (content.size() >= 3 && static_cast<unsigned char>(content[0]) == 0xEF &&
            static_cast<unsigned char>(content[1]) == 0xBB &&
            static_cast<unsigned char>(content[2]) == 0xBF) {
            EXPECT_EQ(content.substr(3), expected);
        } else {
            EXPECT_EQ(content, expected);
        }
    }  // 文件流在这里自动关闭

    // 清理
    fs::remove(tempFile);
}

TEST_F(ExtractionTest, ExtractAllBasic) {
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 提取所有文件到临时目录
    auto tempDir = fs::temp_directory_path() / "extracted";

    // 清理可能存在的旧目录
    if (fs::exists(tempDir)) {
        fs::remove_all(tempDir);
    }

    reader.extractAll(tempDir.wstring());

    // 验证目录存在
    ASSERT_TRUE(fs::exists(tempDir));

    // 验证至少有一个文件被提取
    EXPECT_FALSE(fs::is_empty(tempDir));

    // 清理
    fs::remove_all(tempDir);
}

TEST_F(ExtractionTest, TestArchiveValid) {
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 测试归档完整性
    EXPECT_TRUE(reader.testArchive());
}

TEST_F(ExtractionTest, ExtractWithProgressCallback) {
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 设置进度回调
    uint64_t lastCompleted = 0;
    uint64_t lastTotal = 0;
    int callbackCount = 0;

    reader.setProgressCallback([&](uint64_t completed, uint64_t total) {
        lastCompleted = completed;
        lastTotal = total;
        callbackCount++;
        return true;  // 继续
    });

    // 提取文件
    auto data = reader.extractToMemory(0);

    // 验证回调被调用
    EXPECT_GT(callbackCount, 0);
    EXPECT_GT(lastTotal, 0u);
}

TEST_F(ExtractionTest, ExtractProgressCallbackCancellation) {
    ArchiveReader reader;
    auto testFile = testDataDir_ / "test.7z";
    reader.open(testFile.wstring());

    // 设置会取消操作的进度回调
    reader.setProgressCallback([](uint64_t, uint64_t) {
        return false;  // 取消
    });

    // 提取应该被中止
    EXPECT_THROW(reader.extractToMemory(0), std::exception);
}
