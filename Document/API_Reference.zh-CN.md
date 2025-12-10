# libsevenzip C API 参考文档

**版本**: 0.1.0 (Alpha)  
**最后更新**: 2025-12-10  
**语言**: [English](API_Reference.md) | 中文

本文档提供 libsevenzip C FFI（外部函数接口）API 的完整参考。

---

## 目录

- [概述](#概述)
- [核心类型](#核心类型)
- [错误处理](#错误处理)
- [版本信息](#版本信息)
- [归档操作](#归档操作)
- [归档写入](#归档写入)
- [便捷函数](#便捷函数)
- [内存管理](#内存管理)
- [代码示例](#代码示例)

---

## 概述

libsevenzip FFI 提供与 C 兼容的 API，用于处理 7-Zip 归档文件。所有函数遵循以下约定：

- **前缀**: 所有函数以 `sz_` 开头
- **返回类型**: 大多数函数返回 `sz_result`（错误码）
- **命名规范**: 下划线命名法（例如 `sz_archive_open`）
- **线程安全**: 归档句柄不是线程安全的；每个线程需使用独立句柄

### 头文件

```c
#include <sevenzip/sevenzip_capi.h>
```

此头文件包含所有必要的头文件：
- `sz_types.h` - 类型定义
- `sz_error.h` - 错误码和处理
- `sz_archive.h` - 归档读取
- `sz_writer.h` - 归档写入
- `sz_convenience.h` - 便捷函数
- `sz_version.h` - 版本信息

---

## 核心类型

### sz_result

```c
typedef int32_t sz_result;
```

所有 API 函数的返回类型。错误码详见[错误处理](#错误处理)。

### sz_archive

```c
typedef struct sz_archive sz_archive;
```

不透明的归档句柄。由 `sz_archive_open()` 创建，由 `sz_archive_close()` 释放。

### sz_writer

```c
typedef struct sz_writer sz_writer;
```

不透明的归档写入器句柄。由 `sz_writer_create()` 创建，由 `sz_writer_free()` 释放。

### sz_format

```c
typedef enum {
    SZ_FORMAT_UNKNOWN = 0,
    SZ_FORMAT_7Z      = 1,    // 7-Zip 格式
    SZ_FORMAT_ZIP     = 2,    // Zip 格式
    SZ_FORMAT_TAR     = 3,    // Tar 格式
    SZ_FORMAT_GZIP    = 4,    // Gzip (.gz)
    SZ_FORMAT_BZIP2   = 5,    // Bzip2 (.bz2)
    SZ_FORMAT_XZ      = 6,    // Xz 格式
    SZ_FORMAT_RAR     = 7,    // RAR (只读)
    SZ_FORMAT_RAR5    = 8     // RAR5 (只读)
} sz_format;
```

### sz_item_info

```c
typedef struct {
    char name[512];              // 项目名称（UTF-8）
    uint64_t size;               // 未压缩大小（字节）
    uint64_t compressed_size;    // 压缩后大小（字节）
    uint64_t crc;                // CRC32 校验和
    int is_directory;            // 1 表示目录，0 表示文件
    uint64_t modified_time;      // Unix 时间戳
    uint32_t attributes;         // 文件属性
} sz_item_info;
```

---

## 错误处理

### 错误码

```c
#define SZ_OK                        0    // 成功
#define SZ_ERROR_FAIL                1    // 一般性失败
#define SZ_ERROR_INVALID_PARAM       2    // 无效参数
#define SZ_ERROR_OUT_OF_MEMORY       3    // 内存分配失败
#define SZ_ERROR_OPEN_ARCHIVE        4    // 无法打开归档
#define SZ_ERROR_READ_ARCHIVE        5    // 无法读取归档
#define SZ_ERROR_WRITE_FILE          6    // 无法写入文件
#define SZ_ERROR_CREATE_DIRECTORY    7    // 无法创建目录
#define SZ_ERROR_UNSUPPORTED_FORMAT  8    // 不支持的格式
#define SZ_ERROR_INVALID_ARCHIVE     9    // 无效的归档
#define SZ_ERROR_ITEM_NOT_FOUND     10    // 项目未找到
#define SZ_ERROR_EXTRACTION_FAILED  11    // 解压失败
#define SZ_ERROR_COMPRESSION_FAILED 12    // 压缩失败
#define SZ_ERROR_PASSWORD_REQUIRED  13    // 需要密码
#define SZ_ERROR_WRONG_PASSWORD     14    // 密码错误
#define SZ_ERROR_NOT_IMPLEMENTED    15    // 功能未实现
```

### sz_get_error_message

```c
SZ_API const char* sz_get_error_message(sz_result error_code);
```

**描述**: 获取错误码的可读错误消息。

**参数**:
- `error_code`: API 函数返回的错误码

**返回值**: 指向静态字符串的指针，描述错误信息。不要释放。

**示例**:
```c
sz_result result = sz_archive_open("test.7z", &archive);
if (result != SZ_OK) {
    printf("错误: %s\n", sz_get_error_message(result));
}
```

### sz_get_last_error

```c
SZ_API sz_result sz_get_last_error(char* buffer, size_t buffer_size);
```

**描述**: 获取最后一次错误消息及额外上下文。

**参数**:
- `buffer`: 输出缓冲区，用于存储错误消息
- `buffer_size`: 缓冲区大小（字节）

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
char error_msg[256];
sz_get_last_error(error_msg, sizeof(error_msg));
printf("最后错误: %s\n", error_msg);
```

---

## 版本信息

### sz_get_version

```c
SZ_API sz_result sz_get_version(char* buffer, size_t buffer_size);
```

**描述**: 获取 7-Zip SDK 版本字符串。

**参数**:
- `buffer`: 输出缓冲区，用于存储版本字符串
- `buffer_size`: 缓冲区大小（字节）

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
char version[64];
sz_get_version(version, sizeof(version));
printf("7-Zip SDK 版本: %s\n", version);  // "25.01"
```

---

## 归档操作

### sz_archive_open

```c
SZ_API sz_result sz_archive_open(const char* path, sz_archive** archive);
```

**描述**: 打开归档文件以供读取。

**参数**:
- `path`: 归档文件路径（UTF-8）
- `archive`: 输出归档句柄指针

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_OPEN_ARCHIVE` 文件无法打开
- `SZ_ERROR_INVALID_ARCHIVE` 文件不是有效归档

**示例**:
```c
sz_archive* archive = NULL;
sz_result result = sz_archive_open("test.7z", &archive);
if (result == SZ_OK) {
    // 使用归档
    sz_archive_close(archive);
}
```

**支持的格式**: 7z、Zip、Tar、Gzip、Bzip2、Xz、Rar、Rar5

### sz_archive_close

```c
SZ_API void sz_archive_close(sz_archive* archive);
```

**描述**: 关闭归档并释放资源。

**参数**:
- `archive`: 归档句柄（可以为 NULL）

**示例**:
```c
sz_archive_close(archive);
archive = NULL;  // 良好实践
```

### sz_archive_get_item_count

```c
SZ_API sz_result sz_archive_get_item_count(sz_archive* archive, uint32_t* count);
```

**描述**: 获取归档中的项目数量。

**参数**:
- `archive`: 归档句柄
- `count`: 输出项目数量的指针

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
uint32_t count;
if (sz_archive_get_item_count(archive, &count) == SZ_OK) {
    printf("归档包含 %u 个项目\n", count);
}
```

### sz_archive_get_format

```c
SZ_API sz_result sz_archive_get_format(sz_archive* archive, char* buffer, size_t buffer_size);
```

**描述**: 获取归档格式的字符串表示。

**参数**:
- `archive`: 归档句柄
- `buffer`: 输出缓冲区，用于存储格式名称
- `buffer_size`: 缓冲区大小

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
char format[32];
sz_archive_get_format(archive, format, sizeof(format));
printf("格式: %s\n", format);  // "7z"、"zip"、"tar" 等
```

### sz_archive_get_item_info

```c
SZ_API sz_result sz_archive_get_item_info(
    sz_archive* archive, 
    uint32_t index, 
    sz_item_info* info
);
```

**描述**: 获取归档中某个项目的信息。

**参数**:
- `archive`: 归档句柄
- `index`: 从零开始的项目索引
- `info`: 输出项目信息的结构体

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_INVALID_PARAM` 索引超出范围

**示例**:
```c
sz_item_info info;
if (sz_archive_get_item_info(archive, 0, &info) == SZ_OK) {
    printf("名称: %s\n", info.name);
    printf("大小: %llu 字节\n", info.size);
    printf("压缩后: %llu 字节\n", info.compressed_size);
    printf("目录: %s\n", info.is_directory ? "是" : "否");
}
```

### sz_archive_extract

```c
SZ_API sz_result sz_archive_extract(
    sz_archive* archive, 
    const char* output_dir
);
```

**描述**: 从归档中解压所有项目。

**参数**:
- `archive`: 归档句柄
- `output_dir`: 输出目录路径（若不存在会创建）

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_CREATE_DIRECTORY` 无法创建输出目录
- `SZ_ERROR_EXTRACTION_FAILED` 解压失败

**示例**:
```c
sz_result result = sz_archive_extract(archive, "output/");
if (result == SZ_OK) {
    printf("解压成功\n");
}
```

### sz_archive_extract_item

```c
SZ_API sz_result sz_archive_extract_item(
    sz_archive* archive, 
    uint32_t index, 
    const char* output_path
);
```

**描述**: 从归档中解压单个项目。

**参数**:
- `archive`: 归档句柄
- `index`: 从零开始的项目索引
- `output_path`: 解压文件的完整路径

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_ITEM_NOT_FOUND` 索引无效
- `SZ_ERROR_EXTRACTION_FAILED` 解压失败

**示例**:
```c
sz_result result = sz_archive_extract_item(archive, 0, "output/file.txt");
if (result == SZ_OK) {
    printf("文件已解压\n");
}
```

---

## 归档写入

### sz_writer_create

```c
SZ_API sz_result sz_writer_create(sz_format format, sz_writer** writer);
```

**描述**: 创建新的归档写入器。

**参数**:
- `format`: 归档格式（7z、zip 或 tar）
- `writer`: 输出写入器句柄指针

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_UNSUPPORTED_FORMAT` 格式不支持写入

**示例**:
```c
sz_writer* writer = NULL;
sz_result result = sz_writer_create(SZ_FORMAT_7Z, &writer);
if (result == SZ_OK) {
    // 使用写入器
    sz_writer_free(writer);
}
```

**支持的格式**: `SZ_FORMAT_7Z`、`SZ_FORMAT_ZIP`、`SZ_FORMAT_TAR`

### sz_writer_free

```c
SZ_API void sz_writer_free(sz_writer* writer);
```

**描述**: 释放写入器句柄。

**参数**:
- `writer`: 写入器句柄（可以为 NULL）

**示例**:
```c
sz_writer_free(writer);
writer = NULL;
```

### sz_writer_add_file

```c
SZ_API sz_result sz_writer_add_file(
    sz_writer* writer, 
    const char* file_path,
    const char* archive_path
);
```

**描述**: 向归档添加文件。

**参数**:
- `writer`: 写入器句柄
- `file_path`: 磁盘上的文件路径
- `archive_path`: 归档内的路径（NULL = 仅使用文件名）

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_INVALID_PARAM` 文件不存在

**示例**:
```c
// 使用自定义归档路径添加文件
sz_writer_add_file(writer, "data/file.txt", "files/file.txt");

// 使用自动命名添加文件
sz_writer_add_file(writer, "data/file.txt", NULL);  // 存储为 "file.txt"
```

### sz_writer_add_directory

```c
SZ_API sz_result sz_writer_add_directory(
    sz_writer* writer, 
    const char* dir_path,
    const char* archive_path
);
```

**描述**: 递归添加目录及其内容。

**参数**:
- `writer`: 写入器句柄
- `dir_path`: 磁盘上的目录路径
- `archive_path`: 归档内的路径（NULL = 使用目录名）

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
// 添加整个目录
sz_writer_add_directory(writer, "my_folder", "backup/my_folder");

// 使用自动命名添加目录
sz_writer_add_directory(writer, "my_folder", NULL);
```

### sz_writer_create_archive

```c
SZ_API sz_result sz_writer_create_archive(
    sz_writer* writer, 
    const char* archive_path
);
```

**描述**: 创建包含所有已添加项目的归档文件。

**参数**:
- `writer`: 写入器句柄
- `archive_path`: 输出归档的路径

**返回值**: 
- `SZ_OK` 成功
- `SZ_ERROR_COMPRESSION_FAILED` 归档创建失败

**示例**:
```c
sz_writer* writer;
sz_writer_create(SZ_FORMAT_7Z, &writer);
sz_writer_add_file(writer, "file1.txt", NULL);
sz_writer_add_file(writer, "file2.txt", NULL);
sz_writer_create_archive(writer, "output.7z");
sz_writer_free(writer);
```

---

## 便捷函数

### sz_extract_simple

```c
SZ_API sz_result sz_extract_simple(const char* archive_path, const char* output_dir);
```

**描述**: 一次调用解压归档。

**参数**:
- `archive_path`: 归档路径
- `output_dir`: 输出目录

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
if (sz_extract_simple("archive.7z", "output/") == SZ_OK) {
    printf("解压成功\n");
}
```

### sz_compress_directory

```c
SZ_API sz_result sz_compress_directory(
    const char* dir_path,
    const char* archive_path,
    sz_format format
);
```

**描述**: 一次调用将目录压缩为归档。

**参数**:
- `dir_path`: 要压缩的目录
- `archive_path`: 输出归档路径
- `format`: 归档格式

**返回值**: 成功返回 `SZ_OK`

**示例**:
```c
sz_result result = sz_compress_directory("my_folder", "backup.7z", SZ_FORMAT_7Z);
if (result == SZ_OK) {
    printf("压缩成功\n");
}
```

### sz_detect_format

```c
SZ_API sz_result sz_detect_format(const char* path, sz_format* format);
```

**描述**: 检测归档文件的格式。

**参数**:
- `path`: 文件路径
- `format`: 输出格式

**返回值**: 检测到格式时返回 `SZ_OK`

**示例**:
```c
sz_format format;
if (sz_detect_format("unknown.bin", &format) == SZ_OK) {
    printf("格式: %d\n", format);
}
```

### sz_format_to_string

```c
SZ_API const char* sz_format_to_string(sz_format format);
```

**描述**: 将格式枚举转换为字符串。

**参数**:
- `format`: 格式枚举值

**返回值**: 字符串表示（"7z"、"zip" 等）

**示例**:
```c
printf("格式: %s\n", sz_format_to_string(SZ_FORMAT_7Z));  // "7z"
```

---

## 内存管理

### sz_free_buffer

```c
SZ_API void sz_free_buffer(void* buffer);
```

**描述**: 释放 FFI 函数分配的内存。

**参数**:
- `buffer`: 要释放的缓冲区（可以为 NULL）

**示例**:
```c
// 某些 FFI 函数可能返回已分配的缓冲区
uint8_t* data = /* 来自 FFI 函数 */;
sz_free_buffer(data);
```

---

## 代码示例

### 解压归档

```c
#include <sevenzip/sevenzip_capi.h>
#include <stdio.h>

int main() {
    sz_archive* archive = NULL;
    sz_result result;
    
    // 打开归档
    result = sz_archive_open("test.7z", &archive);
    if (result != SZ_OK) {
        printf("打开归档错误: %s\n", sz_get_error_message(result));
        return 1;
    }
    
    // 获取项目数量
    uint32_t count;
    sz_archive_get_item_count(archive, &count);
    printf("归档包含 %u 个项目\n", count);
    
    // 列出所有项目
    for (uint32_t i = 0; i < count; i++) {
        sz_item_info info;
        sz_archive_get_item_info(archive, i, &info);
        printf("%s (%llu 字节)\n", info.name, info.size);
    }
    
    // 解压所有
    result = sz_archive_extract(archive, "output/");
    if (result == SZ_OK) {
        printf("解压成功\n");
    }
    
    // 清理
    sz_archive_close(archive);
    return 0;
}
```

### 创建归档

```c
#include <sevenzip/sevenzip_capi.h>
#include <stdio.h>

int main() {
    sz_writer* writer = NULL;
    sz_result result;
    
    // 创建写入器
    result = sz_writer_create(SZ_FORMAT_7Z, &writer);
    if (result != SZ_OK) {
        printf("创建写入器错误\n");
        return 1;
    }
    
    // 添加文件
    sz_writer_add_file(writer, "file1.txt", NULL);
    sz_writer_add_file(writer, "file2.txt", NULL);
    sz_writer_add_directory(writer, "my_folder", NULL);
    
    // 创建归档
    result = sz_writer_create_archive(writer, "output.7z");
    if (result == SZ_OK) {
        printf("归档创建成功\n");
    } else {
        printf("错误: %s\n", sz_get_error_message(result));
    }
    
    // 清理
    sz_writer_free(writer);
    return 0;
}
```

---

## 平台说明

### 仅限 Windows

此库目前仅支持 Windows，原因如下：
- 依赖 Windows SDK COM 接口
- 使用 MSVC 特定实现

### 架构支持

- **x64 (64 位)**: 完全支持，推荐使用
- **x86 (32 位)**: 支持，但有限制：
  - 大于 4GB 的文件可能有问题
  - 不同的调用约定（内部 COM 使用 __stdcall）

### 线程安全

- 归档和写入器句柄不是线程安全的
- 每个线程使用独立句柄
- 错误消息存储在线程本地存储中

---

## 版本历史

| 版本 | 日期 | 变更 |
|------|------|------|
| 0.1.0 | 2025-12-10 | 初始 alpha 版本 |

---

## 另见

- [English API Documentation](API_Reference.md)
- [贡献指南](../CONTRIBUTING.zh-CN.md)
- [7-Zip SDK 文档](https://www.7-zip.org/sdk.html)

---

**注意**: 这是 alpha 版本。在 1.0 版本发布前，API 可能会发生变化。
