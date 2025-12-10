// CMake Usage Example for libsevenzip
// This demonstrates how to use SevenZip library after installation

#include <filesystem>
#include <iostream>
#include <sevenzip/sevenzip.hpp>

namespace fs = std::filesystem;

void print_separator() {
    std::cout << "\n========================================\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║  SevenZip CMake Usage Example         ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";

    try {
        // Example 1: Get version info
        print_separator();
        std::cout << "Example 1: Library Version\n";
        print_separator();

        const char* version = sevenzip::version_string();
        std::cout << "SevenZip Library Version: " << version << "\n";

        // Example 2: List supported formats
        print_separator();
        std::cout << "Example 2: Supported Formats\n";
        print_separator();

        const char* formats[] = {"7Z", "ZIP", "TAR", "GZIP", "BZIP2", "XZ"};
        std::cout << "Supported archive formats:\n";
        for (const auto* fmt : formats) {
            std::cout << "  - " << fmt << "\n";
        }

        // Example 3: Demonstrate usage (conceptual)
        print_separator();
        std::cout << "Example 3: Basic Usage\n";
        print_separator();

        std::cout << "\nTo use SevenZip in your project:\n\n";

        std::cout << "1. Create an archive:\n";
        std::cout << "   auto writer = sevenzip::Writer::create(\"output.7z\");\n";
        std::cout << "   writer.addFile(\"file.txt\");\n";
        std::cout << "   writer.finalize();\n\n";

        std::cout << "2. Extract an archive:\n";
        std::cout << "   auto archive = sevenzip::Archive::open(\"input.7z\");\n";
        std::cout << "   archive.extractAll(\"output_dir/\");\n\n";

        std::cout << "3. List archive contents:\n";
        std::cout << "   auto archive = sevenzip::Archive::open(\"input.7z\");\n";
        std::cout << "   for (const auto& item : archive) {\n";
        std::cout << "       std::cout << item.path << std::endl;\n";
        std::cout << "   }\n";

        print_separator();
        std::cout << "\n✓ CMake integration successful!\n";
        std::cout << "  SevenZip library is properly linked and working.\n\n";

        return 0;

    } catch (const sevenzip::SevenZipException& e) {
        std::cerr << "SevenZip error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
