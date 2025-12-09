// Minimal test to debug CompressionLevel::None crash
#include <filesystem>
#include <fstream>
#include <iostream>

#include "wrapper/archive/archive_writer.hpp"

namespace fs = std::filesystem;
using namespace sevenzip::wrapper;

int main() {
    try {
        // Create temp directory
        auto tempDir = fs::temp_directory_path() / "debug_compression_none";
        fs::create_directories(tempDir);

        // Create a test file
        auto testFile = tempDir / "test.txt";
        {
            std::ofstream ofs(testFile);
            ofs << "This is a test file for compression level none.\n";
        }

        std::cout << "Step 1: Created test file: " << testFile << std::endl;

        // Create archive
        auto archivePath = tempDir / "test.7z";

        std::cout << "Step 2: Creating ArchiveWriter..." << std::endl;
        ArchiveWriter writer;

        std::cout << "Step 3: Setting properties (CompressionLevel::None)..." << std::endl;
        ArchiveProperties props;
        props.level = CompressionLevel::None;

        std::cout << "Step 4: Creating archive..." << std::endl;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        std::cout << "Step 5: Setting properties..." << std::endl;
        writer.setProperties(props);

        std::cout << "Step 6: Adding file..." << std::endl;
        writer.addFile(testFile.wstring(), L"test.txt");

        std::cout << "Step 7: Finalizing..." << std::endl;
        writer.finalize();

        std::cout << "SUCCESS! Archive created at: " << archivePath << std::endl;

        // Cleanup
        fs::remove_all(tempDir);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
