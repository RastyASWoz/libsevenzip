#pragma once

/// @file compression.hpp
/// @brief Compression method and level definitions

#include <cstdint>
#include <string_view>

namespace sevenzip {

/// Compression method enumeration
enum class Compression {
    Auto,       ///< Auto-select best method for format
    Copy,       ///< No compression (store)
    LZMA,       ///< LZMA algorithm
    LZMA2,      ///< LZMA2 algorithm (recommended)
    PPMd,       ///< PPMd algorithm
    BZip2,      ///< BZip2 algorithm
    Deflate,    ///< Deflate algorithm (ZIP)
    Deflate64,  ///< Deflate64 algorithm
    // Add more methods as needed
};

/// Compression level enumeration
enum class CompressionLevel : uint32_t {
    None = 0,     ///< No compression (fastest)
    Fastest = 1,  ///< Fastest compression
    Fast = 3,     ///< Fast compression
    Normal = 5,   ///< Normal compression (balanced)
    Maximum = 7,  ///< Maximum compression
    Ultra = 9,    ///< Ultra compression (slowest, best ratio)
};

/// Get compression method name
/// @param method Compression method
/// @return Human-readable method name
std::string_view compression_name(Compression method);

/// Get compression level name
/// @param level Compression level
/// @return Human-readable level name
std::string_view compression_level_name(CompressionLevel level);

// Note: is_method_available() and recommended_method() will be implemented later
// to avoid circular dependency between compression.hpp and format.hpp

/// Compression options structure
struct CompressionOptions {
    Compression method = Compression::Auto;             ///< Compression method
    CompressionLevel level = CompressionLevel::Normal;  ///< Compression level
    uint32_t dictionary_size = 0;                       ///< Dictionary size (0 = auto)
    uint32_t word_size = 0;                             ///< Word size (0 = auto)
    uint32_t num_threads = 0;                           ///< Number of threads (0 = auto)
    bool solid = false;                                 ///< Use solid mode

    /// Create default options
    static CompressionOptions defaults() { return CompressionOptions{}; }

    /// Create fast compression options
    static CompressionOptions fast() {
        CompressionOptions opts;
        opts.level = CompressionLevel::Fast;
        return opts;
    }

    /// Create ultra compression options
    static CompressionOptions ultra() {
        CompressionOptions opts;
        opts.level = CompressionLevel::Ultra;
        return opts;
    }
};

}  // namespace sevenzip
