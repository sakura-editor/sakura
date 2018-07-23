@echo off
set CPPCHECK_MSI=
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
	@echo install for amd64
	set CPPCHECK_MSI=cppcheck-1.84-x64-Setup.msi
) else (
	@echo not supported platform
	exit /b 1
)
@echo installing
msiexec /i %~dp0%CPPCHECK_MSI% /quiet /qn /norestart /log cppcheck-install.log
