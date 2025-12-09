#include <gtest/gtest.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>  // for Sleep
#include <objbase.h>
#endif

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "wrapper/stream/stream_file.hpp"
#include "wrapper/stream/stream_memory.hpp"

// 声明 7-Zip COM 接口 IID
#include "7zip/IStream.h"

EXTERN_C const GUID IID_IInStream;
EXTERN_C const GUID IID_IOutStream;
EXTERN_C const GUID IID_IStreamGetSize;

using namespace sevenzip;
using namespace sevenzip::detail;

namespace fs = std::filesystem;

// ======================== MemoryInStream 测试 ========================

TEST(MemoryInStreamTest, ConstructFromBuffer) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    ASSERT_TRUE(stream);
}

TEST(MemoryInStreamTest, ReadData) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    uint8_t buffer[3];
    UInt32 processed = 0;

    HRESULT hr = stream->Read(buffer, 3, &processed);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(processed, 3u);
    EXPECT_EQ(buffer[0], 10);
    EXPECT_EQ(buffer[1], 20);
    EXPECT_EQ(buffer[2], 30);
}

TEST(MemoryInStreamTest, ReadBeyondEnd) {
    std::vector<uint8_t> data = {1, 2, 3};
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    uint8_t buffer[10];
    UInt32 processed = 0;

    HRESULT hr = stream->Read(buffer, 10, &processed);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(processed, 3u);  // 只能读取 3 字节
}

TEST(MemoryInStreamTest, SeekSet) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    UInt64 newPos = 0;
    HRESULT hr = stream->Seek(2, STREAM_SEEK_SET, &newPos);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(newPos, 2u);

    uint8_t buffer[2];
    UInt32 processed = 0;
    hr = stream->Read(buffer, 2, &processed);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(buffer[0], 30);
    EXPECT_EQ(buffer[1], 40);
}

TEST(MemoryInStreamTest, SeekCurrent) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    // 先读 2 字节
    uint8_t dummy[2];
    stream->Read(dummy, 2, nullptr);

    // 从当前位置 seek +1
    UInt64 newPos = 0;
    HRESULT hr = stream->Seek(1, STREAM_SEEK_CUR, &newPos);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(newPos, 3u);

    uint8_t buffer[1];
    UInt32 processed = 0;
    stream->Read(buffer, 1, &processed);
    EXPECT_EQ(buffer[0], 40);
}

TEST(MemoryInStreamTest, SeekEnd) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    UInt64 newPos = 0;
    HRESULT hr = stream->Seek(-2, STREAM_SEEK_END, &newPos);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(newPos, 3u);

    uint8_t buffer[2];
    UInt32 processed = 0;
    stream->Read(buffer, 2, &processed);
    EXPECT_EQ(buffer[0], 40);
    EXPECT_EQ(buffer[1], 50);
}

TEST(MemoryInStreamTest, GetSize) {
    std::vector<uint8_t> data(100);
    CMyComPtr<MemoryInStream> stream(new MemoryInStream(data));

    UInt64 size = 0;
    HRESULT hr = stream->GetSize(&size);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(size, 100u);
}

// ======================== MemoryOutStream 测试 ========================

TEST(MemoryOutStreamTest, WriteData) {
    std::vector<uint8_t> buffer;
    CMyComPtr<MemoryOutStream> stream(new MemoryOutStream(buffer));

    uint8_t data[] = {10, 20, 30};
    UInt32 processed = 0;

    HRESULT hr = stream->Write(data, 3, &processed);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(processed, 3u);
    EXPECT_EQ(buffer.size(), 3u);
    EXPECT_EQ(buffer[0], 10);
    EXPECT_EQ(buffer[1], 20);
    EXPECT_EQ(buffer[2], 30);
}

TEST(MemoryOutStreamTest, WriteMultipleTimes) {
    std::vector<uint8_t> buffer;
    CMyComPtr<MemoryOutStream> stream(new MemoryOutStream(buffer));

    uint8_t data1[] = {1, 2};
    uint8_t data2[] = {3, 4, 5};

    stream->Write(data1, 2, nullptr);
    stream->Write(data2, 3, nullptr);

    EXPECT_EQ(buffer.size(), 5u);
    EXPECT_EQ(buffer[0], 1);
    EXPECT_EQ(buffer[2], 3);
    EXPECT_EQ(buffer[4], 5);
}

TEST(MemoryOutStreamTest, SeekAndWrite) {
    std::vector<uint8_t> buffer = {1, 2, 3, 4, 5};
    CMyComPtr<MemoryOutStream> stream(new MemoryOutStream(buffer));

    // Seek 到位置 2
    stream->Seek(2, STREAM_SEEK_SET, nullptr);

    // 写入数据（会覆盖）
    uint8_t data[] = {99, 88};
    stream->Write(data, 2, nullptr);

    EXPECT_EQ(buffer[2], 99);
    EXPECT_EQ(buffer[3], 88);
    EXPECT_EQ(buffer[4], 5);  // 未被覆盖
}

TEST(MemoryOutStreamTest, SetSize) {
    std::vector<uint8_t> buffer = {1, 2, 3, 4, 5};
    CMyComPtr<MemoryOutStream> stream(new MemoryOutStream(buffer));

    HRESULT hr = stream->SetSize(3);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(buffer.size(), 3u);
}

TEST(MemoryOutStreamTest, SetSizeLarger) {
    std::vector<uint8_t> buffer = {1, 2, 3};
    CMyComPtr<MemoryOutStream> stream(new MemoryOutStream(buffer));

    HRESULT hr = stream->SetSize(10);
    EXPECT_EQ(hr, S_OK);
    EXPECT_EQ(buffer.size(), 10u);
}

// ======================== FileInStream 测试 ========================

TEST(FileInStreamTest, OpenNonexistentFile) {
    EXPECT_THROW({ FileInStream stream(L"nonexistent_file_12345.txt"); }, Exception);
}

TEST(FileInStreamTest, ReadFile) {
    // 创建临时测试文件
    fs::path tempFile = fs::temp_directory_path() / "test_read.bin";
    {
        std::ofstream ofs(tempFile, std::ios::binary);
        uint8_t data[] = {11, 22, 33, 44, 55};
        ofs.write(reinterpret_cast<const char*>(data), sizeof(data));
    }

    try {
        CMyComPtr<FileInStream> stream(new FileInStream(tempFile.wstring()));

        uint8_t buffer[5];
        UInt32 processed = 0;
        HRESULT hr = stream->Read(buffer, 5, &processed);

        EXPECT_EQ(hr, S_OK);
        EXPECT_EQ(processed, 5u);
        EXPECT_EQ(buffer[0], 11);
        EXPECT_EQ(buffer[4], 55);

        stream.Release();  // 释放文件句柄
#ifdef _WIN32
        Sleep(10);  // Windows 需要短暂延迟让系统释放句柄
#endif
        fs::remove(tempFile);
    } catch (...) {
        fs::remove(tempFile);
        throw;
    }
}

TEST(FileInStreamTest, GetFileSize) {
    // 创建临时测试文件
    fs::path tempFile = fs::temp_directory_path() / "test_size.bin";
    {
        std::ofstream ofs(tempFile, std::ios::binary);
        std::vector<uint8_t> data(1024, 0xFF);
        ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    try {
        CMyComPtr<FileInStream> stream(new FileInStream(tempFile.wstring()));

        UInt64 size = 0;
        HRESULT hr = stream->GetSize(&size);

        EXPECT_EQ(hr, S_OK);
        EXPECT_EQ(size, 1024u);

        stream.Release();  // 释放文件句柄
#ifdef _WIN32
        Sleep(10);  // Windows 需要短暂延迟让系统释放句柄
#endif
        fs::remove(tempFile);
    } catch (...) {
        fs::remove(tempFile);
        throw;
    }
}

TEST(FileInStreamTest, SeekInFile) {
    // 创建临时测试文件
    fs::path tempFile = fs::temp_directory_path() / "test_seek.bin";
    {
        std::ofstream ofs(tempFile, std::ios::binary);
        uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ofs.write(reinterpret_cast<const char*>(data), sizeof(data));
    }

    try {
        CMyComPtr<FileInStream> stream(new FileInStream(tempFile.wstring()));

        // Seek 到位置 5
        UInt64 newPos = 0;
        HRESULT hr = stream->Seek(5, STREAM_SEEK_SET, &newPos);
        EXPECT_EQ(hr, S_OK);
        EXPECT_EQ(newPos, 5u);

        // 读取数据
        uint8_t buffer[3];
        UInt32 processed = 0;
        stream->Read(buffer, 3, &processed);
        EXPECT_EQ(buffer[0], 5);
        EXPECT_EQ(buffer[1], 6);
        EXPECT_EQ(buffer[2], 7);

        stream.Release();  // 释放文件句柄
#ifdef _WIN32
        Sleep(10);  // Windows 需要短暂延迟让系统释放句柄
#endif
        fs::remove(tempFile);
    } catch (...) {
        fs::remove(tempFile);
        throw;
    }
}

// ======================== FileOutStream 测试 ========================

TEST(FileOutStreamTest, CreateAndWrite) {
    fs::path tempFile = fs::temp_directory_path() / "test_write.bin";

    try {
        {
            CMyComPtr<FileOutStream> stream(new FileOutStream(tempFile.wstring()));

            uint8_t data[] = {100, 101, 102};
            UInt32 processed = 0;
            HRESULT hr = stream->Write(data, 3, &processed);

            EXPECT_EQ(hr, S_OK);
            EXPECT_EQ(processed, 3u);
        }  // stream 在此析构并释放文件句柄

        // 验证文件内容
        std::ifstream ifs(tempFile, std::ios::binary);
        uint8_t buffer[3];
        ifs.read(reinterpret_cast<char*>(buffer), 3);
        ifs.close();  // 关闭验证文件句柄

        EXPECT_EQ(buffer[0], 100);
        EXPECT_EQ(buffer[1], 101);
        EXPECT_EQ(buffer[2], 102);

#ifdef _WIN32
        Sleep(10);  // Windows 需要短暂延迟让系统释放句柄
#endif
        fs::remove(tempFile);
    } catch (...) {
        fs::remove(tempFile);
        throw;
    }
}

TEST(FileOutStreamTest, SeekAndWrite) {
    fs::path tempFile = fs::temp_directory_path() / "test_seek_write.bin";

    try {
        {
            CMyComPtr<FileOutStream> stream(new FileOutStream(tempFile.wstring()));

            // 先写入 10 个字节
            uint8_t data1[10] = {0};
            stream->Write(data1, 10, nullptr);

            // Seek 到位置 5
            stream->Seek(5, STREAM_SEEK_SET, nullptr);

            // 写入新数据
            uint8_t data2[] = {77, 88, 99};
            stream->Write(data2, 3, nullptr);
        }

        // 验证文件内容
        std::ifstream ifs(tempFile, std::ios::binary);
        uint8_t buffer[10];
        ifs.read(reinterpret_cast<char*>(buffer), 10);
        ifs.close();  // 关闭验证文件句柄

        EXPECT_EQ(buffer[5], 77);
        EXPECT_EQ(buffer[6], 88);
        EXPECT_EQ(buffer[7], 99);

#ifdef _WIN32
        Sleep(10);  // Windows 需要短暂延迟让系统释放句柄
#endif
        fs::remove(tempFile);
    } catch (...) {
        fs::remove(tempFile);
        throw;
    }
}

TEST(FileOutStreamTest, SetSize) {
    fs::path tempFile = fs::temp_directory_path() / "test_setsize.bin";

    try {
        {
            CMyComPtr<FileOutStream> stream(new FileOutStream(tempFile.wstring()));

            // 写入一些数据
            uint8_t data[20] = {0};
            stream->Write(data, 20, nullptr);

            // 截断到 10 字节
            HRESULT hr = stream->SetSize(10);
            EXPECT_EQ(hr, S_OK);
        }

        // 验证文件大小
        EXPECT_EQ(fs::file_size(tempFile), 10u);

        fs::remove(tempFile);
    } catch (...) {
        fs::remove(tempFile);
        throw;
    }
}
