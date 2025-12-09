#include "sevenzip/convenience.hpp"

#include <filesystem>
#include <fstream>

#include "../wrapper/common/wrapper_error.hpp"

namespace sevenzip {

namespace fs = std::filesystem;

void extract(const fs::path& archivePath, const fs::path& destDir) {
    auto archive = Archive::open(archivePath);
    archive.extractAll(destDir);
}

void extract(const fs::path& archivePath, const fs::path& destDir, const std::string& password) {
    auto archive = Archive::open(archivePath);
    archive.withPassword(password);
    archive.extractAll(destDir);
}

void extract(const fs::path& archivePath, const fs::path& destDir, Format format,
             const std::string& password) {
    // Note: Format parameter is ignored since Archive::open auto-detects format
    auto archive = Archive::open(archivePath);
    if (!password.empty()) {
        archive.withPassword(password);
    }
    archive.extractAll(destDir);
}

void compress(const fs::path& sourcePath, const fs::path& archivePath) {
    compress(sourcePath, archivePath, Format::SevenZip, CompressionLevel::Normal);
}

void compress(const fs::path& sourcePath, const fs::path& archivePath, Format format) {
    compress(sourcePath, archivePath, format, CompressionLevel::Normal);
}

void compress(const fs::path& sourcePath, const fs::path& archivePath, Format format,
              CompressionLevel level) {
    if (!fs::exists(sourcePath)) {
        throw Exception(ErrorCode::FileNotFound,
                        "Source path does not exist: " + sourcePath.string());
    }

    auto archive = Archive::create(archivePath, format);
    archive.withCompressionLevel(level);

    if (fs::is_directory(sourcePath)) {
        archive.addDirectory(sourcePath, true);  // recursive
    } else if (fs::is_regular_file(sourcePath)) {
        archive.addFile(sourcePath);
    } else {
        throw Exception(ErrorCode::InvalidArgument, "Source path must be a file or directory");
    }

    archive.finalize();
}

void compress(const fs::path& sourcePath, const fs::path& archivePath, Format format,
              CompressionLevel level, const std::string& password) {
    if (!fs::exists(sourcePath)) {
        throw Exception(ErrorCode::FileNotFound,
                        "Source path does not exist: " + sourcePath.string());
    }

    auto archive = Archive::create(archivePath, format);
    archive.withCompressionLevel(level);
    archive.withPassword(password);

    if (fs::is_directory(sourcePath)) {
        archive.addDirectory(sourcePath, true);  // recursive
    } else if (fs::is_regular_file(sourcePath)) {
        archive.addFile(sourcePath);
    } else {
        throw Exception(ErrorCode::InvalidArgument, "Source path must be a file or directory");
    }

    archive.finalize();
}

std::vector<uint8_t> compressData(const std::vector<uint8_t>& data, Format format,
                                  CompressionLevel level) {
    std::vector<uint8_t> buffer;
    auto archive = Archive::createToMemory(buffer, format);
    archive.withCompressionLevel(level);
    archive.addFromMemory(data, "data");
    archive.finalize();
    return buffer;
}

std::vector<uint8_t> extractSingleFile(const fs::path& archivePath) {
    auto archive = Archive::open(archivePath);

    if (archive.itemCount() == 0) {
        throw Exception(ErrorCode::InvalidArgument, "Archive is empty");
    }

    if (archive.itemCount() > 1) {
        throw Exception(ErrorCode::InvalidArgument,
                        "Archive contains multiple files, use extractSingleFile with index");
    }

    return archive.extractItemToMemory(0);
}

std::vector<uint8_t> extractSingleFile(const fs::path& archivePath, uint32_t itemIndex) {
    auto archive = Archive::open(archivePath);

    if (itemIndex >= archive.itemCount()) {
        throw Exception(ErrorCode::InvalidArgument, "Item index out of range");
    }

    return archive.extractItemToMemory(itemIndex);
}

std::vector<ItemInfo> list(const fs::path& archivePath) {
    auto archive = Archive::open(archivePath);
    std::vector<ItemInfo> items;

    for (uint32_t i = 0; i < archive.itemCount(); ++i) {
        items.push_back(archive.itemInfo(i));
    }

    return items;
}

std::vector<ItemInfo> list(const fs::path& archivePath, const std::string& password) {
    auto archive = Archive::open(archivePath);
    archive.withPassword(password);

    std::vector<ItemInfo> items;
    for (uint32_t i = 0; i < archive.itemCount(); ++i) {
        items.push_back(archive.itemInfo(i));
    }

    return items;
}

bool testArchive(const fs::path& archivePath) {
    try {
        auto archive = Archive::open(archivePath);
        return archive.test();
    } catch (...) {
        return false;
    }
}

bool testArchive(const fs::path& archivePath, const std::string& password) {
    try {
        auto archive = Archive::open(archivePath);
        archive.withPassword(password);
        return archive.test();
    } catch (...) {
        return false;
    }
}

ArchiveInfo getArchiveInfo(const fs::path& archivePath) {
    auto archive = Archive::open(archivePath);
    return archive.info();
}

bool isArchive(const fs::path& filePath) {
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        return false;
    }

    try {
        auto archive = Archive::open(filePath);
        return archive.itemCount() >= 0;  // If we can open and count, it's valid
    } catch (...) {
        return false;
    }
}

}  // namespace sevenzip
