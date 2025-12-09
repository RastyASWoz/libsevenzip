#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace sevenzip {

enum class ArchiveFormat {
    Unknown = 0,
    Auto,
    SevenZip,
    Zip,
    GZip,
    BZip2,
    Tar,
    Xz,
    Lzma,
    Rar,
    Rar5,
    Iso,
    Wim,
    Cab,
    Arj,
    Cpio,
    Deb,
    Dmg,
    Hfs,
    Lzh,
    Nsis,
    Rpm,
    Udf,
    Vhd,
    Wcs,
    Xar,
    Z
};

struct FormatInfo {
    ArchiveFormat format;
    std::string name;
    std::vector<std::string> extensions;
    bool supportsRead;
    bool supportsWrite;
    bool supportsEncryption;
    bool supportsSolid;
    bool supportsMultiVolume;
    std::string description;
};

namespace detail {

class FormatDetector {
   public:
    // 根据文件扩展名检测格式
    static ArchiveFormat from_extension(const std::filesystem::path& path);

    // 根据文件签名检测格式
    static ArchiveFormat from_signature(const void* data, size_t size);

    // 综合检测（优先签名，回退到扩展名）
    static ArchiveFormat detect(const std::filesystem::path& path);

    // 获取格式的 CLSID（Windows）或格式 ID（跨平台）
    // 返回 void* 避免暴露 GUID 结构
    static const void* get_format_id(ArchiveFormat format);

    // 获取格式名称（用于 CreateObject）
    static const wchar_t* get_format_name(ArchiveFormat format);

   private:
    struct SignatureInfo {
        const uint8_t* signature;
        size_t length;
        size_t offset;  // 从文件开头的偏移
        ArchiveFormat format;
    };

    static const std::vector<SignatureInfo>& get_signatures();
};

}  // namespace detail

// 公共 API
const FormatInfo& get_format_info(ArchiveFormat format);
std::vector<FormatInfo> get_all_formats();

// 辅助函数
std::string to_string(ArchiveFormat format);
std::optional<ArchiveFormat> from_string(const std::string& str);

}  // namespace sevenzip
