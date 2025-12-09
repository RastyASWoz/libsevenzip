/**
 * @file stream_file_win32.cpp
 * @brief Windows 平台的文件 I/O 实现（隔离编译单元）
 *
 * 这个文件仅包含 Windows API，不包含任何 7-Zip 的 COM 头文件，
 * 从而避免 Windows SDK 与 7-Zip COM 接口的冲突。
 */

// 仅包含必要的 Windows 头文件
// 注意: 不定义 WIN32_LEAN_AND_MEAN - 虽然这里不需要COM,但为了与其他编译单元一致
#define NOMINMAX
#include <windows.h>

#include "stream_file_native.h"

// ============================================================================
// 实现
// ============================================================================

NativeFileHandle NativeFile_OpenRead(const wchar_t* path) {
    if (!path) {
        return nullptr;
    }

    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    return static_cast<NativeFileHandle>(hFile);
}

NativeFileHandle NativeFile_OpenWrite(const wchar_t* path, bool create_always) {
    if (!path) {
        return nullptr;
    }

    DWORD creationDisposition = create_always ? CREATE_ALWAYS : OPEN_ALWAYS;

    HANDLE hFile = CreateFileW(path, GENERIC_WRITE,
                               0,  // 独占访问
                               nullptr, creationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    return static_cast<NativeFileHandle>(hFile);
}

void NativeFile_Close(NativeFileHandle handle) {
    if (handle) {
        CloseHandle(static_cast<HANDLE>(handle));
    }
}

bool NativeFile_Read(NativeFileHandle handle, void* data, uint32_t size, uint32_t* processed) {
    if (!handle || !data) {
        if (processed) *processed = 0;
        return false;
    }

    DWORD bytesRead = 0;
    BOOL result = ReadFile(static_cast<HANDLE>(handle), data, size, &bytesRead, nullptr);

    if (processed) {
        *processed = static_cast<uint32_t>(bytesRead);
    }

    return result != FALSE;
}

bool NativeFile_Write(NativeFileHandle handle, const void* data, uint32_t size,
                      uint32_t* processed) {
    if (!handle || !data) {
        if (processed) *processed = 0;
        return false;
    }

    DWORD bytesWritten = 0;
    BOOL result = WriteFile(static_cast<HANDLE>(handle), data, size, &bytesWritten, nullptr);

    if (processed) {
        *processed = static_cast<uint32_t>(bytesWritten);
    }

    return result != FALSE;
}

bool NativeFile_Seek(NativeFileHandle handle, int64_t offset, uint32_t origin,
                     uint64_t* new_position) {
    if (!handle) {
        return false;
    }

    DWORD moveMethod;
    switch (origin) {
        case 0:
            moveMethod = FILE_BEGIN;
            break;
        case 1:
            moveMethod = FILE_CURRENT;
            break;
        case 2:
            moveMethod = FILE_END;
            break;
        default:
            return false;
    }

    LARGE_INTEGER li;
    li.QuadPart = offset;

    LARGE_INTEGER newPos;
    BOOL result = SetFilePointerEx(static_cast<HANDLE>(handle), li, &newPos, moveMethod);

    if (result && new_position) {
        *new_position = static_cast<uint64_t>(newPos.QuadPart);
    }

    return result != FALSE;
}

bool NativeFile_GetSize(NativeFileHandle handle, uint64_t* size) {
    if (!handle || !size) {
        return false;
    }

    LARGE_INTEGER fileSize;
    BOOL result = GetFileSizeEx(static_cast<HANDLE>(handle), &fileSize);

    if (result) {
        *size = static_cast<uint64_t>(fileSize.QuadPart);
    }

    return result != FALSE;
}

bool NativeFile_SetSize(NativeFileHandle handle, uint64_t size) {
    if (!handle) {
        return false;
    }

    // 保存当前位置
    LARGE_INTEGER currentPos;
    LARGE_INTEGER zero = {0};
    if (!SetFilePointerEx(static_cast<HANDLE>(handle), zero, &currentPos, FILE_CURRENT)) {
        return false;
    }

    // Seek 到目标大小
    LARGE_INTEGER newSize;
    newSize.QuadPart = static_cast<LONGLONG>(size);
    if (!SetFilePointerEx(static_cast<HANDLE>(handle), newSize, nullptr, FILE_BEGIN)) {
        return false;
    }

    // 设置文件结束标记
    BOOL result = SetEndOfFile(static_cast<HANDLE>(handle));

    // 恢复原来的位置
    SetFilePointerEx(static_cast<HANDLE>(handle), currentPos, nullptr, FILE_BEGIN);

    return result != FALSE;
}

bool NativeFile_GetAttributes(const wchar_t* path, NativeFileAttributes* attrs) {
    if (!path || !attrs) {
        ::SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!::GetFileAttributesExW(path, GetFileExInfoStandard, &data)) {
        return false;
    }

    attrs->attrib = data.dwFileAttributes;

    // 将 FILETIME 转换为 64 位整数
    attrs->creationTime = (static_cast<uint64_t>(data.ftCreationTime.dwHighDateTime) << 32) |
                          data.ftCreationTime.dwLowDateTime;
    attrs->lastAccessTime = (static_cast<uint64_t>(data.ftLastAccessTime.dwHighDateTime) << 32) |
                            data.ftLastAccessTime.dwLowDateTime;
    attrs->lastWriteTime = (static_cast<uint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) |
                           data.ftLastWriteTime.dwLowDateTime;
    attrs->fileSize = (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;

    return true;
}

uint32_t NativeFile_GetLastError() {
    return static_cast<uint32_t>(::GetLastError());
}
