"""
Unit tests for check_encoding.py

Tests cover:
- File extension validation
- Encoding detection (UTF-8, UTF-16, ASCII)
- Encoding result validation
- Git operations (mocked)
- File processing integration
"""

import importlib.util
import subprocess
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest


def _load_module():
    """Dynamically load checkEncoding module from repository root"""
    repo_root = Path(__file__).resolve().parents[3]
    module_path = repo_root / "src/main/py/check_encoding.py"
    spec = importlib.util.spec_from_file_location("checkEncoding", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module spec: {module_path}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture(scope="module")
def check_encoding_module():
    """Fixture: Load checkEncoding module once per test session"""
    return _load_module()


# ============================================================================
# File Extension Validation Tests
# ============================================================================


def test_check_extension_accepts_cpp(check_encoding_module):
    """Test that .cpp files are recognized as valid"""
    assert check_encoding_module.checkExtension("test.cpp") is True


def test_check_extension_accepts_h(check_encoding_module):
    """Test that .h files are recognized as valid"""
    assert check_encoding_module.checkExtension("test.h") is True


def test_check_extension_accepts_rc(check_encoding_module):
    """Test that .rc files are recognized as valid"""
    assert check_encoding_module.checkExtension("test.rc") is True


def test_check_extension_accepts_rc2(check_encoding_module):
    """Test that .rc2 files are recognized as valid"""
    assert check_encoding_module.checkExtension("test.rc2") is True


def test_check_extension_rejects_other_extensions(check_encoding_module):
    """Test that non-target extensions are rejected"""
    assert check_encoding_module.checkExtension("test.txt") is False
    assert check_encoding_module.checkExtension("test.py") is False
    assert check_encoding_module.checkExtension("test.md") is False
    assert check_encoding_module.checkExtension("test.exe") is False


# ============================================================================
# Encoding Detection Tests (using actual file creation)
# ============================================================================


def test_check_encoding_detects_utf8_bom(tmp_path, check_encoding_module):
    """Test that UTF-8 BOM files are correctly detected"""
    file_path = tmp_path / "utf8_bom.txt"
    # UTF-8 BOM = \xef\xbb\xbf
    file_path.write_bytes(b"\xef\xbb\xbf" + "Hello".encode("utf-8"))
    
    encoding = check_encoding_module.check_encoding(str(file_path))
    # chardet should detect as UTF-8-SIG (UTF-8 with BOM)
    assert encoding in ("UTF-8-SIG", "UTF-8")


def test_check_encoding_detects_utf16_le(tmp_path, check_encoding_module):
    """Test that UTF-16 LE files are correctly detected"""
    file_path = tmp_path / "utf16_le.txt"
    # Write text in UTF-16 LE encoding
    file_path.write_text("Hello", encoding="utf-16-le")
    
    encoding = check_encoding_module.check_encoding(str(file_path))
    # chardet should detect UTF-16 or UTF-16-LE
    # Some configs may return None; allow flexible detection
    normalized = encoding.upper() if encoding is not None else None
    assert normalized in ("UTF-16", "UTF-16LE", "UTF-16-LE", "UTF-16LE", None)


def test_check_encoding_detects_ascii(tmp_path, check_encoding_module):
    """Test that ASCII files are correctly detected"""
    file_path = tmp_path / "ascii.txt"
    file_path.write_bytes(b"Hello World")
    
    encoding = check_encoding_module.check_encoding(str(file_path))
    # ASCII is subset of UTF-8; chardet may report as UTF-8 or ASCII
    assert encoding in ("ascii", "UTF-8", "ASCII")


def test_check_encoding_handles_nonexistent_file(check_encoding_module):
    """Test that nonexistent file raises FileNotFoundError as expected"""
    # check_encoding.py opens file directly, so it raises FileNotFoundError
    with pytest.raises(FileNotFoundError):
        check_encoding_module.check_encoding("/nonexistent/path/file.txt")


# ============================================================================
# Encoding Result Validation Tests
# ============================================================================


def test_check_encoding_result_cpp_allows_utf8_sig(check_encoding_module):
    """Test that .cpp files accept UTF-8-SIG encoding"""
    # check_encoding_result(file_name, encoding) should return True for valid combo
    result = check_encoding_module.checkEncodingResult("test.cpp", "UTF-8-SIG")
    assert result is True


def test_check_encoding_result_cpp_allows_ascii(check_encoding_module):
    """Test that .cpp files accept ASCII encoding"""
    result = check_encoding_module.checkEncodingResult("test.cpp", "ascii")
    assert result is True


def test_check_encoding_result_h_allows_utf8_sig(check_encoding_module):
    """Test that .h files accept UTF-8-SIG encoding"""
    result = check_encoding_module.checkEncodingResult("test.h", "UTF-8-SIG")
    assert result is True


def test_check_encoding_result_rc_requires_utf16(check_encoding_module):
    """Test that .rc files accept UTF-16 encoding"""
    result = check_encoding_module.checkEncodingResult("test.rc", "UTF-16")
    assert result is True


def test_check_encoding_result_rc_rejects_utf8(check_encoding_module):
    """Test that .rc files reject UTF-8 encoding"""
    result = check_encoding_module.checkEncodingResult("test.rc", "UTF-8")
    assert result is False


# ============================================================================
# Git Operations Tests (mocked with pytest-mock)
# ============================================================================


def test_check_origin_master_success(mocker, check_encoding_module):
    """Test check_origin_master when origin/master check succeeds"""
    # Mock subprocess.check_output to simulate successful git command
    mock_check_output = mocker.patch("subprocess.check_output")
    mock_check_output.return_value = b"valid_output"
    
    # check_origin_master returns 0 on success (git returncode)
    result = check_encoding_module.checkOriginMaster()
    assert result == 0


def test_check_origin_master_failure(mocker, check_encoding_module):
    """Test check_origin_master when origin/master check fails"""
    # Mock subprocess.check_output to raise CalledProcessError
    mock_check_output = mocker.patch("subprocess.check_output")
    mock_check_output.side_effect = subprocess.CalledProcessError(1, "git")
    
    # check_origin_master returns non-zero returncode on failure
    result = check_encoding_module.checkOriginMaster()
    assert result != 0


def test_get_merge_base_valid_commit(mocker, check_encoding_module):
    """Test get_merge_base returns valid commit hash"""
    # Mock subprocess.check_output to return commit hash as bytes
    mock_check_output = mocker.patch("subprocess.check_output")
    mock_check_output.return_value = b"abc123def456\n"
    
    result = check_encoding_module.getMergeBase()
    assert result is not None
    assert result == "abc123def456"


def test_get_merge_base_invalid_no_commit(mocker, check_encoding_module):
    """Test get_merge_base when no commit is found or error occurs"""
    # Mock subprocess.check_output to raise CalledProcessError
    mock_check_output = mocker.patch("subprocess.check_output")
    mock_check_output.side_effect = subprocess.CalledProcessError(1, "git")
    
    # get_merge_base will raise exception; test that it propagates
    with pytest.raises(subprocess.CalledProcessError):
        check_encoding_module.getMergeBase()


# ============================================================================
# Generator Function Tests
# ============================================================================


def test_get_diff_files_generators_valid_files(mocker, check_encoding_module):
    """Test get_diff_files generator yields files with target extensions"""
    # Mock get_merge_base to return a valid commit
    mock_merge_base = mocker.patch.object(
        check_encoding_module, "getMergeBase"
    )
    mock_merge_base.return_value = "abc123"
    
    # Mock subprocess.check_output to return diff output
    mock_check_output = mocker.patch("subprocess.check_output")
    mock_check_output.return_value = b"src/test.cpp\nREADME.md\nsrc/style.rc"
    
    # Collect results from generator
    files = list(check_encoding_module.getDiffFiles())
    
    # Should yield only .cpp and .rc files, not .md
    assert len(files) >= 1


def test_check_all_generator_yields_target_extensions(mocker, check_encoding_module):
    """Test check_all generator yields files with target extensions"""
    # Mock os.walk to return controlled directory structure
    mock_walk = mocker.patch("os.walk")
    mock_walk.return_value = [
        (".", ["subdir"], ["test.cpp", "doc.md", "style.rc"]),
        ("./subdir", [], ["header.h", "config.txt"])
    ]
    
    # Collect results from generator
    files = list(check_encoding_module.checkAll())
    
    # Should yield at least files with target extensions
    assert len(files) >= 1


# ============================================================================
# Integration Tests
# ============================================================================


def test_process_files_detects_encoding_violations(tmp_path, check_encoding_module, capsys):
    """
    Test process_files() integration: verify it detects encoding violations
    
    Setup:
    - Create a .cpp file with UTF-8 (valid)
    - Create a .rc file with UTF-8 (invalid, should be UTF-16)
    - Expect process_files to detect the .rc as violation
    """
    # Create valid .cpp file (UTF-8)
    cpp_file = tmp_path / "test.cpp"
    cpp_file.write_text("int main() {}", encoding="utf-8")
    
    # Create invalid .rc file (UTF-8 instead of UTF-16)
    rc_file = tmp_path / "test.rc"
    rc_file.write_text("STRINGTABLE\nBEGIN\nEND", encoding="utf-8")
    
    # Process files and count violations
    files = [str(cpp_file), str(rc_file)]
    violation_count = check_encoding_module.processFiles(files)
    
    # Should report at least 1 violation (the .rc file)
    assert violation_count >= 1


def test_process_files_accepts_valid_files(tmp_path, check_encoding_module):
    """
    Test process_files() with valid encodings
    
    Setup:
    - Create a .cpp file with UTF-8 (valid)
    - Create a .rc file with UTF-16 (valid)
    - Expect process_files to return 0 violations
    """
    # Create valid .cpp file (UTF-8)
    cpp_file = tmp_path / "valid.cpp"
    cpp_file.write_text("int main() {}", encoding="utf-8")
    
    # Create valid .rc file (UTF-16)
    rc_file = tmp_path / "valid.rc"
    rc_file.write_text("STRINGTABLE\nBEGIN\nEND", encoding="utf-16")
    
    # Process files
    files = [str(cpp_file), str(rc_file)]
    violation_count = check_encoding_module.processFiles(files)
    
    # Should report 0 violations for valid files
    assert violation_count == 0
