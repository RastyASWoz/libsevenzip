#pragma once

#include <memory>
#include <string>

#include "stream_base.hpp"

namespace sevenzip::detail {

/**
 * @brief 文件输入流
 *
 * 使用 7-Zip 的跨平台文件 API 实现文件读取
 */
class FileInStream : public COMInStreamImpl {
   public:
    /**
     * @brief 打开文件用于读取
     * @param path 文件路径（宽字符）
     * @throws Exception 如果文件打开失败
     */
    explicit FileInStream(const std::wstring& path);

    /**
     * @brief 打开文件用于读取
     * @param path 文件路径（UTF-8）
     * @throws Exception 如果文件打开失败
     */
    explicit FileInStream(const std::string& path);

    ~FileInStream() override;

    /**
     * @brief 检查文件是否成功打开
     */
    bool isOpen() const;

    /**
     * @brief 获取文件路径
     */
    const std::wstring& path() const { return path_; }

   protected:
    HRESULT DoRead(void* data, UInt32 size, UInt32* processedSize) override;
    HRESULT DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) override;
    HRESULT DoGetSize(UInt64* size) override;
    HRESULT DoGetProps(UInt64* size, FILETIME* cTime, FILETIME* aTime, FILETIME* mTime,
                       UInt32* attrib) override;

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::wstring path_;
};

/**
 * @brief 文件输出流
 *
 * 使用 7-Zip 的跨平台文件 API 实现文件写入
 */
class FileOutStream : public COMOutStreamImpl {
   public:
    /**
     * @brief 创建/打开文件用于写入
     * @param path 文件路径（宽字符）
     * @param createAlways true=总是创建新文件，false=如果存在则打开
     * @throws Exception 如果文件打开失败
     */
    explicit FileOutStream(const std::wstring& path, bool createAlways = true);

    /**
     * @brief 创建/打开文件用于写入
     * @param path 文件路径（UTF-8）
     * @param createAlways true=总是创建新文件，false=如果存在则打开
     * @throws Exception 如果文件打开失败
     */
    explicit FileOutStream(const std::string& path, bool createAlways = true);

    ~FileOutStream() override;

    /**
     * @brief 检查文件是否成功打开
     */
    bool isOpen() const;

    /**
     * @brief 获取文件路径
     */
    const std::wstring& path() const { return path_; }

   protected:
    HRESULT DoWrite(const void* data, UInt32 size, UInt32* processedSize) override;
    HRESULT DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) override;
    HRESULT DoSetSize(UInt64 newSize) override;

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::wstring path_;
};

}  // namespace sevenzip::detail
