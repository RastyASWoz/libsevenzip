// Standalone program to reproduce password + files crash for WinDbg debugging
#include <DbgHelp.h>
#include <Psapi.h>
#include <Windows.h>
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Psapi.lib")

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "wrapper/archive/archive_format.hpp"
#include "wrapper/archive/archive_writer.hpp"

namespace fs = std::filesystem;
using namespace sevenzip;
using namespace sevenzip::detail;

// SEH exception filter to capture crash details
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers) {
    std::cerr << "\n=== SEH EXCEPTION CAUGHT ===" << std::endl;
    std::cerr << "Exception Code: 0x" << std::hex
              << pExceptionPointers->ExceptionRecord->ExceptionCode << std::dec << std::endl;
    std::cerr << "Exception Address: " << pExceptionPointers->ExceptionRecord->ExceptionAddress
              << std::endl;

    // Get module info
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                      (LPCTSTR)pExceptionPointers->ExceptionRecord->ExceptionAddress, &hModule);
    if (hModule) {
        char moduleName[MAX_PATH];
        GetModuleFileNameA(hModule, moduleName, MAX_PATH);
        std::cerr << "Module: " << moduleName << std::endl;

        // Calculate offset within module
        MODULEINFO modInfo;
        GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo));
        size_t offset = (size_t)pExceptionPointers->ExceptionRecord->ExceptionAddress -
                        (size_t)modInfo.lpBaseOfDll;
        std::cerr << "Offset in module: 0x" << std::hex << offset << std::dec << std::endl;
    }

    // Initialize symbols
    HANDLE hProcess = GetCurrentProcess();
    SymInitialize(hProcess, NULL, TRUE);

    // Get symbol name for crash address
    char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    DWORD64 displacement = 0;
    if (SymFromAddr(hProcess, (DWORD64)pExceptionPointers->ExceptionRecord->ExceptionAddress,
                    &displacement, pSymbol)) {
        std::cerr << "Function: " << pSymbol->Name << " + 0x" << std::hex << displacement
                  << std::dec << std::endl;
    }

    // Simple stack trace with symbols
    std::cerr << "\nStack trace:" << std::endl;
    void* stack[20];
    USHORT frames = CaptureStackBackTrace(0, 20, stack, NULL);
    for (USHORT i = 0; i < frames; i++) {
        displacement = 0;
        if (SymFromAddr(hProcess, (DWORD64)stack[i], &displacement, pSymbol)) {
            std::cerr << "  [" << i << "] " << pSymbol->Name << " + 0x" << std::hex << displacement
                      << std::dec << std::endl;
        } else {
            std::cerr << "  [" << i << "] " << stack[i] << std::endl;
        }
    }

    SymCleanup(hProcess);

    return EXCEPTION_EXECUTE_HANDLER;
}

int main() {
    SetUnhandledExceptionFilter(ExceptionFilter);
    try {
        std::cout << "=== Password + File Crash Reproducer ===" << std::endl;

        // Create temp paths
        auto tempDir = fs::temp_directory_path();
        auto archivePath = tempDir / "debug_password_test.7z";
        auto testDataPath = tempDir / "debug_test_data.txt";

        // Clean up old files
        if (fs::exists(archivePath)) {
            fs::remove(archivePath);
        }
        if (fs::exists(testDataPath)) {
            fs::remove(testDataPath);
        }

        // Create a small test file
        std::cout << "1. Creating test file: " << testDataPath << std::endl;
        {
            std::ofstream ofs(testDataPath, std::ios::binary);
            ofs << "Test data for password debugging\n";
        }

        // Test 1: Empty archive with password (should work)
        std::cout << "\n2. Testing empty archive + password (should work)..." << std::endl;
        {
            ArchiveWriter writer;
            ArchiveProperties props;
            props.password = L"test123";

            writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);
            writer.setProperties(props);
            writer.finalize();

            std::cout << "   SUCCESS: Empty archive created" << std::endl;
            fs::remove(archivePath);
        }

        // Test 2: File with password (will crash)
        std::cout << "\n3. Testing file + password (WILL CRASH)..." << std::endl;
        std::cout << "   Creating ArchiveWriter..." << std::endl;

        ArchiveWriter writer;
        ArchiveProperties props;
        props.password = L"test123";  // RE-ENABLED FOR DEBUGGING
        props.numThreads = 1;         // FORCE SINGLE-THREADED MODE

        std::cout << "   Calling create()..." << std::endl;
        writer.create(archivePath.wstring(), ArchiveFormat::SevenZip);

        std::cout << "   Calling setProperties()..." << std::endl;
        writer.setProperties(props);

        std::cout << "   Calling addFile()..." << std::endl;
        writer.addFile(testDataPath.wstring(), L"test.txt");

        std::cout << "   Calling finalize() - CRASH EXPECTED HERE..." << std::endl;
        std::wcerr << L"[MAIN] Before finalize()\n";
        std::wcerr.flush();

        writer.finalize();  // <-- CRASH POINT

        std::wcerr << L"[MAIN] After finalize()\n";

        std::cout << "   SUCCESS: Archive created (unexpected!)" << std::endl;

        // Cleanup
        fs::remove(archivePath);
        fs::remove(testDataPath);

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\nException caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nUnknown exception caught" << std::endl;
        return 2;
    }
}
