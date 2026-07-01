# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Sakura Editor is a free Windows text editor written in C++20, licensed under the zlib License. The primary build target is Windows (MSVC), with experimental MinGW support.

## Build Commands

### Visual Studio (Primary)

Open `sakura.sln` in Visual Studio and build, or use the command line:

```cmd
build-sln.bat <Platform> <Configuration>
# Example: x64 Release
build-sln.bat x64 Release
# Example: Win32 Debug, specifying VS 2019
set ARG_VSVERSION=16
build-sln.bat Win32 Debug
# Run Unit Testing
Win32\Debug\tests1.exe
```

Full build (exe + HTML help + installer):
```cmd
build-all.bat <Platform> <Configuration>
<Platform>\<Configuration>\tests1.exe
```

### MinGW (Experimental — binaries may not work correctly)

```bash
cmake -S . -B build/MinGW -DCMAKE_BUILD_TYPE=Debug -DBUILD_PLATFORM=MinGW
cmake --build build/MinGW
ctest --test-dir build/MinGW --output-on-failure
```

### Useful Build Environment Variables

| Variable | Effect |
|---|---|
| `SKIP_CREATE_GITHASH=1` | Skip regenerating `githash.h` (useful when comparing binaries across refactors) |
| `FORCE_POWERSHELL_ZIP=1` | Force PowerShell for ZIP operations instead of 7z |
| `ARG_VSVERSION=16` | Override Visual Studio version selection (16=VS2019, 17=VS2022) |

### Running Tests

Tests use GoogleTest. With MinGW:
```bash
ctest --test-dir build/MinGW --output-on-failure
```

Tests are in `src/test/cpp/tests1/`. The test binary is `tests1`.

### Static Analysis

```cmd
run-cppcheck.bat <Platform> <Configuration>
```

For cpplint (style check):
```pwsh
pip install cpplint
cpplint --recursive sakura_core
```

## Architecture

### Two-Process Model

Sakura Editor uses a two-process architecture:

- **Control Process** (`CControlProcess`) — A single hidden process (system tray) that manages all shared state across editor instances. It owns `CControlTray`.
- **Editor Process** (`CNormalProcess`) — One per editor window. Creates and manages a `CEditWnd`. Multiple editor processes can run simultaneously.
- **`CProcessFactory`** — Inspects the command line at startup to decide which process type to create. If an editor process is starting and no control process exists yet, it launches one first.
- **`CShareData` / `DLLSHAREDATA`** — Shared memory structure that all processes map into their address space. This is the IPC mechanism between the control process and editor processes.

### Core Class Hierarchy

```
WinMain
  └─ CProcessFactory::Create()
       ├─ CControlProcess (system tray, shared state)
       │    └─ CControlTray
       └─ CNormalProcess (editor window)
            └─ CEditApp
                 └─ CEditWnd (outer frame window)
                      ├─ CMainToolBar, CTabWnd, CMainStatusBar
                      └─ CEditView (the text editing area) ×1–4 (splitter panes)
                           └─ CViewCommander (dispatches EFunctionCode commands)
```

### Document Model

`CEditDoc` aggregates the document subsystems:
- `CDocLineMgr` / `CDocLine` (`doc/logic/`) — Logical line storage (raw text, character encoding)
- `CLayoutMgr` / `CLayout` (`doc/layout/`) — Layout lines (visual wrapping, tab expansion); sits above the logical model
- `CDocEditor` — Edit operations (undo/redo via `COpeBuf`/`COpeBlk`)
- `CDocFile` — File path and encoding metadata
- `CDocFileOperation` — Open/close/save operations
- `CDocType` — Document type (language mode) association

### Command Dispatch

All editor commands are `EFunctionCode` enum values (defined in the auto-generated `Funccode_define.h` / `Funccode_enum.h`, generated from `sakura_core/Funccode_x.hsrc` by `HeaderMake.exe`).

`CViewCommander::HandleCommand()` is the central dispatcher. Implementations are split across:
- `CViewCommander_Edit.cpp`, `CViewCommander_File.cpp`, `CViewCommander_Cursor.cpp`, etc.

Function code ranges:
- `20000–21999`: Plugin commands (20 × 100)
- `30000–32767`: User-assignable commands (menus, keyboard)
- `40000–49511`: Macro functions

### Key Subsystems

| Directory | Responsibility |
|---|---|
| `sakura_core/_main/` | Entry point, process classes, global state |
| `sakura_core/_os/` | OS abstraction (clipboard, drop target, etc.) |
| `sakura_core/window/` | Window classes (`CEditWnd`, `CTabWnd`, toolbar, status bar) |
| `sakura_core/view/` | Text editing view (`CEditView`), caret, ruler, painting |
| `sakura_core/cmd/` | Command implementations (`CViewCommander_*.cpp`) |
| `sakura_core/doc/` | Document model (logic, layout, file ops, type) |
| `sakura_core/env/` | Environment managers (shared data, keyword sets, doc types, file names) |
| `sakura_core/prop/` | Common preferences dialog pages (`CPropCom*.cpp`) |
| `sakura_core/typeprop/` | Per-document-type settings dialog and import/export |
| `sakura_core/types/` | Language definitions (`CType_Cpp.cpp`, `CType_Python.cpp`, etc.) |
| `sakura_core/macro/` | Macro system (key macro, PPA, Python) |
| `sakura_core/plugin/` | DLL plugin system |
| `sakura_core/agent/` | Background agents (auto-save, auto-reload, backup, grep, load/save) |
| `sakura_core/grep/` | Grep file enumeration |
| `sakura_core/charset/` | Character set detection and conversion |
| `sakura_core/extmodule/` | External regex/migemo library wrappers |
| `sakura_core/func/` | Function code table, key binding |
| `sakura_core/util/` | Utilities; `design_template.h` provides `TSingleton`, `TSingleInstance`, `DISALLOW_COPY_AND_ASSIGN` |

### Localization

`sakura_lang/` contains resource-only DLLs for non-Japanese locales (en-US, zh-CN). The main `sakura_rc.rc` is Japanese. Language DLLs are built as separate projects (`sakura_lang_en_US.vcxproj`, etc.).

### External Dependencies

All third-party code lives under `externals/` as git submodules:
- `Onigmo` / `bregonig` — Regular expression engine
- `cmigemo` + `cmigemo-dict` — Migemo incremental Japanese search
- `ctags` — Tag jump support
- `diffutils` — File diff
- `googletest` — Unit testing
- `miniz-cpp` — ZIP handling (tests only)
- `darkmodelib` — Windows dark mode support

### CI

GitHub Actions workflows are in `.github/workflows/`. The main build workflow (`build-sakura.yml`) builds all four combinations of {Win32, x64} × {Debug, Release}. To skip CI for documentation-only changes, include `[ci skip]` or `[skip ci]` in the commit message (note: this does not apply on PR merge).
