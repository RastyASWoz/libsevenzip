#pragma once

#include <algorithm>
#include <cstring>
#include <vector>

#include "stream_base.hpp"

namespace sevenzip::detail {

/**
 * @brief 基于内存缓冲区的输入流
 *
 * 从现有的内存缓冲区读取数据（不拷贝数据）
 */
class MemoryInStream : public COMInStreamImpl {
   public:
    /**
     * @brief 从内存缓冲区构造输入流
     * @param data 数据指针（调用者必须保证生命周期）
     * @param size 数据大小
     */
    MemoryInStream(const void* data, size_t size)
        : data_(static_cast<const Byte*>(data)), size_(size), position_(0) {
        if (!data && size > 0) {
            throw Exception(ErrorCode::InvalidArgument, "data is null but size > 0");
        }
    }

    /**
     * @brief 从 vector 构造输入流
     * @param buffer vector 引用（调用者必须保证生命周期）
     */
    explicit MemoryInStream(const std::vector<Byte>& buffer)
        : MemoryInStream(buffer.data(), buffer.size()) {}

   protected:
    HRESULT DoRead(void* data, UInt32 size, UInt32* processedSize) override {
        if (!data && size > 0) {
            return E_POINTER;
        }

        UInt64 remaining = size_ - position_;
        UInt32 toRead = static_cast<UInt32>(std::min(static_cast<UInt64>(size), remaining));

        if (toRead > 0) {
            std::memcpy(data, data_ + position_, toRead);
            position_ += toRead;
        }

        if (processedSize) {
            *processedSize = toRead;
        }

        return S_OK;
    }

    HRESULT DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) override {
        UInt64 newPos = 0;

        switch (seekOrigin) {
            case STREAM_SEEK_SET:  // 从开始
                newPos = static_cast<UInt64>(offset);
                break;
            case STREAM_SEEK_CUR:  // 从当前位置
                if (offset < 0 && static_cast<UInt64>(-offset) > position_) {
                    return E_INVALIDARG;  // 会变成负数
                }
                newPos = position_ + offset;
                break;
            case STREAM_SEEK_END:  // 从结尾
                if (offset < 0 && static_cast<UInt64>(-offset) > size_) {
                    return E_INVALIDARG;
                }
                newPos = size_ + offset;
                break;
            default:
                return E_INVALIDARG;
        }

        // 允许 seek 到末尾后面（标准行为）
        position_ = newPos;

        if (newPosition) {
            *newPosition = newPos;
        }

        return S_OK;
    }

    HRESULT DoGetSize(UInt64* size) override {
        if (!size) {
            return E_POINTER;
        }
        *size = size_;
        return S_OK;
    }

    HRESULT DoGetProps(UInt64* size, FILETIME* cTime, FILETIME* aTime, FILETIME* mTime,
                       UInt32* attrib) override {
        // Memory streams don't have file attributes
        // Only return size, other parameters are optional
        if (size) {
            *size = size_;
        }

        // Set zero times and default attributes
        if (cTime) {
            cTime->dwLowDateTime = 0;
            cTime->dwHighDateTime = 0;
        }
        if (aTime) {
            aTime->dwLowDateTime = 0;
            aTime->dwHighDateTime = 0;
        }
        if (mTime) {
            mTime->dwLowDateTime = 0;
            mTime->dwHighDateTime = 0;
        }
        if (attrib) {
            *attrib = 0;  // FILE_ATTRIBUTE_NORMAL equivalent
        }

        return S_OK;
    }

   private:
    const Byte* data_;
    UInt64 size_;
    UInt64 position_;
};

/**
 * @brief 基于内存缓冲区的输出流
 *
 * 写入数据到动态增长的 vector
 */
class MemoryOutStream : public COMOutStreamImpl {
   public:
    /**
     * @brief 构造输出流
     * @param buffer 目标 vector（调用者必须保证生命周期）
     * @param initialCapacity 初始容量（可选）
     */
    explicit MemoryOutStream(std::vector<Byte>& buffer, size_t initialCapacity = 0)
        : buffer_(buffer), position_(0) {
        if (initialCapacity > 0) {
            buffer_.reserve(initialCapacity);
        }
    }

    /**
     * @brief 获取当前缓冲区大小
     */
    size_t size() const { return buffer_.size(); }

    /**
     * @brief 获取当前位置
     */
    UInt64 position() const { return position_; }

   protected:
    HRESULT DoWrite(const void* data, UInt32 size, UInt32* processedSize) override {
        if (!data && size > 0) {
            return E_POINTER;
        }

        try {
            // 确保缓冲区足够大
            UInt64 requiredSize = position_ + size;
            if (requiredSize > buffer_.size()) {
                buffer_.resize(static_cast<size_t>(requiredSize));
            }

            // 写入数据
            std::memcpy(buffer_.data() + position_, data, size);
            position_ += size;

            if (processedSize) {
                *processedSize = size;
            }

            return S_OK;
        } catch (const std::bad_alloc&) {
            return E_OUTOFMEMORY;
        } catch (...) {
            return E_FAIL;
        }
    }

    HRESULT DoSeek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) override {
        UInt64 newPos = 0;
        UInt64 currentSize = buffer_.size();

        switch (seekOrigin) {
            case STREAM_SEEK_SET:
                newPos = static_cast<UInt64>(offset);
                break;
            case STREAM_SEEK_CUR:
                if (offset < 0 && static_cast<UInt64>(-offset) > position_) {
                    return E_INVALIDARG;
                }
                newPos = position_ + offset;
                break;
            case STREAM_SEEK_END:
                if (offset < 0 && static_cast<UInt64>(-offset) > currentSize) {
                    return E_INVALIDARG;
                }
                newPos = currentSize + offset;
                break;
            default:
                return E_INVALIDARG;
        }

        position_ = newPos;

        if (newPosition) {
            *newPosition = newPos;
        }

        return S_OK;
    }

    HRESULT DoSetSize(UInt64 newSize) override {
        try {
            buffer_.resize(static_cast<size_t>(newSize));
            // 如果新大小小于当前位置，调整位置
            if (position_ > newSize) {
                position_ = newSize;
            }
            return S_OK;
        } catch (const std::bad_alloc&) {
            return E_OUTOFMEMORY;
        } catch (...) {
            return E_FAIL;
        }
    }

   private:
    std::vector<Byte>& buffer_;
    UInt64 position_;
};

}  // namespace sevenzip::detail
