// test_stress.cpp - 压力和性能测试
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>

#include "../../src/wrapper/archive/archive_reader.hpp"
#include "../../src/wrapper/archive/archive_writer.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

namespace fs = std::filesystem;

// ============================================================================
// 压力测试
// ============================================================================

class StressTest : public ::testing::Test {
   protected:
    void SetUp() override {
        tempDir_ = fs::temp_directory_path() / "libsevenzip_stress_tests";
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

    void createTestFile(const fs::path& path, size_t sizeBytes) {
        fs::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);

        // 创建随机数据
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        const size_t bufferSize = 4096;
        std::vector<char> buffer(bufferSize);
        size_t remaining = sizeBytes;

        while (remaining > 0) {
            size_t chunkSize = (std::min)(bufferSize, remaining);
            for (size_t i = 0; i < chunkSize; ++i) {
                buffer[i] = static_cast<char>(dis(gen));
            }
            file.write(buffer.data(), chunkSize);
            remaining -= chunkSize;
        }
    }

    fs::path tempDir_;
};

// ============================================================================
// 大文件测试
// ============================================================================

TEST_F(StressTest, CompressVeryLargeFile) {
    // 创建 50MB 文件
    auto largeFile = tempDir_ / "large_50mb.bin";
    createTestFile(largeFile, 50 * 1024 * 1024);

    auto archivePath = tempDir_ / "large.7z";

    // 压缩
    auto startCompress = std::chrono::high_resolution_clock::now();
    {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Fast;  // 使用快速压缩以节省时间

        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.setProperties(props);
        writer.addFile(largeFile.wstring(), L"large.bin");
        writer.finalize();
    }
    auto endCompress = std::chrono::high_resolution_clock::now();
    auto compressDuration =
        std::chrono::duration_cast<std::chrono::seconds>(endCompress - startCompress).count();

    EXPECT_TRUE(fs::exists(archivePath));
    EXPECT_GT(fs::file_size(archivePath), 0);

    // 解压
    auto extractDir = tempDir_ / "extracted";
    auto startExtract = std::chrono::high_resolution_clock::now();
    {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        EXPECT_EQ(reader.getItemCount(), 1);
        reader.extractAll(extractDir.wstring());
        reader.close();
    }
    auto endExtract = std::chrono::high_resolution_clock::now();
    auto extractDuration =
        std::chrono::duration_cast<std::chrono::seconds>(endExtract - startExtract).count();

    auto extractedFile = extractDir / "large.bin";
    EXPECT_TRUE(fs::exists(extractedFile));
    EXPECT_EQ(fs::file_size(extractedFile), 50 * 1024 * 1024);

    std::cout << "  50MB file compression time: " << compressDuration << "s\n";
    std::cout << "  50MB file extraction time: " << extractDuration << "s\n";
}

TEST_F(StressTest, CompressMultipleLargeFiles) {
    // 创建5个 10MB 文件
    std::vector<fs::path> files;
    for (int i = 0; i < 5; ++i) {
        auto file = tempDir_ / ("file_" + std::to_string(i) + ".bin");
        createTestFile(file, 10 * 1024 * 1024);
        files.push_back(file);
    }

    auto archivePath = tempDir_ / "multiple_large.7z";

    // 压缩
    auto startTime = std::chrono::high_resolution_clock::now();
    {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Fast;

        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.setProperties(props);

        for (size_t i = 0; i < files.size(); ++i) {
            auto name = L"file_" + std::to_wstring(i) + L".bin";
            writer.addFile(files[i].wstring(), name);
        }

        writer.finalize();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

    EXPECT_TRUE(fs::exists(archivePath));

    // 验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), 5);
    reader.close();

    std::cout << "  5x10MB files compression time: " << duration << "s\n";
}

// ============================================================================
// 大量文件测试
// ============================================================================

TEST_F(StressTest, CompressManySmallFiles) {
    // 创建 1000 个小文件
    const int numFiles = 1000;
    auto filesDir = tempDir_ / "many_files";
    fs::create_directories(filesDir);

    for (int i = 0; i < numFiles; ++i) {
        auto file = filesDir / ("file_" + std::to_string(i) + ".txt");
        std::ofstream out(file);
        out << "Content of file " << i;
    }

    auto archivePath = tempDir_ / "many_files.7z";

    // 压缩
    auto startTime = std::chrono::high_resolution_clock::now();
    {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Fast;

        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.setProperties(props);

        for (int i = 0; i < numFiles; ++i) {
            auto file = filesDir / ("file_" + std::to_string(i) + ".txt");
            auto name = L"file_" + std::to_wstring(i) + L".txt";
            writer.addFile(file.wstring(), name);
        }

        writer.finalize();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    EXPECT_TRUE(fs::exists(archivePath));

    // 验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), numFiles);
    reader.close();

    std::cout << "  1000 files compression time: " << duration << "ms\n";
}

TEST_F(StressTest, CompressDeepDirectoryStructure) {
    // 创建深层目录结构 (20层)
    const int depth = 20;
    auto currentDir = tempDir_ / "deep";
    fs::create_directories(currentDir);

    for (int i = 0; i < depth; ++i) {
        currentDir /= ("level_" + std::to_string(i));
        fs::create_directories(currentDir);

        // 在每一层创建一个文件
        auto file = currentDir / "file.txt";
        std::ofstream out(file);
        out << "Level " << i;
    }

    auto archivePath = tempDir_ / "deep_structure.7z";

    // 压缩
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

    // 递归添加所有文件
    std::function<void(const fs::path&, const std::wstring&)> addDirectory;
    addDirectory = [&](const fs::path& dir, const std::wstring& prefix) {
        for (const auto& entry : fs::directory_iterator(dir)) {
            auto name = prefix.empty() ? entry.path().filename().wstring()
                                       : prefix + L"/" + entry.path().filename().wstring();

            if (entry.is_regular_file()) {
                writer.addFile(entry.path().wstring(), name);
            } else if (entry.is_directory()) {
                addDirectory(entry.path(), name);
            }
        }
    };

    addDirectory(tempDir_ / "deep", L"deep");
    writer.finalize();

    EXPECT_TRUE(fs::exists(archivePath));

    // 验证
    ArchiveReader reader;
    reader.open(archivePath.wstring());
    EXPECT_EQ(reader.getItemCount(), depth);  // 每层一个文件
    reader.close();
}

// ============================================================================
// 极限压缩比测试
// ============================================================================

TEST_F(StressTest, HighlyCompressibleData) {
    // 创建高度可压缩的数据 (全是相同字节)
    auto file = tempDir_ / "compressible.bin";
    {
        std::ofstream out(file, std::ios::binary);
        std::vector<char> buffer(10 * 1024 * 1024, 'A');  // 10MB of 'A'
        out.write(buffer.data(), buffer.size());
    }

    auto archivePath = tempDir_ / "compressible.7z";

    // 使用最大压缩
    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Maximum;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(file.wstring(), L"data.bin");
    writer.finalize();

    auto originalSize = fs::file_size(file);
    auto compressedSize = fs::file_size(archivePath);
    auto ratio = static_cast<double>(compressedSize) / originalSize * 100;

    EXPECT_LT(compressedSize, originalSize);
    EXPECT_LT(ratio, 1.0);  // 应该压缩到不到1%

    std::cout << "  Original: " << originalSize << " bytes\n";
    std::cout << "  Compressed: " << compressedSize << " bytes\n";
    std::cout << "  Ratio: " << ratio << "%\n";
}

TEST_F(StressTest, RandomNonCompressibleData) {
    // 创建随机数据 (不可压缩)
    auto file = tempDir_ / "random.bin";
    createTestFile(file, 10 * 1024 * 1024);  // 10MB 随机数据

    auto archivePath = tempDir_ / "random.7z";

    // 使用最大压缩
    ArchiveWriter writer;
    ArchiveProperties props;
    props.level = CompressionLevel::Maximum;

    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(file.wstring(), L"random.bin");
    writer.finalize();

    auto originalSize = fs::file_size(file);
    auto compressedSize = fs::file_size(archivePath);
    auto ratio = static_cast<double>(compressedSize) / originalSize * 100;

    // 随机数据基本不可压缩，压缩后可能更大
    EXPECT_GT(ratio, 95.0);  // 应该接近100%

    std::cout << "  Random data ratio: " << ratio << "%\n";
}

// ============================================================================
// 持续操作测试
// ============================================================================

TEST_F(StressTest, ContinuousCompressionDecompression) {
    // 连续进行100次压缩解压循环
    const int iterations = 100;

    auto testFile = tempDir_ / "test.txt";
    std::ofstream out(testFile);
    out << "Test content";
    out.close();

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto archivePath = tempDir_ / ("iteration_" + std::to_string(i) + ".7z");

        // 压缩
        {
            ArchiveWriter writer;
            ArchiveProperties props;
            props.level = CompressionLevel::Fast;

            writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
            writer.setProperties(props);
            writer.addFile(testFile.wstring(), L"test.txt");
            writer.finalize();
        }

        // 解压
        {
            ArchiveReader reader;
            reader.open(archivePath.wstring());
            auto extractDir = tempDir_ / ("extract_" + std::to_string(i));
            reader.extractAll(extractDir.wstring());
            reader.close();
        }

        // 清理
        fs::remove(archivePath);
        fs::remove_all(tempDir_ / ("extract_" + std::to_string(i)));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration =
        std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    auto avgDuration = totalDuration * 1000.0 / iterations;

    std::cout << "  100 iterations total time: " << totalDuration << "s\n";
    std::cout << "  Average per iteration: " << avgDuration << "ms\n";

    SUCCEED();
}

// ============================================================================
// 内存密集型测试
// ============================================================================

TEST_F(StressTest, LargeMemoryCompression) {
    // 创建 20MB 内存缓冲区并压缩
    const size_t bufferSize = 20 * 1024 * 1024;
    std::vector<uint8_t> largeBuffer(bufferSize);

    // 填充重复模式
    for (size_t i = 0; i < bufferSize; ++i) {
        largeBuffer[i] = static_cast<uint8_t>(i % 256);
    }

    std::vector<uint8_t> archiveBuffer;

    // 压缩到内存
    auto startTime = std::chrono::high_resolution_clock::now();
    {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Normal;

        writer.createToMemory(archiveBuffer, ArchiveFormat::SevenZip);
        writer.setProperties(props);
        writer.addFileFromMemory(largeBuffer, L"large.bin");
        writer.finalize();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    EXPECT_GT(archiveBuffer.size(), 0);
    EXPECT_LT(archiveBuffer.size(), bufferSize);

    auto ratio = static_cast<double>(archiveBuffer.size()) / bufferSize * 100;
    std::cout << "  20MB memory compression time: " << duration << "ms\n";
    std::cout << "  Compression ratio: " << ratio << "%\n";
}

TEST_F(StressTest, MultipleMemoryBuffers) {
    // 同时处理10个内存缓冲区
    const int numBuffers = 10;
    const size_t bufferSize = 5 * 1024 * 1024;  // 5MB each

    for (int i = 0; i < numBuffers; ++i) {
        std::vector<uint8_t> inputBuffer(bufferSize, static_cast<uint8_t>(i));
        std::vector<uint8_t> archiveBuffer;

        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Fast;

        writer.createToMemory(archiveBuffer, ArchiveFormat::SevenZip);
        writer.setProperties(props);
        writer.addFileFromMemory(inputBuffer, L"data.bin");
        writer.finalize();

        EXPECT_GT(archiveBuffer.size(), 0);
    }

    SUCCEED();
}

// ============================================================================
// 不同格式压力测试
// ============================================================================

TEST_F(StressTest, AllFormatsBigData) {
    // 为所有支持的格式测试大文件
    auto testFile = tempDir_ / "big_test.bin";
    createTestFile(testFile, 5 * 1024 * 1024);  // 5MB

    struct FormatTest {
        ArchiveFormat format;
        std::wstring ext;
    };

    std::vector<FormatTest> formats = {
        {ArchiveFormat::SevenZip, L"7z"}, {ArchiveFormat::Zip, L"zip"},
        {ArchiveFormat::Tar, L"tar"},     {ArchiveFormat::GZip, L"gz"},
        {ArchiveFormat::BZip2, L"bz2"},   {ArchiveFormat::Xz, L"xz"}};

    for (const auto& fmt : formats) {
        auto archivePath = tempDir_ / (L"stress." + fmt.ext);

        // 压缩
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Fast;

        writer.create(archivePath.wstring(), fmt.format);
        writer.setProperties(props);
        writer.addFile(testFile.wstring(), L"big_test.bin");
        writer.finalize();

        EXPECT_TRUE(fs::exists(archivePath));
        EXPECT_GT(fs::file_size(archivePath), 0);

        // 解压验证
        auto extractDir = tempDir_ / (L"extract_" + fmt.ext);
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        reader.extractAll(extractDir.wstring());
        reader.close();

        // 验证文件存在
        bool found = false;
        for (const auto& entry : fs::directory_iterator(extractDir)) {
            if (entry.is_regular_file()) {
                EXPECT_EQ(fs::file_size(entry.path()), 5 * 1024 * 1024);
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found);

        fs::remove(archivePath);
        fs::remove_all(extractDir);
    }

    SUCCEED();
}

// ============================================================================
// 错误恢复压力测试
// ============================================================================

TEST_F(StressTest, RepeatedErrorRecovery) {
    // 重复错误操作后恢复
    for (int i = 0; i < 50; ++i) {
        // 尝试无效操作
        try {
            ArchiveReader reader;
            reader.open(L"nonexistent.7z");
            FAIL();
        } catch (...) {
            // 预期的错误
        }

        try {
            ArchiveWriter writer;
            writer.addFile(L"nonexistent.txt", L"test.txt");
            FAIL();
        } catch (...) {
            // 预期的错误
        }

        // 进行正常操作
        auto archivePath = tempDir_ / ("recovery_" + std::to_string(i) + ".7z");
        auto testFile = tempDir_ / ("test_" + std::to_string(i) + ".txt");

        std::ofstream out(testFile);
        out << "Test " << i;
        out.close();

        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(testFile.wstring(), L"test.txt");
        EXPECT_NO_THROW(writer.finalize());

        fs::remove(archivePath);
        fs::remove(testFile);
    }

    SUCCEED();
}
