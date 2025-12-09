#pragma once

/**
 * @file stream_file_native.h
 * @brief C 接口的文件 I/O 封装，用于隔离 Windows SDK 冲突
 *
 * 这个文件提供纯 C 接口，避免 C++ 头文件污染。
 * 在 stream_file_win32.cpp 中实现（Windows 平台）。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// 不透明的文件句柄类型
typedef void* NativeFileHandle;

/**
 * @brief 创建文件句柄（打开读取）
 * @param path 文件路径（UTF-16 宽字符）
 * @return 文件句柄，失败返回 NULL
 */
NativeFileHandle NativeFile_OpenRead(const wchar_t* path);

/**
 * @brief 创建文件句柄（打开写入）
 * @param path 文件路径（UTF-16 宽字符）
 * @param create_always true=覆盖现有文件，false=追加
 * @return 文件句柄，失败返回 NULL
 */
NativeFileHandle NativeFile_OpenWrite(const wchar_t* path, bool create_always);

/**
 * @brief 关闭文件句柄
 * @param handle 文件句柄
 */
void NativeFile_Close(NativeFileHandle handle);

/**
 * @brief 读取数据
 * @param handle 文件句柄
 * @param data 数据缓冲区
 * @param size 要读取的字节数
 * @param processed 实际读取的字节数（输出）
 * @return true 成功，false 失败
 */
bool NativeFile_Read(NativeFileHandle handle, void* data, uint32_t size, uint32_t* processed);

/**
 * @brief 写入数据
 * @param handle 文件句柄
 * @param data 数据缓冲区
 * @param size 要写入的字节数
 * @param processed 实际写入的字节数（输出）
 * @return true 成功，false 失败
 */
bool NativeFile_Write(NativeFileHandle handle, const void* data, uint32_t size,
                      uint32_t* processed);

/**
 * @brief Seek 操作
 * @param handle 文件句柄
 * @param offset 偏移量
 * @param origin 起始位置（0=开始, 1=当前, 2=结尾）
 * @param new_position 新位置（输出）
 * @return true 成功，false 失败
 */
bool NativeFile_Seek(NativeFileHandle handle, int64_t offset, uint32_t origin,
                     uint64_t* new_position);

/**
 * @brief 获取文件大小
 * @param handle 文件句柄
 * @param size 文件大小（输出）
 * @return true 成功，false 失败
 */
bool NativeFile_GetSize(NativeFileHandle handle, uint64_t* size);

/**
 * @brief 设置文件大小
 * @param handle 文件句柄
 * @param size 新的文件大小
 * @return true 成功，false 失败
 */
bool NativeFile_SetSize(NativeFileHandle handle, uint64_t size);

/**
 * @brief 文件属性结构
 */
typedef struct {
    uint32_t attrib;          // 文件属性 (FILE_ATTRIBUTE_*)
    uint64_t creationTime;    // 创建时间 (FILETIME)
    uint64_t lastAccessTime;  // 访问时间 (FILETIME)
    uint64_t lastWriteTime;   // 修改时间 (FILETIME)
    uint64_t fileSize;        // 文件大小
} NativeFileAttributes;

/**
 * @brief 获取文件属性
 * @param path 文件路径（UTF-16 宽字符）
 * @param attrs 文件属性（输出）
 * @return true 成功，false 失败
 */
bool NativeFile_GetAttributes(const wchar_t* path, NativeFileAttributes* attrs);

/**
 * @brief 获取最后的错误代码（Windows: GetLastError()）
 * @return 错误代码
 */
uint32_t NativeFile_GetLastError();

#ifdef __cplusplus
}
#endif
