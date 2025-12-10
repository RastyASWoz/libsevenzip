# libsevenzip - Modern FFI Library for 7-Zip

**Primary Goal**: Provide stable FFI bindings for Python, Rust, Go, Node.js and other languages.

**Secondary Goal**: Optional modern C++ API (if you need C++, consider using [bit7z](https://github.com/rikyoz/bit7z)).

## Quick Start

### For FFI Users (Python/Rust/Go/etc.)

**Standard CMake build** (recommended):
```bash
# x64 build
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# x86 build
cmake -B build_x86 -G "Visual Studio 17 2022" -A Win32
cmake --build build_x86 --config Release

# Install FFI library
cmake --install build --config Release --prefix ./install
```

**Multi-architecture build** (builds both x64 and x86):
```bash
cmake -B build_x64 -G "Visual Studio 17 2022" -A x64
cmake -B build_x86 -G "Visual Studio 17 2022" -A Win32

cmake --build build_x64 --config Release --target sevenzip_ffi
cmake --build build_x86 --config Release --target sevenzip_ffi

# Collect artifacts
mkdir dist\x64 dist\x86
copy build_x64\bin\Release\sevenzip_ffi.dll dist\x64\
copy build_x86\bin\Release\sevenzip_ffi.dll dist\x86\
```

**Output**:
```
build/bin/Release/sevenzip_ffi.dll    # FFI library (x64 or x86)
include/sevenzip/sevenzip_capi.h      # C API header
```

### For C++ Users (Optional)

If you need C++ API, enable it explicitly:

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64 -DSEVENZIP_BUILD_CPP_API=ON
cmake --build build --config Release
```

**Note**: For production C++ usage, we recommend [bit7z](https://github.com/rikyoz/bit7z) which is more mature.

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `SEVENZIP_BUILD_FFI` | `ON` | Build FFI library (primary target) |
| `SEVENZIP_BUILD_CPP_API` | `OFF` | Build modern C++ API (optional) |
| `SEVENZIP_BUILD_EXAMPLES` | `OFF` | Build example programs |
| `SEVENZIP_BUILD_TESTS` | `OFF` | Build unit tests |
| `SEVENZIP_ENABLE_INSTALL` | `OFF` | Enable CMake package config (for vcpkg/Conan) |

## Usage Examples

### Python (ctypes)

```python
import ctypes
import platform

# Auto-detect architecture
arch = "x64" if platform.machine().endswith("64") else "x86"
lib = ctypes.CDLL(f"sevenzip_ffi_{arch}.dll")

# Get version
lib.sz_version_string.restype = ctypes.c_char_p
print(lib.sz_version_string().decode())
```

### Rust (libloading)

```rust
use libloading::{Library, Symbol};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let arch = if cfg!(target_pointer_width = "64") { "x64" } else { "x86" };
    let lib = unsafe { Library::new(format!("sevenzip_ffi_{}.dll", arch))? };
    
    unsafe {
        let version: Symbol<unsafe extern fn() -> *const i8> = 
            lib.get(b"sz_version_string")?;
        println!("{:?}", std::ffi::CStr::from_ptr(version()));
    }
    Ok(())
}
```

### Go (cgo)

```go
package main

/*
#cgo CFLAGS: -I./include
#cgo windows,amd64 LDFLAGS: -L./x64 -lsevenzip_ffi
#cgo windows,386 LDFLAGS: -L./x86 -lsevenzip_ffi
#include <sevenzip/sevenzip_capi.h>
*/
import "C"
import "fmt"

func main() {
    version := C.GoString(C.sz_version_string())
    fmt.Println("Version:", version)
}
```

## Project Structure

```
libsevenzip/
├── include/sevenzip/          # Public headers
│   └── sevenzip_capi.h        # C API (FFI)
├── src/
│   ├── ffi/                   # FFI implementation (PRIMARY)
│   ├── wrapper/               # COM wrapper (internal)
│   └── api/                   # C++ API (optional)
├── third_party/7zip/          # 7-Zip source code
├── examples/                  # Usage examples
├── tests/                     # Unit tests
└── CMakeLists.txt             # Simple, focused build
```

## Building for Distribution

### Multi-Architecture Distribution

```bash
# Build both architectures
cmake -B build_x64 -A x64
cmake -B build_x86 -A Win32
cmake --build build_x64 --config Release --target sevenzip_ffi
cmake --build build_x86 --config Release --target sevenzip_ffi

# Create distribution package
mkdir ffi_dist\x64 ffi_dist\x86 ffi_dist\include\sevenzip
copy build_x64\bin\Release\sevenzip_ffi.dll ffi_dist\x64\
copy build_x86\bin\Release\sevenzip_ffi.dll ffi_dist\x86\
copy include\sevenzip\sevenzip_capi.h ffi_dist\include\sevenzip\

# Create archive
tar -czf sevenzip_ffi_v0.1.0_windows.tar.gz ffi_dist
```

### Single Architecture (Simpler)

```bash
# Most users only need x64
cmake -B build -A x64
cmake --build build --config Release
cmake --install build --prefix ./dist
```

## Why This Project?

**Primary Use Case**: FFI bindings for modern languages
- Python packages (via ctypes/cffi)
- Rust crates (via bindgen)
- Go modules (via cgo)
- Node.js packages (via ffi-napi)

**Not a C++ Library**: For C++ usage, we recommend [bit7z](https://github.com/rikyoz/bit7z)

## Differences from bit7z

| Feature | libsevenzip | bit7z |
|---------|-------------|-------|
| **Primary Goal** | FFI for other languages | Modern C++ API |
| **API** | C ABI (stable) | C++ (classes/templates) |
| **Target Users** | Python/Rust/Go/Node.js | C++ applications |
| **Maturity** | New (v0.1.0) | Mature production-ready |
| **Recommendation** | Use for language bindings | Use for C++ projects |

## Documentation

- [CMake Installation Guide](docs/CMAKE_INSTALL.md) - For C++ integration
- [FFI Distribution Guide](docs/FFI_DISTRIBUTION.md) - For language bindings
- [C API Reference](docs/Part6_FFI_Design.md) - FFI function documentation
- [Architecture](docs/Part2_Architecture_Analysis.md) - Technical design

## License

- **libsevenzip**: MIT License
- **7-Zip**: LGPL 2.1+ with unRAR restriction

See [LICENSE](LICENSE) for details.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## Alternatives

- **For C++ projects**: Use [bit7z](https://github.com/rikyoz/bit7z) (recommended)
- **For Python**: [py7zr](https://github.com/miurahr/py7zr) (pure Python)
- **For Node.js**: [node-7z](https://github.com/quentinrossetti/node-7z) (CLI wrapper)

libsevenzip fills the gap for stable FFI bindings across multiple languages.
