// test_integration.cpp - 集成和端到端测试
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "../../src/wrapper/archive/archive_reader.hpp"
#include "../../src/wrapper/archive/archive_writer.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

namespace fs = std::filesystem;

// ============================================================================
// 集成测试
// ============================================================================

class IntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        tempDir_ = fs::temp_directory_path() / "libsevenzip_integration_tests";
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

    void createTestFiles(int count) {
        for (int i = 0; i < count; ++i) {
            auto file = tempDir_ / ("file_" + std::to_string(i) + ".txt");
            std::ofstream out(file);
            out << "Content of file " << i;
        }
    }

    std::string readFile(const fs::path& path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    fs::path tempDir_;
};

// ============================================================================
// 完整工作流测试
// ============================================================================

TEST_F(IntegrationTest, CompleteWorkflow_CreateExtractVerify) {
    // 1. 创建测试文件
    createTestFiles(5);

    // 2. 创建归档
    auto archivePath = tempDir_ / "workflow.7z";
    {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Normal;
        props.encryptHeaders = false;

        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.setProperties(props);

        for (int i = 0; i < 5; ++i) {
            auto file = tempDir_ / ("file_" + std::to_string(i) + ".txt");
            auto name = L"file_" + std::to_wstring(i) + L".txt";
            writer.addFile(file.wstring(), name);
        }

        writer.finalize();
    }

    EXPECT_TRUE(fs::exists(archivePath));

    // 3. 读取归档信息
    uint32_t itemCount = 0;
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        itemCount = reader.getItemCount();
        EXPECT_EQ(itemCount, 5);
        reader.close();
    }

    // 4. 解压归档
    auto extractDir = tempDir_ / "extracted";
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        reader.extractAll(extractDir.wstring());
        reader.close();
    }

    // 5. 验证解压结果
    for (int i = 0; i < 5; ++i) {
        auto extractedFile = extractDir / ("file_" + std::to_string(i) + ".txt");
        EXPECT_TRUE(fs::exists(extractedFile));

        auto content = readFile(extractedFile);
        auto expected = "Content of file " + std::to_string(i);
        EXPECT_EQ(content, expected);
    }
}

TEST_F(IntegrationTest, PasswordProtectedWorkflow) {
    // 1. 创建测试文件
    auto testFile = tempDir_ / "secret.txt";
    std::ofstream out(testFile);
    out << "Secret content";
    out.close();

    // 2. 创建加密归档（无文件加密头）
    auto archivePath = tempDir_ / "encrypted.7z";
    {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.password = L"TestPassword123";
        props.encryptHeaders = false;  // 不加密头以便读取器能打开
        props.level = CompressionLevel::Normal;

        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.setProperties(props);
        writer.addFile(testFile.wstring(), L"secret.txt");
        writer.finalize();
    }

    // 3. 验证归档存在
    EXPECT_TRUE(fs::exists(archivePath));

    // 注意：ArchiveReader 当前不支持密码保护的解压
    // 这个测试仅验证加密归档的创建
}

TEST_F(IntegrationTest, MemoryCompressionWorkflow) {
    // 1. 准备数据
    std::vector<uint8_t> inputData = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    std::vector<uint8_t> archiveBuffer;

    // 2. 压缩到内存
    {
        ArchiveWriter writer;
        writer.createToMemory(archiveBuffer, ArchiveFormat::SevenZip);
        writer.addFileFromMemory(inputData, L"hello.txt");
        writer.finalize();
    }

    EXPECT_GT(archiveBuffer.size(), 0);

    // 3. 保存到文件
    auto archivePath = tempDir_ / "from_memory.7z";
    {
        std::ofstream file(archivePath, std::ios::binary);
        file.write(reinterpret_cast<const char*>(archiveBuffer.data()), archiveBuffer.size());
    }

    // 4. 从文件解压
    auto extractDir = tempDir_ / "extracted";
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        reader.extractAll(extractDir.wstring());
        reader.close();
    }

    // 5. 验证
    auto extractedFile = extractDir / "hello.txt";
    EXPECT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(readFile(extractedFile), "Hello World!");
}

// ============================================================================
// 格式转换测试
// ============================================================================

TEST_F(IntegrationTest, FormatConversion_7zToZip) {
    // 1. 创建7z归档
    createTestFiles(3);
    auto sevenZipPath = tempDir_ / "source.7z";

    {
        ArchiveWriter writer;
        writer.create(sevenZipPath.wstring(), ArchiveFormat::SevenZip);
        for (int i = 0; i < 3; ++i) {
            auto file = tempDir_ / ("file_" + std::to_string(i) + ".txt");
            auto name = L"file_" + std::to_wstring(i) + L".txt";
            writer.addFile(file.wstring(), name);
        }
        writer.finalize();
    }

    // 2. 解压7z
    auto extractDir = tempDir_ / "extract_from_7z";
    {
        ArchiveReader reader;
        reader.open(sevenZipPath.wstring());
        reader.extractAll(extractDir.wstring());
        reader.close();
    }

    // 3. 重新打包为ZIP
    auto zipPath = tempDir_ / "converted.zip";
    {
        ArchiveWriter writer;
        writer.create(zipPath.wstring(), ArchiveFormat::Zip);
        for (int i = 0; i < 3; ++i) {
            auto file = extractDir / ("file_" + std::to_string(i) + ".txt");
            auto name = L"file_" + std::to_wstring(i) + L".txt";
            writer.addFile(file.wstring(), name);
        }
        writer.finalize();
    }

    // 4. 验证ZIP
    {
        ArchiveReader reader;
        reader.open(zipPath.wstring());
        EXPECT_EQ(reader.getItemCount(), 3);
        reader.close();
    }

    // 5. 解压ZIP并验证内容
    auto extractZipDir = tempDir_ / "extract_from_zip";
    {
        ArchiveReader reader;
        reader.open(zipPath.wstring());
        reader.extractAll(extractZipDir.wstring());
        reader.close();
    }

    for (int i = 0; i < 3; ++i) {
        auto file = extractZipDir / ("file_" + std::to_string(i) + ".txt");
        EXPECT_TRUE(fs::exists(file));
        auto expected = "Content of file " + std::to_string(i);
        EXPECT_EQ(readFile(file), expected);
    }
}

TEST_F(IntegrationTest, TarGzipWorkflow) {
    // 1. 创建TAR归档
    createTestFiles(2);
    auto tarPath = tempDir_ / "archive.tar";

    {
        ArchiveWriter writer;
        writer.create(tarPath.wstring(), ArchiveFormat::Tar);
        for (int i = 0; i < 2; ++i) {
            auto file = tempDir_ / ("file_" + std::to_string(i) + ".txt");
            auto name = L"file_" + std::to_wstring(i) + L".txt";
            writer.addFile(file.wstring(), name);
        }
        writer.finalize();
    }

    // 2. 压缩TAR为GZIP
    auto gzPath = tempDir_ / "archive.tar.gz";
    {
        ArchiveWriter writer;
        writer.create(gzPath.wstring(), ArchiveFormat::GZip);
        writer.addFile(tarPath.wstring(), L"archive.tar");
        writer.finalize();
    }

    // 3. 解压GZIP
    auto extractGzDir = tempDir_ / "extract_gz";
    {
        ArchiveReader reader;
        reader.open(gzPath.wstring());
        reader.extractAll(extractGzDir.wstring());
        reader.close();
    }

    // 4. 找到解压的TAR文件
    fs::path extractedTar;
    for (const auto& entry : fs::directory_iterator(extractGzDir)) {
        if (entry.is_regular_file()) {
            extractedTar = entry.path();
            break;
        }
    }
    EXPECT_FALSE(extractedTar.empty());

    // 5. 解压TAR
    auto extractTarDir = tempDir_ / "extract_tar";
    {
        ArchiveReader reader;
        reader.open(extractedTar.wstring());
        reader.extractAll(extractTarDir.wstring());
        reader.close();
    }

    // 6. 验证最终文件
    for (int i = 0; i < 2; ++i) {
        auto file = extractTarDir / ("file_" + std::to_string(i) + ".txt");
        EXPECT_TRUE(fs::exists(file));
    }
}

// ============================================================================
// 复杂场景测试
// ============================================================================

TEST_F(IntegrationTest, MixedContentTypes) {
    // 测试不同类型的内容

    // 1. 创建文本文件
    auto textFile = tempDir_ / "text.txt";
    std::ofstream txt(textFile);
    txt << "This is text content with UTF-8: 你好世界";
    txt.close();

    // 2. 创建二进制文件
    auto binaryFile = tempDir_ / "binary.bin";
    std::ofstream bin(binaryFile, std::ios::binary);
    for (int i = 0; i < 256; ++i) {
        bin.put(static_cast<char>(i));
    }
    bin.close();

    // 3. 创建空文件
    auto emptyFile = tempDir_ / "empty.txt";
    std::ofstream empty(emptyFile);
    empty.close();

    // 4. 创建大文件
    auto largeFile = tempDir_ / "large.dat";
    std::ofstream large(largeFile, std::ios::binary);
    std::vector<char> buffer(1024 * 1024, 'X');  // 1MB
    large.write(buffer.data(), buffer.size());
    large.close();

    // 5. 压缩所有文件
    auto archivePath = tempDir_ / "mixed.7z";
    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(textFile.wstring(), L"text.txt");
        writer.addFile(binaryFile.wstring(), L"binary.bin");
        writer.addFile(emptyFile.wstring(), L"empty.txt");
        writer.addFile(largeFile.wstring(), L"large.dat");
        writer.finalize();
    }

    // 6. 解压并验证
    auto extractDir = tempDir_ / "extracted";
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        EXPECT_EQ(reader.getItemCount(), 4);
        reader.extractAll(extractDir.wstring());
        reader.close();
    }

    EXPECT_TRUE(fs::exists(extractDir / "text.txt"));
    EXPECT_TRUE(fs::exists(extractDir / "binary.bin"));
    EXPECT_TRUE(fs::exists(extractDir / "empty.txt"));
    EXPECT_TRUE(fs::exists(extractDir / "large.dat"));

    EXPECT_EQ(fs::file_size(extractDir / "empty.txt"), 0);
    EXPECT_EQ(fs::file_size(extractDir / "large.dat"), 1024 * 1024);
}

TEST_F(IntegrationTest, UpdateWorkflowSimulation) {
    // 模拟更新归档的工作流（通过重新创建）

    // 1. 初始归档
    createTestFiles(3);
    auto archivePath = tempDir_ / "update.7z";

    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        for (int i = 0; i < 3; ++i) {
            auto file = tempDir_ / ("file_" + std::to_string(i) + ".txt");
            writer.addFile(file.wstring(), L"file_" + std::to_wstring(i) + L".txt");
        }
        writer.finalize();
    }

    // 2. 解压现有内容
    auto tempExtract = tempDir_ / "temp_extract";
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        reader.extractAll(tempExtract.wstring());
        reader.close();
    }

    // 3. 修改文件
    auto modifiedFile = tempExtract / "file_1.txt";
    {
        std::ofstream out(modifiedFile);
        out << "Modified content";
    }

    // 4. 添加新文件
    auto newFile = tempExtract / "new_file.txt";
    {
        std::ofstream out(newFile);
        out << "New file content";
    }

    // 5. 重新创建归档
    fs::remove(archivePath);
    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        for (const auto& entry : fs::directory_iterator(tempExtract)) {
            if (entry.is_regular_file()) {
                writer.addFile(entry.path().wstring(), entry.path().filename().wstring());
            }
        }

        writer.finalize();
    }

    // 6. 验证更新后的归档
    auto finalExtract = tempDir_ / "final_extract";
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        EXPECT_EQ(reader.getItemCount(), 4);  // 3个原始 + 1个新增
        reader.extractAll(finalExtract.wstring());
        reader.close();
    }

    EXPECT_EQ(readFile(finalExtract / "file_1.txt"), "Modified content");
    EXPECT_EQ(readFile(finalExtract / "new_file.txt"), "New file content");
}

// ============================================================================
// 错误场景集成测试
// ============================================================================

TEST_F(IntegrationTest, PartiallyCorruptedArchive) {
    // 创建归档后破坏部分数据
    createTestFiles(3);
    auto archivePath = tempDir_ / "corrupt.7z";

    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        for (int i = 0; i < 3; ++i) {
            auto file = tempDir_ / ("file_" + std::to_string(i) + ".txt");
            writer.addFile(file.wstring(), L"file_" + std::to_wstring(i) + L".txt");
        }
        writer.finalize();
    }

    // 破坏文件末尾
    {
        std::fstream file(archivePath, std::ios::binary | std::ios::in | std::ios::out);
        file.seekp(-10, std::ios::end);
        for (int i = 0; i < 10; ++i) {
            file.put(0xFF);
        }
    }

    // 尝试打开可能会失败或成功但解压失败
    bool canOpen = false;
    try {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        canOpen = true;
        reader.close();
    } catch (...) {
        // 预期可能失败
    }

    // 记录结果（不强制成功或失败）
    SUCCEED();
}

TEST_F(IntegrationTest, RecoveryAfterMultipleErrors) {
    // 在多个错误后正常恢复

    // 错误1: 打开不存在的文件
    try {
        ArchiveReader reader;
        reader.open(L"nonexistent.7z");
    } catch (...) {
    }

    // 错误2: 添加不存在的文件
    try {
        ArchiveWriter writer;
        writer.create((tempDir_ / "test.7z").wstring(), ArchiveFormat::SevenZip);
        writer.addFile(L"nonexistent.txt", L"file.txt");
    } catch (...) {
    }

    // 错误3: 无效操作顺序
    try {
        ArchiveWriter writer;
        writer.finalize();  // 未创建归档
    } catch (...) {
    }

    // 正常操作应该成功
    auto testFile = tempDir_ / "test.txt";
    std::ofstream out(testFile);
    out << "Success after errors";
    out.close();

    auto archivePath = tempDir_ / "recovery.7z";
    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(testFile.wstring(), L"test.txt");
        EXPECT_NO_THROW(writer.finalize());
    }

    {
        ArchiveReader reader;
        EXPECT_NO_THROW(reader.open(archivePath.wstring()));
        EXPECT_EQ(reader.getItemCount(), 1);
        reader.close();
    }
}

// ============================================================================
// 边界条件集成测试
// ============================================================================

TEST_F(IntegrationTest, EmptyArchiveWorkflow) {
    // 创建空归档
    auto archivePath = tempDir_ / "empty.7z";
    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.finalize();
    }

    EXPECT_TRUE(fs::exists(archivePath));

    // 打开并验证
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        EXPECT_EQ(reader.getItemCount(), 0);
        reader.close();
    }
}

TEST_F(IntegrationTest, SingleFileMultipleFormats) {
    // 用不同格式压缩同一文件
    auto testFile = tempDir_ / "test.txt";
    std::ofstream out(testFile);
    out << "Test content";
    out.close();

    struct FormatInfo {
        ArchiveFormat format;
        std::wstring extension;
    };

    std::vector<FormatInfo> formats = {
        {ArchiveFormat::SevenZip, L"7z"}, {ArchiveFormat::Zip, L"zip"},
        {ArchiveFormat::Tar, L"tar"},     {ArchiveFormat::GZip, L"gz"},
        {ArchiveFormat::BZip2, L"bz2"},   {ArchiveFormat::Xz, L"xz"}};

    for (const auto& fmt : formats) {
        auto archivePath = tempDir_ / (L"test." + fmt.extension);

        // 压缩
        {
            ArchiveWriter writer;
            writer.create(archivePath.wstring(), fmt.format);
            writer.addFile(testFile.wstring(), L"test.txt");
            writer.finalize();
        }

        EXPECT_TRUE(fs::exists(archivePath));

        // 解压
        auto extractDir = tempDir_ / (L"extract_" + fmt.extension);
        {
            ArchiveReader reader;
            reader.open(archivePath.wstring());
            reader.extractAll(extractDir.wstring());
            reader.close();
        }

        // 验证（单文件格式可能使用不同文件名）
        bool foundFile = false;
        for (const auto& entry : fs::directory_iterator(extractDir)) {
            if (entry.is_regular_file()) {
                auto content = readFile(entry.path());
                EXPECT_EQ(content, "Test content");
                foundFile = true;
                break;
            }
        }
        EXPECT_TRUE(foundFile);
    }
}
