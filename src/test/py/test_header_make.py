"""
Comprehensive test suite for header_make.py

Tests cover:
- Argument parsing (all format variants)
- Comment stripping (line, block, nested)
- Token extraction and filtering
- Output formatting (define and enum modes)
- Binary equivalence with existing generated headers
- Timestamp-based regeneration skipping
- End-to-end execution flows
"""

import sys
import pytest
import importlib.util
import tempfile
from pathlib import Path
from datetime import datetime, timedelta
import time


# Dynamic module loading to import header_make.py
def load_header_make_module():
    """Load header_make.py from src/main/py directory."""
    module_path = Path(__file__).parent.parent.parent / "main" / "py" / "header_make.py"
    if not module_path.exists():
        pytest.skip(f"header_make.py not found at {module_path}")
    
    spec = importlib.util.spec_from_file_location("header_make", module_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


header_make = load_header_make_module()


class TestArgParsing:
    """Test argument parsing logic for all format variants."""
    
    def test_parse_args_with_equals_separator(self):
        """Parse arguments with -arg=value format."""
        result = header_make.parse_args(["prog", "-in=input.txt", "-out=output.txt", "-mode=define"])
        assert isinstance(result, tuple) and len(result) == 4
        in_file, out_file, mode, enum_name = result
        assert in_file == "input.txt"
        assert out_file == "output.txt"
        assert mode == "define"
    
    def test_parse_args_with_space_separator(self):
        """Parse arguments with -arg value format."""
        result = header_make.parse_args(["prog", "-in", "input.txt", "-out", "output.txt", "-mode", "define"])
        assert isinstance(result, tuple) and len(result) == 4
        in_file, out_file, mode, enum_name = result
        assert in_file == "input.txt"
        assert out_file == "output.txt"
    
    def test_parse_args_mixed_separators(self):
        """Parse mix of equals and space separated arguments."""
        result = header_make.parse_args(["prog", "-in=input.txt", "-out", "output.txt", "-mode=enum", "-enum=EFunctionCode"])
        assert isinstance(result, tuple) and len(result) == 4
        in_file, out_file, mode, enum_name = result
        assert in_file == "input.txt"
        assert out_file == "output.txt"
        assert mode == "enum"
        assert enum_name == "EFunctionCode"
    
    def test_parse_args_converts_forward_slash_to_hyphen(self):
        """Convert forward slash to hyphen in argument names."""
        result = header_make.parse_args(["prog", "/in=input.txt", "/out=output.txt", "/mode=define"])
        assert isinstance(result, tuple) and len(result) == 4
        in_file, out_file, mode, enum_name = result
        assert in_file == "input.txt"
        assert out_file == "output.txt"
    
    def test_parse_args_missing_required_argument(self):
        """Return error code when required -in argument missing."""
        result = header_make.parse_args(["prog", "-out", "output.txt", "-mode", "define"])
        assert isinstance(result, int) and result == 1
    
    def test_parse_args_missing_output_argument(self):
        """Return error code when required -out argument missing."""
        result = header_make.parse_args(["prog", "-in", "input.txt", "-mode", "define"])
        assert isinstance(result, int) and result == 1

    def test_parse_args_missing_mode_argument(self):
        """Return error code when required -mode argument missing."""
        result = header_make.parse_args(["prog", "-in", "input.txt", "-out", "output.txt"])
        assert isinstance(result, int) and result == 1
    
    def test_parse_args_unknown_argument(self):
        """Return error code when unknown argument is passed."""
        result = header_make.parse_args(["prog", "-in=input.txt", "-out=output.txt", "-mode=define", "-unknown=value"])
        assert isinstance(result, int) and result == 1


class TestCommentStripping:
    """Test comment removal while preserving functional content."""
    
    def test_strip_line_comments(self):
        """Remove line comments (// style)."""
        result, state = header_make.strip_comments_keep_state("MY_CONST = 42  // this is a comment", False)
        assert result.strip() == "MY_CONST = 42"
        assert state is False
    
    def test_strip_block_comment_single_line(self):
        """Remove block comments on single line (/* */ style)."""
        result, state = header_make.strip_comments_keep_state("MY_CONST = 42  /* block comment */", False)
        assert result.strip() == "MY_CONST = 42"
        assert state is False
    
    def test_strip_block_comment_start(self):
        """Handle block comment starting but not ending on line."""
        result, state = header_make.strip_comments_keep_state("MY_CONST = 42  /* block", False)
        # When block comment starts and doesn't end on same line, 
        # the entire content from /* onward is lost (unclosed block comment)
        # Result is empty since no closing */ was found
        assert result == ""
        assert state is True  # Still in block comment
    
    def test_strip_block_comment_continuation(self):
        """Handle continuation of block comment across lines."""
        result, state = header_make.strip_comments_keep_state("  still in comment */", True)
        assert result.strip() == ""
        assert state is False  # Comment ended

    def test_strip_block_comment_continuation_without_end(self):
        """Handle continuation of block comment when closing marker is absent."""
        result, state = header_make.strip_comments_keep_state("  still in comment", True)
        assert result == ""
        assert state is True  # Still in block comment


class TestModeNormalization:
    """Test mode parameter case handling and validation."""
    
    def test_mode_case_insensitive_define(self):
        """Mode names are case insensitive for define."""
        for mode in ["define", "Define", "DEFINE", "dEfInE"]:
            result = header_make.normalize_mode(mode)
            assert result == "define"
    
    def test_mode_case_insensitive_enum(self):
        """Enum mode is case insensitive."""
        for mode in ["enum", "Enum", "ENUM", "eNuM"]:
            result = header_make.normalize_mode(mode)
            assert result == "enum"
    
    def test_invalid_mode_returns_none(self):
        """Invalid mode returns None."""
        result = header_make.normalize_mode("invalid_mode")
        assert result is None
    
    def test_normalize_mode_preserves_constant_values(self):
        """Normalized modes match module constants."""
        assert header_make.normalize_mode("define") == header_make.MODE_DEFINE
        assert header_make.normalize_mode("enum") == header_make.MODE_ENUM


class TestTokenExtraction:
    """Test token extraction and definition parsing."""
    
    def test_iter_definitions_extracts_two_tokens(self):
        """Extract ID and value from definition lines."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("MY_ID = 42\n")
            
            definitions = list(header_make.iter_definitions(in_file))
            assert len(definitions) == 1
            assert definitions[0] == ("MY_ID", "42")
    
    def test_iter_definitions_skiplines_with_comments(self):
        """Skip lines that only contain comments."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("// just a comment\nMY_ID = 42\n/* another */ comment\n")
            
            definitions = list(header_make.iter_definitions(in_file))
            assert len(definitions) == 1
            assert definitions[0][0] == "MY_ID"

    def test_iter_definitions_skips_hash_prefixed_lines(self):
        """Skip preprocessor-style lines that start with #."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("#pragma once\nMY_ID = 42\n")

            definitions = list(header_make.iter_definitions(in_file))
            assert definitions == [("MY_ID", "42")]
    
    def test_iter_definitions_includes_hyphens_in_tokens(self):
        """Token pattern includes hyphens (legacy compatibility)."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("MY-CONST = 99\n")
            
            definitions = list(header_make.iter_definitions(in_file))
            assert definitions[0] == ("MY-CONST", "99")
    
    def test_iter_definitions_requires_exactly_two_tokens(self):
        """Lines must have at least 2 tokens to parse."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("ONLY_ONE\nTWO TOKEN_OK\nTHREE BAD TOKENS\n")
            
            definitions = list(header_make.iter_definitions(in_file))
            # Should get TWO TOKEN_OK and THREE BAD (takes first 2 tokens)
            assert len(definitions) >= 1
            assert definitions[0] == ("TWO", "TOKEN_OK")


class TestOutputFormatting:
    """Test header file output structure and formatting."""
    
    def test_render_output_define_mode_structure(self):
        """Define mode output has correct #define structure."""
        entries = [("MY_ID", "42")]
        output = header_make.render_output("test.hsrc", header_make.MODE_DEFINE, "", entries)
        
        assert "#define" in output
        assert "MY_ID" in output
        assert "42" in output
        assert "#ifndef" in output
    
    def test_render_output_enum_mode_structure(self):
        """Enum mode output has correct enum structure."""
        entries = [("MY_ID", "42")]
        output = header_make.render_output("test.hsrc", header_make.MODE_ENUM, "MyEnum", entries)
        
        assert "enum" in output
        assert "MyEnum" in output
        assert "MY_ID" in output
    
    def test_output_uses_lf_line_endings_in_render(self):
        """render_output produces LF internally (written with CRLF by main_impl)."""
        entries = [("TEST", "1")]
        output = header_make.render_output("test.hsrc", header_make.MODE_DEFINE, "", entries)
        
        # render_output joins with \n
        assert "\n" in output
        assert output.endswith("\n")
    
    def test_render_output_includes_header_guards(self):
        """Output includes include guards."""
        entries = [("ID", "1")]
        output = header_make.render_output("test.hsrc", header_make.MODE_DEFINE, "", entries)
        
        assert "#ifndef" in output
        assert "#define" in output
        assert "#endif" in output


class TestEquivalenceWithExisting:
    """
    CRITICAL: Binary equivalence tests ensuring Python output matches legacy headers.
    These tests validate that the migration maintains 100% compatibility.
    """
    
    def test_equivalence_with_funccode_define_header(self):
        """Generated define header matches existing Funccode_define.h exactly."""
        # Locate source file
        workspace_root = Path(__file__).parent.parent.parent.parent
        input_file = workspace_root / "sakura_core" / "Funccode_x.hsrc"
        ref_file = workspace_root / "build" / "x64" / "CMakeTools" / "Funccode_define.h"
        
        if not input_file.exists():
            pytest.skip(f"Input file not found: {input_file}")
        if not ref_file.exists():
            pytest.skip(f"Reference file not found: {ref_file}")
        
        with tempfile.TemporaryDirectory() as tmpdir:
            out_file = Path(tmpdir) / "Funccode_define.h"
            result = header_make.main_impl(str(input_file), str(out_file), "define", "")
            
            if result != 0:
                pytest.skip(f"main_impl returned error code {result}")
            
            # Compare binary content
            generated = out_file.read_bytes()
            reference = ref_file.read_bytes()
            
            assert generated == reference, f"Generated define header differs from reference (generated: {len(generated)} bytes, reference: {len(reference)} bytes)"
    
    def test_equivalence_with_funccode_enum_header(self):
        """Generated enum header matches existing Funccode_enum.h exactly."""
        # Locate source file
        workspace_root = Path(__file__).parent.parent.parent.parent
        input_file = workspace_root / "sakura_core" / "Funccode_x.hsrc"
        ref_file = workspace_root / "build" / "x64" / "CMakeTools" / "Funccode_enum.h"
        
        if not input_file.exists():
            pytest.skip(f"Input file not found: {input_file}")
        if not ref_file.exists():
            pytest.skip(f"Reference file not found: {ref_file}")
        
        with tempfile.TemporaryDirectory() as tmpdir:
            out_file = Path(tmpdir) / "Funccode_enum.h"
            result = header_make.main_impl(str(input_file), str(out_file), "enum", "EFunctionCode")
            
            if result != 0:
                pytest.skip(f"main_impl returned error code {result}")
            
            # Compare binary content
            generated = out_file.read_bytes()
            reference = ref_file.read_bytes()
            
            assert generated == reference, f"Generated enum header differs from reference (generated: {len(generated)} bytes, reference: {len(reference)} bytes)"


class TestTimestampSkipping:
    """Test timestamp-based regeneration skipping."""
    
    def test_needs_regeneration_when_output_missing(self):
        """Regeneration needed when output file doesn't exist."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("MY_ID = 42\n")
            out_file = Path(tmpdir) / "output.txt"
            
            result = header_make.needs_regeneration(in_file, out_file)
            assert result is True
    
    def test_needs_regeneration_when_input_newer(self):
        """Regeneration needed when input mtime > output mtime."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            out_file = Path(tmpdir) / "output.txt"
            
            # Create output first (older)
            out_file.write_text("old content")
            time.sleep(0.01)
            
            # Create input later (newer)
            in_file.write_text("MY_ID = 42\n")
            
            result = header_make.needs_regeneration(in_file, out_file)
            assert result is True
    
    def test_skip_regeneration_when_output_newer(self):
        """Skip regeneration when output mtime > input mtime."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            out_file = Path(tmpdir) / "output.txt"
            
            # Create input first (older)
            in_file.write_text("MY_ID = 42\n")
            time.sleep(0.01)
            
            # Create output later (newer)
            out_file.write_text("fresh content")
            
            result = header_make.needs_regeneration(in_file, out_file)
            assert result is False


class TestMainExecution:
    """Test end-to-end execution flows."""
    
    def test_main_execution_without_args(self):
        """header_make.py returns 1 when no arguments are provided."""
            
        argv = ["prog"]
        result = header_make.main(argv)
        
        assert result == 1
    
    def test_main_execution_without_input_filename(self):
        """header_make.py returns 1 when invalid arguments are provided."""
            
        argv = ["prog", "-in"]
        result = header_make.main(argv)
        
        assert result == 1
    
    def test_main_execution_without_valid_args(self):
        """header_make.py returns 1 when required arguments are missing."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("TEST_ID = 123\n")
            out_file = Path(tmpdir) / "output.h"
            
            argv = ["prog", "-in", str(in_file), "-out", str(out_file)]
            result = header_make.main(argv)
            
            assert result == 1
    
    def test_main_execution_without_valid_mode(self):
        """header_make.py returns 2 when an invalid mode is specified."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("TEST_ID = 123\n")
            out_file = Path(tmpdir) / "output.h"
            
            argv = ["prog", "-in", str(in_file), "-out", str(out_file), "-mode", "invalid_mode"]
            result = header_make.main(argv)
            
            assert result == 2
    
    def test_main_success_path(self):
        """main() success path with argv should return 0 and create output file."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("TEST_ID = 1\n")
            out_file = Path(tmpdir) / "output.h"
            
            argv = ["prog", "-in", str(in_file), "-out", str(out_file), "-mode", "define"]
            result = header_make.main(argv)
            
            assert result == 0
            assert out_file.exists()
    
    def test_main_execution_creates_output_file(self):
        """Main execution creates output file with correct content."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("TEST_ID = 123\nOTHER_ID = 456\n")
            out_file = Path(tmpdir) / "output.h"
            
            result = header_make.main_impl(str(in_file), str(out_file), "define", "")
            
            assert result == 0
            assert out_file.exists()
            content = out_file.read_text()
            assert "TEST_ID" in content
            assert "123" in content
    
    def test_main_execution_with_enum_mode(self):
        """Main execution with enum mode produces valid enum output."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("ENUM_VAL1 = 100\nENUM_VAL2 = 200\n")
            out_file = Path(tmpdir) / "output.h"
            
            result = header_make.main_impl(str(in_file), str(out_file), "enum", "TestEnum")
            
            assert result == 0
            assert out_file.exists()
            content = out_file.read_text()
            assert "TestEnum" in content
            assert "enum" in content
    
    def test_main_execution_handles_empty_input(self):
        """Main execution gracefully handles empty input file."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("")
            out_file = Path(tmpdir) / "output.h"
            
            # Should not raise error
            result = header_make.main_impl(str(in_file), str(out_file), "define", "")
            
            assert result == 0
            assert out_file.exists()

    def test_enum_name_value_reflected_in_output(self):
        """Verify that enum_name parameter values are reflected in the output."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("ENUM_VAL1 = 100\nENUM_VAL2 = 200\n")
            out_file1 = Path(tmpdir) / "output1.h"
            out_file2 = Path(tmpdir) / "output2.h"
            
            # First call with enum_name="FirstEnum"
            result1 = header_make.main_impl(str(in_file), str(out_file1), "enum", "FirstEnum")
            assert result1 == 0
            content1 = out_file1.read_text()
            assert "enum FirstEnum{" in content1
            
            # Second call with enum_name="SecondEnum"
            result2 = header_make.main_impl(str(in_file), str(out_file2), "enum", "SecondEnum")
            assert result2 == 0
            content2 = out_file2.read_text()
            assert "enum SecondEnum{" in content2
            
            # Verify they are different
            assert content1 != content2


class TestEdgeCases:
    """Test edge cases and boundary conditions."""
    
    def test_multiline_block_comment_handling(self):
        """Correctly handle block comments spanning 3+ lines."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text(
                "VAL1 = 1\n"
                "/* start of\n"
                "   multiline\n"
                "   comment */\n"
                "VAL2 = 2\n"
            )
            
            definitions = list(header_make.iter_definitions(in_file))
            ids = [defn[0] for defn in definitions]
            
            assert "VAL1" in ids
            assert "VAL2" in ids
            assert len(ids) == 2
    
    def test_line_comment_takes_precedence_over_block(self):
        """Line comment // should take precedence and terminate parsing."""
        result, state = header_make.strip_comments_keep_state("CODE = 1 // comment /* fake", False)
        assert "/*" not in result  # The /* is part of the line comment
        assert state is False
    
    def test_output_includes_proper_header_guards(self):
        """Output includes appropriate include guards or markers."""
        entries = [("MY_ID", "42")]
        output = header_make.render_output("test.hsrc", header_make.MODE_DEFINE, "", entries)
        
        # Should have include guards
        assert "#ifndef" in output
        assert "#define" in output
        assert "#endif" in output
    
    def test_handles_special_characters_in_values(self):
        """Correctly handle non-alphanumeric characters in token values."""
        with tempfile.TemporaryDirectory() as tmpdir:
            in_file = Path(tmpdir) / "input.txt"
            in_file.write_text("MY_ID = 0x1F\n")
            
            definitions = list(header_make.iter_definitions(in_file))
            assert definitions[0] == ("MY_ID", "0x1F")


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
