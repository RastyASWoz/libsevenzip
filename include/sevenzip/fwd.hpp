#pragma once

/// @file fwd.hpp
/// @brief Forward declarations for the sevenzip API

#include <functional>
#include <memory>

namespace sevenzip {

// Main API classes
class ArchiveReader;
class ArchiveWriter;
class Compressor;

// Helper classes
class ArchiveEntry;
class ArchiveInfo;
struct CompressionOptions;
struct ExtractionOptions;

// Stream classes (future)
class InputStream;
class OutputStream;
class MemoryInputStream;
class MemoryOutputStream;
class FileInputStream;
class FileOutputStream;

// Exception classes
class Exception;
class IoException;
class FormatException;
class PasswordException;
class DataException;
class NotSupportedException;

// Enumerations
enum class Format;
enum class Compression;
enum class CompressionLevel;
enum class ErrorCode;

// Callback types
/// Callback for progress reporting
/// @param completed Number of bytes processed
/// @param total Total number of bytes to process
/// @return true to continue, false to cancel
using ProgressCallback = std::function<bool(uint64_t completed, uint64_t total)>;

/// Callback for password retrieval
/// @return Password as a wide string
using PasswordCallback = std::function<std::wstring()>;

// Implementation detail namespace
namespace detail {
// Forward declare implementation classes
class ArchiveReaderImpl;
class ArchiveWriterImpl;
}  // namespace detail

}  // namespace sevenzip
