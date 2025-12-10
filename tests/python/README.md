# Python绑定 pytest测试套件

这个目录包含使用pytest框架的Python绑定单元测试。

## 安装pytest

```bash
pip install pytest
```

## 运行测试

### 运行所有测试
```bash
cd d:\code\7z2501-src\libsevenzip
pytest tests/python/ -v
```

### 运行特定测试文件
```bash
pytest tests/python/test_archive.py -v
pytest tests/python/test_writer.py -v
pytest tests/python/test_convenience.py -v
```

### 运行特定测试
```bash
pytest tests/python/test_archive.py::TestArchive::test_open_archive -v
```

### 显示详细输出
```bash
pytest tests/python/ -v -s
```

## 测试文件说明

### test_archive.py
测试Archive类的读取功能：
- 打开归档
- 获取项目数量
- 遍历项目
- 提取到内存
- 错误处理

### test_writer.py
测试Writer类的创建功能：
- 创建归档
- 添加文件
- 添加目录
- 压缩级别
- 密码保护
- 多种格式

### test_convenience.py
测试便利函数：
- create_archive()
- extract_archive()
- 完整循环测试

## 测试覆盖率

如需测试覆盖率报告，安装并运行：

```bash
pip install pytest-cov
pytest tests/python/ --cov=sevenzip --cov-report=html
```

## CI/CD集成

在CI/CD流程中运行：

```bash
pytest tests/python/ -v --junitxml=test-results.xml
```

## 注意事项

1. 测试需要sevenzip_ffi.dll在正确路径
2. 需要tests/data/archives/test.7z测试文件
3. 测试使用临时目录，自动清理
4. 所有测试相互独立，可并行运行
