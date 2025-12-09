// Minimal test to debug password + files crash
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sevenzip/sevenzip.hpp>
#include <vector>

namespace fs = std::filesystem;
using namespace sevenzip;

TEST(PasswordMinimalDebug, EmptyArchive) {
    // This should work
    auto tempPath = fs::temp_directory_path() / "test_empty_pwd.7z";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.password = L"test123";

    writer.create(tempPath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.finalize();

    EXPECT_TRUE(fs::exists(tempPath));
    fs::remove(tempPath);
}

TEST(PasswordMinimalDebug, TinyFileFromMemory) {
    // Test with minimal memory data
    auto tempPath = fs::temp_directory_path() / "test_tiny_pwd.7z";

    ArchiveWriter writer;
    ArchiveProperties props;
    props.password = L"test123";

    writer.create(tempPath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);

    // Very small data
    std::vector<uint8_t> data = {0x01};
    writer.addFile(L"tiny.bin", data);

    try {
        writer.finalize();  // Should crash here
        SUCCEED() << "Unexpectedly succeeded!";
    } catch (...) {
        FAIL() << "Exception thrown instead of crash";
    }

    if (fs::exists(tempPath)) {
        fs::remove(tempPath);
    }
}

TEST(PasswordMinimalDebug, TinyFileFromDisk) {
    // Test with minimal disk file
    auto tempPath = fs::temp_directory_path() / "test_disk_pwd.7z";
    auto dataPath = fs::temp_directory_path() / "tiny_data.txt";

    // Create tiny file
    {
        std::ofstream ofs(dataPath);
        ofs << "A";
    }

    ArchiveWriter writer;
    ArchiveProperties props;
    props.password = L"test123";

    writer.create(tempPath.wstring(), ArchiveFormat::SevenZip);
    writer.setProperties(props);
    writer.addFile(dataPath.wstring(), L"tiny.txt");

    try {
        writer.finalize();  // Should crash here
        SUCCEED() << "Unexpectedly succeeded!";
    } catch (...) {
        FAIL() << "Exception thrown instead of crash";
    }

    fs::remove(dataPath);
    if (fs::exists(tempPath)) {
        fs::remove(tempPath);
    }
}
