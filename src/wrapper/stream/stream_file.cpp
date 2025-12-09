// File stream implementation using native C interface to avoid Windows SDK conflicts
#include "stream_file.hpp"

#include "../common/wrapper_string.hpp"
#include "stream_file_native.h"

// 声明 7-Zip COM 接口 IID
#include <objbase.h>

#include "7zip/IStream.h"

EXTERN_C const GUID IID_IInStream;
EXTERN_C const GUID IID_IOutStream;
EXTERN_C const GUID IID_IStreamGetSize;

namespace sevenzip::detail {

//=============================================================================
// FileInStream
//=============================================================================

class FileInStream::Impl {
   public:
    NativeFileHandle handle = nullptr;
};

FileInStream::FileInStream(const std::wstring& path)
    : impl_(std::make_unique<Impl>()), path_(path) {
    impl_->handle = NativeFile_OpenRead(path.c_str());
    if (!impl_->handle) {
        throw Exception(ErrorCode::FileNotFound,
                        "Cannot open file for reading: " + wstring_to_utf8(path));
    }
}

FileInStream::FileInStream(const std::string& path) : FileInStream(utf8_to_wstring(path)) {}

FileInStream::~FileInStream() {
    if (impl_ && impl_->handle) {
        NativeFile_Close(impl_->handle);
        impl_->handle = nullptr;
    }
}

bool FileInStream::isOpen() const {
    return impl_ && impl_->handle != nullptr;
}

HRESULT FileInStream::DoRead(void* data, UInt32 size, UInt32* processedSize) {
    if (!data && size > 0) {
        return E_POINTER;
    }

    if (!isOpen()) {
        return E_HANDLE;
    }

    UInt32 realProcessed = 0;
    if (!NativeFile_Read(impl_->handle, data, size, &realProcessed)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    if (processedSize) {
        *processedSize = realProcessed;
    }

    return S_OK;
}

HRESULT FileInStream::DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) {
    if (!isOpen()) {
        return E_HANDLE;
    }

    UInt64 newPos = 0;
    if (!NativeFile_Seek(impl_->handle, offset, seekOrigin, &newPos)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    if (newPosition) {
        *newPosition = newPos;
    }

    return S_OK;
}

HRESULT FileInStream::DoGetSize(UInt64* size) {
    if (!size) {
        return E_POINTER;
    }

    if (!isOpen()) {
        return E_HANDLE;
    }

    UInt64 fileSize = 0;
    if (!NativeFile_GetSize(impl_->handle, &fileSize)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    *size = fileSize;
    return S_OK;
}

HRESULT FileInStream::DoGetProps(UInt64* size, FILETIME* cTime, FILETIME* aTime, FILETIME* mTime,
                                 UInt32* attrib) {
    if (!isOpen()) {
        return E_HANDLE;
    }

    // 获取文件属性
    NativeFileAttributes attrs;
    if (!NativeFile_GetAttributes(path_.c_str(), &attrs)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    // 设置大小
    if (size) {
        *size = attrs.fileSize;
    }

    // 设置时间（64 位整数转换为 FILETIME）
    if (cTime) {
        cTime->dwLowDateTime = static_cast<DWORD>(attrs.creationTime & 0xFFFFFFFF);
        cTime->dwHighDateTime = static_cast<DWORD>(attrs.creationTime >> 32);
    }
    if (aTime) {
        aTime->dwLowDateTime = static_cast<DWORD>(attrs.lastAccessTime & 0xFFFFFFFF);
        aTime->dwHighDateTime = static_cast<DWORD>(attrs.lastAccessTime >> 32);
    }
    if (mTime) {
        mTime->dwLowDateTime = static_cast<DWORD>(attrs.lastWriteTime & 0xFFFFFFFF);
        mTime->dwHighDateTime = static_cast<DWORD>(attrs.lastWriteTime >> 32);
    }

    // 设置属性
    if (attrib) {
        *attrib = attrs.attrib;
    }

    return S_OK;
}

//=============================================================================
// FileOutStream
//=============================================================================

class FileOutStream::Impl {
   public:
    NativeFileHandle handle = nullptr;
};

FileOutStream::FileOutStream(const std::wstring& path, bool createAlways)
    : impl_(std::make_unique<Impl>()), path_(path) {
    impl_->handle = NativeFile_OpenWrite(path.c_str(), createAlways);
    if (!impl_->handle) {
        throw Exception(ErrorCode::AccessDenied,
                        "Cannot open file for writing: " + wstring_to_utf8(path));
    }
}

FileOutStream::FileOutStream(const std::string& path, bool createAlways)
    : FileOutStream(utf8_to_wstring(path), createAlways) {}

FileOutStream::~FileOutStream() {
    if (impl_ && impl_->handle) {
        NativeFile_Close(impl_->handle);
        impl_->handle = nullptr;
    }
}

bool FileOutStream::isOpen() const {
    return impl_ && impl_->handle != nullptr;
}

HRESULT FileOutStream::DoWrite(const void* data, UInt32 size, UInt32* processedSize) {
    if (!data && size > 0) {
        return E_POINTER;
    }

    if (!isOpen()) {
        return E_HANDLE;
    }

    UInt32 realProcessed = 0;
    if (!NativeFile_Write(impl_->handle, data, size, &realProcessed)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    if (processedSize) {
        *processedSize = realProcessed;
    }

    return S_OK;
}

HRESULT FileOutStream::DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) {
    if (!isOpen()) {
        return E_HANDLE;
    }

    UInt64 newPos = 0;
    if (!NativeFile_Seek(impl_->handle, offset, seekOrigin, &newPos)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    if (newPosition) {
        *newPosition = newPos;
    }

    return S_OK;
}

HRESULT FileOutStream::DoSetSize(UInt64 newSize) {
    if (!isOpen()) {
        return E_HANDLE;
    }

    if (!NativeFile_SetSize(impl_->handle, newSize)) {
        return HRESULT_FROM_WIN32(NativeFile_GetLastError());
    }

    return S_OK;
}

}  // namespace sevenzip::detail
