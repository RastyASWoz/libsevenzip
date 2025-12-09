// Very minimal password test - empty archive
#include <gtest/gtest.h>

#include <filesystem>

#include "../../src/wrapper/archive/archive_writer.hpp"

namespace fs = std::filesystem;
using namespace sevenzip;
using namespace sevenzip::detail;

TEST(PasswordDebug, EmptyArchiveWithPassword) {
    auto tempDir = fs::temp_directory_path() / "password_debug_empty";
    fs::create_directories(tempDir);

    try {
        auto archivePath = tempDir / "empty_password.7z";

        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        ArchiveProperties props;
        props.password = L"test123";
        writer.setProperties(props);

        // DON'T add any files - just finalize empty archive
        std::cout << "Finalizing empty archive with password...\n";
        writer.finalize();

        std::cout << "SUCCESS! Empty archive with password created\n";
        EXPECT_TRUE(fs::exists(archivePath));
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        FAIL() << "Exception: " << e.what();
    }

    fs::remove_all(tempDir);
}

TEST(PasswordDebug, EmptyArchiveNoPassword) {
    auto tempDir = fs::temp_directory_path() / "password_debug_empty2";
    fs::create_directories(tempDir);

    try {
        auto archivePath = tempDir / "empty_nopassword.7z";

        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        // No password, no properties
        std::cout << "Finalizing empty archive without password...\n";
        writer.finalize();

        std::cout << "SUCCESS! Empty archive without password created\n";
        EXPECT_TRUE(fs::exists(archivePath));
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        FAIL() << "Exception: " << e.what();
    }

    fs::remove_all(tempDir);
}
