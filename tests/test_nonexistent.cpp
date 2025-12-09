#include <iostream>
#include "sevenzip/convenience.hpp"

int main() {
    try {
        sevenzip::compress("nonexistent.txt", "test.7z");
        std::cout << "ERROR: Should have thrown!" << std::endl;
    } catch (const sevenzip::Exception& e) {
        std::cout << "Caught Exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught std::exception: " << e.what() << std::endl;
    }
    return 0;
}
