@echo off
if not "%APPVEYOR_BUILD_NUMBER%" == "" (
	@echo installing chardet
	pip install chardet --user
) else (
	@echo skip installing chardet
)

@echo ---- start python checkEncoding.py ----
python checkEncoding.py %1 || (echo error checkEncoding.py  && exit /b 1)
@echo ---- end   python checkEncoding.py ----
@echo.
exit /b 0
