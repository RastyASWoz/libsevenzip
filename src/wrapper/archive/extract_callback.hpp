// extract_callback.hpp - Extract callback implementation for 7-Zip
#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

#include "../common/wrapper_error.hpp"
#include "../stream/stream_file.hpp"
#include "../stream/stream_memory.hpp"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"
#include "Common/MyString.h"

namespace sevenzip::detail {

// 提取进度回调
using ExtractProgressCallback = std::function<bool(uint64_t completed, uint64_t total)>;

// 提取到内存的回调
class ExtractToMemoryCallback Z7_final : public IArchiveExtractCallback,
                                         public ICryptoGetTextPassword,
                                         public CMyUnknownImp {
   public:
    // IUnknown methods (手动实现QueryInterface)
    Z7_COM_UNKNOWN_IMP_SPEC(Z7_COM_QI_ENTRY_UNKNOWN(IArchiveExtractCallback)
                                Z7_COM_QI_ENTRY(IArchiveExtractCallback)
                                    Z7_COM_QI_ENTRY(ICryptoGetTextPassword))

    // IProgress methods (inherited via IArchiveExtractCallback)
    __declspec(nothrow) STDMETHODIMP SetTotal(UInt64 total) throw() override final;
    __declspec(nothrow) STDMETHODIMP
    SetCompleted(const UInt64* completeValue) throw() override final;

    // IArchiveExtractCallback methods
    __declspec(nothrow) STDMETHODIMP GetStream(UInt32 index, ISequentialOutStream** outStream,
                                               Int32 askExtractMode) throw() override final;
    __declspec(nothrow) STDMETHODIMP PrepareOperation(Int32 askExtractMode) throw() override final;
    __declspec(nothrow) STDMETHODIMP SetOperationResult(Int32 opRes) throw() override final;

    // ICryptoGetTextPassword method
    __declspec(nothrow) STDMETHODIMP CryptoGetTextPassword(BSTR* password) throw() override final;

   private:
    uint32_t targetIndex_;
    std::vector<uint8_t>& outputBuffer_;
    std::wstring password_;
    ExtractProgressCallback progressCallback_;

    CMyComPtr<ISequentialOutStream> currentOutStream_;
    uint64_t totalSize_ = 0;
    uint64_t completedSize_ = 0;

   public:
    explicit ExtractToMemoryCallback(uint32_t index, std::vector<uint8_t>& buffer,
                                     const std::wstring& password = L"",
                                     ExtractProgressCallback progressCallback = nullptr);

    virtual ~ExtractToMemoryCallback() = default;
};

// 提取到文件的回调
class ExtractToFileCallback Z7_final : public IArchiveExtractCallback,
                                       public ICryptoGetTextPassword,
                                       public CMyUnknownImp {
   public:
    // IUnknown methods
    Z7_COM_UNKNOWN_IMP_SPEC(Z7_COM_QI_ENTRY_UNKNOWN(IArchiveExtractCallback)
                                Z7_COM_QI_ENTRY(IArchiveExtractCallback)
                                    Z7_COM_QI_ENTRY(ICryptoGetTextPassword))

    // IProgress methods (inherited via IArchiveExtractCallback)
    __declspec(nothrow) STDMETHODIMP SetTotal(UInt64 total) throw() override final;
    __declspec(nothrow) STDMETHODIMP
    SetCompleted(const UInt64* completeValue) throw() override final;

    // IArchiveExtractCallback methods
    __declspec(nothrow) STDMETHODIMP GetStream(UInt32 index, ISequentialOutStream** outStream,
                                               Int32 askExtractMode) throw() override final;
    __declspec(nothrow) STDMETHODIMP PrepareOperation(Int32 askExtractMode) throw() override final;
    __declspec(nothrow) STDMETHODIMP SetOperationResult(Int32 opRes) throw() override final;

    // ICryptoGetTextPassword method
    __declspec(nothrow) STDMETHODIMP CryptoGetTextPassword(BSTR* password) throw() override final;

   private:
    ::IInArchive* archive_;
    std::filesystem::path outputPath_;
    std::wstring password_;
    ExtractProgressCallback progressCallback_;

    CMyComPtr<ISequentialOutStream> currentOutStream_;
    uint64_t totalSize_ = 0;
    uint64_t completedSize_ = 0;

   public:
    explicit ExtractToFileCallback(::IInArchive* archive, const std::filesystem::path& outputPath,
                                   const std::wstring& password = L"",
                                   ExtractProgressCallback progressCallback = nullptr);

    virtual ~ExtractToFileCallback() = default;
};

// 提取到目录的回调
class ExtractToDirectoryCallback Z7_final : public IArchiveExtractCallback,
                                            public ICryptoGetTextPassword,
                                            public CMyUnknownImp {
   public:
    // IUnknown methods
    Z7_COM_UNKNOWN_IMP_SPEC(Z7_COM_QI_ENTRY_UNKNOWN(IArchiveExtractCallback)
                                Z7_COM_QI_ENTRY(IArchiveExtractCallback)
                                    Z7_COM_QI_ENTRY(ICryptoGetTextPassword))

    // IProgress methods (inherited via IArchiveExtractCallback)
    __declspec(nothrow) STDMETHODIMP SetTotal(UInt64 total) throw() override final;
    __declspec(nothrow) STDMETHODIMP
    SetCompleted(const UInt64* completeValue) throw() override final;

    // IArchiveExtractCallback methods
    __declspec(nothrow) STDMETHODIMP GetStream(UInt32 index, ISequentialOutStream** outStream,
                                               Int32 askExtractMode) throw() override final;
    __declspec(nothrow) STDMETHODIMP PrepareOperation(Int32 askExtractMode) throw() override final;
    __declspec(nothrow) STDMETHODIMP SetOperationResult(Int32 opRes) throw() override final;

    // ICryptoGetTextPassword method
    __declspec(nothrow) STDMETHODIMP CryptoGetTextPassword(BSTR* password) throw() override final;

   private:
    ::IInArchive* archive_;
    std::filesystem::path outputDir_;
    std::wstring password_;
    ExtractProgressCallback progressCallback_;

    CMyComPtr<ISequentialOutStream> currentOutStream_;
    std::filesystem::path currentFilePath_;
    uint64_t totalSize_ = 0;
    uint64_t completedSize_ = 0;
    uint32_t currentIndex_ = 0;

   public:
    explicit ExtractToDirectoryCallback(IInArchive* archive, const std::filesystem::path& outputDir,
                                        const std::wstring& password = L"",
                                        ExtractProgressCallback progressCallback = nullptr);

    virtual ~ExtractToDirectoryCallback() = default;
};

}  // namespace sevenzip::detail
