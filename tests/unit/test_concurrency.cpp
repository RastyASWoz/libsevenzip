// test_concurrency.cpp - 并发和资源管理测试
#include <gtest/gtest.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <future>
#include <thread>
#include <vector>

#include "../../src/wrapper/archive/archive_reader.hpp"
#include "../../src/wrapper/archive/archive_writer.hpp"
#include "../../src/wrapper/common/wrapper_error.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

namespace fs = std::filesystem;

// ============================================================================
// 并发测试
// ============================================================================

class ConcurrencyTest : public ::testing::Test {
   protected:
    void SetUp() override {
        tempDir_ = fs::temp_directory_path() / "libsevenzip_concurrency_tests";
        if (fs::exists(tempDir_)) {
            fs::remove_all(tempDir_);
        }
        fs::create_directories(tempDir_);

        // 创建测试文件
        testFile_ = tempDir_ / "test.txt";
        std::ofstream file(testFile_);
        file << "Test content for concurrency tests";
        file.close();
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

    fs::path tempDir_;
    fs::path testFile_;
};

// ============================================================================
// 多线程读取测试
// ============================================================================

TEST_F(ConcurrencyTest, MultipleThreadsReadingSameArchive) {
    // 创建一个归档
    auto archivePath = tempDir_ / "concurrent.7z";
    ArchiveWriter writer;
    writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile_.wstring(), L"test.txt");
    writer.finalize();

    // 10个线程同时读取同一个归档
    const int numThreads = 10;
    std::vector<std::future<bool>> futures;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            try {
                ArchiveReader reader;
                reader.open(archivePath.wstring());

                EXPECT_TRUE(reader.isOpen());
                EXPECT_EQ(reader.getItemCount(), 1);

                auto extractDir = tempDir_ / ("extract_" + std::to_string(i));
                reader.extractAll(extractDir.wstring());

                EXPECT_TRUE(fs::exists(extractDir / "test.txt"));

                reader.close();
                successCount++;
                return true;
            } catch (...) {
                return false;
            }
        }));
    }

    // 等待所有线程完成
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }

    EXPECT_EQ(successCount.load(), numThreads);
}

TEST_F(ConcurrencyTest, MultipleThreadsCreatingDifferentArchives) {
    // 10个线程同时创建不同的归档
    const int numThreads = 10;
    std::vector<std::future<bool>> futures;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            try {
                auto threadFile = tempDir_ / ("file_" + std::to_string(i) + ".txt");
                createTestFile(threadFile, "Content " + std::to_string(i));

                auto archivePath = tempDir_ / ("archive_" + std::to_string(i) + ".7z");
                ArchiveWriter writer;
                writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
                writer.addFile(threadFile.wstring(), L"file.txt");
                writer.finalize();

                EXPECT_TRUE(fs::exists(archivePath));
                successCount++;
                return true;
            } catch (...) {
                return false;
            }
        }));
    }

    // 等待所有线程完成
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }

    EXPECT_EQ(successCount.load(), numThreads);
}

TEST_F(ConcurrencyTest, ParallelReadAndWrite) {
    // 创建初始归档
    auto readArchive = tempDir_ / "read.7z";
    ArchiveWriter writer;
    writer.create(readArchive.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile_.wstring(), L"test.txt");
    writer.finalize();

    // 同时进行读取和写入操作
    std::atomic<int> operationCount{0};

    auto readTask = std::async(std::launch::async, [&]() {
        for (int i = 0; i < 5; ++i) {
            ArchiveReader reader;
            reader.open(readArchive.wstring());
            auto extractDir = tempDir_ / ("extract_read_" + std::to_string(i));
            reader.extractAll(extractDir.wstring());
            reader.close();
            operationCount++;
        }
        return true;
    });

    auto writeTask = std::async(std::launch::async, [&]() {
        for (int i = 0; i < 5; ++i) {
            auto writeArchive = tempDir_ / ("write_" + std::to_string(i) + ".7z");
            ArchiveWriter writer;
            writer.create(writeArchive.wstring(), ArchiveFormat::SevenZip);
            writer.addFile(testFile_.wstring(), L"test.txt");
            writer.finalize();
            operationCount++;
        }
        return true;
    });

    EXPECT_TRUE(readTask.get());
    EXPECT_TRUE(writeTask.get());
    EXPECT_EQ(operationCount.load(), 10);
}

// ============================================================================
// 资源泄漏检测测试
// ============================================================================

TEST_F(ConcurrencyTest, NoMemoryLeakOnRepeatedOperations) {
    // 重复创建和销毁归档对象，检测内存泄漏
    for (int i = 0; i < 100; ++i) {
        auto archivePath = tempDir_ / ("leak_test_" + std::to_string(i) + ".7z");

        {
            ArchiveWriter writer;
            writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
            writer.addFile(testFile_.wstring(), L"test.txt");
            writer.finalize();
        }  // writer销毁

        {
            ArchiveReader reader;
            reader.open(archivePath.wstring());
            EXPECT_EQ(reader.getItemCount(), 1);
            reader.close();
        }  // reader销毁

        fs::remove(archivePath);
    }

    // 如果有内存泄漏，这个测试会导致内存使用持续增长
    SUCCEED();
}

TEST_F(ConcurrencyTest, NoLeakOnExceptionPath) {
    // 测试异常路径下的资源释放
    for (int i = 0; i < 50; ++i) {
        try {
            ArchiveReader reader;
            reader.open(L"nonexistent_file.7z");
            FAIL() << "Should have thrown exception";
        } catch (const Exception&) {
            // 预期的异常
        }

        try {
            ArchiveWriter writer;
            writer.create((tempDir_ / "test.7z").wstring(), ArchiveFormat::SevenZip);
            writer.addFile(L"nonexistent.txt", L"file.txt");
            FAIL() << "Should have thrown exception";
        } catch (const Exception&) {
            // 预期的异常
        }
    }

    SUCCEED();
}

TEST_F(ConcurrencyTest, NoLeakWithMoveSemantics) {
    // 测试移动语义不会导致资源泄漏
    for (int i = 0; i < 100; ++i) {
        auto archivePath = tempDir_ / ("move_test_" + std::to_string(i) + ".7z");

        ArchiveWriter writer1;
        writer1.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer1.addFile(testFile_.wstring(), L"test.txt");

        // 移动构造
        ArchiveWriter writer2(std::move(writer1));
        writer2.finalize();

        ArchiveReader reader1;
        reader1.open(archivePath.wstring());

        // 移动赋值
        ArchiveReader reader2;
        reader2 = std::move(reader1);
        reader2.close();

        fs::remove(archivePath);
    }

    SUCCEED();
}

// ============================================================================
// 大规模并发测试
// ============================================================================

TEST_F(ConcurrencyTest, StressTestWithManyThreads) {
    // 创建一个共享归档
    auto sharedArchive = tempDir_ / "shared.7z";
    ArchiveWriter writer;
    writer.create(sharedArchive.wstring(), ArchiveFormat::SevenZip);
    writer.addFile(testFile_.wstring(), L"test.txt");
    writer.finalize();

    // 50个线程同时读取
    const int numThreads = 50;
    std::vector<std::future<bool>> futures;
    std::atomic<int> successCount{0};
    std::atomic<int> errorCount{0};

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            try {
                ArchiveReader reader;
                reader.open(sharedArchive.wstring());

                auto count = reader.getItemCount();
                if (count != 1) {
                    errorCount++;
                    return false;
                }

                reader.close();
                successCount++;
                return true;
            } catch (...) {
                errorCount++;
                return false;
            }
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    EXPECT_EQ(successCount.load(), numThreads);
    EXPECT_EQ(errorCount.load(), 0);
}

// ============================================================================
// 长时间运行测试
// ============================================================================

TEST_F(ConcurrencyTest, LongRunningOperations) {
    // 模拟长时间运行的压缩和解压操作
    auto largeFile = tempDir_ / "large.bin";
    {
        std::ofstream file(largeFile, std::ios::binary);
        std::vector<char> buffer(1024 * 1024, 'X');  // 1MB
        for (int i = 0; i < 5; ++i) {                // 5MB总计
            file.write(buffer.data(), buffer.size());
        }
    }

    auto archivePath = tempDir_ / "large.7z";

    // 压缩任务
    auto compressTask = std::async(std::launch::async, [&]() {
        ArchiveWriter writer;
        ArchiveProperties props;
        props.level = CompressionLevel::Maximum;  // 使用最大压缩以增加耗时

        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.setProperties(props);
        writer.addFile(largeFile.wstring(), L"large.bin");
        writer.finalize();
        return true;
    });

    EXPECT_TRUE(compressTask.get());
    EXPECT_TRUE(fs::exists(archivePath));

    // 解压任务
    auto extractTask = std::async(std::launch::async, [&]() {
        ArchiveReader reader;
        reader.open(archivePath.wstring());
        auto extractDir = tempDir_ / "extracted";
        reader.extractAll(extractDir.wstring());
        reader.close();
        return true;
    });

    EXPECT_TRUE(extractTask.get());
}

// ============================================================================
// 错误恢复测试
// ============================================================================

TEST_F(ConcurrencyTest, RecoveryAfterErrors) {
    // 测试在错误后是否能正常继续操作
    for (int i = 0; i < 10; ++i) {
        // 尝试打开不存在的文件（会失败）
        try {
            ArchiveReader reader;
            reader.open(L"nonexistent.7z");
        } catch (...) {
            // 忽略错误
        }

        // 然后进行正常操作
        auto archivePath = tempDir_ / ("recovery_" + std::to_string(i) + ".7z");
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(testFile_.wstring(), L"test.txt");
        EXPECT_NO_THROW(writer.finalize());

        ArchiveReader reader;
        EXPECT_NO_THROW(reader.open(archivePath.wstring()));
        EXPECT_EQ(reader.getItemCount(), 1);
        reader.close();

        fs::remove(archivePath);
    }

    SUCCEED();
}

// ============================================================================
// RAII 和异常安全测试
// ============================================================================

TEST_F(ConcurrencyTest, RAIIWithExceptions) {
    // 测试RAII模式在异常情况下的正确性
    auto archivePath = tempDir_ / "raii_test.7z";

    // 正常情况
    {
        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
        writer.addFile(testFile_.wstring(), L"test.txt");
        writer.finalize();
        // writer 自动析构
    }

    EXPECT_TRUE(fs::exists(archivePath));

    // 异常情况 - 提前退出作用域
    try {
        ArchiveWriter writer;
        writer.create((tempDir_ / "exception.7z").wstring(), ArchiveFormat::SevenZip);
        writer.addFile(L"nonexistent.txt", L"test.txt");  // 会抛出异常
        FAIL() << "Should have thrown";
    } catch (const Exception&) {
        // writer 应该正确析构，不会泄漏资源
    }

    SUCCEED();
}

// ============================================================================
// 多格式并发测试
// ============================================================================

TEST_F(ConcurrencyTest, ConcurrentDifferentFormats) {
    const int numFormats = 4;
    std::vector<ArchiveFormat> formats = {ArchiveFormat::SevenZip, ArchiveFormat::Zip,
                                          ArchiveFormat::Tar, ArchiveFormat::GZip};

    std::vector<std::future<bool>> futures;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numFormats; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            try {
                std::string ext;
                switch (formats[i]) {
                    case ArchiveFormat::SevenZip:
                        ext = "7z";
                        break;
                    case ArchiveFormat::Zip:
                        ext = "zip";
                        break;
                    case ArchiveFormat::Tar:
                        ext = "tar";
                        break;
                    case ArchiveFormat::GZip:
                        ext = "gz";
                        break;
                    default:
                        ext = "bin";
                        break;
                }

                auto archivePath = tempDir_ / ("format_" + std::to_string(i) + "." + ext);
                ArchiveWriter writer;
                writer.create(archivePath.wstring(), formats[i]);
                writer.addFile(testFile_.wstring(), L"test.txt");
                writer.finalize();

                ArchiveReader reader;
                reader.open(archivePath.wstring());
                EXPECT_EQ(reader.getItemCount(), 1);
                reader.close();

                successCount++;
                return true;
            } catch (...) {
                return false;
            }
        }));
    }

    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }

    EXPECT_EQ(successCount.load(), numFormats);
}

// ============================================================================
// 内存压力测试
// ============================================================================

TEST_F(ConcurrencyTest, MemoryPressure) {
    // 创建多个大的内存缓冲区并压缩
    const int numBuffers = 5;
    std::vector<std::vector<uint8_t>> buffers;

    for (int i = 0; i < numBuffers; ++i) {
        buffers.emplace_back(1024 * 1024, static_cast<uint8_t>(i));  // 1MB each
    }

    std::vector<std::future<bool>> futures;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numBuffers; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            try {
                std::vector<uint8_t> archiveBuffer;
                ArchiveWriter writer;
                ArchiveProperties props;
                props.level = CompressionLevel::Fast;

                writer.createToMemory(archiveBuffer, ArchiveFormat::SevenZip);
                writer.setProperties(props);
                writer.addFileFromMemory(buffers[i], L"data.bin");
                writer.finalize();

                EXPECT_GT(archiveBuffer.size(), 0);
                successCount++;
                return true;
            } catch (...) {
                return false;
            }
        }));
    }

    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }

    EXPECT_EQ(successCount.load(), numBuffers);
}
