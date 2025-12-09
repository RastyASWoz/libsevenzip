// test_writer_debug.cpp - Debug test for ArchiveWriter
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../../src/wrapper/archive/archive_writer.hpp"

namespace fs = std::filesystem;

int main() {
    using namespace sevenzip::detail;

    try {
        // Create test directory
        auto tempDir = fs::temp_directory_path() / "test_writer_debug";
        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
        fs::create_directories(tempDir);

        // Create test file
        auto testFile = tempDir / "test.txt";
        {
            std::ofstream f(testFile);
            f << "Hello, World!";
        }

        std::wcout << L"Test file: " << testFile.wstring() << std::endl;
        std::wcout << L"Exists: " << fs::exists(testFile) << std::endl;
        std::wcout << L"Size: " << fs::file_size(testFile) << std::endl;

        // Create archive
        auto archivePath = tempDir / "test.7z";
        std::wcout << L"Archive path: " << archivePath.wstring() << std::endl;

        ArchiveWriter writer;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        std::wcout << L"Adding file..." << std::endl;
        writer.addFile(testFile.wstring(), L"test.txt");

        std::wcout << L"Finalizing..." << std::endl;
        writer.finalize();

        std::wcout << L"Success! Archive created." << std::endl;
        std::wcout << L"Archive exists: " << fs::exists(archivePath) << std::endl;
        std::wcout << L"Archive size: " << fs::file_size(archivePath) << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
