@echo off
if not "%APPVEYOR_BUILD_NUMBER%" == "" (
	@echo installing chardet
	pip install chardet --user
) else (
	@echo skip installing chardet
)

python checkEncoding.py || exit /b 1
exit /b 0
