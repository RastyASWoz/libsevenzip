"""
pytest测试 - Writer创建归档功能
"""
import pytest
from pathlib import Path
import shutil
from sevenzip import Writer, Archive, SevenZipError


class TestWriter:
    """Writer类测试"""
    
    @pytest.fixture
    def temp_dir(self, tmp_path):
        """临时目录"""
        test_dir = tmp_path / "test_data"
        test_dir.mkdir()
        return test_dir
    
    @pytest.fixture
    def test_file(self, temp_dir):
        """创建测试文件"""
        f = temp_dir / "test.txt"
        f.write_text("Test content\n" * 10, encoding='utf-8')
        return f
    
    def test_create_writer(self, tmp_path):
        """测试创建Writer"""
        output = tmp_path / "test.7z"
        writer = Writer.create(str(output), format='7z')
        assert writer is not None
        writer.finalize()
    
    def test_create_with_context_manager(self, tmp_path, test_file):
        """测试with语句"""
        output = tmp_path / "test.7z"
        
        with Writer.create(str(output), format='7z') as writer:
            writer.add_file(str(test_file), "test.txt")
        
        # 验证归档
        assert output.exists()
        with Archive.open(str(output)) as archive:
            assert archive.item_count == 1
    
    def test_add_file(self, tmp_path, test_file):
        """测试添加文件"""
        output = tmp_path / "test.7z"
        
        with Writer.create(str(output), format='7z') as writer:
            writer.add_file(str(test_file), "data/test.txt")
        
        with Archive.open(str(output)) as archive:
            assert archive.item_count == 1
            item = archive.get_item_info(0)
            assert "test.txt" in item.path
    
    def test_add_directory(self, tmp_path, temp_dir):
        """测试添加目录"""
        # 创建多个文件
        (temp_dir / "file1.txt").write_text("File 1")
        (temp_dir / "file2.txt").write_text("File 2")
        
        output = tmp_path / "test.7z"
        
        with Writer.create(str(output), format='7z') as writer:
            writer.add_directory(str(temp_dir), recursive=True)
        
        with Archive.open(str(output)) as archive:
            # 应该有目录和文件
            assert archive.item_count >= 2
    
    def test_compression_levels(self, tmp_path, test_file):
        """测试压缩级别"""
        levels = ['fast', 'normal', 'maximum']
        
        for level in levels:
            output = tmp_path / f"test_{level}.7z"
            
            with Writer.create(str(output), format='7z') as writer:
                writer.set_compression_level(level)
                writer.add_file(str(test_file), "test.txt")
            
            assert output.exists()
            assert output.stat().st_size > 0
    
    def test_password_protection(self, tmp_path, test_file):
        """测试密码保护"""
        output = tmp_path / "protected.7z"
        password = "secret123"
        
        with Writer.create(str(output), format='7z') as writer:
            writer.set_password(password)
            writer.add_file(str(test_file), "test.txt")
        
        assert output.exists()
    
    def test_multiple_formats(self, tmp_path, test_file):
        """测试多种格式"""
        formats = ['7z', 'zip', 'tar']
        
        for fmt in formats:
            output = tmp_path / f"test.{fmt}"
            
            with Writer.create(str(output), format=fmt) as writer:
                writer.add_file(str(test_file), "test.txt")
            
            assert output.exists()
            
            # 验证可以读取
            with Archive.open(str(output)) as archive:
                assert archive.item_count >= 1
