"""
sevenzip_ffi.py - Python bindings for SevenZip C API using ctypes

This module provides a Pythonic interface to the SevenZip C library.
It demonstrates how to use the C ABI from Python.

Example usage:
    from sevenzip_ffi import SevenZip
    
    # Extract an archive
    sz = SevenZip()
    sz.extract("archive.7z", "output_dir")
    
    # Create an archive
    sz.compress("source_dir", "archive.7z", format="7z", level="normal")
    
    # Get archive info
    info = sz.get_archive_info("archive.7z")
    print(f"Items: {info['item_count']}, Size: {info['total_size']}")
"""

import ctypes
import os
import platform
from pathlib import Path
from typing import Optional, Callable, Any, Dict, List
from enum import IntEnum


# Load the shared library
def _load_library():
    """Load the sevenzip C library based on platform."""
    lib_name = {
        'Windows': 'sevenzip_c.dll',
        'Linux': 'libsevenzip_c.so',
        'Darwin': 'libsevenzip_c.dylib'
    }.get(platform.system())
    
    if not lib_name:
        raise RuntimeError(f"Unsupported platform: {platform.system()}")
    
    # Try to find the library in common locations
    search_paths = [
        Path.cwd() / 'build' / 'Release',
        Path.cwd() / 'build' / 'Debug',
        Path.cwd() / 'build',
        Path('/usr/local/lib'),
        Path('/usr/lib'),
    ]
    
    for path in search_paths:
        lib_path = path / lib_name
        if lib_path.exists():
            return ctypes.CDLL(str(lib_path))
    
    # Try loading from system path
    try:
        return ctypes.CDLL(lib_name)
    except OSError:
        raise RuntimeError(f"Could not find {lib_name}. Please ensure the library is built and in your PATH.")


# Load library
_lib = _load_library()


# Enums matching C API
class SzResult(IntEnum):
    """Error codes."""
    OK = 0
    FAIL = 1
    OUT_OF_MEMORY = 2
    INVALID_ARGUMENT = 3
    FILE_NOT_FOUND = 4
    ACCESS_DENIED = 5
    UNSUPPORTED_FORMAT = 6
    CORRUPTED_ARCHIVE = 7
    WRONG_PASSWORD = 8
    NOT_IMPLEMENTED = 9
    ABORTED = 10
    DISK_FULL = 11
    INTERNAL_ERROR = 12
    BUSY = 13
    INVALID_STATE = 14


class SzFormat(IntEnum):
    """Archive formats."""
    AUTO = 0
    SEVENZIP = 1
    ZIP = 2
    TAR = 3
    GZIP = 4
    BZIP2 = 5
    XZ = 6


class SzCompressionLevel(IntEnum):
    """Compression levels."""
    STORE = 0
    FASTEST = 1
    FAST = 2
    NORMAL = 3
    MAXIMUM = 4
    ULTRA = 5


# Opaque handle types
class _ArchiveHandle(ctypes.c_void_p):
    pass


class _WriterHandle(ctypes.c_void_p):
    pass


class _CompressorHandle(ctypes.c_void_p):
    pass


# Structures
class _SzArchiveInfo(ctypes.Structure):
    _fields_ = [
        ('format', ctypes.c_int),
        ('item_count', ctypes.c_size_t),
        ('total_size', ctypes.c_uint64),
        ('packed_size', ctypes.c_uint64),
        ('is_solid', ctypes.c_int),
        ('is_multi_volume', ctypes.c_int),
        ('has_encrypted_headers', ctypes.c_int),
    ]


class _SzItemInfo(ctypes.Structure):
    _fields_ = [
        ('index', ctypes.c_size_t),
        ('path', ctypes.c_char_p),
        ('size', ctypes.c_uint64),
        ('packed_size', ctypes.c_uint64),
        ('crc', ctypes.c_uint32),
        ('has_crc', ctypes.c_int),
        ('creation_time', ctypes.c_int64),
        ('modification_time', ctypes.c_int64),
        ('is_directory', ctypes.c_int),
        ('is_encrypted', ctypes.c_int),
    ]


# Callback types
_ProgressCallback = ctypes.CFUNCTYPE(
    ctypes.c_int,  # return
    ctypes.c_uint64,  # completed
    ctypes.c_uint64,  # total
    ctypes.c_void_p  # user_data
)


# Function declarations
def _setup_functions():
    """Declare all C API functions."""
    
    # Error handling
    _lib.sz_error_to_string.argtypes = [ctypes.c_int]
    _lib.sz_error_to_string.restype = ctypes.c_char_p
    
    _lib.sz_get_last_error_message.argtypes = []
    _lib.sz_get_last_error_message.restype = ctypes.c_char_p
    
    _lib.sz_clear_error.argtypes = []
    _lib.sz_clear_error.restype = None
    
    # Version
    _lib.sz_version_string.argtypes = []
    _lib.sz_version_string.restype = ctypes.c_char_p
    
    _lib.sz_version_number.argtypes = []
    _lib.sz_version_number.restype = ctypes.c_uint32
    
    _lib.sz_is_format_supported.argtypes = [ctypes.c_int]
    _lib.sz_is_format_supported.restype = ctypes.c_int
    
    # Archive operations
    _lib.sz_archive_open.argtypes = [ctypes.c_char_p, ctypes.POINTER(_ArchiveHandle)]
    _lib.sz_archive_open.restype = ctypes.c_int
    
    _lib.sz_archive_close.argtypes = [_ArchiveHandle]
    _lib.sz_archive_close.restype = None
    
    _lib.sz_archive_get_info.argtypes = [_ArchiveHandle, ctypes.POINTER(_SzArchiveInfo)]
    _lib.sz_archive_get_info.restype = ctypes.c_int
    
    _lib.sz_archive_get_item_count.argtypes = [_ArchiveHandle, ctypes.POINTER(ctypes.c_size_t)]
    _lib.sz_archive_get_item_count.restype = ctypes.c_int
    
    _lib.sz_archive_get_item_info.argtypes = [_ArchiveHandle, ctypes.c_size_t, ctypes.POINTER(_SzItemInfo)]
    _lib.sz_archive_get_item_info.restype = ctypes.c_int
    
    _lib.sz_item_info_free.argtypes = [ctypes.POINTER(_SzItemInfo)]
    _lib.sz_item_info_free.restype = None
    
    _lib.sz_archive_extract_all.argtypes = [_ArchiveHandle, ctypes.c_char_p, _ProgressCallback, ctypes.c_void_p]
    _lib.sz_archive_extract_all.restype = ctypes.c_int
    
    _lib.sz_archive_set_password.argtypes = [_ArchiveHandle, ctypes.c_char_p]
    _lib.sz_archive_set_password.restype = ctypes.c_int
    
    # Writer operations
    _lib.sz_writer_create.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.POINTER(_WriterHandle)]
    _lib.sz_writer_create.restype = ctypes.c_int
    
    _lib.sz_writer_destroy.argtypes = [_WriterHandle]
    _lib.sz_writer_destroy.restype = None
    
    _lib.sz_writer_add_file.argtypes = [_WriterHandle, ctypes.c_char_p, ctypes.c_char_p]
    _lib.sz_writer_add_file.restype = ctypes.c_int
    
    _lib.sz_writer_add_directory.argtypes = [_WriterHandle, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
    _lib.sz_writer_add_directory.restype = ctypes.c_int
    
    _lib.sz_writer_set_compression_level.argtypes = [_WriterHandle, ctypes.c_int]
    _lib.sz_writer_set_compression_level.restype = ctypes.c_int
    
    _lib.sz_writer_set_password.argtypes = [_WriterHandle, ctypes.c_char_p]
    _lib.sz_writer_set_password.restype = ctypes.c_int
    
    _lib.sz_writer_finalize.argtypes = [_WriterHandle]
    _lib.sz_writer_finalize.restype = ctypes.c_int
    
    # Convenience functions
    _lib.sz_extract_simple.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    _lib.sz_extract_simple.restype = ctypes.c_int
    
    _lib.sz_compress_simple.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
    _lib.sz_compress_simple.restype = ctypes.c_int


_setup_functions()


# Exception class
class SevenZipError(Exception):
    """Exception raised for SevenZip errors."""
    
    def __init__(self, result: SzResult, message: Optional[str] = None):
        self.result = result
        if message is None:
            message = _lib.sz_error_to_string(result).decode('utf-8')
            detail = _lib.sz_get_last_error_message().decode('utf-8')
            if detail:
                message = f"{message}: {detail}"
        super().__init__(message)


def _check_result(result: int):
    """Check result code and raise exception if error."""
    if result != SzResult.OK:
        raise SevenZipError(SzResult(result))


# High-level Python API
class SevenZip:
    """High-level interface to SevenZip library."""
    
    @staticmethod
    def get_version() -> str:
        """Get library version string."""
        return _lib.sz_version_string().decode('utf-8')
    
    @staticmethod
    def is_format_supported(format: str) -> bool:
        """Check if a format is supported."""
        format_enum = getattr(SzFormat, format.upper(), None)
        if format_enum is None:
            return False
        return _lib.sz_is_format_supported(format_enum) != 0
    
    def extract(self, archive_path: str, dest_dir: str, password: Optional[str] = None,
                progress_callback: Optional[Callable[[int, int], bool]] = None):
        """
        Extract an archive to a directory.
        
        Args:
            archive_path: Path to the archive file
            dest_dir: Destination directory
            password: Optional password for encrypted archives
            progress_callback: Optional callback(completed, total) -> bool (return False to cancel)
        """
        archive_path = str(Path(archive_path).absolute()).encode('utf-8')
        dest_dir = str(Path(dest_dir).absolute()).encode('utf-8')
        
        handle = _ArchiveHandle()
        result = _lib.sz_archive_open(archive_path, ctypes.byref(handle))
        _check_result(result)
        
        try:
            if password:
                result = _lib.sz_archive_set_password(handle, password.encode('utf-8'))
                _check_result(result)
            
            callback = None
            if progress_callback:
                def wrapper(completed, total, user_data):
                    return 1 if progress_callback(completed, total) else 0
                callback = _ProgressCallback(wrapper)
            
            result = _lib.sz_archive_extract_all(handle, dest_dir, callback, None)
            _check_result(result)
        finally:
            _lib.sz_archive_close(handle)
    
    def compress(self, source_path: str, archive_path: str, format: str = "7z",
                 level: str = "normal", password: Optional[str] = None):
        """
        Create an archive from a file or directory.
        
        Args:
            source_path: Source file or directory
            archive_path: Output archive path
            format: Archive format (7z, zip, tar, etc.)
            level: Compression level (store, fastest, fast, normal, maximum, ultra)
            password: Optional password for encryption
        """
        source_path = str(Path(source_path).absolute()).encode('utf-8')
        archive_path = str(Path(archive_path).absolute()).encode('utf-8')
        format_enum = getattr(SzFormat, format.upper())
        level_enum = getattr(SzCompressionLevel, level.upper())
        
        handle = _WriterHandle()
        result = _lib.sz_writer_create(archive_path, format_enum, ctypes.byref(handle))
        _check_result(result)
        
        try:
            result = _lib.sz_writer_set_compression_level(handle, level_enum)
            _check_result(result)
            
            if password:
                result = _lib.sz_writer_set_password(handle, password.encode('utf-8'))
                _check_result(result)
            
            source = Path(source_path.decode('utf-8'))
            if source.is_dir():
                result = _lib.sz_writer_add_directory(handle, source_path, None, 1)
            else:
                result = _lib.sz_writer_add_file(handle, source_path, None)
            _check_result(result)
            
            result = _lib.sz_writer_finalize(handle)
            _check_result(result)
        finally:
            _lib.sz_writer_destroy(handle)
    
    def get_archive_info(self, archive_path: str) -> Dict[str, Any]:
        """
        Get information about an archive.
        
        Returns:
            Dictionary with archive information
        """
        archive_path = str(Path(archive_path).absolute()).encode('utf-8')
        
        handle = _ArchiveHandle()
        result = _lib.sz_archive_open(archive_path, ctypes.byref(handle))
        _check_result(result)
        
        try:
            info = _SzArchiveInfo()
            result = _lib.sz_archive_get_info(handle, ctypes.byref(info))
            _check_result(result)
            
            return {
                'format': SzFormat(info.format).name.lower(),
                'item_count': info.item_count,
                'total_size': info.total_size,
                'packed_size': info.packed_size,
                'compression_ratio': info.packed_size / info.total_size if info.total_size > 0 else 0,
                'is_solid': bool(info.is_solid),
                'is_multi_volume': bool(info.is_multi_volume),
                'has_encrypted_headers': bool(info.has_encrypted_headers),
            }
        finally:
            _lib.sz_archive_close(handle)
    
    def list_items(self, archive_path: str) -> List[Dict[str, Any]]:
        """
        List all items in an archive.
        
        Returns:
            List of dictionaries with item information
        """
        archive_path = str(Path(archive_path).absolute()).encode('utf-8')
        
        handle = _ArchiveHandle()
        result = _lib.sz_archive_open(archive_path, ctypes.byref(handle))
        _check_result(result)
        
        try:
            count = ctypes.c_size_t()
            result = _lib.sz_archive_get_item_count(handle, ctypes.byref(count))
            _check_result(result)
            
            items = []
            for i in range(count.value):
                item_info = _SzItemInfo()
                result = _lib.sz_archive_get_item_info(handle, i, ctypes.byref(item_info))
                _check_result(result)
                
                try:
                    items.append({
                        'index': item_info.index,
                        'path': item_info.path.decode('utf-8'),
                        'size': item_info.size,
                        'packed_size': item_info.packed_size,
                        'crc': item_info.crc if item_info.has_crc else None,
                        'is_directory': bool(item_info.is_directory),
                        'is_encrypted': bool(item_info.is_encrypted),
                    })
                finally:
                    _lib.sz_item_info_free(ctypes.byref(item_info))
            
            return items
        finally:
            _lib.sz_archive_close(handle)


# Simple convenience functions
def extract(archive_path: str, dest_dir: str):
    """Simple extraction function."""
    result = _lib.sz_extract_simple(
        str(Path(archive_path).absolute()).encode('utf-8'),
        str(Path(dest_dir).absolute()).encode('utf-8')
    )
    _check_result(result)


def compress(source_path: str, archive_path: str, format: str = "7z"):
    """Simple compression function."""
    format_enum = getattr(SzFormat, format.upper())
    result = _lib.sz_compress_simple(
        str(Path(source_path).absolute()).encode('utf-8'),
        str(Path(archive_path).absolute()).encode('utf-8'),
        format_enum
    )
    _check_result(result)


if __name__ == '__main__':
    # Example usage
    print(f"SevenZip Library Version: {SevenZip.get_version()}")
    print(f"7z supported: {SevenZip.is_format_supported('7z')}")
    print(f"ZIP supported: {SevenZip.is_format_supported('zip')}")
