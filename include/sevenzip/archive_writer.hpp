#pragma once

#include <memory>
#include <vector>

#include "archive.hpp"

namespace sevenzip {

/// @brief 专门用于创建归档的类
///
/// 提供流畅的链式调用接口，简化归档创建流程。
/// 适合需要逐步添加文件并配置选项的场景。
class ArchiveWriter {
   public:
    /// @brief 创建新归档
    /// @param path 归档文件路径
    /// @param format 归档格式，默认7z
    /// @return ArchiveWriter对象
    static ArchiveWriter create(const std::filesystem::path& path,
                                Format format = Format::SevenZip);

    /// @brief 创建到内存缓冲区
    /// @param buffer 输出缓冲区
    /// @param format 归档格式，默认7z
    /// @return ArchiveWriter对象
    static ArchiveWriter createToMemory(std::vector<uint8_t>& buffer,
                                        Format format = Format::SevenZip);

    ~ArchiveWriter();

    // 移动语义
    ArchiveWriter(ArchiveWriter&& other) noexcept;
    ArchiveWriter& operator=(ArchiveWriter&& other) noexcept;

    // 禁止拷贝
    ArchiveWriter(const ArchiveWriter&) = delete;
    ArchiveWriter& operator=(const ArchiveWriter&) = delete;

    // ========================================================================
    // 配置选项 - 链式调用
    // ========================================================================

    /// @brief 设置压缩级别
    /// @param level 压缩级别
    /// @return *this 用于链式调用
    ArchiveWriter& withLevel(CompressionLevel level);

    /// @brief 设置密码
    /// @param password 密码
    /// @return *this 用于链式调用
    ArchiveWriter& withPassword(const std::string& password);

    /// @brief 设置是否加密头
    /// @param encrypt 是否加密头
    /// @return *this 用于链式调用
    ArchiveWriter& withEncryptedHeaders(bool encrypt = true);

    /// @brief 设置固实模式
    /// @param solid 是否使用固实压缩
    /// @return *this 用于链式调用
    ArchiveWriter& withSolidMode(bool solid = true);

    /// @brief 设置分卷大小
    /// @param volumeSize 每卷大小（字节）
    /// @return *this 用于链式调用
    ArchiveWriter& withMultiVolume(uint64_t volumeSize);

    /// @brief 设置进度回调
    /// @param callback 进度回调函数
    /// @return *this 用于链式调用
    ArchiveWriter& withProgress(ProgressCallback callback);

    // ========================================================================
    // 添加内容 - 链式调用
    // ========================================================================

    /// @brief 添加文件
    /// @param path 文件路径
    /// @return *this 用于链式调用
    /// @throws Exception 如果文件不存在
    ArchiveWriter& addFile(const std::filesystem::path& path);

    /// @brief 添加文件（指定归档内路径）
    /// @param path 文件路径
    /// @param archiveName 归档内的路径
    /// @return *this 用于链式调用
    /// @throws Exception 如果文件不存在
    ArchiveWriter& addFile(const std::filesystem::path& path,
                           const std::filesystem::path& archiveName);

    /// @brief 添加目录
    /// @param path 目录路径
    /// @param recursive 是否递归，默认true
    /// @return *this 用于链式调用
    /// @throws Exception 如果目录不存在
    ArchiveWriter& addDirectory(const std::filesystem::path& path, bool recursive = true);

    /// @brief 从内存添加文件
    /// @param data 文件数据
    /// @param name 归档内的文件名
    /// @return *this 用于链式调用
    ArchiveWriter& addFromMemory(const std::vector<uint8_t>& data,
                                 const std::filesystem::path& name);

    /// @brief 添加多个文件
    /// @param paths 文件路径列表
    /// @return *this 用于链式调用
    ArchiveWriter& addFiles(const std::vector<std::filesystem::path>& paths);

    // ========================================================================
    // 完成操作
    // ========================================================================

    /// @brief 完成归档创建
    /// @throws Exception 如果操作失败
    void finalize();

    /// @brief 获取已添加的条目数量
    /// @return 条目数量
    size_t pendingCount() const;

   private:
    ArchiveWriter();  // 私有构造函数

    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace sevenzip
