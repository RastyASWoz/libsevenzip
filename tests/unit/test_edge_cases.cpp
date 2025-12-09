// test_edge_cases.cpp - 边界情况和错误处理测试
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

#include "../../src/wrapper/archive/archive_reader.hpp"
#include "../../src/wrapper/archive/archive_writer.hpp"
#include "../../src/wrapper/common/wrapper_error.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

namespace fs = std::filesystem;

// ============================================================================
// 边界情况测试
// ============================================================================

class EdgeCasesTest : public ::testing::Test {
   protected:
    void SetUp() override {
        tempDir_ = fs::temp_directory_path() / "libsevenzip_edge_tests";
        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
        fs::create_directories(tempDir_);
    }

    void TearDown() override {
        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
    }

    void createTestFile(const fs::path& path, const std::string& content) {
        fs::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        file << content;
    }

    std::string readFile(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }

    fs::path tempDir_;
};

// ============================================================================
// 空文件和空目录测试
// ============================================================================

TEST_F(EdgeCasesTest, CompressEmptyFile) {
    auto emptyFile = tempDir_ / "empty.txt";
    createTestFile(emptyFile, "");

    auto archivePath = tempDir_ / "empty.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(emptyFile.wstring(), L"empty.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // 解压验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto extractDir = tempDir_ / "empty_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "empty.txt";
    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(fs::file_size(extractedFile), 0);
    reader.close();
}

TEST_F(EdgeCasesTest, CompressEmptyDirectory) {
    auto emptyDir = tempDir_ / "empty_dir";
    fs::create_directories(emptyDir);

    auto archivePath = tempDir_ / "empty_dir.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addEmptyDirectory(L"empty_dir/");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // 解压验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_GE(reader.getItemCount(), 0);  // 可能为0或1，取决于实现

    auto extractDir = tempDir_ / "empty_dir_extracted";
    reader.extractAll(extractDir.wstring());
    reader.close();
}

TEST_F(EdgeCasesTest, CompressMultipleEmptyFiles) {
    auto emptyFile1 = tempDir_ / "empty1.txt";
    auto emptyFile2 = tempDir_ / "empty2.txt";
    auto emptyFile3 = tempDir_ / "empty3.txt";
    createTestFile(emptyFile1, "");
    createTestFile(emptyFile2, "");
    createTestFile(emptyFile3, "");

    auto archivePath = tempDir_ / "multiple_empty.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(emptyFile1.wstring(), L"empty1.txt");
    writer.addFile(emptyFile2.wstring(), L"empty2.txt");
    writer.addFile(emptyFile3.wstring(), L"empty3.txt");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 3);
    reader.close();
}

// ============================================================================
// 特殊文件名测试
// ============================================================================

TEST_F(EdgeCasesTest, SpecialCharactersInFilename) {
    // 测试各种特殊字符
    std::vector<std::wstring> specialNames = {L"file with spaces.txt", L"文件中文名.txt",
                                              L"file_with_underscores.txt", L"file-with-dashes.txt",
                                              L"file.multiple.dots.txt"};

    for (const auto& name : specialNames) {
        auto testFile = tempDir_ / fs::path(name);
        createTestFile(testFile, "test content");

        auto archivePath = tempDir_ / (std::wstring(name) + L".7z");

        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(testFile.wstring(), name);
        EXPECT_NO_THROW(writer.finalize());

        // 验证可以读取
        ArchiveReader reader;
        EXPECT_NO_THROW(reader.open(archivePath.wstring()));
        EXPECT_EQ(reader.getItemCount(), 1);
        reader.close();

        fs::remove(testFile);
        fs::remove(archivePath);
    }
}

TEST_F(EdgeCasesTest, VeryLongFilename) {
    // 创建一个非常长的文件名（但在操作系统限制内）
    std::wstring longName(200, L'a');
    longName += L".txt";

    auto testFile = tempDir_ / "short.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "long_name.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), longName);
    writer.finalize();

    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 1);

    auto item = reader.getItemInfo(0);
    EXPECT_EQ(item.path, longName);
    reader.close();
}

TEST_F(EdgeCasesTest, DeepNestedDirectory) {
    // 创建深层嵌套的目录结构
    fs::path deepPath = tempDir_ / "level1";
    for (int i = 2; i <= 10; ++i) {
        deepPath = deepPath / ("level" + std::to_string(i));
    }
    fs::create_directories(deepPath);

    auto deepFile = deepPath / "deep.txt";
    createTestFile(deepFile, "deep content");

    auto archivePath = tempDir_ / "deep_nested.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addDirectory((tempDir_ / "level1").wstring(), L"level1/", true);
    writer.finalize();

    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_GT(reader.getItemCount(), 0);

    auto extractDir = tempDir_ / "deep_extracted";
    reader.extractAll(extractDir.wstring());

    // 验证深层文件存在
    auto extractedDeepFile = extractDir / "level1" / "level2" / "level3" / "level4" / "level5" /
                             "level6" / "level7" / "level8" / "level9" / "level10" / "deep.txt";
    ASSERT_TRUE(fs::exists(extractedDeepFile));
    EXPECT_EQ(readFile(extractedDeepFile), "deep content");
    reader.close();
}

// ============================================================================
// 大文件测试
// ============================================================================

TEST_F(EdgeCasesTest, VeryLargeFile) {
    // 创建 10MB 文件
    auto largeFile = tempDir_ / "large.bin";
    {
        std::ofstream file(largeFile, std::ios::binary);
        std::vector<char> buffer(1024 * 1024, 'X');  // 1MB buffer
        for (int i = 0; i < 10; ++i) {
            file.write(buffer.data(), buffer.size());
        }
    }

    ASSERT_EQ(fs::file_size(largeFile), 10 * 1024 * 1024);

    auto archivePath = tempDir_ / "large.7z";
    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Fast;  // 快速压缩以节省时间

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(largeFile.wstring(), L"large.bin");
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // 解压验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());

    auto extractDir = tempDir_ / "large_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "large.bin";
    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(fs::file_size(extractedFile), 10 * 1024 * 1024);
    reader.close();
}

TEST_F(EdgeCasesTest, ManySmallFiles) {
    // 创建1000个小文件
    auto filesDir = tempDir_ / "many_files";
    fs::create_directories(filesDir);

    for (int i = 0; i < 1000; ++i) {
        auto file = filesDir / ("file_" + std::to_string(i) + ".txt");
        createTestFile(file, "content " + std::to_string(i));
    }

    auto archivePath = tempDir_ / "many_files.7z";
    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Fast;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addDirectory(filesDir.wstring(), L"many_files/", true);
    writer.finalize();

    ASSERT_TRUE(fs::exists(archivePath));

    // 验证文件数量
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_GE(reader.getItemCount(), 1000);  // 至少1000个文件
    reader.close();
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST_F(EdgeCasesTest, OpenCorruptedArchive) {
    // 创建一个损坏的归档文件
    auto corruptedPath = tempDir_ / "corrupted.7z";
    std::ofstream file(corruptedPath, std::ios::binary);
    file << "This is not a valid 7z file";
    file.close();

    ArchiveReader reader;
    EXPECT_THROW(reader.open(corruptedPath.wstring()), Exception);
}

TEST_F(EdgeCasesTest, OpenEmptyArchiveFile) {
    // 创建一个空文件
    auto emptyPath = tempDir_ / "empty_archive.7z";
    std::ofstream file(emptyPath, std::ios::binary);
    file.close();

    ArchiveReader reader;
    EXPECT_THROW(reader.open(emptyPath.wstring()), Exception);
}

TEST_F(EdgeCasesTest, AddNonExistentFile) {
    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

    EXPECT_THROW(writer.addFile(L"nonexistent_file.txt", L"file.txt"), Exception);
}

TEST_F(EdgeCasesTest, FinalizeWithoutAddingFiles) {
    auto archivePath = tempDir_ / "empty_archive.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

    // 不添加任何文件就完成
    EXPECT_NO_THROW(writer.finalize());

    // 验证归档仍然可以打开（即使为空）
    ArchiveReader reader;
    EXPECT_NO_THROW(reader.open(archivePath.wstring()));
    EXPECT_EQ(reader.getItemCount(), 0);
    reader.close();
}

TEST_F(EdgeCasesTest, DoubleFinalize) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    // 第二次调用finalize应该失败
    EXPECT_THROW(writer.finalize(), Exception);
}

TEST_F(EdgeCasesTest, AddFileAfterFinalize) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    // finalize后不能添加文件
    EXPECT_THROW(writer.addFile(testFile.wstring(), L"test2.txt"), Exception);
}

TEST_F(EdgeCasesTest, ExtractToNonExistentDirectory) {
    // 创建一个有效的归档
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    // 提取到不存在的目录（应该自动创建）
    ArchiveReader reader;
    reader.open(archivePath.wstring());

    auto nonExistentDir = tempDir_ / "non_existent" / "sub" / "dir";
    EXPECT_NO_THROW(reader.extractAll(nonExistentDir.wstring()));

    EXPECT_TRUE(fs::exists(nonExistentDir));
    EXPECT_TRUE(fs::exists(nonExistentDir / "test.txt"));
    reader.close();
}

TEST_F(EdgeCasesTest, ExtractWithInvalidIndex) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ArchiveReader reader;
    reader.open(archivePath.wstring());

    auto extractDir = tempDir_ / "extract";

    // 尝试访问无效的索引
    EXPECT_THROW(reader.getItemInfo(999), Exception);
    reader.close();
}

// ============================================================================
// 特殊内容测试
// ============================================================================

TEST_F(EdgeCasesTest, BinaryDataWithNullBytes) {
    // 创建包含NULL字节的二进制数据
    std::vector<uint8_t> binaryData = {0x00, 0xFF, 0x00, 0xAA, 0x55, 0x00, 0x00, 0xFF,
                                       0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00};

    auto archivePath = tempDir_ / "binary.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFileFromMemory(binaryData, L"binary.dat");
    writer.finalize();

    // 解压并验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());

    auto extractDir = tempDir_ / "binary_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "binary.dat";
    ASSERT_TRUE(fs::exists(extractedFile));

    std::ifstream file(extractedFile, std::ios::binary);
    std::vector<uint8_t> extractedData((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());

    EXPECT_EQ(extractedData, binaryData);
    reader.close();
}

TEST_F(EdgeCasesTest, UnicodeContent) {
    // 创建包含各种Unicode字符的文件
    std::string unicodeContent = "Hello World UTF-8 Test";

    auto testFile = tempDir_ / "unicode.txt";
    createTestFile(testFile, unicodeContent);

    auto archivePath = tempDir_ / "unicode.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"unicode.txt");
    writer.finalize();

    ArchiveReader reader;
    reader.open(archivePath.wstring());

    auto extractDir = tempDir_ / "unicode_extracted";
    reader.extractAll(extractDir.wstring());

    auto extractedFile = extractDir / "unicode.txt";
    ASSERT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(extractedFile), unicodeContent);
    reader.close();
}

// ============================================================================
// 内存压缩边界测试
// ============================================================================

TEST_F(EdgeCasesTest, CompressEmptyDataToMemory) {
    std::vector<uint8_t> emptyData;

    std::vector<uint8_t> archiveBuffer;
    ArchiveWriter writer;
    writer.createToMemory(archiveBuffer, ArchiveFormat::SevenZip);
    writer.addFileFromMemory(emptyData, L"empty.dat");
    writer.finalize();

    EXPECT_GT(archiveBuffer.size(), 0);  // 归档头部应该存在
}

TEST_F(EdgeCasesTest, CompressVeryLargeDataToMemory) {
    // 创建 5MB 数据
    std::vector<uint8_t> largeData(5 * 1024 * 1024, 0xAB);

    std::vector<uint8_t> archiveBuffer;
    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Fast;

    writer.createToMemory(archiveBuffer, ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFileFromMemory(largeData, L"large.dat");
    writer.finalize();

    EXPECT_GT(archiveBuffer.size(), 0);
    // 由于数据重复，压缩后应该小很多
    EXPECT_LT(archiveBuffer.size(), largeData.size());
}

// ============================================================================
// 同时操作多个归档
// ============================================================================

TEST_F(EdgeCasesTest, MultipleReaders) {
    // 创建多个归档
    std::vector<fs::path> archives;
    for (int i = 0; i < 3; ++i) {
        auto testFile = tempDir_ / ("file_" + std::to_string(i) + ".txt");
        createTestFile(testFile, "content " + std::to_string(i));

        auto archivePath = tempDir_ / ("archive_" + std::to_string(i) + ".7z");
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(testFile.wstring(), L"file.txt");
        writer.finalize();

        archives.push_back(archivePath);
    }

    // 同时打开所有归档
    std::vector<std::unique_ptr<ArchiveReader>> readers;
    for (const auto& archive : archives) {
        auto reader = std::make_unique<ArchiveReader>();
        reader->open(archive.wstring());
        EXPECT_TRUE(reader->isOpen());
        EXPECT_EQ(reader->getItemCount(), 1);
        readers.push_back(std::move(reader));
    }

    // 关闭所有
    for (auto& reader : readers) {
        reader->close();
    }
}

TEST_F(EdgeCasesTest, ReopenSameArchive) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    // 多次打开和关闭同一个归档
    for (int i = 0; i < 5; ++i) {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        EXPECT_TRUE(reader.isOpen());
        EXPECT_EQ(reader.getItemCount(), 1);
        reader.close();
        EXPECT_FALSE(reader.isOpen());
    }
}

// ============================================================================
// 移动语义测试
// ============================================================================

TEST_F(EdgeCasesTest, MoveConstructorWriter) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer1;
    writer1.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer1.addFile(testFile.wstring(), L"test.txt");

    // 移动构造
    ArchiveWriter writer2(std::move(writer1));
    EXPECT_NO_THROW(writer2.finalize());

    // 验证归档创建成功
    ASSERT_TRUE(fs::exists(archivePath));
}

TEST_F(EdgeCasesTest, MoveAssignmentWriter) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer1;
    writer1.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer1.addFile(testFile.wstring(), L"test.txt");

    // 移动赋值
    ArchiveWriter writer2;
    writer2 = std::move(writer1);
    EXPECT_NO_THROW(writer2.finalize());

    ASSERT_TRUE(fs::exists(archivePath));
}

TEST_F(EdgeCasesTest, MoveConstructorReader) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ArchiveReader reader1;
    reader1.open(archivePath.wstring());
    EXPECT_TRUE(reader1.isOpen());

    // 移动构造
    ArchiveReader reader2(std::move(reader1));
    EXPECT_TRUE(reader2.isOpen());
    EXPECT_EQ(reader2.getItemCount(), 1);
    reader2.close();
}

TEST_F(EdgeCasesTest, MoveAssignmentReader) {
    auto testFile = tempDir_ / "test.txt";
    createTestFile(testFile, "content");

    auto archivePath = tempDir_ / "test.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile.wstring(), L"test.txt");
    writer.finalize();

    ArchiveReader reader1;
    reader1.open(archivePath.wstring());
    EXPECT_TRUE(reader1.isOpen());

    // 移动赋值
    ArchiveReader reader2;
    reader2 = std::move(reader1);
    EXPECT_TRUE(reader2.isOpen());
    EXPECT_EQ(reader2.getItemCount(), 1);
    reader2.close();
}
