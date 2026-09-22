"""Run HTML Help Compiler and wait until its output files are ready.

HHC may exit before it releases the generated CHM and Compile.log files, so
process completion alone cannot be used as the success condition.
"""

import os
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Mapping

DEFAULT_HHC = r"C:\Program Files (x86)\HTML Help Workshop\hhc.exe"
DEFAULT_POLL_COUNT = 30
DEFAULT_POLL_INTERVAL = 1.0
DEFAULT_MAX_ATTEMPTS = 3
PROCESS_STOP_TIMEOUT = 5.0


def build_command(project: Path, environment: Mapping[str, str]) -> list[str]:
    """Build the HHC command, optionally wrapped by Locale Emulator."""
    hhc = environment.get("CMD_HHC") or DEFAULT_HHC
    locale_emulator = environment.get("CMD_LEPROC")
    if not locale_emulator:
        return [hhc, str(project)]

    command_processor = environment.get("COMSPEC") or "cmd.exe"
    hhc_command = subprocess.list2cmdline([hhc, str(project)])
    return [locale_emulator, command_processor, "/C", hhc_command]


def _is_locked_windows(path: Path) -> bool:
    """Check a Windows file lock by requesting exclusive read/write access."""
    import ctypes
    from ctypes import wintypes

    create_file = ctypes.WinDLL("kernel32", use_last_error=True).CreateFileW
    create_file.argtypes = (
        wintypes.LPCWSTR,
        wintypes.DWORD,
        wintypes.DWORD,
        wintypes.LPVOID,
        wintypes.DWORD,
        wintypes.DWORD,
        wintypes.HANDLE,
    )
    create_file.restype = wintypes.HANDLE

    handle = create_file(
        str(path),
        0x80000000 | 0x40000000,
        0,
        None,
        3,
        0x80,
        None,
    )
    invalid_handle = ctypes.c_void_p(-1).value
    if handle == invalid_handle:
        error = ctypes.get_last_error()
        if error in (2, 3):
            return False
        if error in (32, 33):
            return True
        raise ctypes.WinError(error)

    ctypes.WinDLL("kernel32", use_last_error=True).CloseHandle(handle)
    return False


def is_locked(path: Path) -> bool:
    """Return whether another process prevents read/write access to a file."""
    if not path.exists():
        return False
    if os.name == "nt":
        return _is_locked_windows(path)

    try:
        with path.open("r+b"):
            return False
    except PermissionError:
        return True


def wait_for_outputs(
    compiled_help: Path,
    compile_log: Path,
    *,
    poll_count: int = DEFAULT_POLL_COUNT,
    poll_interval: float = DEFAULT_POLL_INTERVAL,
) -> bool:
    """Wait until both compiler outputs exist and are no longer locked."""
    for _ in range(poll_count):
        if (
            compiled_help.exists()
            and not is_locked(compiled_help)
            and compile_log.exists()
            and not is_locked(compile_log)
        ):
            return True
        time.sleep(poll_interval)
    return False


def _destination_path(destination: str) -> Path:
    """Validate and return an absolute CHM destination file path."""
    destination_path = Path(destination)
    if (
        not destination_path.is_absolute()
        or ".." in destination_path.parts
        or destination_path.suffix.lower() != ".chm"
        or not destination_path.parent.is_dir()
    ):
        raise ValueError(f"Invalid CHM destination: {destination}")
    return destination_path


def copy_chm(source: Path, destination: str) -> bool:
    """Copy a compiled CHM unless source and destination are identical."""
    if not source.exists():
        return False

    destination_path = _destination_path(destination)
    if os.path.normcase(os.path.abspath(source)) == os.path.normcase(
        os.path.abspath(destination_path)
    ):
        print("Copy is not required.")
        return True

    try:
        shutil.copy2(source, destination_path)
    except OSError as error:
        print(f"Failed to copy {source} to {destination_path}: {error}", file=sys.stderr)
        return False
    return True


def _remove_file(path: Path) -> None:
    try:
        path.unlink()
    except FileNotFoundError:
        pass


def _stop_process(process: subprocess.Popen[bytes]) -> None:
    """Stop a compiler left running after a failed output wait."""
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=PROCESS_STOP_TIMEOUT)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


def compile_project(
    project: Path,
    destination: str,
    environment: Mapping[str, str],
    *,
    max_attempts: int = DEFAULT_MAX_ATTEMPTS,
    poll_count: int = DEFAULT_POLL_COUNT,
    poll_interval: float = DEFAULT_POLL_INTERVAL,
) -> bool:
    """Compile one HHP project and copy its completed CHM to destination."""
    compiled_help = project.with_suffix(".chm")
    compile_log = project.parent / "Compile.Log"
    destination_path = _destination_path(destination)

    _remove_file(compile_log)
    _remove_file(compiled_help)
    if destination_path != compiled_help:
        _remove_file(destination_path)

    command = build_command(project, environment)
    for attempt in range(1, max_attempts + 1):
        print(f"Starting HTML Help Compiler (attempt {attempt}/{max_attempts})")
        try:
            process = subprocess.Popen(command)
        except OSError as error:
            print(f"Failed to start HTML Help Compiler: {error}", file=sys.stderr)
            continue

        if wait_for_outputs(
            compiled_help,
            compile_log,
            poll_count=poll_count,
            poll_interval=poll_interval,
        ) and copy_chm(compiled_help, destination):
            return True

        _stop_process(process)

    print(
        f"HTML Help Compiler did not produce unlocked output within "
        f"{poll_count * poll_interval:g} seconds after {max_attempts} attempts.",
        file=sys.stderr,
    )
    return False


def main(argv: list[str]) -> int:
    """Validate command-line arguments and return a process exit code."""
    if len(argv) != 3:
        print(
            f"Usage: {Path(argv[0]).name if argv else 'compile_chm.py'} "
            "<HtmlHelpProject> <Destination>",
            file=sys.stderr,
        )
        return 2

    project = Path(argv[1]).resolve()
    if project.suffix.lower() != ".hhp" or not project.is_file():
        print(f"Invalid HTML Help project: {project}", file=sys.stderr)
        return 2

    try:
        return 0 if compile_project(project, argv[2], os.environ) else 1
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
