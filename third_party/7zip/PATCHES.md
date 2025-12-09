# 7-Zip PATCHES

此文件记录对 third_party/7zip 源码或构建脚本所做的任何必要修改。

目前更改：

- 2025-12-07: 添加 CMake 支持文件（由项目维护者）
  - `CMakeLists.txt`（third_party/7zip/）: 将 `C` 和 `CPP` 子目录加入 CMake 构建。
  - `C/CMakeLists.txt`: 根据原始 7-Zip C 源文件列举并创建 `7zip_c` 静态库，加入平台宏和编译选项。
  - `CPP/CMakeLists.txt`: 为 CPP 层添加更具容错性的 CMake 文件：按模块列出常见源并仅在存在时添加，以避免源树差异导致配置错误；链入 `SevenZip::C`（如可用）。

- 2025-XX-XX: 修复 codec_guids_manual.cpp 中的 COM 接口 GUID 定义错误
  - **文件**: `CPP/7zip/Compress/codec_guids_manual.cpp`
  - **问题**: 多个 IID 值被错误地定义，导致 COM QueryInterface 返回错误的接口指针
  - **症状**: 使用密码加密文件时崩溃（SEH 0xc0000005）
  - **根本原因**: `IID_ICompressSetInStream` 被错误定义为 `0x20`（与 `IID_ICompressSetCoderProperties` 相同），
    正确值应为 `0x31`。这导致当 `CFilterCoder::QueryInterface` 被调用时，请求 `ICompressSetCoderProperties` 
    实际返回了指向 `ICompressSetInStream` vtable 的指针，进而调用错误的方法导致崩溃。
  - **修复**:
    - `IID_ICompressSetInStream`: `0x20` → `0x31`
    - `IID_ICompressSetOutStream`: `0x21` → `0x32`
    - `IID_ICompressSetInStream2`: `0x25` → `0x37`
    - `IID_ICompressSetBufSize`: `0x22` → `0x35`
    - `IID_ICompressInitEncoder`: `0x27` → `0x36`
    - `IID_ICompressSetFinishMode`: `0x28` → `0x26`
    - `IID_ICompressGetInStreamProcessedSize2`: `0x2E` → `0x27`
    - `IID_ICompressSetCoderMt`: `0x2B` → `0x25`
    - `IID_ICompressSetMemLimit`: `0x2D` → `0x28`
    - `IID_ICompressGetSubStreamSize`: `0x31` → `0x30`
    - `IID_ICompressSetOutStreamSize`: `0x70` → `0x34`
    - `IID_ICompressReadUnusedFromInBuf`: `0x71` → `0x29`
    - 添加缺失的 `IID_ICompressFilter`: `0x40`
  - **参考**: 正确的值见 `CPP/7zip/ICoder.h` 中的 `Z7_IFACE_CONSTR_CODER` 宏定义

后续步骤建议：

- 在对 7-Zip 源码进行任何修改前，先在此记录变更目的与理由。
- 如果对源文件做补丁，请为每个补丁增加一个小节：文件名、修改内容、理由与作者。
