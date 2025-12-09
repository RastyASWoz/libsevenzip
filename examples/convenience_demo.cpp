#include <filesystem>
#include <fstream>
#include <iostream>
#include <sevenzip/convenience.hpp>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        std::cout << "=== libsevenzip Convenience Functions Demo ===\n\n";

        // 创建临时测试目录
        auto tempDir = fs::temp_directory_path() / "libsevenzip_demo";
        fs::create_directories(tempDir);

        auto testFile = tempDir / "test.txt";
        auto archivePath = tempDir / "demo.7z";
        auto extractDir = tempDir / "extracted";

        // 1. 创建测试文件
        std::cout << "1. Creating test file...\n";
        {
            std::ofstream ofs(testFile);
            ofs << "Hello from libsevenzip!\n";
            ofs << "This is a demonstration of convenience functions.\n";
            ofs << "Compression and decompression made easy!\n";
        }
        std::cout << "   Created: " << testFile << "\n\n";

        // 2. 压缩文件
        std::cout << "2. Compressing file to 7z archive...\n";
        sevenzip::compress(testFile, archivePath);
        std::cout << "   Archive created: " << archivePath << "\n";
        std::cout << "   Archive size: " << fs::file_size(archivePath) << " bytes\n\n";

        // 3. 列出归档内容
        std::cout << "3. Listing archive contents:\n";
        auto items = sevenzip::list(archivePath);
        for (const auto& item : items) {
            std::cout << "   - " << item.path << " (" << item.size << " bytes, "
                      << (item.isDirectory ? "dir" : "file") << ")\n";
        }
        std::cout << "\n";

        // 4. 获取归档信息
        std::cout << "4. Archive information:\n";
        auto info = sevenzip::getArchiveInfo(archivePath);
        std::cout << "   Format: 7z\n";
        std::cout << "   Item count: " << info.itemCount << "\n";
        std::cout << "   Packed size: " << info.packedSize << " bytes\n";
        std::cout << "   Original size: " << info.totalSize << " bytes\n";
        if (info.totalSize > 0) {
            double ratio = 100.0 * (1.0 - (double)info.packedSize / info.totalSize);
            std::cout << "   Compression ratio: " << ratio << "%\n";
        }
        std::cout << "\n";

        // 5. 测试归档完整性
        std::cout << "5. Testing archive integrity...\n";
        if (sevenzip::testArchive(archivePath)) {
            std::cout << "   ✓ Archive is valid and not corrupted\n\n";
        } else {
            std::cout << "   ✗ Archive is corrupted\n\n";
        }

        // 6. 检查文件是否为归档
        std::cout << "6. Checking file types:\n";
        std::cout << "   " << archivePath.filename()
                  << " is archive: " << (sevenzip::isArchive(archivePath) ? "YES" : "NO") << "\n";
        std::cout << "   " << testFile.filename()
                  << " is archive: " << (sevenzip::isArchive(testFile) ? "YES" : "NO") << "\n\n";

        // 7. 解压到内存
        std::cout << "7. Extracting to memory...\n";
        auto data = sevenzip::extractSingleFile(archivePath);
        std::string content(data.begin(), data.end());
        std::cout << "   Extracted " << data.size() << " bytes\n";
        std::cout << "   Content preview:\n";
        std::cout << "   " << content.substr(0, 50) << "...\n\n";

        // 8. 解压到目录
        std::cout << "8. Extracting to directory...\n";
        sevenzip::extract(archivePath, extractDir);
        std::cout << "   Extracted to: " << extractDir << "\n";

        auto extractedFile = extractDir / "test.txt";
        if (fs::exists(extractedFile)) {
            std::cout << "   ✓ File extracted successfully\n";
            std::cout << "   Size: " << fs::file_size(extractedFile) << " bytes\n";
        }
        std::cout << "\n";

        // 9. 压缩内存数据
        std::cout << "9. Compressing data in memory...\n";
        std::vector<uint8_t> testData(1000);
        for (size_t i = 0; i < testData.size(); ++i) {
            testData[i] = static_cast<uint8_t>(i % 256);
        }
        auto compressed = sevenzip::compressData(testData);
        std::cout << "   Original size: " << testData.size() << " bytes\n";
        std::cout << "   Compressed size: " << compressed.size() << " bytes\n";
        double memRatio = 100.0 * (1.0 - (double)compressed.size() / testData.size());
        std::cout << "   Compression ratio: " << memRatio << "%\n\n";

        // 10. 测试不同格式
        std::cout << "10. Testing different formats:\n";
        auto zipPath = tempDir / "demo.zip";
        auto tarPath = tempDir / "demo.tar";

        sevenzip::compress(testFile, zipPath, sevenzip::Format::Zip);
        std::cout << "   ZIP created: " << fs::file_size(zipPath) << " bytes\n";

        sevenzip::compress(testFile, tarPath, sevenzip::Format::Tar);
        std::cout << "   TAR created: " << fs::file_size(tarPath) << " bytes\n\n";

        // 清理
        std::cout << "Demo completed successfully!\n";
        std::cout << "Temporary files created in: " << tempDir << "\n";
        std::cout << "(You can manually delete this directory)\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
