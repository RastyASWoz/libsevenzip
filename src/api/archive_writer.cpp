#include "sevenzip/archive_writer.hpp"

namespace sevenzip {

// ============================================================================
// ArchiveWriter::Impl
// ============================================================================

class ArchiveWriter::Impl {
   public:
    Archive archive;
    size_t itemCount{0};

    explicit Impl(const std::filesystem::path& path, Format format)
        : archive(Archive::create(path, format)) {}

    Impl(std::vector<uint8_t>& buffer, Format format)
        : archive(Archive::createToMemory(buffer, format)) {}
};

// ============================================================================
// ArchiveWriter
// ============================================================================

ArchiveWriter::ArchiveWriter() = default;

ArchiveWriter ArchiveWriter::create(const std::filesystem::path& path, Format format) {
    ArchiveWriter writer;
    writer.impl_ = std::make_unique<Impl>(path, format);
    return writer;
}

ArchiveWriter ArchiveWriter::createToMemory(std::vector<uint8_t>& buffer, Format format) {
    ArchiveWriter writer;
    writer.impl_ = std::make_unique<Impl>(buffer, format);
    return writer;
}

ArchiveWriter::~ArchiveWriter() = default;

ArchiveWriter::ArchiveWriter(ArchiveWriter&& other) noexcept = default;
ArchiveWriter& ArchiveWriter::operator=(ArchiveWriter&& other) noexcept = default;

ArchiveWriter& ArchiveWriter::withLevel(CompressionLevel level) {
    impl_->archive.withCompressionLevel(level);
    return *this;
}

ArchiveWriter& ArchiveWriter::withPassword(const std::string& password) {
    impl_->archive.withPassword(password);
    return *this;
}

ArchiveWriter& ArchiveWriter::withEncryptedHeaders(bool encrypt) {
    impl_->archive.withEncryptedHeaders(encrypt);
    return *this;
}

ArchiveWriter& ArchiveWriter::withSolidMode(bool solid) {
    impl_->archive.withSolidMode(solid);
    return *this;
}

ArchiveWriter& ArchiveWriter::withMultiVolume(uint64_t volumeSize) {
    impl_->archive.withMultiVolume(volumeSize);
    return *this;
}

ArchiveWriter& ArchiveWriter::withProgress(ProgressCallback callback) {
    impl_->archive.withProgress(callback);
    return *this;
}

ArchiveWriter& ArchiveWriter::addFile(const std::filesystem::path& path) {
    impl_->archive.addFile(path);
    ++impl_->itemCount;
    return *this;
}

ArchiveWriter& ArchiveWriter::addFile(const std::filesystem::path& path,
                                      const std::filesystem::path& archiveName) {
    impl_->archive.addFile(path, archiveName);
    ++impl_->itemCount;
    return *this;
}

ArchiveWriter& ArchiveWriter::addDirectory(const std::filesystem::path& path, bool recursive) {
    impl_->archive.addDirectory(path, recursive);
    ++impl_->itemCount;
    return *this;
}

ArchiveWriter& ArchiveWriter::addFromMemory(const std::vector<uint8_t>& data,
                                            const std::filesystem::path& name) {
    impl_->archive.addFromMemory(data, name);
    ++impl_->itemCount;
    return *this;
}

ArchiveWriter& ArchiveWriter::addFiles(const std::vector<std::filesystem::path>& paths) {
    for (const auto& path : paths) {
        addFile(path);
    }
    return *this;
}

void ArchiveWriter::finalize() {
    impl_->archive.finalize();
}

size_t ArchiveWriter::pendingCount() const {
    return impl_->itemCount;
}

}  // namespace sevenzip
