// archive_writer.hpp - Archive writer implementation
#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../common/wrapper_error.hpp"
#include "archive_format.hpp"

// Forward declaration for 7-Zip COM interfaces
struct IOutStream;

namespace sevenzip::detail {

namespace fs = std::filesystem;

// 添加到归档的项目类型
enum class UpdateItemType {
    File,       // 普通文件
    Directory,  // 目录
    SymLink,    // 符号链接
    HardLink    // 硬链接
};

// 压缩级别
enum class CompressionLevel {
    None = 0,     // 仅存储
    Fastest = 1,  // 最快
    Fast = 3,     // 快速
    Normal = 5,   // 正常
    Maximum = 7,  // 最大压缩
    Ultra = 9     // 超级压缩
};

// 添加到归档的项目信息
struct UpdateItemInfo {
    // 必需字段
    std::wstring archivePath;  // 在归档中的路径
    UpdateItemType itemType;   // 项目类型

    // 文件属性
    std::optional<std::wstring> sourcePath;    // 源文件路径（对于从磁盘添加的文件）
    std::optional<std::vector<uint8_t>> data;  // 内存中的数据（与sourcePath互斥）

    uint64_t size = 0;           // 文件大小（目录为0）
    uint64_t lastWriteTime = 0;  // 修改时间（FILETIME格式）
    uint32_t attributes = 0;     // 文件属性

    // 可选字段
    std::optional<uint64_t> creationTime;
    std::optional<uint64_t> lastAccessTime;
    std::optional<uint32_t> crc;

    // 链接相关
    std::optional<std::wstring> linkTarget;  // 符号链接或硬链接的目标
};

// 压缩方法（针对7z格式）
enum class CompressionMethod {
    LZMA,     // 默认
    LZMA2,    // 多线程支持
    PPMd,     // 文本压缩
    BZip2,    // BZip2
    Deflate,  // ZIP兼容
    Copy      // 仅存储
};

// 归档属性配置
struct ArchiveProperties {
    // 压缩设置
    CompressionLevel level = CompressionLevel::Normal;
    std::optional<CompressionMethod> method;  // 不指定则使用格式默认值

    // 字典大小（字节），不指定则使用默认值
    std::optional<uint64_t> dictionarySize;

    // 固实压缩（仅7z）
    bool solid = true;

    // 多线程
    std::optional<uint32_t> numThreads;  // 0或未指定=自动

    // 加密
    std::optional<std::wstring> password;
    bool encryptHeaders = false;  // 加密文件名（仅7z）

    // 多卷
    std::optional<uint64_t> volumeSize;  // 字节，0或未指定=单卷
};

// 更新回调：返回 false 取消操作
using UpdateProgressCallback = std::function<bool(uint64_t completed, uint64_t total)>;

// 归档写入器
class ArchiveWriter {
   public:
    ArchiveWriter();
    ~ArchiveWriter();

    // 禁止拷贝
    ArchiveWriter(const ArchiveWriter&) = delete;
    ArchiveWriter& operator=(const ArchiveWriter&) = delete;

    // 允许移动
    ArchiveWriter(ArchiveWriter&&) noexcept;
    ArchiveWriter& operator=(ArchiveWriter&&) noexcept;

    // 创建新归档（指定格式）
    void create(const std::wstring& path, ArchiveFormat format);

    // 从流创建归档
    void createToStream(::IOutStream* stream, ArchiveFormat format);

    // 设置归档属性（必须在添加项目前调用）
    void setProperties(const ArchiveProperties& props);

    // 关闭归档（自动在析构时调用）
    void close();

    // 添加文件到归档
    void addFile(const std::wstring& sourcePath, const std::wstring& archivePath);

    // 添加目录到归档（递归）
    void addDirectory(const std::wstring& sourcePath, const std::wstring& archivePath,
                      bool recursive = true);

    // 从内存添加文件
    void addFileFromMemory(const std::vector<uint8_t>& data, const std::wstring& archivePath);

    // 添加空目录
    void addEmptyDirectory(const std::wstring& archivePath);

    // 添加自定义项目
    void addItem(const UpdateItemInfo& item);

    // 批量添加项目并写入归档（必须在所有addXxx调用后调用）
    void finalize();

    // 是否已完成写入
    bool isFinalized() const;

    // 获取已添加的项目数量
    uint32_t getItemCount() const;

    // 设置进度回调
    void setProgressCallback(UpdateProgressCallback callback);

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace sevenzip::detail
