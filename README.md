# libsevenzip

[![Build Status](https://github.com/yourusername/libsevenzip/workflows/Build%20FFI%20Multi-Architecture/badge.svg)](https://github.com/yourusername/libsevenzip/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![7-Zip SDK](https://img.shields.io/badge/7--Zip%20SDK-25.01-green.svg)](https://www.7-zip.org/sdk.html)

**A modern FFI (Foreign Function Interface) library for 7-Zip SDK, designed for cross-language integration.**

[中文文档](README.zh-CN.md) | **English**

---

## 🎯 Project Goals

### Primary Goal: FFI Bindings for Multiple Languages

This project's **main purpose** is to provide stable, easy-to-use FFI bindings (to be implemented in the future) for:

- 🐍 **Python**
- 🦀 **Rust**
- 🎯 **Dart**
- 💎 **C#**
- 🌐 Other languages supporting C FFI

### Secondary Goal: Optional C++ API

A modern C++ API is provided for those who need it, but:

⚠️ **Important Notices for C++ Users:**
- **Windows-only**: Due to current implementation using Windows SDK COM interfaces, cross-platform support is not available
- **Not fully tested**: The C++ API has not undergone comprehensive testing
- **Recommended alternative**: If you need a mature C++ 7-Zip library, we recommend **[bit7z](https://github.com/rikyoz/bit7z)** instead

The C++ API is maintained primarily as an internal dependency for FFI implementation.

---

## 📦 Installation

### Pre-built Binaries (FFI)

Download pre-built DLLs from Releases.

### Language-Specific Packages (Coming Soon)

---

## 🚀 Quick Start

### For FFI Users (Python/Rust/Dart/C#)

**Python Example:**
```python
import ctypes

# Load the DLL
lib = ctypes.CDLL("sevenzip_ffi.dll")

# Extract archive
result = lib.sz_extract_simple(b"archive.7z", b"output/")
if result != 0:  # SZ_OK = 0
    print("Extraction failed")
```

**Rust Example:**
```rust
use libloading::{Library, Symbol};

unsafe {
    let lib = Library::new("sevenzip_ffi.dll")?;
    let extract: Symbol<unsafe extern fn(*const u8, *const u8) -> i32> 
        = lib.get(b"sz_extract_simple\\0")?;
    
    let result = extract(b"archive.7z\\0".as_ptr(), b"output/\\0".as_ptr());
    if result != 0 {
        eprintln!("Extraction failed");
    }
}
```

📚 **Full API Documentation**: See [`docs/API_Reference.md`](docs/API_Reference.md)

---

## 🛠️ Building from Source

### Prerequisites

- **Windows 10/11** (Windows SDK 10.0.19041.0, other versions not verified, Win11 SDK known to be unsupported)
- **Visual Studio 2022** (or Build Tools with MSVC 14.4+)
- **CMake 3.20+**

### Standard CMake Build

```bash
# x64 build (64-bit)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target sevenzip_ffi

# x86 build (32-bit)
cmake -B build_x86 -G "Visual Studio 17 2022" -A Win32
cmake --build build_x86 --config Release --target sevenzip_ffi

# Output: build/bin/Release/sevenzip_ffi.dll
```

### Build Both Architectures

```bash
# Configure
cmake -B build_x64 -G "Visual Studio 17 2022" -A x64
cmake -B build_x86 -G "Visual Studio 17 2022" -A Win32

# Build
cmake --build build_x64 --config Release --target sevenzip_ffi --parallel
cmake --build build_x86 --config Release --target sevenzip_ffi --parallel

# Collect artifacts
mkdir dist\\x64 dist\\x86
copy build_x64\\bin\\Release\\sevenzip_ffi.dll dist\\x64\\
copy build_x86\\bin\\Release\\sevenzip_ffi.dll dist\\x86\\
copy include\\sevenzip\\sevenzip_capi.h dist\\include\\sevenzip\\
```

### Optional: Build C++ API

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64 \\
  -DSEVENZIP_BUILD_CPP_API=ON \\
  -DSEVENZIP_BUILD_EXAMPLES=ON

cmake --build build --config Release
```

⚠️ **Remember**: C++ API is Windows-only and not fully tested.

---

## 📋 Features

### Current FFI API Status

✅ **Implemented:**
- Version information (`sz_get_version`)
- Error handling (`sz_get_error_message`, `sz_get_last_error`)
- Archive reading (open, list files, extract)
- Archive writing (create 7z/zip/tar archives)
- Basic compression/decompression

📝 **Planned Features:**
- Advanced compression options
- Password-protected archives (full support)
- Solid archives configuration
- Multi-volume archives
- Archive updates (add/remove files)

*Note: Detailed feature roadmap is maintained in `docs/FFI_TODO.md` (private development document, not publicly tracked).*

### Supported Formats

| Format | Read | Write | Notes |
|--------|------|-------|-------|
| 7z     | ✅   | ✅    | Full support |
| Zip    | ✅   | ✅    | Full support |
| Tar    | ✅   | ✅    | Full support |
| Gzip   | ✅   | ✅    | `.gz` files |
| Bzip2  | ✅   | ✅    | `.bz2` files |
| Xz     | ✅   | ✅    | Full support |
| Rar    | ✅   | ❌    | Read-only (proprietary) |
| Rar5   | ✅   | ❌    | Read-only (proprietary) |

---

## 📚 Documentation

- **[API Reference](Document/API_Reference.md)** - Complete C API documentation
- **[API Reference (中文)](Document/API_Reference.zh-CN.md)** - 完整的 C API 文档
- **[Contributing](CONTRIBUTING.md)** - How to contribute

---

## 🤝 Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Areas Needing Help

- 🐍 **Python package**
- 🦀 **Rust crate**
- 🎯 **Dart package**
- 💎 **C# package**
- 📖 Documentation improvements
- 🧪 FFI API testing

---

## 📜 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file.

### Third-Party Licenses

- **7-Zip SDK**: Public Domain + GNU LGPL (unRAR code)
  - License: https://www.7-zip.org/license.txt
  - Version: 25.01

---

## 🔗 Related Projects

- **[7-Zip](https://www.7-zip.org/)** - Original 7-Zip archiver
- **[bit7z](https://github.com/rikyoz/bit7z)** - Recommended C++ library for 7-Zip

---

## 📞 Contact & Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/libsevenzip/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/libsevenzip/discussions)

---

## 🏗️ Project Status

- **Version**: 0.1.0 (Alpha)
- **Status**: Active Development
- **Platform**: Windows only (due to Windows SDK COM dependency)
- **Target Audience**: FFI users (Python, Rust, Dart, C#)

**Note**: This is an early-stage project. APIs may change before v1.0.0 release.
