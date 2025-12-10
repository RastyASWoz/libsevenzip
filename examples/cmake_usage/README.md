# CMake Usage Example

This example demonstrates how to use libsevenzip in a CMake project after installation.

## Prerequisites

1. Install libsevenzip:
   ```powershell
   # In libsevenzip root directory
   cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="C:/Program Files/SevenZip"
   cmake --build build --config Release
   cmake --install build --config Release  # Requires admin rights
   ```

2. Verify installation:
   ```powershell
   dir "C:\Program Files\SevenZip"
   # Should see: include/, lib/, bin/ directories
   ```

## Building This Example

### Method 1: Using CMAKE_PREFIX_PATH (Recommended)

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:/Program Files/SevenZip"

cmake --build build --config Release

.\build\Release\cmake_example.exe
```

### Method 2: Using SevenZip_DIR

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DSevenZip_DIR="C:/Program Files/SevenZip/lib/cmake/SevenZip"

cmake --build build --config Release

.\build\Release\cmake_example.exe
```

### Method 3: Custom Install Location

If you installed to a custom location:

```powershell
# Assuming installation at D:/local/SevenZip
cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/local/SevenZip"

cmake --build build --config Release
```

## Expected Output

```
╔════════════════════════════════════════╗
║  SevenZip CMake Usage Example         ║
╚════════════════════════════════════════╝

========================================
Example 1: Library Version
========================================
SevenZip Library Version: 0.1.0

========================================
Example 2: Supported Formats
========================================
Supported archive formats:
  - 7Z
  - ZIP
  - TAR
  - GZIP
  - BZIP2
  - XZ

========================================
Example 3: Basic Usage
========================================

To use SevenZip in your project:
...

✓ CMake integration successful!
  SevenZip library is properly linked and working.
```

## Troubleshooting

### Error: "Could not find a package configuration file provided by SevenZip"

**Solution**: Ensure CMAKE_PREFIX_PATH or SevenZip_DIR points to the correct installation directory.

### Error: "DLL not found"

**Solution**: Copy `sevenzip.dll` to your executable directory or add install location to PATH:

```powershell
copy "C:\Program Files\SevenZip\bin\sevenzip.dll" .\build\Release\
```

## Integration in Your Project

Copy the `CMakeLists.txt` pattern from this example:

```cmake
find_package(SevenZip 0.1 REQUIRED)
target_link_libraries(your_target PRIVATE SevenZip::sevenzip)
```

That's it! No need to manually specify include directories or library paths.

## See Also

- [CMAKE_INSTALL.md](../../docs/CMAKE_INSTALL.md) - Complete install guide
- [CONTRIBUTING.md](../../CONTRIBUTING.md) - Development guidelines
- [README.md](../../README.md) - Project overview
