# 7-Zip PATCHES

此文件记录对 third_party/7zip 源码或构建脚本所做的任何必要修改。

目前更改：

- 2025-12-07: 添加 CMake 支持文件（由项目维护者）
  - `CMakeLists.txt`（third_party/7zip/）: 将 `C` 和 `CPP` 子目录加入 CMake 构建。
  - `C/CMakeLists.txt`: 根据原始 7-Zip C 源文件列举并创建 `7zip_c` 静态库，加入平台宏和编译选项。
  - `CPP/CMakeLists.txt`: 为 CPP 层添加更具容错性的 CMake 文件：按模块列出常见源并仅在存在时添加，以避免源树差异导致配置错误；链入 `SevenZip::C`（如可用）。

后续步骤建议：

- 在对 7-Zip 源码进行任何修改前，先在此记录变更目的与理由。
- 如果对源文件做补丁，请为每个补丁增加一个小节：文件名、修改内容、理由与作者。
