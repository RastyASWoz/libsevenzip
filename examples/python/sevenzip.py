"""
SevenZip Python Bindings - 使用ctypes封装FFI C API

这是libsevenzip的Python绑定，提供了简洁的Python API来操作7z/ZIP/TAR等归档格式。

示例用法:
    from sevenzip import Archive, Writer, Compressor
    
    # 打开并读取归档
    with Archive.open('test.7z') as archive:
        print(f"包含 {archive.item_count} 个文件")
        for item in archive:
            print(f"  - {item.path} ({item.size} bytes)")
    
    # 创建新归档
    with Writer.create('output.7z', format='7z') as writer:
        writer.add_file('file.txt')
        writer.add_directory('folder', recursive=True)
        writer.set_password('secret')
    
    # 压缩单个文件
    compressor = Compressor('gzip', level='maximum')
    compressed = compressor.compress(data)
"""

import ctypes
from ctypes import c_char_p, c_void_p, c_uint32, c_uint64, c_size_t, c_int, POINTER, Structure
from enum import IntEnum
from pathlib import Path
from typing import Optional, Union, List, Iterator
import os
import sys


# ============================================================================
# 加载DLL
# ============================================================================

def _load_library():
    """加载sevenzip_ffi动态库"""
    if sys.platform == 'win32':
        lib_name = 'sevenzip_ffi.dll'
    elif sys.platform == 'darwin':
        lib_name = 'libsevenzip_ffi.dylib'
    else:
        lib_name = 'libsevenzip_ffi.so'
    
    # 尝试多个路径
    search_paths = [
        Path(__file__).parent / 'lib' / lib_name,  # 相对于此文件
        Path.cwd() / 'build' / 'windows-release' / 'bin' / 'Release' / lib_name,  # 开发环境 bin 目录
        Path.cwd() / 'build' / 'windows-release' / 'lib' / 'Release' / lib_name,  # 开发环境 lib 目录（旧）
        Path(lib_name),  # 系统路径
    ]
    
    for path in search_paths:
        if path.exists():
            return ctypes.CDLL(str(path))
    
    # 最后尝试直接加载
    try:
        return ctypes.CDLL(lib_name)
    except OSError as e:
        raise ImportError(f"无法加载 {lib_name}。尝试过的路径: {search_paths}") from e


_lib = _load_library()


# ============================================================================
# C类型定义
# ============================================================================

class SzResult(IntEnum):
    """错误码枚举"""
    OK = 0
    FAIL = 1
    OUT_OF_MEMORY = 2
    FILE_NOT_FOUND = 3
    ACCESS_DENIED = 4
    INVALID_ARGUMENT = 5
    UNSUPPORTED_FORMAT = 6
    CORRUPTED_ARCHIVE = 7
    WRONG_PASSWORD = 8
    UNKNOWN_ERROR = 9
    INDEX_OUT_OF_RANGE = 10
    BUFFER_TOO_SMALL = 11
    OPERATION_CANCELLED = 12
    WRITE_ERROR = 13
    READ_ERROR = 14
    NOT_IMPLEMENTED = 15
    DISK_FULL = 16


class SzFormat(IntEnum):
    """归档格式枚举"""
    SEVEN_Z = 0
    ZIP = 1
    TAR = 2
    GZIP = 3
    BZIP2 = 4
    XZ = 5
    RAR = 6
    CAB = 7
    ISO = 8
    WIM = 9


class SzCompressionLevel(IntEnum):
    """压缩级别枚举"""
    NONE = 0
    FAST = 1
    NORMAL = 2
    MAXIMUM = 3
    ULTRA = 4


# 不透明句柄类型
sz_archive_handle = c_void_p
sz_writer_handle = c_void_p
sz_compressor_handle = c_void_p


class SzItemInfo(Structure):
    """归档项目信息结构 - 必须完全匹配 C 结构体定义"""
    _fields_ = [
        ("index", c_size_t),           # size_t index
        ("path", c_char_p),            # char* path
        ("size", c_uint64),            # uint64_t size
        ("packed_size", c_uint64),     # uint64_t packed_size
        ("crc", c_uint32),             # uint32_t crc
        ("has_crc", c_int),            # int has_crc
        ("creation_time", ctypes.c_int64),     # int64_t creation_time
        ("modification_time", ctypes.c_int64), # int64_t modification_time
        ("is_directory", c_int),       # int is_directory
        ("is_encrypted", c_int),       # int is_encrypted
    ]


class SzArchiveInfo(Structure):
    """归档信息结构"""
    _fields_ = [
        ("format", c_int),
        ("total_size", c_uint64),
        ("item_count", c_uint32),
        ("is_solid", c_int),
        ("has_encrypted_headers", c_int),
    ]


# ============================================================================
# 函数签名声明
# ============================================================================

# 错误处理
_lib.sz_get_last_error_message.restype = c_char_p
_lib.sz_get_last_error_message.argtypes = []

_lib.sz_clear_error.restype = None
_lib.sz_clear_error.argtypes = []

# Archive Reader
_lib.sz_archive_open.restype = c_int
_lib.sz_archive_open.argtypes = [c_char_p, POINTER(sz_archive_handle)]

_lib.sz_archive_close.restype = None
_lib.sz_archive_close.argtypes = [sz_archive_handle]

_lib.sz_archive_get_item_count.restype = c_int
_lib.sz_archive_get_item_count.argtypes = [sz_archive_handle, POINTER(c_size_t)]

_lib.sz_archive_get_item_info.restype = c_int
_lib.sz_archive_get_item_info.argtypes = [sz_archive_handle, c_size_t, POINTER(SzItemInfo)]

_lib.sz_archive_extract_to_memory.restype = c_int
_lib.sz_archive_extract_to_memory.argtypes = [
    sz_archive_handle, c_size_t, POINTER(c_void_p), POINTER(c_size_t)
]

_lib.sz_memory_free.restype = None
_lib.sz_memory_free.argtypes = [c_void_p]

# Archive Writer
_lib.sz_writer_create.restype = c_int
_lib.sz_writer_create.argtypes = [c_char_p, c_int, POINTER(sz_writer_handle)]

_lib.sz_writer_add_file.restype = c_int
_lib.sz_writer_add_file.argtypes = [sz_writer_handle, c_char_p, c_char_p]

_lib.sz_writer_add_directory.restype = c_int
_lib.sz_writer_add_directory.argtypes = [sz_writer_handle, c_char_p, c_int]

_lib.sz_writer_add_memory.restype = c_int
_lib.sz_writer_add_memory.argtypes = [sz_writer_handle, c_void_p, c_size_t, c_char_p]

_lib.sz_writer_set_password.restype = c_int
_lib.sz_writer_set_password.argtypes = [sz_writer_handle, c_char_p]

_lib.sz_writer_set_compression_level.restype = c_int
_lib.sz_writer_set_compression_level.argtypes = [sz_writer_handle, c_int]

_lib.sz_writer_finalize.restype = c_int
_lib.sz_writer_finalize.argtypes = [sz_writer_handle]

_lib.sz_writer_cancel.restype = None
_lib.sz_writer_cancel.argtypes = [sz_writer_handle]


# ============================================================================
# Python异常类
# ============================================================================

class SevenZipError(Exception):
    """SevenZip错误基类"""
    def __init__(self, message: str, code: SzResult = SzResult.FAIL):
        super().__init__(message)
        self.code = code


def _check_result(result: int, operation: str = "操作"):
    """检查C API返回值，失败时抛出异常"""
    if result != SzResult.OK:
        error_msg = _lib.sz_get_last_error_message()
        if error_msg:
            error_msg = error_msg.decode('utf-8')
        else:
            error_msg = f"{operation}失败"
        raise SevenZipError(f"{error_msg} (code={result})", SzResult(result))


# ============================================================================
# Item类 - 归档项目
# ============================================================================

class Item:
    """归档中的单个项目（文件或目录）"""
    
    def __init__(self, info: SzItemInfo):
        self.index = info.index
        self.path = info.path.decode('utf-8') if info.path else ""
        self.size = info.size
        self.packed_size = info.packed_size
        self.compressed_size = info.packed_size  # 别名，保持兼容
        self.is_directory = bool(info.is_directory)
        self.is_encrypted = bool(info.is_encrypted)
        self.crc = info.crc if info.has_crc else None
        self.has_crc = bool(info.has_crc)
        self.creation_time = info.creation_time
        self.modification_time = info.modification_time
    
    def __repr__(self):
        typ = "DIR" if self.is_directory else "FILE"
        return f"<Item {typ} '{self.path}' size={self.size}>"


# ============================================================================
# Archive类 - 归档读取器
# ============================================================================

class Archive:
    """归档读取器 - 用于读取和提取归档文件"""
    
    def __init__(self):
        self._handle: Optional[sz_archive_handle] = None
    
    @classmethod
    def open(cls, path: Union[str, Path], password: Optional[str] = None) -> 'Archive':
        """
        打开归档文件
        
        Args:
            path: 归档文件路径
            password: 可选的密码
        
        Returns:
            Archive实例
        """
        archive = cls()
        path_bytes = str(path).encode('utf-8')
        handle = sz_archive_handle()
        
        result = _lib.sz_archive_open(path_bytes, ctypes.byref(handle))
        _check_result(result, "打开归档")
        
        archive._handle = handle
        return archive
    
    def close(self):
        """关闭归档"""
        if self._handle:
            _lib.sz_archive_close(self._handle)
            self._handle = None
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False
    
    @property
    def item_count(self) -> int:
        """获取归档中的项目数量"""
        if not self._handle:
            raise SevenZipError("归档未打开")
        
        count = c_size_t()
        result = _lib.sz_archive_get_item_count(self._handle, ctypes.byref(count))
        _check_result(result, "获取项目数量")
        return count.value
    
    def get_item_info(self, index: int) -> Item:
        """获取指定索引的项目信息"""
        if not self._handle:
            raise SevenZipError("归档未打开")
        
        info = SzItemInfo()
        result = _lib.sz_archive_get_item_info(self._handle, index, ctypes.byref(info))
        _check_result(result, "获取项目信息")
        return Item(info)
    
    def extract_to_memory(self, index: int) -> bytes:
        """将指定项目提取到内存"""
        if not self._handle:
            raise SevenZipError("归档未打开")
        
        data_ptr = c_void_p()
        size = c_size_t()
        
        result = _lib.sz_archive_extract_to_memory(
            self._handle, index, ctypes.byref(data_ptr), ctypes.byref(size)
        )
        _check_result(result, "提取到内存")
        
        try:
            # 复制数据到Python bytes对象
            data = ctypes.string_at(data_ptr, size.value)
            return data
        finally:
            # 释放C分配的内存
            _lib.sz_memory_free(data_ptr)
    
    def __iter__(self) -> Iterator[Item]:
        """迭代归档中的所有项目"""
        count = self.item_count
        for i in range(count):
            yield self.get_item_info(i)
    
    def __len__(self) -> int:
        return self.item_count


# ============================================================================
# Writer类 - 归档写入器
# ============================================================================

class Writer:
    """归档写入器 - 用于创建新归档"""
    
    def __init__(self):
        self._handle: Optional[sz_writer_handle] = None
        self._finalized = False
    
    @classmethod
    def create(cls, path: Union[str, Path], format: Union[str, SzFormat] = 'SevenZip') -> 'Writer':
        """
        创建新归档
        
        Args:
            path: 归档文件路径
            format: 归档格式 ('7z', 'zip', 'tar', 'gzip', 'bzip2', 'xz')
        
        Returns:
            Writer实例
        """
        writer = cls()
        path_bytes = str(path).encode('utf-8')
        
        # 转换格式字符串
        if isinstance(format, str):
            format_map = {
                '7z': SzFormat.SEVEN_Z,
                'sevenzip': SzFormat.SEVEN_Z,
                'zip': SzFormat.ZIP,
                'tar': SzFormat.TAR,
                'gzip': SzFormat.GZIP,
                'gz': SzFormat.GZIP,
                'bzip2': SzFormat.BZIP2,
                'bz2': SzFormat.BZIP2,
                'xz': SzFormat.XZ,
            }
            format = format_map.get(format.lower(), SzFormat.SEVEN_Z)
        
        handle = sz_writer_handle()
        result = _lib.sz_writer_create(path_bytes, format, ctypes.byref(handle))
        _check_result(result, "创建归档写入器")
        
        writer._handle = handle
        return writer
    
    def add_file(self, file_path: Union[str, Path], archive_path: Optional[str] = None):
        """
        添加文件到归档
        
        Args:
            file_path: 源文件路径
            archive_path: 归档中的路径（默认使用文件名）
        """
        if not self._handle:
            raise SevenZipError("写入器未初始化")
        if self._finalized:
            raise SevenZipError("归档已完成，不能添加更多文件")
        
        file_path_bytes = str(file_path).encode('utf-8')
        archive_path_bytes = (archive_path or Path(file_path).name).encode('utf-8')
        
        result = _lib.sz_writer_add_file(self._handle, file_path_bytes, archive_path_bytes)
        _check_result(result, "添加文件")
    
    def add_directory(self, dir_path: Union[str, Path], recursive: bool = True):
        """
        添加目录到归档
        
        Args:
            dir_path: 目录路径
            recursive: 是否递归添加子目录
        """
        if not self._handle:
            raise SevenZipError("写入器未初始化")
        if self._finalized:
            raise SevenZipError("归档已完成，不能添加更多目录")
        
        dir_path_bytes = str(dir_path).encode('utf-8')
        result = _lib.sz_writer_add_directory(self._handle, dir_path_bytes, int(recursive))
        _check_result(result, "添加目录")
    
    def add_memory(self, data: bytes, archive_path: str):
        """
        从内存添加数据到归档
        
        Args:
            data: 字节数据
            archive_path: 归档中的文件路径
        """
        if not self._handle:
            raise SevenZipError("写入器未初始化")
        if self._finalized:
            raise SevenZipError("归档已完成，不能添加更多数据")
        
        archive_path_bytes = archive_path.encode('utf-8')
        data_ptr = ctypes.cast(data, c_void_p)
        
        result = _lib.sz_writer_add_memory(
            self._handle, data_ptr, len(data), archive_path_bytes
        )
        _check_result(result, "添加内存数据")
    
    def set_password(self, password: str):
        """设置归档密码"""
        if not self._handle:
            raise SevenZipError("写入器未初始化")
        if self._finalized:
            raise SevenZipError("归档已完成，不能设置密码")
        
        password_bytes = password.encode('utf-8')
        result = _lib.sz_writer_set_password(self._handle, password_bytes)
        _check_result(result, "设置密码")
    
    def set_compression_level(self, level: Union[str, SzCompressionLevel]):
        """
        设置压缩级别
        
        Args:
            level: 压缩级别 ('none', 'fast', 'normal', 'maximum', 'ultra')
        """
        if not self._handle:
            raise SevenZipError("写入器未初始化")
        if self._finalized:
            raise SevenZipError("归档已完成，不能设置压缩级别")
        
        if isinstance(level, str):
            level_map = {
                'none': SzCompressionLevel.NONE,
                'fast': SzCompressionLevel.FAST,
                'normal': SzCompressionLevel.NORMAL,
                'maximum': SzCompressionLevel.MAXIMUM,
                'ultra': SzCompressionLevel.ULTRA,
            }
            level = level_map.get(level.lower(), SzCompressionLevel.NORMAL)
        
        result = _lib.sz_writer_set_compression_level(self._handle, level)
        _check_result(result, "设置压缩级别")
    
    def finalize(self):
        """完成归档写入"""
        if not self._handle:
            raise SevenZipError("写入器未初始化")
        if self._finalized:
            return
        
        result = _lib.sz_writer_finalize(self._handle)
        _check_result(result, "完成归档")
        self._finalized = True
    
    def cancel(self):
        """取消并清理写入器"""
        if self._handle:
            _lib.sz_writer_cancel(self._handle)
            self._handle = None
            self._finalized = True
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is None:
            self.finalize()
        self.cancel()
        return False


# ============================================================================
# 便捷函数
# ============================================================================

def extract_archive(archive_path: Union[str, Path], 
                   output_dir: Union[str, Path],
                   password: Optional[str] = None):
    """
    提取整个归档到目录
    
    Args:
        archive_path: 归档文件路径
        output_dir: 输出目录
        password: 可选的密码
    """
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    with Archive.open(archive_path, password) as archive:
        for item in archive:
            if item.is_directory:
                continue
            
            # 提取文件
            data = archive.extract_to_memory(item.index)
            
            # 写入文件
            output_path = output_dir / item.path
            output_path.parent.mkdir(parents=True, exist_ok=True)
            output_path.write_bytes(data)


def create_archive(output_path: Union[str, Path],
                  files: List[Union[str, Path]],
                  format: str = '7z',
                  password: Optional[str] = None,
                  level: str = 'normal'):
    """
    创建归档文件
    
    Args:
        output_path: 输出归档路径
        files: 要添加的文件列表
        format: 归档格式
        password: 可选的密码
        level: 压缩级别
    """
    with Writer.create(output_path, format) as writer:
        writer.set_compression_level(level)
        
        if password:
            writer.set_password(password)
        
        for file_path in files:
            file_path = Path(file_path)
            if file_path.is_dir():
                writer.add_directory(file_path, recursive=True)
            else:
                writer.add_file(file_path)


__all__ = [
    'Archive', 'Writer', 'Item',
    'SevenZipError', 'SzResult', 'SzFormat', 'SzCompressionLevel',
    'extract_archive', 'create_archive',
]


if __name__ == '__main__':
    print("SevenZip Python Bindings")
    print(f"可用类: {', '.join(__all__)}")
