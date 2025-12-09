#pragma once

// 必须最先包含:7-Zip Windows兼容层
#include "../common/wrapper_error.hpp"
#include "Common/MyWindows.h"

// 7-Zip headers
#include <cstdint>
#include <memory>

#include "7zip/IStream.h"
#include "Common/MyCom.h"

namespace sevenzip::detail {

// Type aliases
using UInt32 = uint32_t;
using UInt64 = uint64_t;
using Int64 = int64_t;
using Byte = uint8_t;

/**
 * @brief COM IInStream 实现的基类
 *
 * 提供 IInStream, IStreamGetSize 和 IStreamGetProps 接口的默认实现，
 * 派生类只需实现 DoRead, DoSeek, DoGetSize, DoGetProps 方法
 */
class COMInStreamImpl : public IInStream,
                        public IStreamGetSize,
                        public IStreamGetProps,
                        public CMyUnknownImp {
   public:
    Z7_COM_UNKNOWN_IMP_3(IInStream, IStreamGetSize, IStreamGetProps)
    template <class T>
    friend class CMyComPtr;  // 允许CMyComPtr访问AddRef/Release

   public:  // 重新声明public，因为Z7_COM_UNKNOWN_IMP宏内部有private声明
    // ISequentialInStream
    STDMETHOD(Read)(void* data, UInt32 size, UInt32* processedSize) override final {
        try {
            return DoRead(data, size, processedSize);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

    // IInStream
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) override final {
        try {
            return DoSeek(offset, seekOrigin, newPosition);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

    // IStreamGetSize
    STDMETHOD(GetSize)(UInt64* size) override final {
        try {
            return DoGetSize(size);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

    // IStreamGetProps
    STDMETHOD(GetProps)(UInt64* size, FILETIME* cTime, FILETIME* aTime, FILETIME* mTime,
                        UInt32* attrib) override final {
        try {
            return DoGetProps(size, cTime, aTime, mTime, attrib);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

   protected:
    virtual ~COMInStreamImpl() = default;

    // 派生类需要实现的方法
    virtual HRESULT DoRead(void* data, UInt32 size, UInt32* processedSize) = 0;
    virtual HRESULT DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) = 0;
    virtual HRESULT DoGetSize(UInt64* size) = 0;
    virtual HRESULT DoGetProps(UInt64* size, FILETIME* cTime, FILETIME* aTime, FILETIME* mTime,
                               UInt32* attrib) = 0;
};

/**
 * @brief COM IOutStream 实现的基类
 *
 * 提供 IOutStream 接口的默认实现，
 * 派生类只需实现 DoWrite, DoSeek, DoSetSize 方法
 */
class COMOutStreamImpl : public IOutStream, public CMyUnknownImp {
   public:
    Z7_COM_UNKNOWN_IMP_1(IOutStream)
    template <class T>
    friend class CMyComPtr;  // 允许CMyComPtr访问AddRef/Release

   public:  // 重新声明public，因为Z7_COM_UNKNOWN_IMP宏内部有private声明
    // ISequentialOutStream
    STDMETHOD(Write)(const void* data, UInt32 size, UInt32* processedSize) override final {
        try {
            return DoWrite(data, size, processedSize);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

    // IOutStream
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) override final {
        try {
            return DoSeek(offset, seekOrigin, newPosition);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

    STDMETHOD(SetSize)(UInt64 newSize) override final {
        try {
            return DoSetSize(newSize);
        } catch (const Exception& e) {
            return E_FAIL;
        } catch (...) {
            return E_FAIL;
        }
    }

   protected:
    virtual ~COMOutStreamImpl() = default;

    // 派生类需要实现的方法
    virtual HRESULT DoWrite(const void* data, UInt32 size, UInt32* processedSize) = 0;
    virtual HRESULT DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) = 0;
    virtual HRESULT DoSetSize(UInt64 newSize) = 0;
};

// COM 智能指针类型别名
template <typename T>
using ComPtr = CMyComPtr<T>;

}  // namespace sevenzip::detail
