#pragma once

// 公共头文件 - 仅包含标准库和前向声明
// 7-Zip 内部头文件只在 .cpp 文件中包含

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace sevenzip {

// 前向声明
class InputStream;
class OutputStream;
class ArchiveReader;
class ArchiveWriter;
class Encoder;
class Decoder;

namespace detail {

// 类型别名
using Byte = uint8_t;
using UInt32 = uint32_t;
using UInt64 = uint64_t;
using Int64 = int64_t;

// 内部类前向声明
class COMInStreamAdapter;
class COMOutStreamAdapter;
class COMArchiveReader;
class COMArchiveWriter;

}  // namespace detail
}  // namespace sevenzip
