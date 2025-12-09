#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "archive.hpp"

namespace sevenzip {

/// @brief 解压归档到目录 (最简单方式)
/// @param archivePath 归档文件路径
/// @param destDir 目标目录
/// @throws Exception 如果操作失败
void extract(const std::filesystem::path& archivePath, const std::filesystem::path& destDir);

/// @brief 解压归档到目录 (带密码)
/// @param archivePath 归档文件路径
/// @param destDir 目标目录
/// @param password 密码
/// @throws Exception 如果操作失败
void extract(const std::filesystem::path& archivePath, const std::filesystem::path& destDir,
             const std::string& password);

/// @brief 解压归档到目录 (带格式和密码)
/// @param archivePath 归档文件路径
/// @param destDir 目标目录
/// @param format 归档格式
/// @param password 密码 (可选)
/// @throws Exception 如果操作失败
void extract(const std::filesystem::path& archivePath, const std::filesystem::path& destDir,
             Format format, const std::string& password = "");

/// @brief 压缩文件或目录到归档 (默认7z格式)
/// @param sourcePath 源文件或目录路径
/// @param archivePath 目标归档路径
/// @throws Exception 如果操作失败
void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& archivePath);

/// @brief 压缩文件或目录到归档 (指定格式)
/// @param sourcePath 源文件或目录路径
/// @param archivePath 目标归档路径
/// @param format 归档格式
/// @throws Exception 如果操作失败
void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& archivePath,
              Format format);

/// @brief 压缩文件或目录到归档 (指定格式和压缩级别)
/// @param sourcePath 源文件或目录路径
/// @param archivePath 目标归档路径
/// @param format 归档格式
/// @param level 压缩级别 (Fast, Normal, Maximum)
/// @throws Exception 如果操作失败
void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& archivePath,
              Format format, CompressionLevel level);

/// @brief 压缩文件或目录到归档 (带密码)
/// @param sourcePath 源文件或目录路径
/// @param archivePath 目标归档路径
/// @param format 归档格式
/// @param level 压缩级别
/// @param password 密码
/// @throws Exception 如果操作失败
void compress(const std::filesystem::path& sourcePath, const std::filesystem::path& archivePath,
              Format format, CompressionLevel level, const std::string& password);

/// @brief 压缩内存数据到归档
/// @param data 要压缩的数据
/// @param format 归档格式 (默认 7z)
/// @param level 压缩级别 (默认 Normal)
/// @return 压缩后的数据
/// @throws Exception 如果操作失败
std::vector<uint8_t> compressData(const std::vector<uint8_t>& data,
                                  Format format = Format::SevenZip,
                                  CompressionLevel level = CompressionLevel::Normal);

/// @brief 解压归档到内存 (假设只有一个文件)
/// @param archivePath 归档文件路径
/// @return 解压后的数据
/// @throws Exception 如果操作失败或有多个文件
std::vector<uint8_t> extractSingleFile(const std::filesystem::path& archivePath);

/// @brief 解压归档到内存 (指定条目索引)
/// @param archivePath 归档文件路径
/// @param itemIndex 条目索引
/// @return 解压后的数据
/// @throws Exception 如果操作失败
std::vector<uint8_t> extractSingleFile(const std::filesystem::path& archivePath,
                                       uint32_t itemIndex);

/// @brief 列出归档内容
/// @param archivePath 归档文件路径
/// @return 条目信息列表
/// @throws Exception 如果操作失败
std::vector<ItemInfo> list(const std::filesystem::path& archivePath);

/// @brief 列出归档内容 (带密码)
/// @param archivePath 归档文件路径
/// @param password 密码
/// @return 条目信息列表
/// @throws Exception 如果操作失败
std::vector<ItemInfo> list(const std::filesystem::path& archivePath, const std::string& password);

/// @brief 测试归档完整性
/// @param archivePath 归档文件路径
/// @return true 如果完整, false 如果损坏
/// @throws Exception 如果无法打开归档
bool testArchive(const std::filesystem::path& archivePath);

/// @brief 测试归档完整性 (带密码)
/// @param archivePath 归档文件路径
/// @param password 密码
/// @return true 如果完整, false 如果损坏
/// @throws Exception 如果无法打开归档
bool testArchive(const std::filesystem::path& archivePath, const std::string& password);

/// @brief 获取归档信息
/// @param archivePath 归档文件路径
/// @return 归档信息
/// @throws Exception 如果操作失败
ArchiveInfo getArchiveInfo(const std::filesystem::path& archivePath);

/// @brief 检查文件是否为归档格式
/// @param filePath 文件路径
/// @return true 如果是支持的归档格式
bool isArchive(const std::filesystem::path& filePath);

}  // namespace sevenzip
