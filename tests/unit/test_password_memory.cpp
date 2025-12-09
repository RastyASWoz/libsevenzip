// Test password with memory file
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "../../src/wrapper/archive/archive_writer.hpp"

namespace fs = std::filesystem;
using namespace sevenzip;
using namespace sevenzip::detail;

TEST(PasswordDebug, MemoryFileWithPassword) {
    auto tempDir = fs::temp_directory_path() / "password_debug_memory";
    fs::create_directories(tempDir);

    try {
        auto archivePath = tempDir / "memory_password.7z";

        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        ArchiveProperties props;
        props.password = L"test123";
        writer.setProperties(props);

        // Add file from memory instead of disk
        std::string content = "Test content from memory";
        std::vector<uint8_t> data(content.begin(), content.end());

        std::cout << "Adding file from memory with password...\n";
        writer.addFileFromMemory(data, L"test.txt");

        std::cout << "Finalizing...\n";
        writer.finalize();

        std::cout << "SUCCESS! Archive with memory file and password created\n";
        EXPECT_TRUE(fs::exists(archivePath));
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        FAIL() << "Exception: " << e.what();
    }

    fs::remove_all(tempDir);
}
