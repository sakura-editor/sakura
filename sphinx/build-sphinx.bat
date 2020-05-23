@echo off

set MAKE_BAT=%~dp0sakura\make.bat
set HHP_SAKURA=%~dp0sakura\build\htmlhelp\SAKURAEditordoc.hhp

python -c "import sphinx"
if errorlevel 1 (
	pip install sphinx
	python -c "import sphinx"
)
if errorlevel 1 (
	echo Please run 'pip install sphinx' in advance.
	exit /b 1
)

pushd "%~dp0"

call "%MAKE_BAT%" htmlhelp
if errorlevel 1 (
	echo failed to create htmlhelp project files.
	popd
	exit /b 1
)

call "%~dp0..\tools\find-tools.bat"
if "%CMD_HHC%" == "" (
	echo hhc.exe was not found.
	popd
	exit /b 1
)

"%CMD_HHC%" "%HHP_SAKURA%"
if not errorlevel 1 (
	echo error "%HHP_SAKURA%" errorlevel %errorlevel%
	"%CMD_HHC%" "%HHP_SAKURA%"
)
if not errorlevel 1 (
	echo error "%HHP_SAKURA%" errorlevel %errorlevel%
	popd
	exit /b 1
)

popd
exit /b 0
