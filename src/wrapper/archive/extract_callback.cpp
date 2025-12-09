// extract_callback.cpp - Extract callback implementation
#include "extract_callback.hpp"

#include <comdef.h>

#include "Common/StringConvert.h"
#include "Windows/PropVariant.h"

namespace sevenzip::detail {

// ============================================================================
// ExtractToMemoryCallback
// ============================================================================

ExtractToMemoryCallback::ExtractToMemoryCallback(uint32_t index, std::vector<uint8_t>& buffer,
                                                 const std::wstring& password,
                                                 ExtractProgressCallback progressCallback)
    : targetIndex_(index),
      outputBuffer_(buffer),
      password_(password),
      progressCallback_(progressCallback) {}

// IProgress methods
Z7_COM7F_IMF(ExtractToMemoryCallback::SetTotal(UInt64 total)) {
    totalSize_ = total;
    return S_OK;
}

Z7_COM7F_IMF(ExtractToMemoryCallback::SetCompleted(const UInt64* completeValue)) {
    if (completeValue) {
        completedSize_ = *completeValue;
        if (progressCallback_) {
            if (!progressCallback_(completedSize_, totalSize_)) {
                return E_ABORT;
            }
        }
    }
    return S_OK;
}

Z7_COM7F_IMF(ExtractToMemoryCallback::GetStream(UInt32 index, ISequentialOutStream** outStream,
                                                Int32 askExtractMode)) {
    *outStream = nullptr;
    currentOutStream_ = nullptr;

    // 只处理目标索引
    if (index != targetIndex_) {
        return S_OK;
    }

    // 如果是测试模式或跳过模式，不需要输出流
    // kExtract = 0
    if (askExtractMode != 0) {
        return S_OK;
    }

    // 创建内存输出流
    currentOutStream_ = new MemoryOutStream(outputBuffer_);
    *outStream = currentOutStream_.operator->();
    (*outStream)->AddRef();

    return S_OK;
}

Z7_COM7F_IMF(ExtractToMemoryCallback::PrepareOperation(Int32 askExtractMode)) {
    return S_OK;
}

Z7_COM7F_IMF(ExtractToMemoryCallback::SetOperationResult(Int32 opRes)) {
    currentOutStream_ = nullptr;

    // kOK = 0, kUnsupportedMethod = 1, kDataError = 2, kCRCError = 3
    if (opRes != 0) {  // kOK
        switch (opRes) {
            case 1:  // kUnsupportedMethod
                return E_NOTIMPL;
            case 2:  // kDataError
                return E_FAIL;
            case 3:  // kCRCError
                return E_FAIL;
            default:
                return E_FAIL;
        }
    }

    return S_OK;
}

Z7_COM7F_IMF(ExtractToMemoryCallback::CryptoGetTextPassword(BSTR* password)) {
    if (password_.empty()) {
        return E_ABORT;  // 需要密码但未提供
    }
    *password = ::SysAllocString(password_.c_str());
    return S_OK;
}

// ============================================================================
// ExtractToFileCallback
// ============================================================================

ExtractToFileCallback::ExtractToFileCallback(::IInArchive* archive,
                                             const std::filesystem::path& outputPath,
                                             const std::wstring& password,
                                             ExtractProgressCallback progressCallback)
    : archive_(archive),
      outputPath_(outputPath),
      password_(password),
      progressCallback_(progressCallback) {}

// IProgress methods
Z7_COM7F_IMF(ExtractToFileCallback::SetTotal(UInt64 total)) {
    totalSize_ = total;
    return S_OK;
}

Z7_COM7F_IMF(ExtractToFileCallback::SetCompleted(const UInt64* completeValue)) {
    if (completeValue) {
        completedSize_ = *completeValue;
        if (progressCallback_) {
            if (!progressCallback_(completedSize_, totalSize_)) {
                return E_ABORT;
            }
        }
    }
    return S_OK;
}

Z7_COM7F_IMF(ExtractToFileCallback::GetStream(UInt32 index, ISequentialOutStream** outStream,
                                              Int32 askExtractMode)) {
    *outStream = nullptr;
    currentOutStream_ = nullptr;

    // 如果是测试模式或跳过模式，不需要输出流
    // kExtract = 0
    if (askExtractMode != 0) {
        return S_OK;
    }

    // 获取项目信息，检查是否是目录
    NWindows::NCOM::CPropVariant prop;
    HRESULT hr = archive_->GetProperty(index, kpidIsDir, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BOOL && prop.boolVal != VARIANT_FALSE) {
        // 是目录，创建目录
        std::filesystem::create_directories(outputPath_);
        return S_OK;
    }

    // 确保父目录存在
    if (outputPath_.has_parent_path()) {
        std::filesystem::create_directories(outputPath_.parent_path());
    }

    // 创建文件输出流
    try {
        currentOutStream_ = new FileOutStream(outputPath_);
        *outStream = currentOutStream_.operator->();
        (*outStream)->AddRef();
    } catch (const std::exception& e) {
        return E_FAIL;
    }

    return S_OK;
}

Z7_COM7F_IMF(ExtractToFileCallback::PrepareOperation(Int32 askExtractMode)) {
    return S_OK;
}

Z7_COM7F_IMF(ExtractToFileCallback::SetOperationResult(Int32 opRes)) {
    currentOutStream_ = nullptr;

    // kOK = 0, kUnsupportedMethod = 1, kDataError = 2, kCRCError = 3
    if (opRes != 0) {  // kOK
        // 提取失败，删除可能创建的文件
        if (std::filesystem::exists(outputPath_)) {
            std::filesystem::remove(outputPath_);
        }

        switch (opRes) {
            case 1:  // kUnsupportedMethod
                return E_NOTIMPL;
            case 2:  // kDataError
                return E_FAIL;
            case 3:  // kCRCError
                return E_FAIL;
            default:
                return E_FAIL;
        }
    }

    return S_OK;
}

Z7_COM7F_IMF(ExtractToFileCallback::CryptoGetTextPassword(BSTR* password)) {
    if (password_.empty()) {
        return E_ABORT;
    }
    *password = ::SysAllocString(password_.c_str());
    return S_OK;
}

// ============================================================================
// ExtractToDirectoryCallback
// ============================================================================

ExtractToDirectoryCallback::ExtractToDirectoryCallback(::IInArchive* archive,
                                                       const std::filesystem::path& outputDir,
                                                       const std::wstring& password,
                                                       ExtractProgressCallback progressCallback)
    : archive_(archive),
      outputDir_(outputDir),
      password_(password),
      progressCallback_(progressCallback) {}

// IProgress methods
Z7_COM7F_IMF(ExtractToDirectoryCallback::SetTotal(UInt64 total)) {
    totalSize_ = total;
    return S_OK;
}

Z7_COM7F_IMF(ExtractToDirectoryCallback::SetCompleted(const UInt64* completeValue)) {
    if (completeValue) {
        completedSize_ = *completeValue;
        if (progressCallback_) {
            if (!progressCallback_(completedSize_, totalSize_)) {
                return E_ABORT;
            }
        }
    }
    return S_OK;
}

Z7_COM7F_IMF(ExtractToDirectoryCallback::GetStream(UInt32 index, ISequentialOutStream** outStream,
                                                   Int32 askExtractMode)) {
    *outStream = nullptr;
    currentOutStream_ = nullptr;
    currentIndex_ = index;

    // 如果是测试模式或跳过模式，不需要输出流
    // kExtract = 0
    if (askExtractMode != 0) {
        return S_OK;
    }

    // 获取文件路径
    NWindows::NCOM::CPropVariant prop;
    HRESULT hr = archive_->GetProperty(index, kpidPath, &prop);
    std::wstring itemPath;
    if (SUCCEEDED(hr) && prop.vt == VT_BSTR) {
        itemPath = prop.bstrVal;
    } else {
        // 如果没有路径，使用索引作为文件名
        itemPath = L"file_" + std::to_wstring(index);
    }

    // 构建完整输出路径
    currentFilePath_ = outputDir_ / itemPath;

    // 检查是否是目录
    prop.Clear();
    hr = archive_->GetProperty(index, kpidIsDir, &prop);
    if (SUCCEEDED(hr) && prop.vt == VT_BOOL && prop.boolVal != VARIANT_FALSE) {
        // 是目录，创建目录
        std::filesystem::create_directories(currentFilePath_);
        return S_OK;
    }

    // 确保父目录存在
    if (currentFilePath_.has_parent_path()) {
        std::filesystem::create_directories(currentFilePath_.parent_path());
    }

    // 创建文件输出流
    try {
        currentOutStream_ = new FileOutStream(currentFilePath_);
        *outStream = currentOutStream_.operator->();
        (*outStream)->AddRef();
    } catch (const std::exception& e) {
        return E_FAIL;
    }

    return S_OK;
}

Z7_COM7F_IMF(ExtractToDirectoryCallback::PrepareOperation(Int32 askExtractMode)) {
    return S_OK;
}

Z7_COM7F_IMF(ExtractToDirectoryCallback::SetOperationResult(Int32 opRes)) {
    currentOutStream_ = nullptr;

    // kOK = 0, kUnsupportedMethod = 1, kDataError = 2, kCRCError = 3
    if (opRes != 0) {  // kOK
        // 提取失败，删除可能创建的文件
        if (!currentFilePath_.empty() && std::filesystem::exists(currentFilePath_)) {
            if (!std::filesystem::is_directory(currentFilePath_)) {
                std::filesystem::remove(currentFilePath_);
            }
        }

        switch (opRes) {
            case 1:  // kUnsupportedMethod
                return E_NOTIMPL;
            case 2:  // kDataError
                return E_FAIL;
            case 3:  // kCRCError
                return E_FAIL;
            default:
                return E_FAIL;
        }
    }

    currentFilePath_.clear();
    return S_OK;
}

Z7_COM7F_IMF(ExtractToDirectoryCallback::CryptoGetTextPassword(BSTR* password)) {
    if (password_.empty()) {
        return E_ABORT;
    }
    *password = ::SysAllocString(password_.c_str());
    return S_OK;
}

}  // namespace sevenzip::detail
