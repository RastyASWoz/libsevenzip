#pragma once

#include <cstdint>
#include <vector>

#include "archive.hpp"

namespace sevenzip {

/// @brief 独立压缩器类，用于单文件压缩/解压（不创建归档格式）
///
/// 支持的格式:
/// - GZIP (.gz)
/// - BZIP2 (.bz2)
/// - XZ (.xz)
///
/// 这些格式只压缩单个数据流，不包含文件名、时间戳等元数据。
/// 如果需要压缩多个文件或保留元数据，请使用 Archive 类。
class Compressor {
   public:
    /// @brief 创建压缩器
    /// @param format 压缩格式 (GZip, BZip2, Xz)
    /// @param level 压缩级别
    explicit Compressor(Format format = Format::GZip,
                        CompressionLevel level = CompressionLevel::Normal);

    ~Compressor();

    // 移动语义
    Compressor(Compressor&& other) noexcept;
    Compressor& operator=(Compressor&& other) noexcept;

    // 禁止拷贝
    Compressor(const Compressor&) = delete;
    Compressor& operator=(const Compressor&) = delete;

    /// @brief 设置压缩级别
    /// @param level 压缩级别
    /// @return *this 用于链式调用
    Compressor& withLevel(CompressionLevel level);

    /// @brief 压缩数据
    /// @param input 输入数据
    /// @return 压缩后的数据
    /// @throws Exception 如果压缩失败
    std::vector<uint8_t> compress(const std::vector<uint8_t>& input);

    /// @brief 解压数据
    /// @param input 压缩的数据
    /// @return 解压后的数据
    /// @throws Exception 如果解压失败
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& input);

    /// @brief 压缩文件
    /// @param inputPath 输入文件路径
    /// @param outputPath 输出文件路径
    /// @throws Exception 如果压缩失败
    void compressFile(const std::filesystem::path& inputPath,
                      const std::filesystem::path& outputPath);

    /// @brief 解压文件
    /// @param inputPath 输入文件路径
    /// @param outputPath 输出文件路径
    /// @throws Exception 如果解压失败
    void decompressFile(const std::filesystem::path& inputPath,
                        const std::filesystem::path& outputPath);

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace sevenzip
