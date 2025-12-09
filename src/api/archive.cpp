// archive.cpp - Archive类实现
#define NOMINMAX  // Prevent Windows min/max macros

#include "sevenzip/archive.hpp"

#include <algorithm>
#include <stdexcept>

#include "../wrapper/archive/archive_format.hpp"
#include "../wrapper/archive/archive_reader.hpp"
#include "../wrapper/archive/archive_writer.hpp"
#include "../wrapper/common/wrapper_error.hpp"
#include "../wrapper/common/wrapper_string.hpp"
#include "../wrapper/stream/stream_memory.hpp"
#include "Common/MyCom.h"

namespace sevenzip {

namespace {

// 转换Format到detail::ArchiveFormat
ArchiveFormat toDetailFormat(Format format) {
    switch (format) {
        case Format::SevenZip:
            return ArchiveFormat::SevenZip;
        case Format::Zip:
            return ArchiveFormat::Zip;
        case Format::Tar:
            return ArchiveFormat::Tar;
        case Format::GZip:
            return ArchiveFormat::GZip;
        case Format::BZip2:
            return ArchiveFormat::BZip2;
        case Format::Xz:
            return ArchiveFormat::Xz;
        default:
            throw Exception(ErrorCode::InvalidArgument, "Invalid format");
    }
}

// 转换ArchiveFormat到Format
Format fromDetailFormat(ArchiveFormat format) {
    switch (format) {
        case ArchiveFormat::SevenZip:
            return Format::SevenZip;
        case ArchiveFormat::Zip:
            return Format::Zip;
        case ArchiveFormat::Tar:
            return Format::Tar;
        case ArchiveFormat::GZip:
            return Format::GZip;
        case ArchiveFormat::BZip2:
            return Format::BZip2;
        case ArchiveFormat::Xz:
            return Format::Xz;
        default:
            return Format::Auto;
    }
}

// 转换CompressionLevel到detail::CompressionLevel
detail::CompressionLevel toDetailLevel(CompressionLevel level) {
    switch (level) {
        case CompressionLevel::None:
            return detail::CompressionLevel::None;
        case CompressionLevel::Fast:
            return detail::CompressionLevel::Fast;
        case CompressionLevel::Normal:
            return detail::CompressionLevel::Normal;
        case CompressionLevel::Maximum:
            return detail::CompressionLevel::Maximum;
        case CompressionLevel::Ultra:
            return detail::CompressionLevel::Ultra;
        default:
            return detail::CompressionLevel::Normal;
    }
}

// 转换时间�?
std::chrono::system_clock::time_point convertFileTime(uint64_t fileTime) {
    if (fileTime == 0) {
        return std::chrono::system_clock::time_point{};
    }

    // FILETIME是从1601-01-01开始的100纳秒计数
    // Unix时间戳是�?970-01-01开始的秒数
    const uint64_t TICKS_PER_SECOND = 10000000;
    const uint64_t EPOCH_DIFFERENCE = 11644473600ULL;  // 1601�?970的秒�?

    uint64_t seconds = fileTime / TICKS_PER_SECOND - EPOCH_DIFFERENCE;
    return std::chrono::system_clock::from_time_t(static_cast<time_t>(seconds));
}

}  // anonymous namespace

// ============================================================================
// 注意：Exception在wrapper_error.hpp中实现，这里不需要重复实现
// ============================================================================

// ============================================================================
// Archive::Impl - PIMPL实现
// ============================================================================

class Archive::Impl {
   public:
    enum class Mode { None, Create, Open };

    Mode mode{Mode::None};

    // 创建模式
    std::unique_ptr<detail::ArchiveWriter> writer;
    detail::ArchiveProperties properties;
    ProgressCallback progressCallback;

    // 打开模式
    std::unique_ptr<detail::ArchiveReader> reader;
    CMyComPtr<::IInStream> memoryStream;  // Keep memory stream alive with COM reference counting
    std::vector<uint8_t> memoryBuffer;    // Keep buffer data alive for memory streams

    // 检查模�?
    void ensureCreateMode() const {
        if (mode != Mode::Create) {
            throw Exception(ErrorCode::InvalidHandle, "Archive not in create mode");
        }
    }

    void ensureOpenMode() const {
        if (mode != Mode::Open) {
            throw Exception(ErrorCode::InvalidHandle, "Archive not in open mode");
        }
    }
};

// ============================================================================
// Archive - 构�?析构/移动
// ============================================================================

Archive::Archive() : impl_(std::make_unique<Impl>()) {}

Archive::~Archive() = default;

Archive::Archive(Archive&& other) noexcept = default;

Archive& Archive::operator=(Archive&& other) noexcept = default;

// ============================================================================
// Archive - 静态工厂方�?
// ============================================================================

Archive Archive::create(const std::filesystem::path& path, Format format) {
    Archive archive;
    archive.impl_->mode = Impl::Mode::Create;
    archive.impl_->writer = std::make_unique<detail::ArchiveWriter>();

    try {
        archive.impl_->writer->create(path.wstring(), toDetailFormat(format));
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to create archive: ") + e.what());
    }

    return archive;
}

Archive Archive::createToMemory(std::vector<uint8_t>& buffer, Format format) {
    Archive archive;
    archive.impl_->mode = Impl::Mode::Create;
    archive.impl_->writer = std::make_unique<detail::ArchiveWriter>();

    try {
        archive.impl_->writer->createToMemory(buffer, toDetailFormat(format));
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to create archive: ") + e.what());
    }

    return archive;
}

Archive Archive::open(const std::filesystem::path& path) {
    Archive archive;
    archive.impl_->mode = Impl::Mode::Open;
    archive.impl_->reader = std::make_unique<detail::ArchiveReader>();

    try {
        archive.impl_->reader->open(path.wstring());
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to open archive: ") + e.what());
    }

    return archive;
}

Archive Archive::openFromMemory(const std::vector<uint8_t>& buffer) {
    Archive archive;
    archive.impl_->mode = Impl::Mode::Open;
    archive.impl_->reader = std::make_unique<detail::ArchiveReader>();

    try {
        // Copy buffer to keep data alive
        archive.impl_->memoryBuffer = buffer;

        // Create memory stream from our copy and store in CMyComPtr
        detail::MemoryInStream* rawStream = new detail::MemoryInStream(archive.impl_->memoryBuffer);
        archive.impl_->memoryStream = rawStream;  // CMyComPtr will AddRef

        // Pass the raw pointer to openFromStream
        // Use SevenZip as default format (for auto-detection, use the overload)
        // ArchiveReader will also AddRef, so stream will have ref count >= 2
        archive.impl_->reader->openFromStream(rawStream, ArchiveFormat::SevenZip);
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to open archive from memory: ") + e.what());
    }

    return archive;
}

Archive Archive::openFromMemory(const std::vector<uint8_t>& buffer, Format format) {
    Archive archive;
    archive.impl_->mode = Impl::Mode::Open;
    archive.impl_->reader = std::make_unique<detail::ArchiveReader>();

    try {
        // Copy buffer to keep data alive
        archive.impl_->memoryBuffer = buffer;

        // Create memory stream from our copy and store in CMyComPtr
        detail::MemoryInStream* rawStream = new detail::MemoryInStream(archive.impl_->memoryBuffer);
        archive.impl_->memoryStream = rawStream;  // CMyComPtr will AddRef

        // Pass the raw pointer to openFromStream with specified format
        // ArchiveReader will also AddRef, so stream will have ref count >= 2
        archive.impl_->reader->openFromStream(rawStream, toDetailFormat(format));
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to open archive from memory: ") + e.what());
    }

    return archive;
}

// ============================================================================
// Archive - 压缩操作
// ============================================================================

Archive& Archive::addFile(const std::filesystem::path& path) {
    return addFile(path, path.filename());
}

Archive& Archive::addFile(const std::filesystem::path& path,
                          const std::filesystem::path& archiveName) {
    impl_->ensureCreateMode();

    try {
        impl_->writer->addFile(path.wstring(), archiveName.wstring());
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to add file: ") + e.what());
    }

    return *this;
}

Archive& Archive::addDirectory(const std::filesystem::path& path, bool recursive) {
    impl_->ensureCreateMode();

    if (!std::filesystem::exists(path)) {
        throw Exception(ErrorCode::FileNotFound, "Directory not found: " + path.string());
    }

    if (!std::filesystem::is_directory(path)) {
        throw Exception(ErrorCode::InvalidArgument, "Not a directory: " + path.string());
    }

    try {
        // 添加整个目录到归�?
        // archivePath使用目录名本�?
        impl_->writer->addDirectory(path.wstring(), path.filename().wstring(), recursive);
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to add directory: ") + e.what());
    } catch (const std::exception& e) {
        throw Exception(ErrorCode::Unknown, std::string("Failed to add directory: ") + e.what());
    }

    return *this;
}

Archive& Archive::addFromMemory(const std::vector<uint8_t>& data,
                                const std::filesystem::path& name) {
    impl_->ensureCreateMode();

    try {
        impl_->writer->addFileFromMemory(data, name.wstring());
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to add from memory: ") + e.what());
    }

    return *this;
}

// ============================================================================
// Archive - 压缩配置
// ============================================================================

Archive& Archive::withCompressionLevel(CompressionLevel level) {
    impl_->ensureCreateMode();
    impl_->properties.level = toDetailLevel(level);
    return *this;
}

Archive& Archive::withPassword(const std::string& password) {
    // 支持两种模式:创建模式和打开模式
    if (impl_->mode == Impl::Mode::Create) {
        impl_->properties.password = detail::utf8_to_wstring(password);
    } else if (impl_->mode == Impl::Mode::Open) {
        // 打开模式:设置密码回调用于解密
        if (impl_->reader) {
            impl_->reader->setPasswordCallback(
                [password]() { return detail::utf8_to_wstring(password); });
        }
    } else {
        throw Exception(ErrorCode::InvalidHandle, "Archive not initialized");
    }
    return *this;
}

Archive& Archive::withEncryptedHeaders(bool encrypt) {
    impl_->ensureCreateMode();
    impl_->properties.encryptHeaders = encrypt;
    return *this;
}

Archive& Archive::withSolidMode(bool solid) {
    impl_->ensureCreateMode();
    impl_->properties.solid = solid;
    return *this;
}

Archive& Archive::withMultiVolume(uint64_t volumeSize) {
    impl_->ensureCreateMode();
    impl_->properties.volumeSize = volumeSize;
    return *this;
}

Archive& Archive::withProgress(ProgressCallback callback) {
    impl_->ensureCreateMode();
    impl_->progressCallback = std::move(callback);

    // 设置底层回调
    if (impl_->progressCallback) {
        impl_->writer->setProgressCallback([this](uint64_t completed, uint64_t total) {
            return impl_->progressCallback(completed, total);
        });
    }

    return *this;
}

void Archive::finalize() {
    impl_->ensureCreateMode();

    try {
        // 设置属�?
        impl_->writer->setProperties(impl_->properties);

        // 完成压缩
        impl_->writer->finalize();
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to finalize archive: ") + e.what());
    }
}

// ============================================================================
// Archive - 解压操作
// ============================================================================

void Archive::extractAll(const std::filesystem::path& destination) {
    impl_->ensureOpenMode();

    try {
        impl_->reader->extractAll(destination.wstring());
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to extract: ") + e.what());
    }
}

void Archive::extractItem(size_t index, const std::filesystem::path& destination) {
    impl_->ensureOpenMode();

    try {
        // Extract single item to the destination directory
        // The extractItems method will create the file with its original name in the destination
        impl_->reader->extractItems({static_cast<uint32_t>(index)}, destination.wstring());
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to extract item: ") + e.what());
    }
}

std::vector<uint8_t> Archive::extractItemToMemory(size_t index) {
    impl_->ensureOpenMode();

    try {
        return impl_->reader->extractToMemory(static_cast<uint32_t>(index));
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to extract to memory: ") + e.what());
    }
}

// ============================================================================
// Archive - 查询操作
// ============================================================================

ArchiveInfo Archive::info() const {
    impl_->ensureOpenMode();

    try {
        auto detailInfo = impl_->reader->getArchiveInfo();

        ArchiveInfo info;
        info.format = fromDetailFormat(detailInfo.format);
        info.itemCount = detailInfo.itemCount;
        info.totalSize = 0;  // TODO: 计算总大�?
        info.packedSize = detailInfo.physicalSize;
        info.isSolid = detailInfo.isSolid;
        info.isMultiVolume = detailInfo.isMultiVolume;
        info.hasEncryptedHeaders = detailInfo.hasEncryptedHeader;

        return info;
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to get archive info: ") + e.what());
    }
}

size_t Archive::itemCount() const {
    impl_->ensureOpenMode();

    try {
        return impl_->reader->getItemCount();
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to get item count: ") + e.what());
    }
}

ItemInfo Archive::itemInfo(size_t index) const {
    impl_->ensureOpenMode();

    try {
        auto detailInfo = impl_->reader->getItemInfo(static_cast<uint32_t>(index));

        ItemInfo info;
        info.index = detailInfo.index;
        info.path = detailInfo.path;
        info.size = detailInfo.size;
        info.packedSize = detailInfo.packedSize;

        if (detailInfo.crc != 0) {
            info.crc = detailInfo.crc;
        }

        if (detailInfo.creationTime.has_value()) {
            info.creationTime = convertFileTime(detailInfo.creationTime.value());
        }
        if (detailInfo.lastWriteTime.has_value()) {
            info.modificationTime = convertFileTime(detailInfo.lastWriteTime.value());
        }
        info.isDirectory = detailInfo.isDirectory;
        info.isEncrypted = false;  // TODO: 检测加�?

        return info;
    } catch (const Exception& e) {
        throw Exception(e.code(), std::string("Failed to get item info: ") + e.what());
    }
}

// ============================================================================
// Archive - 迭代�?
// ============================================================================

ItemIterator Archive::begin() const {
    return ItemIterator(this, 0);
}

ItemIterator Archive::end() const {
    return ItemIterator(this, itemCount());
}

// ============================================================================
// Archive - 其他操作
// ============================================================================

bool Archive::test() const {
    impl_->ensureOpenMode();

    // TODO: ArchiveReader doesn't have test() method yet
    // For now, just check if we can get item count
    try {
        impl_->reader->getItemCount();
        return true;
    } catch (const Exception&) {
        return false;
    }
}

bool Archive::isOpen() const {
    if (impl_->mode == Impl::Mode::Open && impl_->reader) {
        return impl_->reader->isOpen();
    }
    return impl_->mode != Impl::Mode::None;
}

// ============================================================================
// ItemIterator - 实现
// ============================================================================

ItemIterator::ItemIterator(const Archive* archive, size_t index)
    : archive_(archive), index_(index) {}

ItemIterator& ItemIterator::operator++() {
    ++index_;
    current_.reset();
    return *this;
}

ItemIterator ItemIterator::operator++(int) {
    ItemIterator tmp = *this;
    ++(*this);
    return tmp;
}

ItemIterator::reference ItemIterator::operator*() const {
    if (!current_) {
        current_ = archive_->itemInfo(index_);
    }
    return *current_;
}

ItemIterator::pointer ItemIterator::operator->() const {
    if (!current_) {
        current_ = archive_->itemInfo(index_);
    }
    return &(*current_);
}

bool ItemIterator::operator==(const ItemIterator& other) const {
    return archive_ == other.archive_ && index_ == other.index_;
}

bool ItemIterator::operator!=(const ItemIterator& other) const {
    return !(*this == other);
}

}  // namespace sevenzip
