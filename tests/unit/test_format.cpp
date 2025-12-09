#include <gtest/gtest.h>

#include <fstream>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#endif

#include "wrapper/archive/archive_format.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

// 测试扩展名检测
TEST(FormatTest, FromExtensionBasic) {
    EXPECT_EQ(FormatDetector::from_extension("test.7z"), ArchiveFormat::SevenZip);
    EXPECT_EQ(FormatDetector::from_extension("test.zip"), ArchiveFormat::Zip);
    EXPECT_EQ(FormatDetector::from_extension("test.tar"), ArchiveFormat::Tar);
    EXPECT_EQ(FormatDetector::from_extension("test.gz"), ArchiveFormat::GZip);
    EXPECT_EQ(FormatDetector::from_extension("test.bz2"), ArchiveFormat::BZip2);
    EXPECT_EQ(FormatDetector::from_extension("test.xz"), ArchiveFormat::Xz);
}

TEST(FormatTest, FromExtensionCaseInsensitive) {
    EXPECT_EQ(FormatDetector::from_extension("TEST.7Z"), ArchiveFormat::SevenZip);
    EXPECT_EQ(FormatDetector::from_extension("Test.Zip"), ArchiveFormat::Zip);
    EXPECT_EQ(FormatDetector::from_extension("TeSt.TaR"), ArchiveFormat::Tar);
}

TEST(FormatTest, FromExtensionCompound) {
    // 复合扩展名返回最后一个扩展名对应的格式（压缩格式）
    EXPECT_EQ(FormatDetector::from_extension("test.tar.gz"), ArchiveFormat::GZip);
    EXPECT_EQ(FormatDetector::from_extension("test.tar.bz2"), ArchiveFormat::BZip2);
    EXPECT_EQ(FormatDetector::from_extension("test.tar.xz"), ArchiveFormat::Xz);
    EXPECT_EQ(FormatDetector::from_extension("test.tar.lzma"), ArchiveFormat::Lzma);
}

TEST(FormatTest, FromExtensionShortcuts) {
    EXPECT_EQ(FormatDetector::from_extension("test.tgz"), ArchiveFormat::Tar);
    EXPECT_EQ(FormatDetector::from_extension("test.tbz"), ArchiveFormat::Tar);
    EXPECT_EQ(FormatDetector::from_extension("test.tbz2"), ArchiveFormat::Tar);
    EXPECT_EQ(FormatDetector::from_extension("test.txz"), ArchiveFormat::Tar);
}

TEST(FormatTest, FromExtensionUnknown) {
    EXPECT_EQ(FormatDetector::from_extension("test.txt"), ArchiveFormat::Unknown);
    EXPECT_EQ(FormatDetector::from_extension("test.pdf"), ArchiveFormat::Unknown);
    EXPECT_EQ(FormatDetector::from_extension("test"), ArchiveFormat::Unknown);
}

TEST(FormatTest, FromExtensionRare) {
    EXPECT_EQ(FormatDetector::from_extension("test.rar"), ArchiveFormat::Rar);
    EXPECT_EQ(FormatDetector::from_extension("test.iso"), ArchiveFormat::Iso);
    EXPECT_EQ(FormatDetector::from_extension("test.cab"), ArchiveFormat::Cab);
    EXPECT_EQ(FormatDetector::from_extension("test.wim"), ArchiveFormat::Wim);
}

// 测试签名检测
TEST(FormatTest, FromSignature7z) {
    uint8_t sig[] = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::SevenZip);
}

TEST(FormatTest, FromSignatureZip) {
    uint8_t sig[] = {0x50, 0x4B, 0x03, 0x04};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Zip);
}

TEST(FormatTest, FromSignatureGzip) {
    uint8_t sig[] = {0x1F, 0x8B, 0x08};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::GZip);
}

TEST(FormatTest, FromSignatureBzip2) {
    uint8_t sig[] = {0x42, 0x5A, 0x68};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::BZip2);
}

TEST(FormatTest, FromSignatureXz) {
    uint8_t sig[] = {0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Xz);
}

TEST(FormatTest, FromSignatureRar) {
    uint8_t sig[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Rar);
}

TEST(FormatTest, FromSignatureRar5) {
    uint8_t sig[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Rar5);
}

TEST(FormatTest, FromSignatureCab) {
    uint8_t sig[] = {0x4D, 0x53, 0x43, 0x46};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Cab);
}

TEST(FormatTest, FromSignatureUnknown) {
    uint8_t sig[] = {0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Unknown);
}

TEST(FormatTest, FromSignatureTooShort) {
    uint8_t sig[] = {0x37, 0x7A};
    EXPECT_EQ(FormatDetector::from_signature(sig, sizeof(sig)), ArchiveFormat::Unknown);
}

// 测试格式信息
TEST(FormatTest, GetFormatInfo7z) {
    const auto& info = get_format_info(ArchiveFormat::SevenZip);

    EXPECT_EQ(info.format, ArchiveFormat::SevenZip);
    EXPECT_EQ(info.name, "7z");
    EXPECT_TRUE(info.supportsRead);
    EXPECT_TRUE(info.supportsWrite);
    EXPECT_TRUE(info.supportsEncryption);
    EXPECT_TRUE(info.supportsSolid);
    EXPECT_TRUE(info.supportsMultiVolume);
}

TEST(FormatTest, GetFormatInfoZip) {
    const auto& info = get_format_info(ArchiveFormat::Zip);

    EXPECT_EQ(info.format, ArchiveFormat::Zip);
    EXPECT_EQ(info.name, "zip");
    EXPECT_TRUE(info.supportsRead);
    EXPECT_TRUE(info.supportsWrite);
    EXPECT_TRUE(info.supportsEncryption);
    EXPECT_FALSE(info.supportsSolid);
}

TEST(FormatTest, GetFormatInfoUnknown) {
    const auto& info = get_format_info(ArchiveFormat::Unknown);

    EXPECT_EQ(info.format, ArchiveFormat::Unknown);
    EXPECT_FALSE(info.supportsRead);
    EXPECT_FALSE(info.supportsWrite);
}

// 测试格式列表
TEST(FormatTest, GetAllFormats) {
    auto formats = get_all_formats();

    EXPECT_FALSE(formats.empty());

    // 应该包含常见格式
    bool has_7z = false;
    bool has_zip = false;
    bool has_tar = false;

    for (const auto& info : formats) {
        if (info.format == ArchiveFormat::SevenZip) has_7z = true;
        if (info.format == ArchiveFormat::Zip) has_zip = true;
        if (info.format == ArchiveFormat::Tar) has_tar = true;
    }

    EXPECT_TRUE(has_7z);
    EXPECT_TRUE(has_zip);
    EXPECT_TRUE(has_tar);
}

// 测试字符串转换
TEST(FormatTest, ToStringConversion) {
    EXPECT_EQ(to_string(ArchiveFormat::SevenZip), "7z");
    EXPECT_EQ(to_string(ArchiveFormat::Zip), "zip");
    EXPECT_EQ(to_string(ArchiveFormat::Tar), "tar");
    EXPECT_EQ(to_string(ArchiveFormat::GZip), "gzip");
}

TEST(FormatTest, FromStringConversion) {
    EXPECT_EQ(from_string("7z"), ArchiveFormat::SevenZip);
    EXPECT_EQ(from_string("zip"), ArchiveFormat::Zip);
    EXPECT_EQ(from_string("tar"), ArchiveFormat::Tar);
    EXPECT_EQ(from_string("gzip"), ArchiveFormat::GZip);
}

TEST(FormatTest, FromStringCaseInsensitive) {
    EXPECT_EQ(from_string("7Z"), ArchiveFormat::SevenZip);
    EXPECT_EQ(from_string("ZIP"), ArchiveFormat::Zip);
    EXPECT_EQ(from_string("Tar"), ArchiveFormat::Tar);
}

TEST(FormatTest, FromStringUnknown) {
    auto result = from_string("unknown_format");
    EXPECT_FALSE(result.has_value());
}

// 测试格式名称
TEST(FormatTest, GetFormatName) {
    EXPECT_STREQ(FormatDetector::get_format_name(ArchiveFormat::SevenZip), L"7z");
    EXPECT_STREQ(FormatDetector::get_format_name(ArchiveFormat::Zip), L"zip");
    EXPECT_STREQ(FormatDetector::get_format_name(ArchiveFormat::Tar), L"tar");
}

// 测试文件检测（需要临时文件）
class FormatDetectFileTest : public ::testing::Test {
   protected:
    void SetUp() override {
        temp_dir = std::filesystem::temp_directory_path() / "sevenzip_test";
        std::filesystem::create_directories(temp_dir);
    }

    void TearDown() override { std::filesystem::remove_all(temp_dir); }

    void create_test_file(const std::string& name, const std::vector<uint8_t>& data) {
        auto path = temp_dir / name;
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    std::filesystem::path temp_dir;
};

TEST_F(FormatDetectFileTest, Detect7z) {
    std::vector<uint8_t> sig = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C};
    create_test_file("test.7z", sig);

    auto format = FormatDetector::detect(temp_dir / "test.7z");
    EXPECT_EQ(format, ArchiveFormat::SevenZip);
}

TEST_F(FormatDetectFileTest, DetectZip) {
    std::vector<uint8_t> sig = {0x50, 0x4B, 0x03, 0x04};
    create_test_file("test.zip", sig);

    auto format = FormatDetector::detect(temp_dir / "test.zip");
    EXPECT_EQ(format, ArchiveFormat::Zip);
}

TEST_F(FormatDetectFileTest, DetectFallbackToExtension) {
    std::vector<uint8_t> dummy = {0x00, 0x00, 0x00, 0x00};
    create_test_file("test.7z", dummy);

    // 签名不匹配，应该回退到扩展名
    auto format = FormatDetector::detect(temp_dir / "test.7z");
    EXPECT_EQ(format, ArchiveFormat::SevenZip);
}

TEST_F(FormatDetectFileTest, DetectNonexistentFile) {
    auto format = FormatDetector::detect(temp_dir / "nonexistent.7z");
    // 文件不存在时应该使用扩展名
    EXPECT_EQ(format, ArchiveFormat::SevenZip);
}
