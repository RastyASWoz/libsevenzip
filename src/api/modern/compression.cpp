#include "sevenzip/compression.hpp"

namespace sevenzip {

std::string_view compression_name(Compression method) {
    switch (method) {
        case Compression::Auto:
            return "Auto";
        case Compression::Copy:
            return "Copy";
        case Compression::LZMA:
            return "LZMA";
        case Compression::LZMA2:
            return "LZMA2";
        case Compression::PPMd:
            return "PPMd";
        case Compression::BZip2:
            return "BZip2";
        case Compression::Deflate:
            return "Deflate";
        case Compression::Deflate64:
            return "Deflate64";
        default:
            return "Unknown";
    }
}

std::string_view compression_level_name(CompressionLevel level) {
    switch (level) {
        case CompressionLevel::None:
            return "None";
        case CompressionLevel::Fastest:
            return "Fastest";
        case CompressionLevel::Fast:
            return "Fast";
        case CompressionLevel::Normal:
            return "Normal";
        case CompressionLevel::Maximum:
            return "Maximum";
        case CompressionLevel::Ultra:
            return "Ultra";
        default:
            return "Unknown";
    }
}

}  // namespace sevenzip
