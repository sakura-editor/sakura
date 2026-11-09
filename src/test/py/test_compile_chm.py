"""Tests for the HTML Help Compiler process and output synchronization."""

import importlib.util
import ctypes
import os
import subprocess
from pathlib import Path
from unittest.mock import MagicMock

import pytest


def _load_module():
    repo_root = Path(__file__).resolve().parents[3]
    module_path = repo_root / "src/main/py/compile_chm.py"
    spec = importlib.util.spec_from_file_location("compile_chm", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module spec: {module_path}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture(scope="module")
def compile_chm_module():
    return _load_module()


# Command construction

def test_build_command_runs_hhc_directly(compile_chm_module):
    project = Path("C:/work/help/sakura.hhp")

    command = compile_chm_module.build_command(
        project,
        {"CMD_HHC": "C:/Program Files/HTML Help Workshop/hhc.exe"},
    )

    assert command == [
        "C:/Program Files/HTML Help Workshop/hhc.exe",
        str(project),
    ]


def test_build_command_uses_default_hhc(compile_chm_module):
    project = Path("C:/work/help/sakura.hhp")

    command = compile_chm_module.build_command(project, {})

    assert command == [compile_chm_module.DEFAULT_HHC, str(project)]


def test_build_command_runs_hhc_through_locale_emulator(compile_chm_module):
    project = Path("C:/work/help/sakura.hhp")
    environment = {
        "CMD_HHC": "C:/Program Files/HTML Help Workshop/hhc.exe",
        "CMD_LEPROC": "C:/tools/LEProc.exe",
        "COMSPEC": "C:/Windows/System32/cmd.exe",
    }

    command = compile_chm_module.build_command(project, environment)

    assert command[:3] == [
        "C:/tools/LEProc.exe",
        "C:/Windows/System32/cmd.exe",
        "/C",
    ]
    assert command[3] == subprocess.list2cmdline(
        [environment["CMD_HHC"], str(project)]
    )


# Output copying

def test_copy_chm_skips_copy_for_same_path(tmp_path, mocker, compile_chm_module):
    source = tmp_path / "sakura.chm"
    source.write_bytes(b"chm")
    copy = mocker.patch.object(compile_chm_module.shutil, "copy2")

    assert compile_chm_module.copy_chm(source, str(source)) is True
    copy.assert_not_called()


def test_copy_chm_copies_to_absolute_file_destination(
    tmp_path, compile_chm_module
):
    source = tmp_path / "source" / "sakura.chm"
    source.parent.mkdir()
    source.write_bytes(b"chm")
    destination = tmp_path / "destination"
    destination.mkdir()
    destination_file = destination / source.name

    assert compile_chm_module.copy_chm(source, str(destination_file)) is True
    assert destination_file.read_bytes() == b"chm"


@pytest.mark.parametrize(
    "destination",
    [
        "../outside.chm",
        "output/../../outside.chm",
        f"output{os.sep}",
        "output.txt",
    ],
)
def test_destination_path_rejects_unsafe_destination(
    destination, tmp_path, compile_chm_module
):
    with pytest.raises(ValueError):
        compile_chm_module._destination_path(destination)


def test_destination_path_rejects_missing_parent(tmp_path, compile_chm_module):
    with pytest.raises(ValueError):
        compile_chm_module._destination_path(
            str(tmp_path / "missing" / "output.chm")
        )


def test_destination_path_rejects_absolute_parent_traversal(
    tmp_path, compile_chm_module
):
    destination = tmp_path / "safe" / ".." / "outside.chm"

    with pytest.raises(ValueError):
        compile_chm_module._destination_path(str(destination))


def test_copy_chm_returns_false_when_source_is_missing(
    tmp_path, compile_chm_module
):
    assert compile_chm_module.copy_chm(
        tmp_path / "missing.chm",
        str(tmp_path / "destination.chm"),
    ) is False


def test_copy_chm_returns_false_when_copy_fails(
    tmp_path, mocker, compile_chm_module
):
    source = tmp_path / "sakura.chm"
    source.write_bytes(b"chm")
    mocker.patch.object(
        compile_chm_module.shutil,
        "copy2",
        side_effect=PermissionError("locked"),
    )

    assert compile_chm_module.copy_chm(
        source,
        str(tmp_path / "destination.chm"),
    ) is False


# Output synchronization

def test_is_locked_windows_closes_unlocked_file(mocker, compile_chm_module):
    create_file = MagicMock(return_value=42)
    close_handle = MagicMock()
    kernel32 = MagicMock()
    kernel32.CreateFileW = create_file
    kernel32.CloseHandle = close_handle
    mocker.patch.object(ctypes, "WinDLL", return_value=kernel32)

    assert compile_chm_module._is_locked_windows(Path("output.chm")) is False
    close_handle.assert_called_once_with(42)


@pytest.mark.parametrize(
    ("error_code", "expected"),
    [
        (2, False),
        (3, False),
        (32, True),
        (33, True),
    ],
)
def test_is_locked_windows_handles_expected_errors(
    error_code, expected, mocker, compile_chm_module
):
    kernel32 = MagicMock()
    kernel32.CreateFileW.return_value = ctypes.c_void_p(-1).value
    mocker.patch.object(ctypes, "WinDLL", return_value=kernel32)
    mocker.patch.object(ctypes, "get_last_error", return_value=error_code)

    assert compile_chm_module._is_locked_windows(Path("output.chm")) is expected


def test_is_locked_windows_raises_unexpected_error(mocker, compile_chm_module):
    kernel32 = MagicMock()
    kernel32.CreateFileW.return_value = ctypes.c_void_p(-1).value
    mocker.patch.object(ctypes, "WinDLL", return_value=kernel32)
    mocker.patch.object(ctypes, "get_last_error", return_value=5)
    win_error = mocker.patch.object(
        ctypes,
        "WinError",
        return_value=PermissionError("access denied"),
    )

    with pytest.raises(PermissionError, match="access denied"):
        compile_chm_module._is_locked_windows(Path("output.chm"))
    win_error.assert_called_once_with(5)


def test_is_locked_returns_false_for_missing_file(tmp_path, compile_chm_module):
    assert compile_chm_module.is_locked(tmp_path / "missing.chm") is False


def test_is_locked_uses_windows_lock_check(tmp_path, mocker, compile_chm_module):
    output = tmp_path / "output.chm"
    output.write_bytes(b"chm")
    windows_check = mocker.patch.object(
        compile_chm_module,
        "_is_locked_windows",
        return_value=True,
    )

    assert compile_chm_module.is_locked(output) is True
    windows_check.assert_called_once_with(output)


def test_is_locked_fallback_opens_file_for_writing(
    tmp_path, mocker, compile_chm_module
):
    output = tmp_path / "output.chm"
    output.write_bytes(b"chm")
    mocker.patch.object(compile_chm_module.os, "name", "posix")

    assert compile_chm_module.is_locked(output) is False


def test_is_locked_fallback_reports_permission_error(mocker, compile_chm_module):
    path = MagicMock(spec=Path)
    path.exists.return_value = True
    path.open.side_effect = PermissionError("locked")
    mocker.patch.object(compile_chm_module.os, "name", "posix")

    assert compile_chm_module.is_locked(path) is True


def test_wait_for_outputs_waits_until_files_are_unlocked(
    tmp_path, mocker, compile_chm_module
):
    compiled_help = tmp_path / "sakura.chm"
    compile_log = tmp_path / "Compile.log"
    compiled_help.write_bytes(b"chm")
    compile_log.write_text("log", encoding="utf-8")
    is_locked = mocker.patch.object(
        compile_chm_module,
        "is_locked",
        side_effect=[True, False, False],
    )
    sleep = mocker.patch.object(compile_chm_module.time, "sleep")

    result = compile_chm_module.wait_for_outputs(
        compiled_help,
        compile_log,
        poll_count=2,
        poll_interval=1,
    )

    assert result is True
    assert is_locked.call_count == 3
    sleep.assert_called_once_with(1)


def test_wait_for_outputs_times_out_when_output_is_missing(
    tmp_path, mocker, compile_chm_module
):
    sleep = mocker.patch.object(compile_chm_module.time, "sleep")

    result = compile_chm_module.wait_for_outputs(
        tmp_path / "missing.chm",
        tmp_path / "Compile.log",
        poll_count=3,
        poll_interval=1,
    )

    assert result is False
    assert sleep.call_count == 3


# Compile retries and process cleanup

def test_compile_project_retries_then_copies(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    destination = tmp_path / "output" / "sakura.chm"
    destination.parent.mkdir()
    processes = [MagicMock(), MagicMock()]
    for process in processes:
        process.poll.return_value = None
    popen = mocker.patch.object(
        compile_chm_module.subprocess,
        "Popen",
        side_effect=processes,
    )
    wait_for_outputs = mocker.patch.object(
        compile_chm_module,
        "wait_for_outputs",
        side_effect=[False, True],
    )
    copy_chm = mocker.patch.object(
        compile_chm_module,
        "copy_chm",
        return_value=True,
    )

    result = compile_chm_module.compile_project(
        project,
        str(destination),
        {"CMD_HHC": "hhc.exe"},
        max_attempts=3,
        poll_count=1,
        poll_interval=0,
    )

    assert result is True
    assert popen.call_count == 2
    assert wait_for_outputs.call_count == 2
    copy_chm.assert_called_once_with(project.with_suffix(".chm"), str(destination))
    processes[0].terminate.assert_called_once_with()
    processes[0].wait.assert_called_once_with(
        timeout=compile_chm_module.PROCESS_STOP_TIMEOUT
    )


def test_stop_process_kills_process_that_does_not_terminate(compile_chm_module):
    process = MagicMock()
    process.poll.return_value = None
    process.wait.side_effect = [
        subprocess.TimeoutExpired("hhc.exe", 5),
        0,
    ]

    compile_chm_module._stop_process(process)

    process.terminate.assert_called_once_with()
    process.kill.assert_called_once_with()
    assert process.wait.call_count == 2


def test_compile_project_removes_stale_outputs(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    compiled_help = project.with_suffix(".chm")
    compile_log = project.parent / "Compile.Log"
    destination = tmp_path / "destination.chm"
    for path in (compiled_help, compile_log, destination):
        path.write_bytes(b"stale")
    mocker.patch.object(compile_chm_module.subprocess, "Popen")
    mocker.patch.object(
        compile_chm_module,
        "wait_for_outputs",
        return_value=True,
    )
    mocker.patch.object(
        compile_chm_module,
        "copy_chm",
        return_value=True,
    )

    result = compile_chm_module.compile_project(
        project,
        str(destination),
        {"CMD_HHC": "hhc.exe"},
    )

    assert result is True
    assert not compiled_help.exists()
    assert not compile_log.exists()
    assert not destination.exists()


def test_compile_project_fails_after_maximum_attempts(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    processes = [MagicMock(), MagicMock(), MagicMock()]
    for process in processes:
        process.poll.return_value = None
    popen = mocker.patch.object(
        compile_chm_module.subprocess,
        "Popen",
        side_effect=processes,
    )
    mocker.patch.object(
        compile_chm_module,
        "wait_for_outputs",
        return_value=False,
    )

    result = compile_chm_module.compile_project(
        project,
        str(tmp_path / "sakura.chm"),
        {"CMD_HHC": "hhc.exe"},
        max_attempts=3,
        poll_count=1,
        poll_interval=0,
    )

    assert result is False
    assert popen.call_count == 3
    assert all(process.terminate.call_count == 1 for process in processes)


def test_compile_project_retries_process_start_failure(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    popen = mocker.patch.object(
        compile_chm_module.subprocess,
        "Popen",
        side_effect=OSError("not found"),
    )

    result = compile_chm_module.compile_project(
        project,
        str(tmp_path / "sakura.chm"),
        {"CMD_HHC": "missing.exe"},
        max_attempts=3,
    )

    assert result is False
    assert popen.call_count == 3


# Command-line interface

@pytest.mark.parametrize(
    ("arguments", "expected"),
    [
        (["compile_chm.py"], 2),
        (["compile_chm.py", "help.txt", "help.chm"], 2),
    ],
)
def test_main_rejects_invalid_arguments(arguments, expected, compile_chm_module):
    assert compile_chm_module.main(arguments) == expected


def test_main_returns_success_when_compilation_succeeds(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    compile_project = mocker.patch.object(
        compile_chm_module,
        "compile_project",
        return_value=True,
    )

    result = compile_chm_module.main(
        ["compile_chm.py", str(project), str(tmp_path / "sakura.chm")]
    )

    assert result == 0
    compile_project.assert_called_once()


def test_main_returns_failure_when_compilation_fails(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    mocker.patch.object(
        compile_chm_module,
        "compile_project",
        return_value=False,
    )

    result = compile_chm_module.main(
        ["compile_chm.py", str(project), str(tmp_path / "sakura.chm")]
    )

    assert result == 1


def test_main_rejects_unsafe_destination_before_compilation(
    tmp_path, mocker, compile_chm_module
):
    project = tmp_path / "sakura.hhp"
    project.write_text("project", encoding="utf-8")
    compile_process = mocker.patch.object(
        compile_chm_module.subprocess,
        "Popen",
    )

    result = compile_chm_module.main(
        ["compile_chm.py", str(project), "../outside.chm"]
    )

    assert result == 2
    compile_process.assert_not_called()
