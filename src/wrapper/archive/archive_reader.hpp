// archive_reader.hpp - Archive reader implementation
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../common/wrapper_error.hpp"
#include "../stream/stream_file.hpp"
#include "archive_format.hpp"

// Forward declaration for 7-Zip COM interfaces
struct IInStream;

namespace sevenzip::detail {

// 归档项属性
struct ArchiveItemInfo {
    uint32_t index;
    std::wstring path;
    bool isDirectory;
    uint64_t size;
    uint64_t packedSize;
    std::optional<uint32_t> crc;
    std::optional<uint64_t> creationTime;  // FILETIME as uint64
    std::optional<uint64_t> lastWriteTime;
    std::optional<uint64_t> lastAccessTime;
    std::optional<uint32_t> attributes;
    bool isEncrypted;
    std::optional<std::wstring> comment;
};

// 归档信息
struct ArchiveInfo {
    ArchiveFormat format;
    uint64_t physicalSize;
    uint32_t itemCount;
    bool isMultiVolume;
    bool isSolid;
    bool hasEncryptedHeader;
    std::optional<std::wstring> comment;
};

// 进度回调：返回 false 取消操作
using ProgressCallback = std::function<bool(uint64_t completed, uint64_t total)>;

// 密码回调：返回密码字符串
using PasswordCallback = std::function<std::wstring()>;

// 归档读取器
class ArchiveReader {
   public:
    ArchiveReader();
    ~ArchiveReader();

    // 禁止拷贝
    ArchiveReader(const ArchiveReader&) = delete;
    ArchiveReader& operator=(const ArchiveReader&) = delete;

    // 允许移动
    ArchiveReader(ArchiveReader&&) noexcept;
    ArchiveReader& operator=(ArchiveReader&&) noexcept;

    // 打开归档（自动检测格式）
    void open(const std::wstring& path);

    // 打开归档（指定格式）
    void open(const std::wstring& path, ArchiveFormat format);

    // 从流打开归档
    void openFromStream(::IInStream* stream, ArchiveFormat format);

    // 关闭归档
    void close();

    // 是否已打开
    bool isOpen() const;

    // 获取归档信息
    ArchiveInfo getArchiveInfo() const;

    // 获取项目数量
    uint32_t getItemCount() const;

    // 获取项目信息
    ArchiveItemInfo getItemInfo(uint32_t index) const;

    // 遍历所有项目
    template <typename Func>
    void forEachItem(Func&& func) const {
        uint32_t count = getItemCount();
        for (uint32_t i = 0; i < count; ++i) {
            if (!func(getItemInfo(i))) break;
        }
    }

    // 解压单个文件到内存
    std::vector<uint8_t> extractToMemory(uint32_t index);

    // 解压单个文件到磁盘
    void extractToFile(uint32_t index, const std::wstring& destPath);

    // 解压多个文件到目录
    void extractItems(const std::vector<uint32_t>& indices, const std::wstring& destDir);

    // 解压所有文件到目录
    void extractAll(const std::wstring& destDir);

    // 测试归档完整性
    bool testArchive();

    // 设置密码回调
    void setPasswordCallback(PasswordCallback callback);

    // 设置进度回调
    void setProgressCallback(ProgressCallback callback);

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace sevenzip::detail
