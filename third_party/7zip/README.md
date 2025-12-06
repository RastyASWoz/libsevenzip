7-Zip third_party placement

This directory is intended to contain a copy of the 7-Zip source used by the project.

Recommended (preferred): copy the `C/`, `CPP/`, and optional `Asm/` directories
from the provided 7z source into this folder.

If you'd rather keep a single copy at the repo root, update `CMakeLists.txt` to
point to the existing `../C` and `../CPP` paths.

Current workspace already contains `C/`, `CPP/`, `Asm/` at the repository root.
