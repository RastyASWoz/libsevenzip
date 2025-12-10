"""
pytest配置文件
"""
import sys
from pathlib import Path

# 添加examples/python到Python路径
sys.path.insert(0, str(Path(__file__).parent / 'examples' / 'python'))
