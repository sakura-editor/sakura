@echo off
@echo installing
set DOXYGEN_INSTALLER=doxygen-1.8.14-setup.exe
if not "%APPVEYOR_BUILD_NUMBER%" == "" (
	%~dp0%DOXYGEN_INSTALLER% /silent /suppressmsgboxes
) else (
	@echo skip installing %DOXYGEN_INSTALLER%
)
