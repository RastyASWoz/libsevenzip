"""
example_usage.py - Demonstrates using the SevenZip Python bindings

This example shows common usage patterns for the SevenZip library.
"""

import sys
from pathlib import Path

# Add parent directory to path to import sevenzip_ffi
sys.path.insert(0, str(Path(__file__).parent))

from sevenzip_ffi import SevenZip, extract, compress, SevenZipError


def print_section(title: str):
    """Print a formatted section header."""
    print(f"\n{'=' * 60}")
    print(f" {title}")
    print('=' * 60)


def example_version_info():
    """Show version information."""
    print_section("Version Information")
    
    sz = SevenZip()
    print(f"Library Version: {sz.get_version()}")
    
    formats = ['7z', 'zip', 'tar', 'gzip', 'bzip2', 'xz']
    print("\nSupported Formats:")
    for fmt in formats:
        supported = "✓" if sz.is_format_supported(fmt) else "✗"
        print(f"  {supported} {fmt.upper()}")


def example_create_archive():
    """Create a test archive."""
    print_section("Creating Test Archive")
    
    # Create test data
    test_dir = Path("test_data")
    test_dir.mkdir(exist_ok=True)
    
    (test_dir / "file1.txt").write_text("Hello from file 1!")
    (test_dir / "file2.txt").write_text("Hello from file 2!")
    
    subdir = test_dir / "subdir"
    subdir.mkdir(exist_ok=True)
    (subdir / "file3.txt").write_text("Hello from subdirectory!")
    
    print(f"Created test data in: {test_dir}")
    
    # Create archive using high-level API
    sz = SevenZip()
    archive_path = "test_archive.7z"
    
    print(f"\nCompressing to: {archive_path}")
    try:
        sz.compress(
            source_path=str(test_dir),
            archive_path=archive_path,
            format="7z",
            level="normal"
        )
        print("✓ Archive created successfully!")
        return archive_path
    except SevenZipError as e:
        print(f"✗ Error: {e}")
        return None


def example_archive_info(archive_path: str):
    """Display archive information."""
    print_section("Archive Information")
    
    sz = SevenZip()
    
    try:
        info = sz.get_archive_info(archive_path)
        
        print(f"Archive: {archive_path}")
        print(f"Format: {info['format'].upper()}")
        print(f"Items: {info['item_count']}")
        print(f"Total Size: {info['total_size']:,} bytes")
        print(f"Packed Size: {info['packed_size']:,} bytes")
        print(f"Compression Ratio: {info['compression_ratio']:.1%}")
        print(f"Solid: {'Yes' if info['is_solid'] else 'No'}")
        print(f"Encrypted Headers: {'Yes' if info['has_encrypted_headers'] else 'No'}")
        
    except SevenZipError as e:
        print(f"✗ Error: {e}")


def example_list_items(archive_path: str):
    """List all items in archive."""
    print_section("Archive Contents")
    
    sz = SevenZip()
    
    try:
        items = sz.list_items(archive_path)
        
        print(f"Found {len(items)} items:\n")
        print(f"{'Index':<6} {'Type':<5} {'Size':>12} {'Packed':>12} {'Path'}")
        print("-" * 60)
        
        for item in items:
            item_type = "DIR" if item['is_directory'] else "FILE"
            size_str = "-" if item['is_directory'] else f"{item['size']:,}"
            packed_str = "-" if item['is_directory'] else f"{item['packed_size']:,}"
            encrypted = "🔒" if item['is_encrypted'] else ""
            
            print(f"{item['index']:<6} {item_type:<5} {size_str:>12} {packed_str:>12} {encrypted} {item['path']}")
        
    except SevenZipError as e:
        print(f"✗ Error: {e}")


def example_extract_archive(archive_path: str):
    """Extract archive to directory."""
    print_section("Extracting Archive")
    
    output_dir = Path("extracted_output")
    output_dir.mkdir(exist_ok=True)
    
    sz = SevenZip()
    
    print(f"Extracting to: {output_dir}")
    
    # Progress callback
    def progress(completed: int, total: int) -> bool:
        if total > 0:
            percent = (completed / total) * 100
            print(f"\rProgress: {percent:.1f}% ({completed:,} / {total:,} bytes)", end='')
        return True  # Continue extraction
    
    try:
        sz.extract(
            archive_path=archive_path,
            dest_dir=str(output_dir),
            progress_callback=progress
        )
        print("\n✓ Extraction complete!")
        
        # List extracted files
        print("\nExtracted files:")
        for path in sorted(output_dir.rglob("*")):
            if path.is_file():
                rel_path = path.relative_to(output_dir)
                print(f"  {rel_path}")
        
    except SevenZipError as e:
        print(f"\n✗ Error: {e}")


def example_simple_functions():
    """Demonstrate simple convenience functions."""
    print_section("Simple Convenience Functions")
    
    # Create simple test file
    test_file = Path("simple_test.txt")
    test_file.write_text("This is a simple test file.")
    
    print("Using simple compress() function:")
    try:
        compress("simple_test.txt", "simple.zip", format="zip")
        print("✓ Created simple.zip")
    except SevenZipError as e:
        print(f"✗ Error: {e}")
    
    print("\nUsing simple extract() function:")
    simple_output = Path("simple_output")
    simple_output.mkdir(exist_ok=True)
    
    try:
        extract("simple.zip", str(simple_output))
        print(f"✓ Extracted to {simple_output}")
    except SevenZipError as e:
        print(f"✗ Error: {e}")


def example_encrypted_archive():
    """Demonstrate password-protected archives."""
    print_section("Encrypted Archives")
    
    # Create test data
    secret_dir = Path("secret_data")
    secret_dir.mkdir(exist_ok=True)
    (secret_dir / "secret.txt").write_text("This is confidential information!")
    
    sz = SevenZip()
    encrypted_archive = "encrypted.7z"
    password = "MySecretPassword123"
    
    print("Creating encrypted archive...")
    try:
        sz.compress(
            source_path=str(secret_dir),
            archive_path=encrypted_archive,
            format="7z",
            level="normal",
            password=password
        )
        print(f"✓ Created encrypted archive: {encrypted_archive}")
    except SevenZipError as e:
        print(f"✗ Error: {e}")
        return
    
    # Try extracting with wrong password
    print("\nTrying to extract with wrong password...")
    try:
        sz.extract(encrypted_archive, "wrong_password_output", password="WrongPassword")
        print("✗ Should have failed!")
    except SevenZipError as e:
        print(f"✓ Correctly rejected wrong password: {e.result.name}")
    
    # Extract with correct password
    print("\nExtracting with correct password...")
    try:
        sz.extract(encrypted_archive, "correct_password_output", password=password)
        print("✓ Successfully extracted with correct password!")
    except SevenZipError as e:
        print(f"✗ Error: {e}")


def example_error_handling():
    """Demonstrate error handling."""
    print_section("Error Handling")
    
    sz = SevenZip()
    
    # Try to open non-existent file
    print("Attempting to open non-existent file...")
    try:
        sz.get_archive_info("nonexistent_file.7z")
        print("✗ Should have raised an error!")
    except SevenZipError as e:
        print(f"✓ Caught error: {e.result.name}")
        print(f"  Message: {e}")
    
    # Try invalid format
    print("\nAttempting to use invalid format...")
    try:
        sz.compress("test.txt", "output.xxx", format="invalid_format")
        print("✗ Should have raised an error!")
    except (SevenZipError, AttributeError) as e:
        print(f"✓ Caught error: {type(e).__name__}")


def cleanup():
    """Clean up test files."""
    import shutil
    
    print_section("Cleanup")
    
    paths_to_remove = [
        "test_data",
        "test_archive.7z",
        "extracted_output",
        "simple_test.txt",
        "simple.zip",
        "simple_output",
        "secret_data",
        "encrypted.7z",
        "wrong_password_output",
        "correct_password_output"
    ]
    
    for path_str in paths_to_remove:
        path = Path(path_str)
        if path.exists():
            if path.is_dir():
                shutil.rmtree(path)
            else:
                path.unlink()
            print(f"Removed: {path}")
    
    print("\n✓ Cleanup complete!")


def main():
    """Run all examples."""
    print("=" * 60)
    print(" SevenZip Python Bindings - Example Usage")
    print("=" * 60)
    
    try:
        # Show version info
        example_version_info()
        
        # Create and work with archives
        archive_path = example_create_archive()
        if archive_path:
            example_archive_info(archive_path)
            example_list_items(archive_path)
            example_extract_archive(archive_path)
        
        # Simple functions
        example_simple_functions()
        
        # Encrypted archives
        example_encrypted_archive()
        
        # Error handling
        example_error_handling()
        
    except Exception as e:
        print(f"\n✗ Unexpected error: {e}")
        import traceback
        traceback.print_exc()
    
    finally:
        # Clean up
        cleanup()


if __name__ == '__main__':
    main()
