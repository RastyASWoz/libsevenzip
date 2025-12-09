// update_callback.hpp - Archive update callback implementations
#pragma once

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>
#include <Common/MyCom.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../common/wrapper_error.hpp"

namespace sevenzip::detail {

// Forward declarations
struct UpdateItemInfo;
enum class UpdateItemType;

// 更新回调实现
// 注意: 7-Zip 的 7z 格式处理器:
//   1. 使用 IArchiveUpdateCallbackFile::GetStream2 而不是 IArchiveUpdateCallback::GetStream
//   2. 查询 ICryptoGetTextPassword (不是 ICryptoGetTextPassword2) 来获取密码
class ArchiveUpdateCallback Z7_final : public IArchiveUpdateCallback,
                                       public IArchiveUpdateCallbackFile,
                                       public ICryptoGetTextPassword,
                                       public ICryptoGetTextPassword2,
                                       public ICompressProgressInfo,
                                       public CMyUnknownImp {
   public:
    // Factory method (MUST BE FIRST to avoid access issues)
    static ArchiveUpdateCallback* Create(
        const std::vector<UpdateItemInfo>& items,
        const std::optional<std::wstring>& password = std::nullopt);

    // Set progress callback (MUST BE EARLY)
    void setProgressCallback(std::function<bool(uint64_t, uint64_t)> callback);

    // COM interface implementation
    Z7_COM_UNKNOWN_IMP_SPEC(Z7_COM_QI_ENTRY_UNKNOWN(IArchiveUpdateCallback)
                                Z7_COM_QI_ENTRY(IArchiveUpdateCallback)
                                    Z7_COM_QI_ENTRY(IArchiveUpdateCallbackFile)
                                        Z7_COM_QI_ENTRY(ICryptoGetTextPassword)
                                            Z7_COM_QI_ENTRY(ICryptoGetTextPassword2)
                                                Z7_COM_QI_ENTRY(ICompressProgressInfo))

    // IProgress methods (inherited by IArchiveUpdateCallback)
    __declspec(nothrow) STDMETHODIMP SetTotal(UInt64 total) throw() override final;
    __declspec(nothrow) STDMETHODIMP
    SetCompleted(const UInt64* completeValue) throw() override final;

    // IArchiveUpdateCallback methods
    __declspec(nothrow) STDMETHODIMP GetUpdateItemInfo(
        UInt32 index,
        Int32* newData,         // 1 = new data, 0 = old data from archive
        Int32* newProps,        // 1 = new properties, 0 = old properties
        UInt32* indexInArchive  // index in old archive, or -1 if new item
        ) throw() override final;

    __declspec(nothrow) STDMETHODIMP GetProperty(UInt32 index, PROPID propID,
                                                 PROPVARIANT* value) throw() override final;

    __declspec(nothrow) STDMETHODIMP
    GetStream(UInt32 index, ISequentialInStream** inStream) throw() override final;

    __declspec(nothrow) STDMETHODIMP
    SetOperationResult(Int32 operationResult) throw() override final;

    // IArchiveUpdateCallbackFile methods (7z format uses this instead of GetStream)
    __declspec(nothrow) STDMETHODIMP GetStream2(UInt32 index, ISequentialInStream** inStream,
                                                UInt32 notifyOp) throw() override final;

    __declspec(nothrow) STDMETHODIMP ReportOperation(UInt32 indexType, UInt32 index,
                                                     UInt32 notifyOp) throw() override final;

    // ICompressProgressInfo methods
    __declspec(nothrow) STDMETHODIMP SetRatioInfo(const UInt64* inSize,
                                                  const UInt64* outSize) throw() override final;

    // ICryptoGetTextPassword methods (7z 格式处理器查询这个接口获取密码)
    __declspec(nothrow) STDMETHODIMP CryptoGetTextPassword(BSTR* password) throw() override final;

    // ICryptoGetTextPassword2 methods
    __declspec(nothrow) STDMETHODIMP CryptoGetTextPassword2(Int32* passwordIsDefined,
                                                            BSTR* password) throw() override final;

    // Constructor
    ArchiveUpdateCallback(const std::vector<UpdateItemInfo>& items,
                          const std::optional<std::wstring>& password = std::nullopt);

    ~ArchiveUpdateCallback();

   private:
    // Update items
    std::vector<UpdateItemInfo> items_;

    // Password
    std::optional<std::wstring> password_;

    // Progress tracking
    uint64_t totalSize_;
    uint64_t completedSize_;
    std::function<bool(uint64_t, uint64_t)> progressCallback_;

    // Current operation index
    uint32_t currentIndex_;

    // Calculate total size
    void calculateTotalSize();

    // Report progress
    void reportProgress();
};

}  // namespace sevenzip::detail
