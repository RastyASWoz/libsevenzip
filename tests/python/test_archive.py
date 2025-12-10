"""
pytest测试 - Archive读取功能
"""
import pytest
from pathlib import Path
from sevenzip import Archive, SevenZipError


class TestArchive:
    """Archive类测试"""
    
    @pytest.fixture
    def test_archive_path(self):
        """测试归档路径"""
        return Path(__file__).parent.parent.parent / "tests" / "data" / "archives" / "test.7z"
    
    def test_open_archive(self, test_archive_path):
        """测试打开归档"""
        archive = Archive.open(str(test_archive_path))
        assert archive is not None
        archive.close()
    
    def test_open_with_context_manager(self, test_archive_path):
        """测试with语句"""
        with Archive.open(str(test_archive_path)) as archive:
            assert archive is not None
    
    def test_item_count(self, test_archive_path):
        """测试获取项目数量"""
        with Archive.open(str(test_archive_path)) as archive:
            count = archive.item_count
            assert count == 3
    
    def test_iterate_items(self, test_archive_path):
        """测试遍历项目"""
        with Archive.open(str(test_archive_path)) as archive:
            items = list(archive)
            assert len(items) == 3
            
            # 验证第一个项目
            item = items[0]
            assert item.path == "test1.txt"
            assert item.size == 64
            assert not item.is_directory
    
    def test_get_item_info(self, test_archive_path):
        """测试获取项目信息"""
        with Archive.open(str(test_archive_path)) as archive:
            item = archive.get_item_info(0)
            assert item is not None
            assert item.path == "test1.txt"
            assert item.size > 0
    
    def test_extract_to_memory(self, test_archive_path):
        """测试提取到内存"""
        with Archive.open(str(test_archive_path)) as archive:
            data = archive.extract_to_memory(0)
            assert data is not None
            assert len(data) > 0
            assert isinstance(data, bytes)
    
    def test_open_nonexistent_file(self):
        """测试打开不存在的文件"""
        with pytest.raises(SevenZipError):
            Archive.open("nonexistent.7z")
    
    def test_invalid_index(self, test_archive_path):
        """测试无效索引"""
        with Archive.open(str(test_archive_path)) as archive:
            with pytest.raises(SevenZipError):
                archive.get_item_info(999)
