#include "archive_format.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>

#include "../common/wrapper_error.hpp"

#ifdef _WIN32
#include <guiddef.h>
#include <initguid.h>
#endif

namespace sevenzip {
namespace detail {

// 文件签名定义
static constexpr uint8_t SIG_7Z[] = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C};
static constexpr uint8_t SIG_ZIP[] = {0x50, 0x4B, 0x03, 0x04};
static constexpr uint8_t SIG_ZIP_EMPTY[] = {0x50, 0x4B, 0x05, 0x06};
static constexpr uint8_t SIG_ZIP_SPANNED[] = {0x50, 0x4B, 0x07, 0x08};
static constexpr uint8_t SIG_RAR[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};
static constexpr uint8_t SIG_RAR5[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00};
static constexpr uint8_t SIG_GZIP[] = {0x1F, 0x8B};
static constexpr uint8_t SIG_BZIP2[] = {0x42, 0x5A, 0x68};
static constexpr uint8_t SIG_XZ[] = {0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00};
static constexpr uint8_t SIG_LZMA[] = {0x5D, 0x00, 0x00};
static constexpr uint8_t SIG_CAB[] = {0x4D, 0x53, 0x43, 0x46};
static constexpr uint8_t SIG_ISO[] = {0x43, 0x44, 0x30, 0x30, 0x31};  // offset 0x8001
static constexpr uint8_t SIG_WIM[] = {0x4D, 0x53, 0x57, 0x49, 0x4D, 0x00, 0x00, 0x00};
static constexpr uint8_t SIG_RPM[] = {0xED, 0xAB, 0xEE, 0xDB};
static constexpr uint8_t SIG_CPIO[] = {0x30, 0x37, 0x30, 0x37, 0x30, 0x37};
static constexpr uint8_t SIG_DEB[] = {0x21, 0x3C, 0x61, 0x72, 0x63, 0x68, 0x3E};
static constexpr uint8_t SIG_ARJ[] = {0x60, 0xEA};
static constexpr uint8_t SIG_Z[] = {0x1F, 0x9D};
static constexpr uint8_t SIG_LZH[] = {0x2D, 0x6C, 0x68};  // offset 2

const std::vector<FormatDetector::SignatureInfo>& FormatDetector::get_signatures() {
    static const std::vector<SignatureInfo> signatures = {
        {SIG_7Z, sizeof(SIG_7Z), 0, ArchiveFormat::SevenZip},
        {SIG_RAR5, sizeof(SIG_RAR5), 0, ArchiveFormat::Rar5},
        {SIG_RAR, sizeof(SIG_RAR), 0, ArchiveFormat::Rar},
        {SIG_ZIP, sizeof(SIG_ZIP), 0, ArchiveFormat::Zip},
        {SIG_ZIP_EMPTY, sizeof(SIG_ZIP_EMPTY), 0, ArchiveFormat::Zip},
        {SIG_ZIP_SPANNED, sizeof(SIG_ZIP_SPANNED), 0, ArchiveFormat::Zip},
        {SIG_GZIP, sizeof(SIG_GZIP), 0, ArchiveFormat::GZip},
        {SIG_BZIP2, sizeof(SIG_BZIP2), 0, ArchiveFormat::BZip2},
        {SIG_XZ, sizeof(SIG_XZ), 0, ArchiveFormat::Xz},
        {SIG_LZMA, sizeof(SIG_LZMA), 0, ArchiveFormat::Lzma},
        {SIG_CAB, sizeof(SIG_CAB), 0, ArchiveFormat::Cab},
        {SIG_WIM, sizeof(SIG_WIM), 0, ArchiveFormat::Wim},
        {SIG_RPM, sizeof(SIG_RPM), 0, ArchiveFormat::Rpm},
        {SIG_CPIO, sizeof(SIG_CPIO), 0, ArchiveFormat::Cpio},
        {SIG_DEB, sizeof(SIG_DEB), 0, ArchiveFormat::Deb},
        {SIG_ARJ, sizeof(SIG_ARJ), 0, ArchiveFormat::Arj},
        {SIG_Z, sizeof(SIG_Z), 0, ArchiveFormat::Z},
        {SIG_LZH, sizeof(SIG_LZH), 2, ArchiveFormat::Lzh},
        {SIG_ISO, sizeof(SIG_ISO), 0x8001, ArchiveFormat::Iso},
    };
    return signatures;
}

ArchiveFormat FormatDetector::from_extension(const std::filesystem::path& path) {
    static const std::map<std::string, ArchiveFormat> ext_map = {
        {".7z", ArchiveFormat::SevenZip}, {".zip", ArchiveFormat::Zip},
        {".gz", ArchiveFormat::GZip},     {".gzip", ArchiveFormat::GZip},
        {".tgz", ArchiveFormat::Tar},  // tar.gz
        {".bz2", ArchiveFormat::BZip2},   {".bzip2", ArchiveFormat::BZip2},
        {".tbz", ArchiveFormat::Tar},  // tar.bz2
        {".tbz2", ArchiveFormat::Tar},    {".tar", ArchiveFormat::Tar},
        {".xz", ArchiveFormat::Xz},       {".txz", ArchiveFormat::Tar},  // tar.xz
        {".lzma", ArchiveFormat::Lzma},   {".rar", ArchiveFormat::Rar},
        {".iso", ArchiveFormat::Iso},     {".img", ArchiveFormat::Iso},
        {".wim", ArchiveFormat::Wim},     {".swm", ArchiveFormat::Wim},
        {".esd", ArchiveFormat::Wim},     {".cab", ArchiveFormat::Cab},
        {".arj", ArchiveFormat::Arj},     {".cpio", ArchiveFormat::Cpio},
        {".deb", ArchiveFormat::Deb},     {".dmg", ArchiveFormat::Dmg},
        {".hfs", ArchiveFormat::Hfs},     {".lzh", ArchiveFormat::Lzh},
        {".lha", ArchiveFormat::Lzh},     {".rpm", ArchiveFormat::Rpm},
        {".udf", ArchiveFormat::Udf},     {".vhd", ArchiveFormat::Vhd},
        {".vhdx", ArchiveFormat::Vhd},    {".xar", ArchiveFormat::Xar},
        {".z", ArchiveFormat::Z},
    };

    auto ext = path.extension().string();
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    auto it = ext_map.find(ext);
    if (it != ext_map.end()) {
        return it->second;
    }

    // 检查复合扩展名（如 .tar.gz, .tar.bz2）
    auto stem = path.stem();
    if (stem.has_extension()) {
        auto stem_ext = stem.extension().string();
        std::transform(stem_ext.begin(), stem_ext.end(), stem_ext.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (stem_ext == ".tar") {
            // .tar.* 格式
            if (ext == ".gz") return ArchiveFormat::Tar;
            if (ext == ".bz2") return ArchiveFormat::Tar;
            if (ext == ".xz") return ArchiveFormat::Tar;
            if (ext == ".lzma") return ArchiveFormat::Tar;
        }
    }

    return ArchiveFormat::Unknown;
}

ArchiveFormat FormatDetector::from_signature(const void* data, size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);

    for (const auto& sig_info : get_signatures()) {
        if (size >= sig_info.offset + sig_info.length) {
            if (std::memcmp(bytes + sig_info.offset, sig_info.signature, sig_info.length) == 0) {
                return sig_info.format;
            }
        }
    }

    return ArchiveFormat::Unknown;
}

ArchiveFormat FormatDetector::detect(const std::filesystem::path& path) {
    // 先尝试读取文件头进行签名检测
    std::ifstream file(path, std::ios::binary);
    if (file) {
        // 读取足够的字节以检测所有签名
        // ISO 签名在 0x8001 位置，所以至少需要 0x8006 字节
        constexpr size_t header_size = 0x8010;
        std::vector<uint8_t> header(header_size);

        file.read(reinterpret_cast<char*>(header.data()), header.size());
        size_t read_size = static_cast<size_t>(file.gcount());

        if (read_size > 0) {
            auto fmt = from_signature(header.data(), read_size);
            if (fmt != ArchiveFormat::Unknown) {
                return fmt;
            }
        }
    }

    // 回退到扩展名检测
    return from_extension(path);
}

const void* FormatDetector::get_format_id(ArchiveFormat format) {
    // TODO: 在需要时返回对应格式的 CLSID
    // 这需要从 7-Zip 源码中获取各格式的 GUID
    // 目前返回 nullptr，使用格式名称作为替代
    (void)format;
    return nullptr;
}

const wchar_t* FormatDetector::get_format_name(ArchiveFormat format) {
    switch (format) {
        case ArchiveFormat::SevenZip:
            return L"7z";
        case ArchiveFormat::Zip:
            return L"zip";
        case ArchiveFormat::GZip:
            return L"gzip";
        case ArchiveFormat::BZip2:
            return L"bzip2";
        case ArchiveFormat::Tar:
            return L"tar";
        case ArchiveFormat::Xz:
            return L"xz";
        case ArchiveFormat::Lzma:
            return L"lzma";
        case ArchiveFormat::Rar:
            return L"rar";
        case ArchiveFormat::Rar5:
            return L"rar5";
        case ArchiveFormat::Iso:
            return L"iso";
        case ArchiveFormat::Wim:
            return L"wim";
        case ArchiveFormat::Cab:
            return L"cab";
        case ArchiveFormat::Arj:
            return L"arj";
        case ArchiveFormat::Cpio:
            return L"cpio";
        case ArchiveFormat::Deb:
            return L"deb";
        case ArchiveFormat::Dmg:
            return L"dmg";
        case ArchiveFormat::Hfs:
            return L"hfs";
        case ArchiveFormat::Lzh:
            return L"lzh";
        case ArchiveFormat::Nsis:
            return L"nsis";
        case ArchiveFormat::Rpm:
            return L"rpm";
        case ArchiveFormat::Udf:
            return L"udf";
        case ArchiveFormat::Vhd:
            return L"vhd";
        case ArchiveFormat::Xar:
            return L"xar";
        case ArchiveFormat::Z:
            return L"z";
        default:
            return L"";
    }
}

}  // namespace detail

// 格式信息
const FormatInfo& get_format_info(ArchiveFormat format) {
    static const std::map<ArchiveFormat, FormatInfo> info_map = {
        {ArchiveFormat::SevenZip,
         {ArchiveFormat::SevenZip,
          "7z",
          {".7z"},
          true,
          true,
          true,
          true,
          true,
          "7-Zip archive format"}},
        {ArchiveFormat::Zip,
         {ArchiveFormat::Zip,
          "zip",
          {".zip"},
          true,
          true,
          true,
          false,
          true,
          "ZIP archive format"}},
        {ArchiveFormat::GZip,
         {ArchiveFormat::GZip,
          "gzip",
          {".gz", ".gzip"},
          true,
          true,
          false,
          false,
          false,
          "GZip compressed format"}},
        {ArchiveFormat::BZip2,
         {ArchiveFormat::BZip2,
          "bzip2",
          {".bz2", ".bzip2"},
          true,
          true,
          false,
          false,
          false,
          "BZip2 compressed format"}},
        {ArchiveFormat::Tar,
         {ArchiveFormat::Tar,
          "tar",
          {".tar", ".tgz", ".tbz2", ".txz"},
          true,
          true,
          false,
          false,
          false,
          "TAR archive format"}},
        {ArchiveFormat::Xz,
         {ArchiveFormat::Xz,
          "xz",
          {".xz"},
          true,
          true,
          false,
          false,
          false,
          "XZ compressed format"}},
        {ArchiveFormat::Lzma,
         {ArchiveFormat::Lzma,
          "lzma",
          {".lzma"},
          true,
          true,
          false,
          false,
          false,
          "LZMA compressed format"}},
        {ArchiveFormat::Rar,
         {ArchiveFormat::Rar,
          "rar",
          {".rar"},
          true,
          false,
          true,
          true,
          true,
          "RAR archive format (v4 and earlier)"}},
        {ArchiveFormat::Rar5,
         {ArchiveFormat::Rar5,
          "rar5",
          {".rar"},
          true,
          false,
          true,
          true,
          true,
          "RAR5 archive format"}},
        {ArchiveFormat::Iso,
         {ArchiveFormat::Iso,
          "iso",
          {".iso", ".img"},
          true,
          false,
          false,
          false,
          false,
          "ISO disk image format"}},
        {ArchiveFormat::Wim,
         {ArchiveFormat::Wim,
          "wim",
          {".wim", ".swm", ".esd"},
          true,
          true,
          false,
          false,
          true,
          "Windows Imaging Format"}},
        {ArchiveFormat::Cab,
         {ArchiveFormat::Cab,
          "cab",
          {".cab"},
          true,
          false,
          false,
          false,
          true,
          "Microsoft Cabinet format"}},
        {ArchiveFormat::Arj,
         {ArchiveFormat::Arj,
          "arj",
          {".arj"},
          true,
          false,
          false,
          false,
          true,
          "ARJ archive format"}},
        {ArchiveFormat::Cpio,
         {ArchiveFormat::Cpio,
          "cpio",
          {".cpio"},
          true,
          false,
          false,
          false,
          false,
          "CPIO archive format"}},
        {ArchiveFormat::Deb,
         {ArchiveFormat::Deb,
          "deb",
          {".deb"},
          true,
          false,
          false,
          false,
          false,
          "Debian package format"}},
        {ArchiveFormat::Dmg,
         {ArchiveFormat::Dmg,
          "dmg",
          {".dmg"},
          true,
          false,
          false,
          false,
          false,
          "Apple Disk Image"}},
        {ArchiveFormat::Rpm,
         {ArchiveFormat::Rpm,
          "rpm",
          {".rpm"},
          true,
          false,
          false,
          false,
          false,
          "RPM package format"}},
    };

    auto it = info_map.find(format);
    if (it != info_map.end()) {
        return it->second;
    }

    static const FormatInfo unknown{
        ArchiveFormat::Unknown, "unknown", {}, false, false, false, false, false, "Unknown format"};
    return unknown;
}

std::vector<FormatInfo> get_all_formats() {
    std::vector<FormatInfo> formats;

    // 遍历所有已定义的格式
    for (int i = static_cast<int>(ArchiveFormat::SevenZip); i <= static_cast<int>(ArchiveFormat::Z);
         ++i) {
        auto fmt = static_cast<ArchiveFormat>(i);
        const auto& info = get_format_info(fmt);
        if (info.format != ArchiveFormat::Unknown) {
            formats.push_back(info);
        }
    }

    return formats;
}

std::string to_string(ArchiveFormat format) {
    return get_format_info(format).name;
}

std::optional<ArchiveFormat> from_string(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (int i = static_cast<int>(ArchiveFormat::SevenZip); i <= static_cast<int>(ArchiveFormat::Z);
         ++i) {
        auto fmt = static_cast<ArchiveFormat>(i);
        const auto& info = get_format_info(fmt);
        if (info.name == lower_str) {
            return fmt;
        }
    }

    return std::nullopt;
}

}  // namespace sevenzip
