#pragma once

/// @file format.hpp
/// @brief Archive format definitions and utilities

#include <string_view>

#include "types.hpp"

namespace sevenzip {

/// Archive format enumeration
enum class Format {
    Auto,      ///< Auto-detect format
    SevenZip,  ///< 7z format
    Zip,       ///< ZIP format
    Rar,       ///< RAR format (read-only)
    Tar,       ///< TAR format
    GZip,      ///< GZip compression
    BZip2,     ///< BZip2 compression
    Xz,        ///< XZ compression
    Lzma,      ///< LZMA compression
    Cab,       ///< CAB format
    Iso,       ///< ISO image
    Wim,       ///< WIM format
    // Add more formats as needed
};

/// Format information structure
struct FormatInfo {
    Format format;                ///< Format identifier
    std::string_view name;        ///< Human-readable name
    std::string_view extensions;  ///< Semicolon-separated extensions (e.g., ".7z")
    bool supports_read;           ///< Can read this format
    bool supports_write;          ///< Can write this format
    bool supports_encryption;     ///< Supports password encryption
    bool supports_solid;          ///< Supports solid archives
    bool supports_multi_volume;   ///< Supports multi-volume archives
};

/// Get format name as string
/// @param format Format to get name for
/// @return Human-readable format name
std::string_view format_name(Format format);

/// Get detailed format information
/// @param format Format to get info for
/// @return Format information structure
const FormatInfo& format_info(Format format);

/// Guess format from file path
/// @param path File path
/// @return Guessed format, or Format::Auto if unknown
Format guess_format(const Path& path);

/// Guess format from file extension
/// @param extension File extension (with or without leading dot)
/// @return Guessed format, or Format::Auto if unknown
Format guess_format_from_extension(std::string_view extension);

/// Check if format supports reading
/// @param format Format to check
/// @return true if format can be read
bool supports_read(Format format);

/// Check if format supports writing
/// @param format Format to check
/// @return true if format can be written
bool supports_write(Format format);

/// Check if format supports encryption
/// @param format Format to check
/// @return true if format supports password encryption
bool supports_encryption(Format format);

/// Check if format supports solid archives
/// @param format Format to check
/// @return true if format supports solid mode
bool supports_solid(Format format);

}  // namespace sevenzip
