# libsevenzip

[![构建状态](https://github.com/yourusername/libsevenzip/workflows/Build%20FFI%20Multi-Architecture/badge.svg)](https://github.com/yourusername/libsevenzip/actions)
[![许可证](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![7-Zip SDK](https://img.shields.io/badge/7--Zip%20SDK-25.01-green.svg)](https://www.7-zip.org/sdk.html)

**现代化的 7-Zip SDK FFI（外部函数接口）库，专为跨语言集成设计。**

**中文** | [English](README.md)

---

## 🎯 项目目标

### 首要目标：为多种编程语言提供 FFI 绑定

本项目的**主要目的**是为以下语言提供稳定、易用的 FFI 绑定(将在未来实现)：

- 🐍 **Python**
- 🦀 **Rust**
- 🎯 **Dart**
- 💎 **C#**
- 🌐 其他支持 C FFI 的语言

### 次要目标：可选的 C++ API

为有需要的用户提供了现代化的 C++ API，但请注意：

⚠️ **C++ 用户重要提示：**
- **仅限 Windows**：由于当前实现使用了 Windows SDK COM 接口，不支持跨平台
- **未经完整测试**：C++ API 尚未经过全面测试
- **推荐替代方案**：如果您需要成熟的 C++ 7-Zip 库，我们推荐使用 **[bit7z](https://github.com/rikyoz/bit7z)**

C++ API 主要作为 FFI 实现的内部依赖而维护。

---

## 📦 安装

### 预编译二进制文件（FFI）

从Releases下载预编译 DLL：

### 语言特定包（即将推出）

---

## 🚀 快速开始

### FFI 用户（Python/Rust/Dart/C#）

**Python 示例：**
```python
import ctypes

# 加载 DLL
lib = ctypes.CDLL("sevenzip_ffi.dll")

# 解压档案
result = lib.sz_extract_simple(b"archive.7z", b"output/")
if result != 0:  # SZ_OK = 0
    print("解压失败")
```

**Rust 示例：**
```rust
use libloading::{Library, Symbol};

unsafe {
    let lib = Library::new("sevenzip_ffi.dll")?;
    let extract: Symbol<unsafe extern fn(*const u8, *const u8) -> i32> 
        = lib.get(b"sz_extract_simple\\0")?;
    
    let result = extract(b"archive.7z\\0".as_ptr(), b"output/\\0".as_ptr());
    if result != 0 {
        eprintln!("解压失败");
    }
}
```

**C# 示例：**
```csharp
using System.Runtime.InteropServices;

class Program {
    [DllImport("sevenzip_ffi.dll")]
    static extern int sz_extract_simple(
        [MarshalAs(UnmanagedType.LPStr)] string archivePath,
        [MarshalAs(UnmanagedType.LPStr)] string outputDir
    );
    
    static void Main() {
        int result = sz_extract_simple("archive.7z", "output/");
        if (result != 0) {
            Console.WriteLine("解压失败");
        }
    }
}
```

**Dart 示例：**
```dart
import 'dart:ffi';
import 'package:ffi/ffi.dart';

typedef ExtractSimpleNative = Int32 Function(
  Pointer<Utf8>, Pointer<Utf8>
);
typedef ExtractSimpleDart = int Function(
  Pointer<Utf8>, Pointer<Utf8>
);

void main() {
  final lib = DynamicLibrary.open('sevenzip_ffi.dll');
  final extract = lib.lookupFunction<
    ExtractSimpleNative, ExtractSimpleDart
  >('sz_extract_simple');
  
  final archive = 'archive.7z'.toNativeUtf8();
  final output = 'output/'.toNativeUtf8();
  
  final result = extract(archive, output);
  if (result != 0) {
    print('解压失败');
  }
  
  malloc.free(archive);
  malloc.free(output);
}
```

📚 **完整 API 文档**：参见 [`Document/API_Reference.zh-CN.md`](Document/API_Reference.zh-CN.md)

---

## 🛠️ 从源码构建

### 前置要求

- **Windows 10/11**（Windows SDK 10.0.19041.0,其他版本未经验证，已知win11 sdk不支持）
- **Visual Studio 2022**（或带有 MSVC 14.4+ 的 Build Tools）
- **CMake 3.20+**

### 标准 CMake 构建

```bash
# x64 构建（64 位）
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target sevenzip_ffi

# x86 构建（32 位）
cmake -B build_x86 -G "Visual Studio 17 2022" -A Win32
cmake --build build_x86 --config Release --target sevenzip_ffi

# 输出：build/bin/Release/sevenzip_ffi.dll
```

### 同时构建两种架构

```bash
# 配置
cmake -B build_x64 -G "Visual Studio 17 2022" -A x64
cmake -B build_x86 -G "Visual Studio 17 2022" -A Win32

# 构建
cmake --build build_x64 --config Release --target sevenzip_ffi --parallel
cmake --build build_x86 --config Release --target sevenzip_ffi --parallel

# 收集产物
mkdir dist\\x64 dist\\x86
copy build_x64\\bin\\Release\\sevenzip_ffi.dll dist\\x64\\
copy build_x86\\bin\\Release\\sevenzip_ffi.dll dist\\x86\\
copy include\\sevenzip\\sevenzip_capi.h dist\\include\\sevenzip\\
```

### 可选：构建 C++ API

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
  -DSEVENZIP_BUILD_CPP_API=ON ^
  -DSEVENZIP_BUILD_EXAMPLES=ON

cmake --build build --config Release
```

⚠️ **请记住**：C++ API 仅限 Windows 且未经完整测试。

---

## 📋 功能特性

### 当前 FFI API 状态

✅ **已实现：**
- 版本信息（`sz_get_version`）
- 错误处理（`sz_get_error_message`、`sz_get_last_error`）
- 档案读取（打开、列出文件、解压）
- 档案写入（创建 7z/zip/tar 档案）
- 基本压缩/解压

📝 **计划功能：**
- 高级压缩选项
- 密码保护档案（完整支持）
- 固实档案配置
- 分卷档案
- 档案更新（添加/删除文件）

详细功能路线图参见本地开发文档 `docs/FFI_TODO.md`（未公开）。

### 支持的格式

| 格式  | 读取 | 写入 | 备注 |
|-------|------|------|------|
| 7z    | ✅   | ✅   | 完整支持 |
| Zip   | ✅   | ✅   | 完整支持 |
| Tar   | ✅   | ✅   | 完整支持 |
| Gzip  | ✅   | ✅   | `.gz` 文件 |
| Bzip2 | ✅   | ✅   | `.bz2` 文件 |
| Xz    | ✅   | ✅   | 完整支持 |
| Rar   | ✅   | ❌   | 只读（专有格式）|
| Rar5  | ✅   | ❌   | 只读（专有格式）|

---

## 📚 文档

- **[API 参考 (中文)](Document/API_Reference.zh-CN.md)** - 完整的 C API 文档
- **[API Reference (English)](Document/API_Reference.md)** - Complete C API documentation
- **[贡献指南](CONTRIBUTING.zh-CN.md)** - 如何贡献

---

## 🤝 贡献

欢迎贡献！请参阅 [CONTRIBUTING.zh-CN.md](CONTRIBUTING.zh-CN.md) 了解指南。

### 需要帮助的领域

- 🐍 **Python 包**
- 🦀 **Rust crate**
- 🎯 **Dart 包**
- 💎 **C# 包**
- 📖 文档改进
- 🧪 FFI API 测试

---

### 第三方许可证

- **7-Zip SDK**：公有领域 + GNU LGPL（unRAR 代码）
  - 许可证：https://www.7-zip.org/license.txt
  - 版本：25.01

---

## 🔗 相关项目

- **[7-Zip](https://www.7-zip.org/)** - 原版 7-Zip 压缩工具
- **[bit7z](https://github.com/rikyoz/bit7z)** - 推荐的 C++ 7-Zip 库

---

## 🏗️ 项目状态

- **版本**：0.1.0（Alpha）
- **状态**：活跃开发中
- **平台**：仅限 Windows（由于 Windows SDK COM 依赖）
- **目标用户**：FFI 用户（Python、Rust、Dart、C#）

**注意**：这是一个早期项目。在 v1.0.0 发布之前，API 可能会发生变化。
