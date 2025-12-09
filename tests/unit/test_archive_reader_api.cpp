#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "sevenzip/archive.hpp"
#include "sevenzip/archive_reader.hpp"

namespace fs = std::filesystem;

class ArchiveReaderAPITest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "sevenzip_reader_api_test";
        fs::create_directories(testDir);

        // 创建测试文件
        testFile1 = testDir / "file1.txt";
        testFile2 = testDir / "file2.txt";

        std::ofstream ofs1(testFile1);
        ofs1 << "Content of file 1";
        ofs1.close();

        std::ofstream ofs2(testFile2);
        ofs2 << "Content of file 2";
        ofs2.close();

        // 创建测试归档
        archivePath = testDir / "test.7z";
        auto archive = sevenzip::Archive::create(archivePath);
        archive.addFile(testFile1);
        archive.addFile(testFile2);
        archive.finalize();
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

TEST_F(ArchiveReaderAPITest, OpenArchive) {
    sevenzip::ArchiveReader reader(archivePath);
    EXPECT_EQ(reader.itemCount(), 2u);
}

TEST_F(ArchiveReaderAPITest, GetInfo) {
    sevenzip::ArchiveReader reader(archivePath);
    auto info = reader.info();

    EXPECT_EQ(info.format, sevenzip::Format::SevenZip);
    EXPECT_EQ(info.itemCount, 2u);
}

TEST_F(ArchiveReaderAPITest, GetItemInfo) {
    sevenzip::ArchiveReader reader(archivePath);

    auto item0 = reader.itemInfo(0);
    EXPECT_EQ(item0.path, "file1.txt");
    EXPECT_FALSE(item0.isDirectory);
    EXPECT_GT(item0.size, 0u);

    auto item1 = reader.itemInfo(1);
    EXPECT_EQ(item1.path, "file2.txt");
}

TEST_F(ArchiveReaderAPITest, FindItem) {
    sevenzip::ArchiveReader reader(archivePath);

    auto found = reader.findItem("file1.txt");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->path, "file1.txt");

    auto notFound = reader.findItem("nonexistent.txt");
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(ArchiveReaderAPITest, Contains) {
    sevenzip::ArchiveReader reader(archivePath);

    EXPECT_TRUE(reader.contains("file1.txt"));
    EXPECT_TRUE(reader.contains("file2.txt"));
    EXPECT_FALSE(reader.contains("file3.txt"));
}

TEST_F(ArchiveReaderAPITest, ExtractToMemory) {
    sevenzip::ArchiveReader reader(archivePath);

    auto data = reader.extract(0);
    std::string content(data.begin(), data.end());
    EXPECT_EQ(content, "Content of file 1");
}

TEST_F(ArchiveReaderAPITest, ExtractToFile) {
    sevenzip::ArchiveReader reader(archivePath);

    auto extractDir = testDir / "extracted";
    fs::create_directories(extractDir);

    // extractTo expects a destination directory, it will use the original filename
    reader.extractTo(0, extractDir);

    // The file should be extracted with its original name "file1.txt"
    auto outPath = extractDir / "file1.txt";
    EXPECT_TRUE(fs::exists(outPath));

    std::ifstream ifs(outPath);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "Content of file 1");
}

TEST_F(ArchiveReaderAPITest, ExtractAll) {
    sevenzip::ArchiveReader reader(archivePath);

    auto extractDir = testDir / "extracted";
    reader.extractAll(extractDir);

    EXPECT_TRUE(fs::exists(extractDir / "file1.txt"));
    EXPECT_TRUE(fs::exists(extractDir / "file2.txt"));
}

TEST_F(ArchiveReaderAPITest, ExtractSelected) {
    sevenzip::ArchiveReader reader(archivePath);

    auto extractDir = testDir / "extracted";
    reader.extractItems({0}, extractDir);

    EXPECT_TRUE(fs::exists(extractDir / "file1.txt"));
}

TEST_F(ArchiveReaderAPITest, TestArchive) {
    sevenzip::ArchiveReader reader(archivePath);
    EXPECT_TRUE(reader.test());
}

TEST_F(ArchiveReaderAPITest, Iterator) {
    sevenzip::ArchiveReader reader(archivePath);

    size_t count = 0;
    for (const auto& item : reader) {
        EXPECT_FALSE(item.path.empty());
        ++count;
    }

    EXPECT_EQ(count, 2u);
}

TEST_F(ArchiveReaderAPITest, IteratorOperations) {
    sevenzip::ArchiveReader reader(archivePath);

    auto it = reader.begin();
    EXPECT_NE(it, reader.end());

    auto item = *it;
    EXPECT_EQ(item.path, "file1.txt");

    ++it;
    EXPECT_NE(it, reader.end());
    EXPECT_EQ(it->path, "file2.txt");

    ++it;
    EXPECT_EQ(it, reader.end());
}

TEST_F(ArchiveReaderAPITest, ChainedConfiguration) {
    sevenzip::ArchiveReader reader(archivePath);

    // Note: Progress callback for ArchiveReader is not yet fully implemented
    // but the chained API should work without errors
    bool progressCalled = false;
    reader.withProgress([&progressCalled](uint64_t, uint64_t) {
        progressCalled = true;
        return true;
    });

    auto extractDir = testDir / "extracted";
    reader.extractAll(extractDir);

    EXPECT_TRUE(fs::exists(extractDir / "file1.txt"));
    // Progress callback is not yet called, so we don't check progressCalled
}

TEST_F(ArchiveReaderAPITest, OpenFromMemory) {
    std::ifstream ifs(archivePath, std::ios::binary);
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());

    sevenzip::ArchiveReader reader(buffer);
    EXPECT_EQ(reader.itemCount(), 2u);
}

TEST_F(ArchiveReaderAPITest, MoveSemantics) {
    sevenzip::ArchiveReader reader1(archivePath);
    EXPECT_EQ(reader1.itemCount(), 2u);

    sevenzip::ArchiveReader reader2(std::move(reader1));
    EXPECT_EQ(reader2.itemCount(), 2u);
}

// ============================================================================
// 新增测试：边界条件和错误处理
// ============================================================================

TEST_F(ArchiveReaderAPITest, InvalidIndex) {
    sevenzip::ArchiveReader reader(archivePath);

    EXPECT_THROW(reader.itemInfo(999), std::exception);
    EXPECT_THROW(reader.extract(999), std::exception);
}

TEST_F(ArchiveReaderAPITest, EmptyPathFind) {
    sevenzip::ArchiveReader reader(archivePath);

    auto notFound = reader.findItem("");
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(ArchiveReaderAPITest, WithPassword) {
    sevenzip::ArchiveReader reader(archivePath);

    // Setting password on non-encrypted archive should not fail
    reader.withPassword("testpass");

    auto extractDir = testDir / "extracted";
    reader.extractAll(extractDir);

    EXPECT_TRUE(fs::exists(extractDir / "file1.txt"));
}
