// archive_writer.cpp - Archive writer implementation
#include "archive_writer.hpp"

#include <Common/IntToString.h>
#include <Common/MyCom.h>
#include <Common/StringConvert.h>
#include <Windows.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../common/wrapper_string.hpp"
#include "../stream/stream_file.hpp"
#include "../stream/stream_memory.hpp"
#include "archive_format.hpp"
#include "archive_init.hpp"
#include "update_callback.hpp"

// External function from 7-Zip library
extern "C" {
STDAPI CreateArchiver(const GUID* clsid, const GUID* iid, void** outObject);
}

namespace sevenzip::detail {

// Helper: Get format CLSID
static GUID GetFormatCLSID(ArchiveFormat format) {
    // 7-Zip base CLSID: {23170F69-40C1-278A-1000-000110xx0000}
    // xx is format ID
    GUID base = {0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00}};

    // Set format ID
    switch (format) {
        case ArchiveFormat::SevenZip:
            base.Data4[5] = 0x07;  // '07'
            break;
        case ArchiveFormat::Zip:
            base.Data4[5] = 0x01;  // '01'
            break;
        case ArchiveFormat::Tar:
            base.Data4[5] = 0xEE;  // 'ee'
            break;
        case ArchiveFormat::GZip:
            base.Data4[5] = 0xEF;  // 'ef'
            break;
        case ArchiveFormat::BZip2:
            base.Data4[5] = 0x02;  // '02'
            break;
        case ArchiveFormat::Xz:
            base.Data4[5] = 0x0C;  // '0c'
            break;
        case ArchiveFormat::Rar:
            base.Data4[5] = 0x03;  // '03'
            break;
        default:
            throw Exception(ErrorCode::UnsupportedFormat, "Unsupported archive format");
    }

    return base;
}

// Pimpl implementation
class ArchiveWriter::Impl {
   public:
    Impl() = default;
    ~Impl() = default;

    void create(const std::wstring& path, ArchiveFormat format);
    void createToMemory(std::vector<uint8_t>& buffer, ArchiveFormat format);
    void createToStream(::IOutStream* stream, ArchiveFormat format);
    void setProperties(const ArchiveProperties& props);
    void close();

    void addFile(const std::wstring& sourcePath, const std::wstring& archivePath);
    void addDirectory(const std::wstring& sourcePath, const std::wstring& archivePath,
                      bool recursive);
    void addFileFromMemory(const std::vector<uint8_t>& data, const std::wstring& archivePath);
    void addEmptyDirectory(const std::wstring& archivePath);
    void addItem(const UpdateItemInfo& item);

    void finalize();
    bool isFinalized() const { return finalized_; }
    uint32_t getItemCount() const { return static_cast<uint32_t>(items_.size()); }

    void setProgressCallback(UpdateProgressCallback callback) {
        progressCallback_ = std::move(callback);
    }

   private:
    // COM objects
    CMyComPtr<IOutArchive> outArchive_;
    CMyComPtr<IOutStream> outStream_;

    // Archive settings
    ArchiveFormat format_ = ArchiveFormat::SevenZip;
    std::optional<ArchiveProperties> properties_;

    // Items to add
    std::vector<UpdateItemInfo> items_;

    // State
    bool finalized_ = false;

    // Progress callback
    UpdateProgressCallback progressCallback_;

    // Helper functions
    void ensureNotFinalized();
    void ensureFormat();
    void applyProperties();
    UpdateItemInfo createFileItem(const std::wstring& sourcePath, const std::wstring& archivePath);
    void addDirectoryRecursive(const fs::path& sourcePath, const std::wstring& archivePath);
};

// ArchiveWriter implementation

ArchiveWriter::ArchiveWriter() : impl_(std::make_unique<Impl>()) {}

ArchiveWriter::~ArchiveWriter() = default;

ArchiveWriter::ArchiveWriter(ArchiveWriter&&) noexcept = default;
ArchiveWriter& ArchiveWriter::operator=(ArchiveWriter&&) noexcept = default;

void ArchiveWriter::create(const std::wstring& path, ArchiveFormat format) {
    impl_->create(path, format);
}

void ArchiveWriter::createToMemory(std::vector<uint8_t>& buffer, ArchiveFormat format) {
    impl_->createToMemory(buffer, format);
}

void ArchiveWriter::createToStream(::IOutStream* stream, ArchiveFormat format) {
    impl_->createToStream(stream, format);
}

void ArchiveWriter::setProperties(const ArchiveProperties& props) {
    impl_->setProperties(props);
}

void ArchiveWriter::close() {
    impl_->close();
}

void ArchiveWriter::addFile(const std::wstring& sourcePath, const std::wstring& archivePath) {
    impl_->addFile(sourcePath, archivePath);
}

void ArchiveWriter::addDirectory(const std::wstring& sourcePath, const std::wstring& archivePath,
                                 bool recursive) {
    impl_->addDirectory(sourcePath, archivePath, recursive);
}

void ArchiveWriter::addFileFromMemory(const std::vector<uint8_t>& data,
                                      const std::wstring& archivePath) {
    impl_->addFileFromMemory(data, archivePath);
}

void ArchiveWriter::addEmptyDirectory(const std::wstring& archivePath) {
    impl_->addEmptyDirectory(archivePath);
}

void ArchiveWriter::addItem(const UpdateItemInfo& item) {
    impl_->addItem(item);
}

void ArchiveWriter::finalize() {
    impl_->finalize();
}

bool ArchiveWriter::isFinalized() const {
    return impl_->isFinalized();
}

uint32_t ArchiveWriter::getItemCount() const {
    return impl_->getItemCount();
}

void ArchiveWriter::setProgressCallback(UpdateProgressCallback callback) {
    impl_->setProgressCallback(std::move(callback));
}

// Impl implementation

void ArchiveWriter::Impl::create(const std::wstring& path, ArchiveFormat format) {
    if (finalized_) {
        throw Exception(ErrorCode::InvalidState, "Archive already finalized");
    }

    format_ = format;

    // Create output stream
    try {
        auto fileStream = new FileOutStream(path);
        if (!fileStream->isOpen()) {
            delete fileStream;
            throw Exception(ErrorCode::CannotOpenFile,
                            "Cannot create output file: " + std::string(path.begin(), path.end()));
        }
        outStream_ = fileStream;
    } catch (...) {
        throw Exception(ErrorCode::CannotOpenFile,
                        "Cannot create output file: " + std::string(path.begin(), path.end()));
    }

    ensureFormat();
}

void ArchiveWriter::Impl::createToMemory(std::vector<uint8_t>& buffer, ArchiveFormat format) {
    if (finalized_) {
        throw Exception(ErrorCode::InvalidState, "Archive already finalized");
    }

    format_ = format;

    // Create memory output stream
    try {
        auto memStream = new MemoryOutStream(buffer);
        outStream_ = memStream;
    } catch (...) {
        throw Exception(ErrorCode::OutOfMemory, "Cannot create memory stream");
    }

    ensureFormat();
}

void ArchiveWriter::Impl::createToStream(::IOutStream* stream, ArchiveFormat format) {
    if (finalized_) {
        throw Exception(ErrorCode::InvalidState, "Archive already finalized");
    }

    format_ = format;
    outStream_ = stream;

    ensureFormat();
}

void ArchiveWriter::Impl::setProperties(const ArchiveProperties& props) {
    ensureNotFinalized();
    properties_ = props;
}

void ArchiveWriter::Impl::close() {
    outArchive_.Release();
    outStream_.Release();
}

void ArchiveWriter::Impl::addFile(const std::wstring& sourcePath, const std::wstring& archivePath) {
    ensureNotFinalized();

    auto item = createFileItem(sourcePath, archivePath);
    items_.push_back(std::move(item));
}

void ArchiveWriter::Impl::addDirectory(const std::wstring& sourcePath,
                                       const std::wstring& archivePath, bool recursive) {
    ensureNotFinalized();

    if (recursive) {
        addDirectoryRecursive(sourcePath, archivePath);
    } else {
        // Just add the directory entry
        addEmptyDirectory(archivePath);
    }
}

void ArchiveWriter::Impl::addFileFromMemory(const std::vector<uint8_t>& data,
                                            const std::wstring& archivePath) {
    ensureNotFinalized();

    UpdateItemInfo item;
    item.archivePath = archivePath;
    item.itemType = UpdateItemType::File;
    item.data = data;
    item.size = data.size();
    item.attributes = FILE_ATTRIBUTE_NORMAL;

    // Set current time
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    item.lastWriteTime = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

    items_.push_back(std::move(item));
}

void ArchiveWriter::Impl::addEmptyDirectory(const std::wstring& archivePath) {
    ensureNotFinalized();

    UpdateItemInfo item;
    item.archivePath = archivePath;
    item.itemType = UpdateItemType::Directory;
    item.size = 0;
    item.attributes = FILE_ATTRIBUTE_DIRECTORY;

    // Set current time
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    item.lastWriteTime = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

    items_.push_back(std::move(item));
}

void ArchiveWriter::Impl::addItem(const UpdateItemInfo& item) {
    ensureNotFinalized();
    items_.push_back(item);
}

void ArchiveWriter::Impl::finalize() {
    if (finalized_) {
        throw Exception(ErrorCode::InvalidState, "Archive already finalized");
    }

    if (!outStream_) {
        throw Exception(ErrorCode::InvalidState, "No output stream");
    }

    ensureFormat();
    applyProperties();

    // Create update callback
    std::optional<std::wstring> password;
    if (properties_.has_value() && properties_->password.has_value()) {
        password = properties_->password;
    }

    CMyComPtr<IArchiveUpdateCallback> updateCallback;
    {
        ArchiveUpdateCallback* callbackPtr = ArchiveUpdateCallback::Create(items_, password);
        if (progressCallback_) {
            callbackPtr->setProgressCallback(progressCallback_);
        }
        updateCallback = callbackPtr;
    }

    // Execute update
    HRESULT hr =
        outArchive_->UpdateItems(outStream_, static_cast<UInt32>(items_.size()), updateCallback);

    if (FAILED(hr)) {
        char hexStr[32];
        sprintf_s(hexStr, "0x%08X", static_cast<unsigned>(hr));
        std::string errorMsg = "Failed to write archive with " + std::to_string(items_.size()) +
                               " items, HRESULT: " + std::string(hexStr);
        if (!items_.empty()) {
            errorMsg += ", first item: " + wstring_to_utf8(items_[0].archivePath);
        }
        throw Exception(ErrorCode::ArchiveWriteError, errorMsg);
    }

    // Release streams to ensure data is flushed to disk
    outStream_.Release();
    outArchive_.Release();

    finalized_ = true;
}

void ArchiveWriter::Impl::ensureNotFinalized() {
    if (finalized_) {
        throw Exception(ErrorCode::InvalidState, "Cannot add items after archive is finalized");
    }
}

void ArchiveWriter::Impl::ensureFormat() {
    if (!outArchive_) {
        // Ensure CRC and other critical initialization is done
        InitializeArchiveFormats();

        // Get format CLSID
        GUID clsid = GetFormatCLSID(format_);

        // Create IOutArchive
        HRESULT hr =
            CreateArchiver(&clsid, &IID_IOutArchive, reinterpret_cast<void**>(&outArchive_));

        if (FAILED(hr) || !outArchive_) {
            throw Exception(ErrorCode::UnsupportedFormat,
                            "Cannot create archive handler for format: " +
                                std::to_string(static_cast<int>(format_)));
        }
    }
}

void ArchiveWriter::Impl::applyProperties() {
    if (!properties_.has_value() || !outArchive_) {
        return;
    }

    const auto& props = *properties_;

    // Query ISetProperties interface
    CMyComPtr<ISetProperties> setProps;
    if (outArchive_->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&setProps)) !=
        S_OK) {
        // Format doesn't support properties
        return;
    }

    std::vector<const wchar_t*> names;
    std::vector<PROPVARIANT> values;

    auto addProp = [&](const wchar_t* name, const PROPVARIANT& value) {
        names.push_back(name);
        values.push_back(value);
    };

    // Compression level
    PROPVARIANT propVar;
    memset(&propVar, 0, sizeof(propVar));
    propVar.vt = VT_UI4;
    propVar.ulVal = static_cast<uint32_t>(props.level);
    addProp(L"x", propVar);

    // Solid mode (7z only)
    if (format_ == ArchiveFormat::SevenZip) {
        memset(&propVar, 0, sizeof(propVar));
        propVar.vt = VT_BOOL;
        propVar.boolVal = props.solid ? VARIANT_TRUE : VARIANT_FALSE;
        addProp(L"s", propVar);
    }

    // Dictionary size
    if (props.dictionarySize.has_value()) {
        memset(&propVar, 0, sizeof(propVar));
        propVar.vt = VT_UI4;
        propVar.ulVal = static_cast<uint32_t>(*props.dictionarySize);
        addProp(L"d", propVar);
    }

    // Number of threads
    if (props.numThreads.has_value()) {
        memset(&propVar, 0, sizeof(propVar));
        propVar.vt = VT_UI4;
        propVar.ulVal = *props.numThreads;
        addProp(L"mt", propVar);
    }

    // Compression method
    if (props.method.has_value()) {
        memset(&propVar, 0, sizeof(propVar));
        propVar.vt = VT_BSTR;
        switch (*props.method) {
            case CompressionMethod::LZMA:
                propVar.bstrVal = ::SysAllocString(L"LZMA");
                break;
            case CompressionMethod::LZMA2:
                propVar.bstrVal = ::SysAllocString(L"LZMA2");
                break;
            case CompressionMethod::PPMd:
                propVar.bstrVal = ::SysAllocString(L"PPMd");
                break;
            case CompressionMethod::BZip2:
                propVar.bstrVal = ::SysAllocString(L"BZip2");
                break;
            case CompressionMethod::Deflate:
                propVar.bstrVal = ::SysAllocString(L"Deflate");
                break;
            case CompressionMethod::Copy:
                propVar.bstrVal = ::SysAllocString(L"Copy");
                break;
        }
        if (propVar.bstrVal) {
            addProp(L"m", propVar);
        }
    }

    // Encrypt headers (7z only)
    if (format_ == ArchiveFormat::SevenZip && props.encryptHeaders) {
        memset(&propVar, 0, sizeof(propVar));
        propVar.vt = VT_BOOL;
        propVar.boolVal = VARIANT_TRUE;
        addProp(L"he", propVar);
    }

    // Apply properties
    if (!names.empty()) {
        HRESULT hr =
            setProps->SetProperties(names.data(), values.data(), static_cast<UInt32>(names.size()));
        if (FAILED(hr)) {
            throw Exception(ErrorCode::InvalidArgument, "Failed to set archive properties");
        }
    }

    // Clean up BSTR values
    for (auto& val : values) {
        if (val.vt == VT_BSTR && val.bstrVal) {
            ::SysFreeString(val.bstrVal);
        }
    }
}

UpdateItemInfo ArchiveWriter::Impl::createFileItem(const std::wstring& sourcePath,
                                                   const std::wstring& archivePath) {
    UpdateItemInfo item;
    item.archivePath = archivePath;
    item.itemType = UpdateItemType::File;
    item.sourcePath = sourcePath;

    // Get file attributes
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExW(sourcePath.c_str(), GetFileExInfoStandard, &fileInfo)) {
        throw Exception(
            ErrorCode::CannotOpenFile,
            "Cannot get file attributes: " + std::string(sourcePath.begin(), sourcePath.end()));
    }

    item.size = (static_cast<uint64_t>(fileInfo.nFileSizeHigh) << 32) | fileInfo.nFileSizeLow;
    item.attributes = fileInfo.dwFileAttributes;
    item.lastWriteTime = (static_cast<uint64_t>(fileInfo.ftLastWriteTime.dwHighDateTime) << 32) |
                         fileInfo.ftLastWriteTime.dwLowDateTime;
    item.creationTime = (static_cast<uint64_t>(fileInfo.ftCreationTime.dwHighDateTime) << 32) |
                        fileInfo.ftCreationTime.dwLowDateTime;
    item.lastAccessTime = (static_cast<uint64_t>(fileInfo.ftLastAccessTime.dwHighDateTime) << 32) |
                          fileInfo.ftLastAccessTime.dwLowDateTime;

    return item;
}

void ArchiveWriter::Impl::addDirectoryRecursive(const fs::path& sourcePath,
                                                const std::wstring& archivePath) {
    // Add directory entry
    addEmptyDirectory(archivePath);

    // Add all files and subdirectories
    for (const auto& entry : fs::directory_iterator(sourcePath)) {
        auto filename = entry.path().filename().wstring();
        auto newArchivePath = archivePath + L"/" + filename;

        if (entry.is_directory()) {
            addDirectoryRecursive(entry.path(), newArchivePath);
        } else if (entry.is_regular_file()) {
            addFile(entry.path().wstring(), newArchivePath);
        }
    }
}

}  // namespace sevenzip::detail
