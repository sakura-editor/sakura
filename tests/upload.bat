@echo off
set UPLOAD_XML=%1
set UPLOAD_PS=%~dp0upload.ps1

if "%APPVEYOR_BUILD_NUMBER%" == "" (
	@echo This is not appveyor build. skip uploading xml files.
	exit /b 0
)

powershell -ExecutionPolicy RemoteSigned -File "%UPLOAD_PS%" "%UPLOAD_XML%"
