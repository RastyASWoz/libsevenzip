// archive_reader.cpp - Archive reader implementation
#include "archive_reader.hpp"

#include <filesystem>

#include "../common/wrapper_string.hpp"
#include "../stream/stream_file.hpp"
#include "extract_callback.hpp"

// 7-Zip headers
#include "7zip/Archive/IArchive.h"

// Forward declaration
extern "C" void InitializeArchiveFormats();
#include "7zip/IPassword.h"
#include "7zip/IStream.h"
#include "7zip/PropID.h"
#include "Common/MyCom.h"
#include "Windows/PropVariant.h"

namespace fs = std::filesystem;

// Import types from parent namespace
using sevenzip::ArchiveFormat;
using sevenzip::ErrorCode;
using sevenzip::Exception;

namespace sevenzip::detail {

// Import FormatDetector from detail namespace
using sevenzip::detail::FormatDetector;

// Forward declare CreateArchiver function from 7-Zip
extern "C" {
STDAPI CreateArchiver(const GUID* clsid, const GUID* iid, void** outObject);
}

// ============================================================================
// Open callback for reading encrypted archives
// ============================================================================

// Define GUIDs for COM interfaces used in QueryInterface
// IID_IArchiveOpenCallback: {23170F69-40C1-278A-0000-000000100000}
static const GUID IID_IArchiveOpenCallback = {
    0x23170F69, 0x40C1, 0x278A, {0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00}};

class CArchiveOpenCallback : public IArchiveOpenCallback,
                             public ICryptoGetTextPassword,
                             public CMyUnknownImp {
   public:
    Z7_COM_QI_BEGIN2(IArchiveOpenCallback)
    Z7_COM_QI_ENTRY(ICryptoGetTextPassword)
    Z7_COM_QI_END
    Z7_COM_ADDREF_RELEASE

   public:  // Z7_COM_ADDREF_RELEASE ends with "private:", so we need to restore public access
    Z7_COM7F_IMF(SetTotal(const UInt64* files, const UInt64* bytes)) {
        // Optional progress reporting
        return S_OK;
    }

    Z7_COM7F_IMF(SetCompleted(const UInt64* files, const UInt64* bytes)) {
        // Optional progress reporting
        return S_OK;
    }

    Z7_COM7F_IMF(CryptoGetTextPassword(BSTR* password)) {
        if (!password) {
            return E_INVALIDARG;
        }

        if (passwordCallback_) {
            std::wstring pwd = passwordCallback_();
            *password = ::SysAllocString(pwd.c_str());
            return *password ? S_OK : E_OUTOFMEMORY;
        }

        return E_ABORT;  // No password available
    }

    void SetPasswordCallback(PasswordCallback callback) { passwordCallback_ = std::move(callback); }

   private:
    PasswordCallback passwordCallback_;
};

// ============================================================================
// Helper functions
// ============================================================================

// 获取格式的 CLSID
static GUID GetFormatCLSID(ArchiveFormat format) {
    // 7-Zip 的基础 CLSID: {23170F69-40C1-278A-1000-000110xx0000}
    // xx 是格式 ID
    GUID base = {0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00}};

    // 根据格式设置 ID
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
        case ArchiveFormat::Rar:
            base.Data4[5] = 0x03;  // '03'
            break;
        default:
            throw Exception(ErrorCode::UnsupportedFormat, "Unsupported archive format");
    }

    return base;
}

// ============================================================================
// ArchiveReader::Impl
// ============================================================================

class ArchiveReader::Impl {
   public:
    CMyComPtr<::IInArchive> archive;  // 7-Zip COM IInArchive
    CMyComPtr<::IInStream> stream;    // 7-Zip COM IInStream
    ArchiveFormat format = ArchiveFormat::Unknown;
    PasswordCallback passwordCallback;
    ProgressCallback progressCallback;
    bool isOpen = false;
};

// ============================================================================
// ArchiveReader Implementation
// ============================================================================

ArchiveReader::ArchiveReader() : impl_(std::make_unique<Impl>()) {}

ArchiveReader::~ArchiveReader() {
    if (impl_) {  // 检查 impl_ 是否有效（移动后可能为 nullptr）
        close();
    }
}

ArchiveReader::ArchiveReader(ArchiveReader&&) noexcept = default;
ArchiveReader& ArchiveReader::operator=(ArchiveReader&&) noexcept = default;

void ArchiveReader::open(const std::wstring& path) {
    // Auto-detect format by reading file signature
    fs::path p(path);
    ArchiveFormat detectedFormat = FormatDetector::detect(p);

    if (detectedFormat == ArchiveFormat::Unknown) {
        throw Exception(ErrorCode::UnsupportedFormat, "Cannot detect archive format");
    }

    open(path, detectedFormat);
}

void ArchiveReader::open(const std::wstring& path, ArchiveFormat format) {
    if (impl_->isOpen) {
        close();
    }

    // Ensure CRC and other critical initialization is done
    InitializeArchiveFormats();
    // 创建文件输入流
    auto fileStream = new FileInStream(path);
    impl_->stream = fileStream;

    // 获取格式 CLSID 并创建归档对象
    GUID clsid = GetFormatCLSID(format);

    HRESULT hr = CreateArchiver(&clsid, &IID_IInArchive, reinterpret_cast<void**>(&impl_->archive));

    if (FAILED(hr) || !impl_->archive) {
        impl_->stream = nullptr;
        throw Exception(ErrorCode::UnsupportedFormat, "Failed to create archive handler");
    }

    // 创建打开回调（用于密码回调等）
    CMyComPtr<IArchiveOpenCallback> openCallback;
    if (impl_->passwordCallback) {
        CArchiveOpenCallback* openCallbackPtr = new CArchiveOpenCallback();
        openCallbackPtr->SetPasswordCallback(impl_->passwordCallback);
        // Use CMyComPtr for automatic COM lifetime management
        // Starts with refcount=0, CMyComPtr constructor will AddRef (refcount=1)
        openCallback = openCallbackPtr;
    }

    // 打开归档（使用较大的maxCheckStartPosition以支持自解压档案等）
    const UInt64* maxCheckStartPosition = nullptr;  // nullptr means check entire file
    UInt64 maxCheck = 1 << 20;                      // 1MB - enough for most archive headers
    maxCheckStartPosition = &maxCheck;

    hr = impl_->archive->Open(fileStream, maxCheckStartPosition, openCallback);
    // CMyComPtr destructor will Release the callback when we exit this function

    // S_FALSE (1) means the archive was partially opened but there might be issues
    // For 7-Zip, S_OK (0) means success, S_FALSE usually means format not recognized
    if (hr != S_OK) {
        impl_->archive = nullptr;
        impl_->stream = nullptr;
        throw Exception(sevenzip::hresult_to_error_code(hr),
                        "Failed to open archive (format not recognized or corrupted)");
    }

    impl_->format = format;
    impl_->isOpen = true;
}

void ArchiveReader::openFromStream(::IInStream* stream, ArchiveFormat format) {
    if (impl_->isOpen) {
        close();
    }

    if (!stream) {
        throw Exception(ErrorCode::InvalidArgument, "Stream cannot be null");
    }

    // 获取格式 CLSID 并创建归档对象
    GUID clsid = GetFormatCLSID(format);
    HRESULT hr = CreateArchiver(&clsid, &IID_IInArchive, reinterpret_cast<void**>(&impl_->archive));

    if (FAILED(hr) || !impl_->archive) {
        throw Exception(ErrorCode::UnsupportedFormat, "Failed to create archive handler");
    }

    // 创建打开回调（用于密码回调等）
    CMyComPtr<IArchiveOpenCallback> openCallback;
    if (impl_->passwordCallback) {
        CArchiveOpenCallback* openCallbackPtr = new CArchiveOpenCallback();
        openCallbackPtr->SetPasswordCallback(impl_->passwordCallback);
        // Use CMyComPtr for automatic COM lifetime management
        openCallback = openCallbackPtr;
    }

    // 打开归档
    hr = impl_->archive->Open(stream, nullptr, openCallback);
    // CMyComPtr destructor will Release the callback when we exit this function

    if (FAILED(hr)) {
        impl_->archive = nullptr;
        throw Exception(sevenzip::hresult_to_error_code(hr), "Failed to open archive from stream");
    }

    impl_->stream = stream;
    impl_->format = format;
    impl_->isOpen = true;
}

void ArchiveReader::close() {
    if (!impl_) return;  // 移动后的对象，impl_ 为 nullptr

    if (impl_->archive) {
        impl_->archive->Close();
        impl_->archive = nullptr;
    }
    impl_->stream = nullptr;
    impl_->isOpen = false;
}

bool ArchiveReader::isOpen() const {
    return impl_ && impl_->isOpen;
}

uint32_t ArchiveReader::getItemCount() const {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    UInt32 numItems = 0;
    HRESULT hr = impl_->archive->GetNumberOfItems(&numItems);

    if (FAILED(hr)) {
        throw Exception(sevenzip::hresult_to_error_code(hr), "Failed to get item count");
    }

    return static_cast<uint32_t>(numItems);
}

ArchiveInfo ArchiveReader::getArchiveInfo() const {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    ArchiveInfo info;
    info.format = impl_->format;
    info.itemCount = getItemCount();

    // 获取归档物理大小
    NWindows::NCOM::CPropVariant prop;
    HRESULT hr = impl_->archive->GetArchiveProperty(kpidPhySize, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_UI8) {
        info.physicalSize = prop.uhVal.QuadPart;
    }

    // 获取是否多卷
    prop.Clear();
    hr = impl_->archive->GetArchiveProperty(kpidIsVolume, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BOOL) {
        info.isMultiVolume = (prop.boolVal != VARIANT_FALSE);
    }

    // 获取是否固实
    prop.Clear();
    hr = impl_->archive->GetArchiveProperty(kpidSolid, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BOOL) {
        info.isSolid = (prop.boolVal != VARIANT_FALSE);
    }

    info.hasEncryptedHeader = false;  // TODO: 检测加密头

    return info;
}

ArchiveItemInfo ArchiveReader::getItemInfo(uint32_t index) const {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    ArchiveItemInfo info;
    info.index = index;

    NWindows::NCOM::CPropVariant prop;

    // 获取路径
    HRESULT hr = impl_->archive->GetProperty(index, kpidPath, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BSTR) {
        info.path = prop.bstrVal;
    }

    // 获取是否目录
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidIsDir, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BOOL) {
        info.isDirectory = (prop.boolVal != VARIANT_FALSE);
    } else {
        info.isDirectory = false;
    }

    // 获取解压后大小
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidSize, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_UI8) {
        info.size = prop.uhVal.QuadPart;
    } else {
        info.size = 0;
    }

    // 获取压缩后大小
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidPackSize, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_UI8) {
        info.packedSize = prop.uhVal.QuadPart;
    } else {
        info.packedSize = 0;
    }

    // 获取 CRC
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidCRC, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_UI4) {
        info.crc = prop.ulVal;
    }

    // 获取创建时间
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidCTime, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_FILETIME) {
        info.creationTime =
            ((uint64_t)prop.filetime.dwHighDateTime << 32) | prop.filetime.dwLowDateTime;
    }

    // 获取修改时间
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidMTime, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_FILETIME) {
        info.lastWriteTime =
            ((uint64_t)prop.filetime.dwHighDateTime << 32) | prop.filetime.dwLowDateTime;
    }

    // 获取访问时间
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidATime, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_FILETIME) {
        info.lastAccessTime =
            ((uint64_t)prop.filetime.dwHighDateTime << 32) | prop.filetime.dwLowDateTime;
    }

    // 获取文件属性
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidAttrib, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_UI4) {
        info.attributes = prop.ulVal;
    }

    // 获取是否加密
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidEncrypted, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BOOL) {
        info.isEncrypted = (prop.boolVal != VARIANT_FALSE);
    } else {
        info.isEncrypted = false;
    }

    // 获取注释
    prop.Clear();
    hr = impl_->archive->GetProperty(index, kpidComment, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BSTR) {
        info.comment = prop.bstrVal;
    }

    return info;
}

std::vector<uint8_t> ArchiveReader::extractToMemory(uint32_t index) {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    // 检查索引有效性
    uint32_t itemCount = getItemCount();
    if (index >= itemCount) {
        throw Exception(ErrorCode::InvalidArgument, "Invalid item index");
    }

    // 准备输出缓冲区
    std::vector<uint8_t> buffer;

    // 创建提取回调
    std::wstring password;
    if (impl_->passwordCallback) {
        password = impl_->passwordCallback();
    }

    ExtractToMemoryCallback* callbackImpl =
        new ExtractToMemoryCallback(index, buffer, password, impl_->progressCallback);
    CMyComPtr<IArchiveExtractCallback> extractCallback = callbackImpl;

    // 执行提取
    const UInt32 indices[] = {index};
    HRESULT hr = impl_->archive->Extract(indices, 1, 0, extractCallback);

    if (FAILED(hr)) {
        throw Exception(sevenzip::hresult_to_error_code(hr), "Failed to extract to memory");
    }

    return buffer;
}

void ArchiveReader::extractToFile(uint32_t index, const std::wstring& destPath) {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    // 检查索引有效性
    uint32_t itemCount = getItemCount();
    if (index >= itemCount) {
        throw Exception(ErrorCode::InvalidArgument, "Invalid item index");
    }

    // 获取密码
    std::wstring password;
    if (impl_->passwordCallback) {
        password = impl_->passwordCallback();
    }

    // 创建提取回调
    fs::path outputPath(destPath);
    ExtractToFileCallback* callbackImpl =
        new ExtractToFileCallback(impl_->archive, outputPath, password, impl_->progressCallback);
    CMyComPtr<IArchiveExtractCallback> extractCallback = callbackImpl;

    // 执行提取
    const UInt32 indices[] = {index};
    HRESULT hr = impl_->archive->Extract(indices, 1, 0, extractCallback);

    if (FAILED(hr)) {
        throw Exception(sevenzip::hresult_to_error_code(hr), "Failed to extract to file");
    }
}

void ArchiveReader::extractItems(const std::vector<uint32_t>& indices,
                                 const std::wstring& destDir) {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    if (indices.empty()) {
        return;  // 没有项目需要提取
    }

    // 检查索引有效性
    uint32_t itemCount = getItemCount();
    for (uint32_t index : indices) {
        if (index >= itemCount) {
            throw Exception(ErrorCode::InvalidArgument, "Invalid item index");
        }
    }

    // 获取密码
    std::wstring password;
    if (impl_->passwordCallback) {
        password = impl_->passwordCallback();
    }

    // 创建输出目录
    fs::path outputDir(destDir);
    fs::create_directories(outputDir);

    // 创建提取回调
    ExtractToDirectoryCallback* callbackImpl = new ExtractToDirectoryCallback(
        impl_->archive, outputDir, password, impl_->progressCallback);
    CMyComPtr<IArchiveExtractCallback> extractCallback = callbackImpl;

    // 执行提取
    HRESULT hr = impl_->archive->Extract(indices.data(), static_cast<UInt32>(indices.size()),
                                         0,  // testMode = 0 (extract)
                                         extractCallback);

    if (FAILED(hr)) {
        throw Exception(sevenzip::hresult_to_error_code(hr), "Failed to extract items");
    }
}

void ArchiveReader::extractAll(const std::wstring& destDir) {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    // 获取密码
    std::wstring password;
    if (impl_->passwordCallback) {
        password = impl_->passwordCallback();
    }

    // 创建输出目录
    fs::path outputDir(destDir);
    fs::create_directories(outputDir);

    // 创建提取回调
    ExtractToDirectoryCallback* callbackImpl = new ExtractToDirectoryCallback(
        impl_->archive, outputDir, password, impl_->progressCallback);
    CMyComPtr<IArchiveExtractCallback> extractCallback = callbackImpl;

    // 执行提取（nullptr 表示提取所有项目）
    HRESULT hr = impl_->archive->Extract(nullptr,                  // 提取所有项目
                                         static_cast<UInt32>(-1),  // numItems = -1 表示所有
                                         0,                        // testMode = 0 (extract)
                                         extractCallback);

    if (FAILED(hr)) {
        throw Exception(sevenzip::hresult_to_error_code(hr), "Failed to extract all items");
    }
}

bool ArchiveReader::testArchive() {
    if (!impl_->isOpen) {
        throw Exception(ErrorCode::InvalidHandle, "Archive not open");
    }

    // 获取密码
    std::wstring password;
    if (impl_->passwordCallback) {
        password = impl_->passwordCallback();
    }

    // 创建临时目录用于测试（不实际写入文件）
    fs::path tempDir = fs::temp_directory_path() / "libsevenzip_test";

    // 创建提取回调
    ExtractToDirectoryCallback* callbackImpl =
        new ExtractToDirectoryCallback(impl_->archive, tempDir, password, impl_->progressCallback);
    CMyComPtr<IArchiveExtractCallback> extractCallback = callbackImpl;

    // 执行测试（testMode = 1）
    HRESULT hr = impl_->archive->Extract(nullptr,                  // 测试所有项目
                                         static_cast<UInt32>(-1),  // numItems = -1 表示所有
                                         1,                        // testMode = 1 (test)
                                         extractCallback);

    return SUCCEEDED(hr);
}

void ArchiveReader::setPasswordCallback(PasswordCallback callback) {
    impl_->passwordCallback = callback;
}

void ArchiveReader::setProgressCallback(ProgressCallback callback) {
    impl_->progressCallback = callback;
}

}  // namespace sevenzip::detail
