// update_callback.cpp - Archive update callback implementations
#include "update_callback.hpp"

#include <7zip/ICoder.h>
#include <Common/MyCom.h>
#include <Windows.h>
#include <propvarutil.h>

#include <iostream>

#include "../common/wrapper_propvariant.hpp"
#include "../common/wrapper_string.hpp"
#include "../stream/stream_file.hpp"
#include "../stream/stream_memory.hpp"
#include "archive_writer.hpp"

namespace sevenzip::detail {

ArchiveUpdateCallback::ArchiveUpdateCallback(const std::vector<UpdateItemInfo>& items,
                                             const std::optional<std::wstring>& password)
    : items_(items), password_(password), totalSize_(0), completedSize_(0), currentIndex_(0) {
    calculateTotalSize();
}

ArchiveUpdateCallback::~ArchiveUpdateCallback() = default;

// Factory method
ArchiveUpdateCallback* ArchiveUpdateCallback::Create(const std::vector<UpdateItemInfo>& items,
                                                     const std::optional<std::wstring>& password) {
    return new ArchiveUpdateCallback(items, password);
}

void ArchiveUpdateCallback::setProgressCallback(std::function<bool(uint64_t, uint64_t)> callback) {
    progressCallback_ = std::move(callback);
}

void ArchiveUpdateCallback::calculateTotalSize() {
    totalSize_ = 0;
    for (const auto& item : items_) {
        if (item.itemType == UpdateItemType::File) {
            totalSize_ += item.size;
        }
    }
}

void ArchiveUpdateCallback::reportProgress() {
    if (progressCallback_) {
        if (!progressCallback_(completedSize_, totalSize_)) {
            // User canceled
            throw Exception(ErrorCode::Aborted, "Operation canceled by user");
        }
    }
}

// IProgress methods

Z7_COM7F_IMF(ArchiveUpdateCallback::SetTotal(UInt64 total)) {
    totalSize_ = total;
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetCompleted(const UInt64* completeValue)) {
    if (completeValue) {
        completedSize_ = *completeValue;
        try {
            reportProgress();
        } catch (const Exception&) {
            return E_ABORT;
        }
    }
    return S_OK;
}

// IArchiveUpdateCallback methods

Z7_COM7F_IMF(ArchiveUpdateCallback::GetUpdateItemInfo(UInt32 index, Int32* newData, Int32* newProps,
                                                      UInt32* indexInArchive)) {
    if (index >= items_.size()) {
        return E_INVALIDARG;
    }

    // All items are new (not from existing archive)
    *newData = 1;
    *newProps = 1;
    *indexInArchive = static_cast<UInt32>(-1);

    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT* value)) {
    ::PropVariantClear(value);

    if (index >= items_.size()) {
        return E_INVALIDARG;
    }

    const auto& item = items_[index];

    try {
        switch (propID) {
            case kpidPath: {
                wstring_to_prop(item.archivePath, value);
                return S_OK;
            }

            case kpidIsDir: {
                value->vt = VT_BOOL;
                value->boolVal =
                    (item.itemType == UpdateItemType::Directory) ? VARIANT_TRUE : VARIANT_FALSE;
                return S_OK;
            }

            case kpidSize: {
                if (item.itemType == UpdateItemType::File) {
                    value->vt = VT_UI8;
                    value->uhVal.QuadPart = item.size;
                }
                return S_OK;
            }

            case kpidAttrib: {
                value->vt = VT_UI4;
                value->ulVal = item.attributes;
                return S_OK;
            }

            case kpidMTime: {
                if (item.lastWriteTime != 0) {
                    value->vt = VT_FILETIME;
                    value->filetime.dwLowDateTime = static_cast<DWORD>(item.lastWriteTime);
                    value->filetime.dwHighDateTime = static_cast<DWORD>(item.lastWriteTime >> 32);
                }
                return S_OK;
            }

            case kpidCTime: {
                if (item.creationTime.has_value()) {
                    value->vt = VT_FILETIME;
                    value->filetime.dwLowDateTime = static_cast<DWORD>(*item.creationTime);
                    value->filetime.dwHighDateTime = static_cast<DWORD>(*item.creationTime >> 32);
                }
                return S_OK;
            }

            case kpidATime: {
                if (item.lastAccessTime.has_value()) {
                    value->vt = VT_FILETIME;
                    value->filetime.dwLowDateTime = static_cast<DWORD>(*item.lastAccessTime);
                    value->filetime.dwHighDateTime = static_cast<DWORD>(*item.lastAccessTime >> 32);
                }
                return S_OK;
            }

            case kpidPosixAttrib: {
                // Not supported yet
                return S_OK;
            }

            default:
                return S_OK;
        }
    } catch (const std::exception&) {
        return E_FAIL;
    }
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream** inStream)) {
    *inStream = nullptr;

    if (index >= items_.size()) {
        return E_INVALIDARG;
    }

    const auto& item = items_[index];
    currentIndex_ = index;

    try {
        // Directories don't have streams
        if (item.itemType == UpdateItemType::Directory) {
            return S_OK;
        }

        // Create stream based on source
        if (item.sourcePath.has_value()) {
            // From file
            try {
                FileInStream* fileStreamSpec = new FileInStream(*item.sourcePath);
                if (!fileStreamSpec->isOpen()) {
                    delete fileStreamSpec;
                    return E_FAIL;
                }
                // Use CMyComPtr to manage reference counting properly
                // The constructor will call AddRef(), and Detach() transfers ownership
                CMyComPtr<ISequentialInStream> fileStreamLoc(fileStreamSpec);
                *inStream = fileStreamLoc.Detach();
            } catch (...) {
                return E_FAIL;
            }
        } else if (item.data.has_value()) {
            // From memory
            MemoryInStream* memStreamSpec = new MemoryInStream(item.data.value());
            // Use CMyComPtr to manage reference counting properly
            CMyComPtr<ISequentialInStream> memStreamLoc(memStreamSpec);
            *inStream = memStreamLoc.Detach();
        } else {
            // No source specified
            return E_INVALIDARG;
        }

        return S_OK;
    } catch (const std::exception&) {
        return E_FAIL;
    }
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetOperationResult(Int32 operationResult)) {
    if (operationResult != 0) {
        // Operation failed
        return E_FAIL;
    }

    // Update completed size
    if (currentIndex_ < items_.size()) {
        const auto& item = items_[currentIndex_];
        if (item.itemType == UpdateItemType::File) {
            completedSize_ += item.size;
            try {
                reportProgress();
            } catch (const Exception&) {
                return E_ABORT;
            }
        }
    }

    return S_OK;
}

// ICompressProgressInfo methods

Z7_COM7F_IMF(ArchiveUpdateCallback::SetRatioInfo(const UInt64* inSize, const UInt64* outSize)) {
    // Update progress based on compression ratio
    // For now, just return S_OK to indicate we handled it
    return S_OK;
}

// IArchiveUpdateCallbackFile methods
// 注意: 7-Zip 的 7z 格式处理器调用 GetStream2 而不是 GetStream

Z7_COM7F_IMF(ArchiveUpdateCallback::GetStream2(UInt32 index, ISequentialInStream** inStream,
                                               UInt32 notifyOp)) {
    // notifyOp values from NUpdateNotifyOp:
    // kAdd = 0, kUpdate = 1, kAnalyze = 2, kReplicate = 3, kRepack = 4, kSkip = 5, kDelete = 6,
    // kHeader = 7, kHashRead = 8, kInFileInfo = 9

    // For operations that don't need a stream, just return S_OK
    // kAnalyze (2), kReplicate (3), kRepack (4), kSkip (5), kDelete (6), kHeader (7) don't need
    // streams
    if (notifyOp >= 2) {
        *inStream = nullptr;
        return S_OK;
    }

    // For kAdd (0) and kUpdate (1), delegate to GetStream
    return GetStream(index, inStream);
}

Z7_COM7F_IMF(ArchiveUpdateCallback::ReportOperation(UInt32 indexType, UInt32 index,
                                                    UInt32 notifyOp)) {
    // indexType: 0 = in archive, 1 = out archive
    // notifyOp: same as GetStream2's notifyOp
    // Just report and continue
    return S_OK;
}

// ICryptoGetTextPassword methods
// 7z 格式处理器查询这个接口获取密码（不是 ICryptoGetTextPassword2）

Z7_COM7F_IMF(ArchiveUpdateCallback::CryptoGetTextPassword(BSTR* password)) {
    if (password_.has_value()) {
        *password = ::SysAllocString(password_->c_str());
        return *password ? S_OK : E_OUTOFMEMORY;
    } else {
        *password = nullptr;
        return S_OK;
    }
}

// ICryptoGetTextPassword2 methods

Z7_COM7F_IMF(ArchiveUpdateCallback::CryptoGetTextPassword2(Int32* passwordIsDefined,
                                                           BSTR* password)) {
    if (password_.has_value()) {
        *passwordIsDefined = 1;
        *password = ::SysAllocString(password_->c_str());
        return *password ? S_OK : E_OUTOFMEMORY;
    } else {
        *passwordIsDefined = 0;
        *password = nullptr;
        return S_OK;
    }
}

}  // namespace sevenzip::detail
