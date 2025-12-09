#include <iostream>
#include <sevenzip/archive.hpp>
#include <vector>

int main() {
    try {
        std::cout << "Testing createToMemory with GZip..." << std::endl;

        std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
        std::vector<uint8_t> buffer;

        {
            auto archive = sevenzip::Archive::createToMemory(buffer, sevenzip::Format::GZip);
            std::cout << "Created archive" << std::endl;

            archive.addFromMemory(testData, "data");
            std::cout << "Added data" << std::endl;

            archive.finalize();
            std::cout << "Finalized, buffer size: " << buffer.size() << std::endl;
        }

        std::cout << "SUCCESS!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
