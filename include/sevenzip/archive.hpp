// archive.hpp - 现代C++ API主入口
#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sevenzip {

// 前向声明
class ItemIterator;
class Archive;

// ============================================================================
// 枚举类型
// ============================================================================

/// 压缩级别
enum class CompressionLevel {
    None,     ///< 不压缩（仅存储）
    Fast,     ///< 快速压缩
    Normal,   ///< 标准压缩
    Maximum,  ///< 最大压缩
    Ultra     ///< 极限压缩
};

/// 归档格式
enum class Format {
    Auto,      ///< 自动检测（仅用于打开）
    SevenZip,  ///< 7z格式
    Zip,       ///< ZIP格式
    Tar,       ///< TAR格式
    GZip,      ///< GZIP格式
    BZip2,     ///< BZIP2格式
    Xz         ///< XZ格式
};

// ============================================================================
// 回调类型
// ============================================================================

/// 进度回调函数
/// @param completed 已完成的字节数
/// @param total 总字节数
/// @return 返回true继续，false取消操作
using ProgressCallback = std::function<bool(uint64_t completed, uint64_t total)>;

/// 密码回调函数
/// @return 返回密码，或空字符串取消
using PasswordCallback = std::function<std::string()>;

// ============================================================================
// 信息结构
// ============================================================================

/// 归档信息
struct ArchiveInfo {
    Format format;                       ///< 归档格式
    size_t itemCount{0};                 ///< 项目数量
    uint64_t totalSize{0};               ///< 解压后总大小
    uint64_t packedSize{0};              ///< 压缩后大小
    bool isSolid{false};                 ///< 是否固实压缩
    bool isMultiVolume{false};           ///< 是否分卷
    bool hasEncryptedHeaders{false};     ///< 是否加密头
    std::optional<std::string> comment;  ///< 注释
};

/// 项目信息
struct ItemInfo {
    size_t index{0};                                         ///< 索引
    std::filesystem::path path;                              ///< 路径
    uint64_t size{0};                                        ///< 解压后大小
    uint64_t packedSize{0};                                  ///< 压缩后大小
    std::optional<uint32_t> crc;                             ///< CRC32校验值
    std::chrono::system_clock::time_point creationTime;      ///< 创建时间
    std::chrono::system_clock::time_point modificationTime;  ///< 修改时间
    bool isDirectory{false};                                 ///< 是否目录
    bool isEncrypted{false};                                 ///< 是否加密
};

// ============================================================================
// Archive类 - 主API入口
// ============================================================================

/// 归档操作主类
///
/// 提供现代C++风格的7-Zip归档操作接口，支持链式调用和RAII资源管理。
///
/// 基本用法：
/// @code
/// // 创建归档
/// Archive::create("output.7z")
///     .addFile("file.txt")
///     .withCompressionLevel(CompressionLevel::Maximum)
///     .finalize();
///
/// // 打开并解压
/// Archive::open("archive.7z")
///     .extractAll("output/");
///
/// // 遍历归档
/// for (const auto& item : Archive::open("archive.7z")) {
///     std::cout << item.path << "\n";
/// }
/// @endcode
class Archive {
   public:
    // ========================================================================
    // 静态工厂方法
    // ========================================================================

    /// 创建新归档到文件
    /// @param path 归档文件路径
    /// @param format 归档格式，默认7z
    /// @return Archive对象，可进行链式调用
    /// @throws Exception 如果无法创建文件
    static Archive create(const std::filesystem::path& path, Format format = Format::SevenZip);

    /// 创建归档到内存缓冲区
    /// @param buffer 输出缓冲区（会被追加数据）
    /// @param format 归档格式，默认7z
    /// @return Archive对象，可进行链式调用
    static Archive createToMemory(std::vector<uint8_t>& buffer, Format format = Format::SevenZip);

    /// 打开现有归档文件
    /// @param path 归档文件路径
    /// @return Archive对象，可进行解压等操作
    /// @throws Exception 如果文件不存在或格式不支持
    static Archive open(const std::filesystem::path& path);

    /// 从内存缓冲区打开归档
    /// @param buffer 包含归档数据的缓冲区
    /// @return Archive对象，可进行解压等操作
    /// @throws Exception 如果数据无效
    static Archive openFromMemory(const std::vector<uint8_t>& buffer);

    // ========================================================================
    // 构造/析构/移动
    // ========================================================================

    /// 默认构造（仅供内部使用）
    Archive();

    /// 析构函数
    ~Archive();

    /// 移动构造
    Archive(Archive&& other) noexcept;

    /// 移动赋值
    Archive& operator=(Archive&& other) noexcept;

    /// 禁止拷贝
    Archive(const Archive&) = delete;
    Archive& operator=(const Archive&) = delete;

    // ========================================================================
    // 压缩操作 - 链式调用
    // ========================================================================

    /// 添加文件到归档
    /// @param path 文件路径
    /// @return *this 用于链式调用
    /// @throws Exception 如果文件不存在
    Archive& addFile(const std::filesystem::path& path);

    /// 添加文件到归档（指定归档内路径）
    /// @param path 文件路径
    /// @param archiveName 归档内的路径
    /// @return *this 用于链式调用
    /// @throws Exception 如果文件不存在
    Archive& addFile(const std::filesystem::path& path, const std::filesystem::path& archiveName);

    /// 添加目录到归档
    /// @param path 目录路径
    /// @param recursive 是否递归添加子目录，默认true
    /// @return *this 用于链式调用
    /// @throws Exception 如果目录不存在
    Archive& addDirectory(const std::filesystem::path& path, bool recursive = true);

    /// 从内存添加文件到归档
    /// @param data 文件数据
    /// @param name 归档内的文件名
    /// @return *this 用于链式调用
    Archive& addFromMemory(const std::vector<uint8_t>& data, const std::filesystem::path& name);

    // ========================================================================
    // 压缩配置 - 链式调用
    // ========================================================================

    /// 设置压缩级别
    /// @param level 压缩级别
    /// @return *this 用于链式调用
    Archive& withCompressionLevel(CompressionLevel level);

    /// 设置密码
    /// @param password 密码（UTF-8编码）
    /// @return *this 用于链式调用
    Archive& withPassword(const std::string& password);

    /// 设置是否加密头
    /// @param encrypt 是否加密，默认true
    /// @return *this 用于链式调用
    /// @note 仅7z格式支持
    Archive& withEncryptedHeaders(bool encrypt = true);

    /// 设置固实模式
    /// @param solid 是否使用固实压缩，默认true
    /// @return *this 用于链式调用
    /// @note 固实压缩可获得更好压缩比，但不利于随机访问
    Archive& withSolidMode(bool solid = true);

    /// 设置分卷大小
    /// @param volumeSize 每卷大小（字节）
    /// @return *this 用于链式调用
    /// @note 仅7z和ZIP格式支持
    Archive& withMultiVolume(uint64_t volumeSize);

    /// 设置进度回调
    /// @param callback 进度回调函数
    /// @return *this 用于链式调用
    Archive& withProgress(ProgressCallback callback);

    /// 完成压缩操作
    /// @throws Exception 如果压缩失败
    void finalize();

    // ========================================================================
    // 解压操作
    // ========================================================================

    /// 解压所有项目到目录
    /// @param destination 目标目录
    /// @throws Exception 如果解压失败
    void extractAll(const std::filesystem::path& destination);

    /// 解压指定项目到目录
    /// @param index 项目索引
    /// @param destination 目标目录
    /// @throws Exception 如果索引无效或解压失败
    void extractItem(size_t index, const std::filesystem::path& destination);

    /// 解压项目到内存
    /// @param index 项目索引
    /// @return 文件数据
    /// @throws Exception 如果索引无效或解压失败
    std::vector<uint8_t> extractItemToMemory(size_t index);

    // ========================================================================
    // 查询操作
    // ========================================================================

    /// 获取归档信息
    /// @return 归档信息结构
    /// @throws Exception 如果归档未打开
    ArchiveInfo info() const;

    /// 获取项目数量
    /// @return 项目数量
    /// @throws Exception 如果归档未打开
    size_t itemCount() const;

    /// 获取项目信息
    /// @param index 项目索引
    /// @return 项目信息结构
    /// @throws Exception 如果索引无效
    ItemInfo itemInfo(size_t index) const;

    // ========================================================================
    // 迭代器支持
    // ========================================================================

    /// 获取开始迭代器
    ItemIterator begin() const;

    /// 获取结束迭代器
    ItemIterator end() const;

    // ========================================================================
    // 其他操作
    // ========================================================================

    /// 测试归档完整性
    /// @return 如果归档完整返回true
    bool test() const;

    /// 检查是否已打开
    /// @return 如果已打开返回true
    bool isOpen() const;

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// ============================================================================
// ItemIterator类 - 支持范围for
// ============================================================================

/// 归档项目迭代器
///
/// 支持标准迭代器接口和范围for循环
class ItemIterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = ItemInfo;
    using difference_type = std::ptrdiff_t;
    using pointer = const ItemInfo*;
    using reference = const ItemInfo&;

    /// 构造迭代器
    ItemIterator(const Archive* archive, size_t index);

    /// 前置递增
    ItemIterator& operator++();

    /// 后置递增
    ItemIterator operator++(int);

    /// 解引用
    reference operator*() const;

    /// 成员访问
    pointer operator->() const;

    /// 相等比较
    bool operator==(const ItemIterator& other) const;

    /// 不等比较
    bool operator!=(const ItemIterator& other) const;

   private:
    const Archive* archive_;
    size_t index_;
    mutable std::optional<ItemInfo> current_;
};

}  // namespace sevenzip
