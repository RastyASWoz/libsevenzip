# libsevenzip - Modern C++ Wrapper for 7-Zip

A cross-platform, modern C++ wrapper around the 7-Zip library, providing a clean API for archive manipulation operations.

## Status

**✅ PRODUCTION READY** - Core features implemented and tested.

### Implementation Progress

| Layer | Status | Tests | Coverage |
|-------|--------|-------|----------|
| **COM Wrapper** | ✅ Complete | 162/162 | 100% |
| **C++ API** | ✅ Complete | 354/354 | ~95% |
| **C ABI (FFI)** | 🚧 In Progress | - | - |

- **COM Wrapper Layer**: Fully implemented (162 tests, 100%)
  - ArchiveReader: 134 tests
  - ArchiveWriter: 28 tests
- **Modern C++ API**: Fully implemented (354 tests, 100%)
  - Archive, ArchiveReader, ArchiveWriter classes
  - Compressor class for standalone compression
  - Convenience functions
  - Memory operations support
- **C FFI Layer**: Design phase (see [docs/design/Stage4_FFI_Design.md](docs/design/Stage4_FFI_Design.md))

## Quick Start

### Building

```bash
cd build/windows-debug
cmake ../.. -G "Visual Studio 17 2022"
cmake --build . --config Release
ctest -C Release
```

### Basic Usage

#### Modern C++ API (Recommended)

**Simple Operations**:
```cpp
#include <sevenzip/convenience.hpp>

// One-line extract
sevenzip::extract("archive.7z", "output/");

// One-line compress
sevenzip::compress("folder/", "archive.7z");
```

**Advanced Usage**:
```cpp
#include <sevenzip/archive.hpp>

using namespace sevenzip;

// Create archive with configuration
Archive::create("archive.7z")
    .withCompressionLevel(CompressionLevel::Maximum)
    .withSolidMode(true)
    .addFile("file.txt")
    .addDirectory("folder/", true)
    .finalize();

// Read and iterate
auto archive = Archive::open("archive.7z");
for (const auto& item : archive) {
    std::cout << item.path << " - " << item.size << " bytes\n";
}
archive.extractAll("output/");
```

**Memory Operations**:
```cpp
#include <sevenzip/archive.hpp>

// Create archive in memory
std::vector<uint8_t> buffer;
Archive::createToMemory(buffer)
    .addFromMemory(data, "file.bin")
    .finalize();

// Open from memory
auto archive = Archive::openFromMemory(buffer);
auto extracted = archive.extractItemToMemory(0);
```

#### Low-Level COM Wrapper API

For advanced use cases requiring direct COM control:
```cpp
#include "wrapper/archive/archive_reader.hpp"

using namespace sevenzip::wrapper;

ArchiveReader reader;
reader.open(L"archive.7z");

// Extract with detailed control
reader.extractAll(L"output_directory");
```

## Features

### Supported Operations

#### Reading
- ✅ Open 7z, ZIP, TAR, RAR, and other formats
- ✅ List archive contents with metadata
- ✅ Extract all or specific files
- ✅ Extract to disk or memory
- ✅ Progress callbacks
- ✅ Password-protected archives (decryption)

#### Writing
- ✅ Create 7z, ZIP, TAR archives
- ✅ Add files from disk or memory
- ✅ Add directories recursively
- ✅ Compression levels (Fastest to Ultra)
- ✅ Compression methods (LZMA, LZMA2, PPMd, BZip2, Deflate)
- ✅ Solid and non-solid archives
- ✅ Multi-threaded compression
- ✅ Progress callbacks with cancellation
- ✅ Unicode file names and long paths
- ✅ Password-protected archive reading (decryption)
- ⚠️ Password-protected archive creation (limited - see Known Limitations)

### Known Limitations

See [ArchiveWriter_Limitations.md](docs/ArchiveWriter_Limitations.md) for details:

1. **Password-protected archive creation**: Currently not supported due to 7-Zip encoder internal crashes when combining encryption with file streams. Workarounds:
   - Use external tools (7z.exe) to add encryption after archive creation
   - Password-protected archive **reading** (decryption) works perfectly
   - Empty password-protected archives can be created (testing only)
2. **CompressionLevel::None with 7z format**: 7z format supports Copy method but ZIP is recommended for better compatibility with stored files

## Project Structure

```
libsevenzip/
├── include/sevenzip/        # Public API headers
├── src/
│   ├── wrapper/             # C++ wrapper implementation
│   │   ├── archive/         # ArchiveReader, ArchiveWriter
│   │   ├── stream/          # Stream implementations
│   │   └── common/          # Utilities, error handling
│   ├── api/                 # Modern C++ API layer (placeholder)
│   └── ffi/                 # C ABI / FFI layer (placeholder)
├── tests/
│   ├── unit/                # Unit tests (163 tests)
│   └── integration/         # Integration tests
├── third_party/7zip/        # 7-Zip library sources
│   ├── C/                   # C implementation
│   └── CPP/                 # C++ implementation
├── docs/                    # Documentation
│   ├── Part*.md             # Design documents
│   ├── TODO_*.md            # Implementation checklists
│   └── ArchiveWriter_Limitations.md  # Known issues
└── CMakeLists.txt

## Testing

**Run all tests**:
```bash
cd build/windows-debug
ctest -C Release --output-on-failure
```

**Run specific test suite**:
```bash
ctest -C Release -R ArchiveReaderTest
ctest -C Release -R ArchiveWriterTest
```

**Test Coverage**:
- **ArchiveReaderTest**: 134 tests (extraction, metadata, formats, passwords)
- **ArchiveWriterTest**: 29 tests (creation, compression, formats, edge cases)
- **StreamTest**: File and memory streams
- **FormatTest**: Format detection and handling
- **ErrorTest**: Error handling and exceptions
- **PropVariantTest**: Property system

## Documentation

- [Part 1: COM Background](docs/Part1_COM_Background.md) - Understanding 7-Zip's COM architecture
- [Part 2: Architecture Analysis](docs/Part2_Architecture_Analysis.md) - Wrapper design decisions
- [Part 3: Technical Routes](docs/Part3_Technical_Routes.md) - Implementation strategies
- [Part 4: Cross-Platform](docs/Part4_Cross_Platform.md) - Platform support
- [Part 5: CMake Build](docs/Part5_CMake_Build.md) - Build system
- [Part 6: FFI Design](docs/Part6_FFI_Design.md) - C API design (planned)
- [Known Limitations](docs/ArchiveWriter_Limitations.md) - Current issues and workarounds

## Requirements

- **Compiler**: MSVC 2022, GCC 10+, or Clang 12+
- **C++ Standard**: C++17 or later
- **CMake**: 3.20+
- **Platform**: Windows (tested), Linux (planned), macOS (planned)
- **Dependencies**: 
  - 7-Zip 25.01 (included in `third_party/7zip/`)
  - Google Test (via vcpkg or system)

## License

This wrapper is distributed under the MIT License (pending - check LICENSE file).

7-Zip library is licensed under GNU LGPL + unRAR restriction.
See `third_party/7zip/License.txt` for details.

## Contributing

Contributions welcome! Priority areas:
1. **Investigate password+files crash** - Deep dive into 7-Zip encoder to fix SEH exception
2. **Cross-platform testing** - Linux, macOS support
3. **C API layer implementation** - FFI interface for other languages
4. **Streaming API** - For large files
5. **Additional format support** - More archive types

## Changelog

### 2025-12-09 - v0.2.0 (COM Wrapper Layer Complete)
- ✅ **ArchiveReader** fully implemented (134 tests, 100%)
- ✅ **ArchiveWriter** fully implemented (28 tests, 100%)
- ✅ **162/162 tests passing** (100% pass rate)
- ✅ Fixed COM GUID collision issue (IID_ICompressSetInStream)
- ✅ Implemented CArchiveOpenCallback for encrypted header support
- ✅ Proper COM lifecycle management with CMyComPtr
- ✅ Support for 7z, ZIP, TAR formats
- ✅ Compression levels (Fastest to Ultra) and methods (LZMA, LZMA2, PPMd, BZip2, Deflate)
- ✅ Solid/non-solid, multi-threaded compression
- ✅ Progress callbacks with cancellation
- ✅ Unicode filenames and long paths (260+ characters)
- ✅ Password-protected archive **reading** (decryption fully working)
- ✅ Error handling with detailed HRESULT reporting
- ⚠️ **Known limitation**: Password-protected archive **creation** not supported (7-Zip encoder internal limitation)
- ⚠️ C API not yet implemented

**Next Phase**: Modern C++ API Layer (TODO_03)

## Contact

For issues, questions, or contributions, please visit the project repository.
