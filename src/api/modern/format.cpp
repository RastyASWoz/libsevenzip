#include "sevenzip/format.hpp"

#include <unordered_map>

namespace sevenzip {

namespace {

// Format information table
const std::unordered_map<Format, FormatInfo> kFormatInfoTable = {
    {Format::SevenZip, {Format::SevenZip, "7z", ".7z", true, true, true, true, true}},
    {Format::Zip, {Format::Zip, "ZIP", ".zip", true, true, true, false, true}},
    {Format::Rar, {Format::Rar, "RAR", ".rar", true, false, true, true, true}},
    {Format::Tar, {Format::Tar, "TAR", ".tar", true, true, false, false, false}},
    {Format::GZip, {Format::GZip, "GZip", ".gz", true, true, false, false, false}},
    {Format::BZip2, {Format::BZip2, "BZip2", ".bz2", true, true, false, false, false}},
    {Format::Xz, {Format::Xz, "XZ", ".xz", true, true, false, false, false}},
    {Format::Lzma, {Format::Lzma, "LZMA", ".lzma", true, true, false, false, false}},
    {Format::Cab, {Format::Cab, "CAB", ".cab", true, false, false, false, true}},
    {Format::Iso, {Format::Iso, "ISO", ".iso", true, false, false, false, false}},
    {Format::Wim, {Format::Wim, "WIM", ".wim", true, false, false, false, false}},
};

// Extension to format mapping
const std::unordered_map<std::string_view, Format> kExtensionMap = {
    {".7z", Format::SevenZip}, {".zip", Format::Zip},     {".rar", Format::Rar},
    {".tar", Format::Tar},     {".gz", Format::GZip},     {".gzip", Format::GZip},
    {".bz2", Format::BZip2},   {".bzip2", Format::BZip2}, {".xz", Format::Xz},
    {".lzma", Format::Lzma},   {".cab", Format::Cab},     {".iso", Format::Iso},
    {".wim", Format::Wim},
};

}  // anonymous namespace

std::string_view format_name(Format format) {
    auto it = kFormatInfoTable.find(format);
    if (it != kFormatInfoTable.end()) {
        return it->second.name;
    }
    return "Unknown";
}

const FormatInfo& format_info(Format format) {
    static const FormatInfo kUnknownFormat = {Format::Auto, "Unknown", "",    false,
                                              false,        false,     false, false};

    auto it = kFormatInfoTable.find(format);
    if (it != kFormatInfoTable.end()) {
        return it->second;
    }
    return kUnknownFormat;
}

Format guess_format(const Path& path) {
    std::string ext = path.extension().string();

    // Convert to lowercase
    for (char& c : ext) {
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }

    return guess_format_from_extension(ext);
}

Format guess_format_from_extension(std::string_view extension) {
    // Remove leading dot if present
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }

    // Add dot back for lookup
    std::string ext_with_dot = ".";
    ext_with_dot += extension;

    auto it = kExtensionMap.find(ext_with_dot);
    if (it != kExtensionMap.end()) {
        return it->second;
    }

    return Format::Auto;
}

bool supports_read(Format format) {
    return format_info(format).supports_read;
}

bool supports_write(Format format) {
    return format_info(format).supports_write;
}

bool supports_encryption(Format format) {
    return format_info(format).supports_encryption;
}

bool supports_solid(Format format) {
    return format_info(format).supports_solid;
}

}  // namespace sevenzip
