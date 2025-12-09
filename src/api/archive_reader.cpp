#include "sevenzip/archive_reader.hpp"

namespace sevenzip {

// ============================================================================
// ArchiveReader::Impl
// ============================================================================

class ArchiveReader::Impl {
   public:
    Archive archive;

    explicit Impl(const std::filesystem::path& path) : archive(Archive::open(path)) {}

    explicit Impl(const std::vector<uint8_t>& buffer) : archive(Archive::openFromMemory(buffer)) {}
};

// ============================================================================
// ArchiveReader
// ============================================================================

ArchiveReader::ArchiveReader(const std::filesystem::path& path)
    : impl_(std::make_unique<Impl>(path)) {}

ArchiveReader::ArchiveReader(const std::vector<uint8_t>& buffer)
    : impl_(std::make_unique<Impl>(buffer)) {}

ArchiveReader::~ArchiveReader() = default;

ArchiveReader::ArchiveReader(ArchiveReader&& other) noexcept = default;
ArchiveReader& ArchiveReader::operator=(ArchiveReader&& other) noexcept = default;

ArchiveInfo ArchiveReader::info() const {
    return impl_->archive.info();
}

size_t ArchiveReader::itemCount() const {
    return impl_->archive.itemCount();
}

ItemInfo ArchiveReader::itemInfo(size_t index) const {
    return impl_->archive.itemInfo(static_cast<uint32_t>(index));
}

std::optional<ItemInfo> ArchiveReader::findItem(const std::filesystem::path& path) const {
    for (size_t i = 0; i < itemCount(); ++i) {
        auto info = itemInfo(i);
        if (info.path == path) {
            return info;
        }
    }
    return std::nullopt;
}

bool ArchiveReader::contains(const std::filesystem::path& path) const {
    return findItem(path).has_value();
}

std::vector<uint8_t> ArchiveReader::extract(size_t index) const {
    return impl_->archive.extractItemToMemory(static_cast<uint32_t>(index));
}

void ArchiveReader::extractTo(size_t index, const std::filesystem::path& destPath) const {
    impl_->archive.extractItem(static_cast<uint32_t>(index), destPath);
}

void ArchiveReader::extractAll(const std::filesystem::path& destDir) const {
    impl_->archive.extractAll(destDir);
}

void ArchiveReader::extractItems(const std::vector<size_t>& indices,
                                 const std::filesystem::path& destDir) const {
    for (auto idx : indices) {
        impl_->archive.extractItem(static_cast<uint32_t>(idx), destDir);
    }
}

bool ArchiveReader::test() const {
    return impl_->archive.test();
}

ArchiveReader& ArchiveReader::withPassword(const std::string& password) {
    impl_->archive.withPassword(password);
    return *this;
}

ArchiveReader& ArchiveReader::withProgress(ProgressCallback callback) {
    // ArchiveReader uses Archive in Open mode, so we can't use Archive::withProgress
    // Instead, we need to set the callback directly on the reader
    // For now, this is a no-op since the underlying reader doesn't expose progress callbacks
    // TODO: Implement progress callback support in Archive's Open mode
    (void)callback;  // Suppress unused parameter warning
    return *this;
}

// ============================================================================
// ArchiveReader::Iterator
// ============================================================================

ArchiveReader::Iterator::Iterator(const ArchiveReader* reader, size_t index)
    : reader_(reader), index_(index) {}

ArchiveReader::Iterator& ArchiveReader::Iterator::operator++() {
    ++index_;
    current_.reset();
    return *this;
}

ArchiveReader::Iterator ArchiveReader::Iterator::operator++(int) {
    Iterator temp = *this;
    ++(*this);
    return temp;
}

bool ArchiveReader::Iterator::operator==(const Iterator& other) const {
    return reader_ == other.reader_ && index_ == other.index_;
}

bool ArchiveReader::Iterator::operator!=(const Iterator& other) const {
    return !(*this == other);
}

auto ArchiveReader::Iterator::operator*() const -> reference {
    if (!current_) {
        current_ = reader_->itemInfo(index_);
    }
    return *current_;
}

auto ArchiveReader::Iterator::operator->() const -> pointer {
    if (!current_) {
        current_ = reader_->itemInfo(index_);
    }
    return &(*current_);
}

ArchiveReader::Iterator ArchiveReader::begin() const {
    return Iterator(this, 0);
}

ArchiveReader::Iterator ArchiveReader::end() const {
    return Iterator(this, itemCount());
}

}  // namespace sevenzip
