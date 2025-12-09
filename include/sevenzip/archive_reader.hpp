#pragma once

#include <memory>
#include <optional>

#include "archive.hpp"

namespace sevenzip {

/// @brief 专门用于读取归档的类
///
/// 提供只读接口，不支持修改归档。
/// 适合需要频繁查询或遍历归档内容的场景。
class ArchiveReader {
   public:
    /// @brief 打开归档文件
    /// @param path 归档文件路径
    /// @throws Exception 如果文件不存在或格式不支持
    explicit ArchiveReader(const std::filesystem::path& path);

    /// @brief 从内存打开归档
    /// @param buffer 包含归档数据的缓冲区
    /// @throws Exception 如果数据无效
    explicit ArchiveReader(const std::vector<uint8_t>& buffer);

    ~ArchiveReader();

    // 移动语义
    ArchiveReader(ArchiveReader&& other) noexcept;
    ArchiveReader& operator=(ArchiveReader&& other) noexcept;

    // 禁止拷贝
    ArchiveReader(const ArchiveReader&) = delete;
    ArchiveReader& operator=(const ArchiveReader&) = delete;

    // ========================================================================
    // 查询操作
    // ========================================================================

    /// @brief 获取归档信息
    /// @return 归档信息
    ArchiveInfo info() const;

    /// @brief 获取条目数量
    /// @return 条目数量
    size_t itemCount() const;

    /// @brief 获取指定索引的条目信息
    /// @param index 条目索引
    /// @return 条目信息
    /// @throws Exception 如果索引越界
    ItemInfo itemInfo(size_t index) const;

    /// @brief 查找指定路径的条目
    /// @param path 条目路径
    /// @return 条目信息，如果未找到则返回空
    std::optional<ItemInfo> findItem(const std::filesystem::path& path) const;

    /// @brief 检查归档是否包含指定路径
    /// @param path 条目路径
    /// @return 如果存在返回true
    bool contains(const std::filesystem::path& path) const;

    // ========================================================================
    // 提取操作
    // ========================================================================

    /// @brief 提取单个条目到内存
    /// @param index 条目索引
    /// @return 条目数据
    /// @throws Exception 如果索引越界或提取失败
    std::vector<uint8_t> extract(size_t index) const;

    /// @brief 提取单个条目到文件
    /// @param index 条目索引
    /// @param destPath 目标文件路径
    /// @throws Exception 如果索引越界或提取失败
    void extractTo(size_t index, const std::filesystem::path& destPath) const;

    /// @brief 提取所有条目到目录
    /// @param destDir 目标目录
    /// @throws Exception 如果提取失败
    void extractAll(const std::filesystem::path& destDir) const;

    /// @brief 提取选定条目到目录
    /// @param indices 条目索引列表
    /// @param destDir 目标目录
    /// @throws Exception 如果提取失败
    void extractItems(const std::vector<size_t>& indices,
                      const std::filesystem::path& destDir) const;

    /// @brief 测试归档完整性
    /// @return 如果归档完整返回true
    bool test() const;

    // ========================================================================
    // 配置
    // ========================================================================

    /// @brief 设置密码
    /// @param password 密码
    /// @return *this 用于链式调用
    ArchiveReader& withPassword(const std::string& password);

    /// @brief 设置进度回调
    /// @param callback 进度回调函数
    /// @return *this 用于链式调用
    ArchiveReader& withProgress(ProgressCallback callback);

    // ========================================================================
    // 迭代器支持
    // ========================================================================

    /// @brief 迭代器类
    class Iterator {
       public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ItemInfo;
        using difference_type = std::ptrdiff_t;
        using pointer = const ItemInfo*;
        using reference = const ItemInfo&;

        Iterator(const ArchiveReader* reader, size_t index);

        Iterator& operator++();
        Iterator operator++(int);
        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;
        reference operator*() const;
        pointer operator->() const;

       private:
        const ArchiveReader* reader_;
        size_t index_;
        mutable std::optional<ItemInfo> current_;
    };

    /// @brief 开始迭代器
    Iterator begin() const;

    /// @brief 结束迭代器
    Iterator end() const;

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace sevenzip
