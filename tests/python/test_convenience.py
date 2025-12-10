"""
pytest测试 - 便利函数
"""
import pytest
from pathlib import Path
import shutil
from sevenzip import create_archive, extract_archive, Archive


class TestConvenienceFunctions:
    """便利函数测试"""
    
    @pytest.fixture
    def temp_files(self, tmp_path):
        """创建测试文件"""
        files = []
        for i in range(3):
            f = tmp_path / f"file{i+1}.txt"
            f.write_text(f"Content {i+1}\n" * 5, encoding='utf-8')
            files.append(str(f))
        return files
    
    def test_create_archive(self, tmp_path, temp_files):
        """测试create_archive"""
        output = tmp_path / "test.7z"
        
        create_archive(
            output_path=str(output),
            files=temp_files,
            format='7z'
        )
        
        assert output.exists()
        
        # 验证
        with Archive.open(str(output)) as archive:
            assert archive.item_count == 3
    
    def test_create_archive_with_level(self, tmp_path, temp_files):
        """测试create_archive with压缩级别"""
        output = tmp_path / "test_max.7z"
        
        create_archive(
            output_path=str(output),
            files=temp_files,
            format='7z',
            level='maximum'
        )
        
        assert output.exists()
    
    def test_extract_archive(self, tmp_path):
        """测试extract_archive"""
        # 使用项目中的测试归档
        test_archive = Path(__file__).parent.parent.parent / "tests" / "data" / "archives" / "test.7z"
        output_dir = tmp_path / "extracted"
        
        extract_archive(
            archive_path=str(test_archive),
            output_dir=str(output_dir)
        )
        
        assert output_dir.exists()
        
        # 检查提取的文件
        extracted_files = list(output_dir.rglob("*.txt"))
        assert len(extracted_files) >= 3
    
    def test_round_trip(self, tmp_path):
        """测试完整循环：创建->提取->验证"""
        # 创建源文件
        source_file = tmp_path / "source.txt"
        test_content = "Round trip test\n" * 20
        source_file.write_text(test_content, encoding='utf-8')
        
        # 创建归档
        archive_path = tmp_path / "roundtrip.7z"
        create_archive(
            output_path=str(archive_path),
            files=[str(source_file)],
            format='7z'
        )
        
        # 提取归档
        extract_dir = tmp_path / "extracted"
        extract_archive(str(archive_path), str(extract_dir))
        
        # 验证内容
        extracted_files = list(extract_dir.rglob("*.txt"))
        assert len(extracted_files) >= 1
        
        # 验证文件内容
        extracted_content = extracted_files[0].read_text(encoding='utf-8')
        assert "Round trip test" in extracted_content


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
