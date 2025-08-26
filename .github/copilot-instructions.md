# Sakura Editor - GitHub Copilot Instructions

**Always reference these instructions first and only fallback to search or bash commands when you encounter unexpected information that does not match the info here.**

Sakura Editor is a Japanese text editor for Windows written in C++. This is a **Windows-only codebase** that requires Visual Studio and Windows-specific dependencies.

## Working Effectively

### Essential Setup Requirements
**CRITICAL**: This project ONLY builds and runs on Windows. Do NOT attempt to build on Linux/macOS.

Required tools:
- **Visual Studio 2017 or later** (Community/Professional/Enterprise)
  - Must include Windows 10 SDK
  - HTML Help Compiler (hhc.exe) for documentation builds
- **7-Zip** (optional but recommended for external dependencies)
- **Inno Setup 6.x** (ISCC.exe) for installer builds
- **Git** with submodules support

### Build Commands and Timing

**NEVER CANCEL builds or tests.** Set appropriate timeouts and wait for completion.

#### Basic Build (Visual Studio Solution Only)
```cmd
build-sln.bat Win32 Release
build-sln.bat x64 Release
```
- **Expected time**: 2-3 minutes
- **Timeout setting**: Use 10+ minutes minimum
- **Builds**: sakura.exe, sakura_core library, language packs

#### Complete Build (Everything)
```cmd
build-all.bat Win32 Release
build-all.bat x64 Release
```
- **Expected time**: 5-8 minutes total
- **Timeout setting**: Use 15+ minutes minimum
- **NEVER CANCEL**: Full pipeline includes solution build, help compilation, and installer creation
- **Builds**: Executables + HTML Help (.chm) + Windows installer

#### Unit Tests
```cmd
tests\build-and-test.bat Win32 Release
tests\build-and-test.bat x64 Release
```
- **Expected time**: 3-5 minutes (includes build + test execution)
- **Timeout setting**: Use 10+ minutes minimum
- **Test framework**: GoogleTest with CMake
- **NEVER CANCEL**: Tests may take time to initialize and run

### Environment Setup Commands
```cmd
git submodule update --init --recursive
```
- Initializes GoogleTest submodule (required for unit tests)

### Development Workflow

#### Making Code Changes
1. **Always build first** to ensure clean starting state:
   ```cmd
   build-sln.bat Win32 Release
   ```

2. **Make your changes** to C++ source files in:
   - `sakura_core/` - Core editor functionality
   - `sakura/` - Main application
   - `tests/unittests/` - Unit tests

3. **Build and test your changes**:
   ```cmd
   build-sln.bat Win32 Release
   tests\build-and-test.bat Win32 Release
   ```

4. **For complete validation** (before submitting):
   ```cmd
   build-all.bat Win32 Release
   ```

#### CI Validation Steps
Always run these before committing to ensure CI will pass:
```cmd
build-all.bat Win32 Release
build-all.bat x64 Release
tests\build-and-test.bat Win32 Release
tests\build-and-test.bat x64 Release
```

## Validation and Testing

### Manual Validation Requirements
After making changes, **ALWAYS** test actual functionality:

1. **Build and run the editor**:
   ```cmd
   Win32\Release\sakura.exe
   ```

2. **Test basic functionality**:
   - Create/open/save text files
   - Test Japanese text input and display
   - Verify your specific changes work as expected

3. **Test with different configurations**:
   - Both Win32 and x64 builds if your changes affect both
   - Test with different file encodings if relevant

### Cannot Do / Limitations
- **Cannot build or run on Linux/macOS**: This is Windows-only software
- **MinGW build exists but does not work correctly**: Use Visual Studio builds only
- **Cannot test UI functionality in headless environments**: Manual testing required on Windows
- **Cannot run installer on non-Windows**: Installer creation requires Windows

## Project Structure

### Key Directories
- **`sakura_core/`** - Core editor engine and functionality
- **`sakura/`** - Main application and UI
- **`sakura_lang_en_US/`** - English language resources
- **`tests/unittests/`** - GoogleTest unit tests
- **`help/`** - HTML Help source files
- **`installer/`** - Inno Setup installer configuration
- **`tools/`** - Build utilities and helper tools

### Important Files
- **`sakura.sln`** - Main Visual Studio solution
- **`build-sln.bat`** - Basic build script
- **`build-all.bat`** - Complete build pipeline
- **`build.md`** - Detailed build documentation (Japanese)
- **`.vsconfig`** - Visual Studio component requirements

### Build Artifacts
After successful build, find outputs in:
- **`Win32\Release\`** or **`x64\Release\`** - Executables and DLLs
- **`help\`** - Compiled .chm help files
- **`installer\Output-{Platform}\`** - Windows installers

## Common Development Tasks

### Adding New Features
1. Identify affected components in `sakura_core/` or `sakura/`
2. Add unit tests in `tests/unittests/` if applicable
3. Build and test: `build-sln.bat Win32 Release`
4. Manual validation with actual editor usage
5. Full validation: `build-all.bat Win32 Release`

### Fixing Bugs
1. Reproduce the issue with actual editor
2. Identify source files in `sakura_core/` or `sakura/`
3. Make minimal changes
4. Test fix: `build-sln.bat Win32 Release`
5. Verify fix with manual testing
6. Run unit tests if applicable

### Updating Documentation
- Source files in `help/` directory
- Requires HTML Help Compiler (hhc.exe)
- Test with: `build-chm.bat`
- Full build: `build-all.bat Win32 Release`

## Timing Expectations
- **Simple solution build**: 2-3 minutes
- **Unit test execution**: 1-2 minutes  
- **Help compilation**: 1-2 minutes
- **Installer creation**: 1-2 minutes
- **Complete pipeline**: 5-8 minutes total
- **CRITICAL**: Always set timeouts to 2x expected time minimum

## Environment Variables
```cmd
set SKIP_CREATE_GITHASH=1
```
- Skips git hash generation for reproducible builds during development

## Repository Workflow
This project follows standard GitHub workflow:
- Main branch: `master`
- CI runs on: Windows runners with GitHub Actions
- Builds: Win32 and x64 Release/Debug configurations
- Artifacts: Executables, installers, and logs available from CI runs