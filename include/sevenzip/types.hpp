#pragma once

/// @file types.hpp
/// @brief Basic type definitions for the sevenzip API

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace sevenzip {

// Path types
/// Filesystem path type
using Path = std::filesystem::path;

// Time types
/// Time point type for file timestamps
using TimePoint = std::chrono::system_clock::time_point;

// Byte data types
/// Vector of bytes
using ByteVector = std::vector<uint8_t>;

/// Span of constant bytes
using ByteSpan = std::span<const uint8_t>;

// Size types
/// Size type for memory operations
using Size = std::size_t;

/// File size type (64-bit)
using FileSize = uint64_t;

// Index types
/// Index type for archive entries
using Index = uint32_t;

// Constants
namespace constants {
/// Maximum path length supported
constexpr Size kMaxPathLength = 32768;

/// Default buffer size for stream operations
constexpr Size kDefaultBufferSize = 65536;

/// Invalid index value
constexpr Index kInvalidIndex = static_cast<Index>(-1);
}  // namespace constants

}  // namespace sevenzip
