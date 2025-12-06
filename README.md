libsevenzip - Modern C++ wrapper for 7-Zip

This directory contains the project scaffold for `libsevenzip`. It is intended
to hold a copied snapshot of the 7-Zip sources under `third_party/7zip/` and the
wrapper/FFI/C++ API layers in `src/`.

Quick notes:
- `third_party/7zip/VERSION.txt` documents the 7-Zip snapshot used.
- To include the 7-Zip sources in-tree, copy the `C/` and `CPP/` folders from
  the repository root (which are currently present) into
  `libsevenzip/third_party/7zip/`.

Next recommended steps:
1. `cd libsevenzip`
2. `git init` (if creating a new repo)
3. Copy the 7-Zip `C/` and `CPP/` directories into `third_party/7zip/` or
   update CMake to point to the existing root-level `C/` and `CPP/`.
