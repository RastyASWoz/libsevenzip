// Minimal password test
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "../../src/wrapper/archive/archive_writer.hpp"

namespace fs = std::filesystem;
using namespace sevenzip;
using namespace sevenzip::detail;

TEST(PasswordDebug, SimplePasswordTest) {
    // Create temp directory
    auto tempDir = fs::temp_directory_path() / "password_debug_test";
    fs::create_directories(tempDir);

    // Create a test file
    auto testFile = tempDir / "test.txt";
    {
        std::ofstream ofs(testFile);
        ofs << "Test content for password archive.\n";
    }

    std::cout << "Step 1: Created test file\n";

    try {
        auto archivePath = tempDir / "test_password.7z";

        std::cout << "Step 2: Creating ArchiveWriter\n";
        ArchiveWriter writer;

        std::cout << "Step 3: Creating archive\n";
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        std::cout << "Step 4: Setting password with non-solid compression\n";
        ArchiveProperties props;
        props.password = L"test123";
        props.solid = false;  // Try non-solid compression
        std::cout << "Step 4b: Setting properties\n";
        writer.setProperties(props);

        std::cout << "Step 5: Adding file\n";
        writer.addFile(testFile.wstring(), L"test.txt");

        std::cout << "Step 6: Finalizing (this is where crash might occur)\n";
        writer.finalize();

        std::cout << "SUCCESS! Archive created with password\n";

        EXPECT_TRUE(fs::exists(archivePath));
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        FAIL() << "Exception: " << e.what();
    }

    // Cleanup
    fs::remove_all(tempDir);
}
