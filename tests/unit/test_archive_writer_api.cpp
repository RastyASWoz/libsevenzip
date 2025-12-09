#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "sevenzip/archive.hpp"
#include "sevenzip/archive_writer.hpp"

namespace fs = std::filesystem;

class ArchiveWriterTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "sevenzip_writer_test";
        fs::create_directories(testDir);

        testFile1 = testDir / "file1.txt";
        testFile2 = testDir / "file2.txt";

        std::ofstream ofs1(testFile1);
        ofs1 << "Content of file 1";
        ofs1.close();

        std::ofstream ofs2(testFile2);
        ofs2 << "Content of file 2";
        ofs2.close();

        archivePath = testDir / "test.7z";
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
    fs::path testFile1;
    fs::path testFile2;
    fs::path archivePath;
};

TEST_F(ArchiveWriterTest, CreateArchive) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addFile(testFile1);
    writer.finalize();

    EXPECT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, AddMultipleFiles) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addFile(testFile1).addFile(testFile2);
    writer.finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 2u);
}

TEST_F(ArchiveWriterTest, AddFilesMethod) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addFiles({testFile1, testFile2});
    writer.finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 2u);
}

TEST_F(ArchiveWriterTest, WithCompressionLevel) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.withLevel(sevenzip::CompressionLevel::Maximum).addFile(testFile1);
    writer.finalize();

    EXPECT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, ChainedAPI) {
    sevenzip::ArchiveWriter::create(archivePath)
        .withLevel(sevenzip::CompressionLevel::Fast)
        .addFile(testFile1)
        .addFile(testFile2)
        .finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 2u);
}

TEST_F(ArchiveWriterTest, AddFromMemory) {
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o'};

    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addFromMemory(data, "memory.txt");
    writer.finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 1u);

    auto extracted = archive.extractItemToMemory(0);
    EXPECT_EQ(extracted, data);
}

TEST_F(ArchiveWriterTest, CreateToMemory) {
    std::vector<uint8_t> buffer;

    auto writer = sevenzip::ArchiveWriter::createToMemory(buffer);
    writer.addFile(testFile1);
    writer.finalize();

    EXPECT_GT(buffer.size(), 0u);

    auto archive = sevenzip::Archive::openFromMemory(buffer);
    EXPECT_EQ(archive.itemCount(), 1u);
}

TEST_F(ArchiveWriterTest, PendingCount) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);

    EXPECT_EQ(writer.pendingCount(), 0u);

    writer.addFile(testFile1);
    EXPECT_EQ(writer.pendingCount(), 1u);

    writer.addFile(testFile2);
    EXPECT_EQ(writer.pendingCount(), 2u);

    writer.finalize();
}

TEST_F(ArchiveWriterTest, WithSolidMode) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.withSolidMode(true).addFile(testFile1).addFile(testFile2);
    writer.finalize();

    EXPECT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, DifferentFormats) {
    auto zipPath = testDir / "test.zip";
    auto tarPath = testDir / "test.tar";

    sevenzip::ArchiveWriter::create(zipPath, sevenzip::Format::Zip).addFile(testFile1).finalize();

    sevenzip::ArchiveWriter::create(tarPath, sevenzip::Format::Tar).addFile(testFile1).finalize();

    EXPECT_TRUE(fs::exists(zipPath));
    EXPECT_TRUE(fs::exists(tarPath));
}

TEST_F(ArchiveWriterTest, AddDirectory) {
    auto subDir = testDir / "subdir";
    fs::create_directories(subDir);

    auto subFile = subDir / "sub.txt";
    std::ofstream ofs(subFile);
    ofs << "Sub file content";
    ofs.close();

    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addDirectory(subDir);
    writer.finalize();

    EXPECT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, WithProgress) {
    bool progressCalled = false;

    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.withProgress([&progressCalled](uint64_t, uint64_t) {
        progressCalled = true;
        return true;
    });

    writer.addFile(testFile1);
    writer.finalize();

    // Progress may or may not be called for small files
    EXPECT_TRUE(fs::exists(archivePath));
}

TEST_F(ArchiveWriterTest, MoveSemantics) {
    auto writer1 = sevenzip::ArchiveWriter::create(archivePath);
    writer1.addFile(testFile1);

    auto writer2 = std::move(writer1);
    writer2.addFile(testFile2);
    writer2.finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 2u);
}

// ============================================================================
// 新增测试：链式配置和边界条件
// ============================================================================

TEST_F(ArchiveWriterTest, CompleteChainedAPI) {
    sevenzip::ArchiveWriter::create(archivePath)
        .withLevel(sevenzip::CompressionLevel::Maximum)
        .withSolidMode(true)
        .addFile(testFile1)
        .addFile(testFile2)
        .finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 2u);
}

TEST_F(ArchiveWriterTest, AddFileWithArchiveName) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addFile(testFile1, "custom/path/renamed.txt");
    writer.finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_EQ(archive.itemCount(), 1u);

    auto item = archive.itemInfo(0);
    EXPECT_TRUE(item.path.string().find("renamed.txt") != std::string::npos);
}

TEST_F(ArchiveWriterTest, WithEncryptedHeaders) {
    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.withPassword("secret123").withEncryptedHeaders(true);
    writer.addFile(testFile1);
    writer.finalize();

    EXPECT_TRUE(fs::exists(archivePath));
    // Note: We can't easily verify encrypted headers without opening with password
}

TEST_F(ArchiveWriterTest, AddDirectoryRecursive) {
    auto deepDir = testDir / "deep" / "nested" / "dir";
    fs::create_directories(deepDir);

    std::ofstream(deepDir / "deep.txt") << "Deep content";

    auto writer = sevenzip::ArchiveWriter::create(archivePath);
    writer.addDirectory(testDir / "deep", true);
    writer.finalize();

    auto archive = sevenzip::Archive::open(archivePath);
    EXPECT_GT(archive.itemCount(), 0u);
}

TEST_F(ArchiveWriterTest, CreateToMemoryWithFormat) {
    std::vector<uint8_t> buffer;

    auto writer = sevenzip::ArchiveWriter::createToMemory(buffer, sevenzip::Format::Zip);
    writer.addFile(testFile1);
    writer.finalize();

    EXPECT_GT(buffer.size(), 0u);

    auto archive = sevenzip::Archive::openFromMemory(buffer, sevenzip::Format::Zip);
    EXPECT_EQ(archive.itemCount(), 1u);
}
