@echo off
@echo ---- start python checkEncoding.py ----
python checkEncoding.py %1 || (echo error checkEncoding.py  && exit /b 1)
@echo ---- end   python checkEncoding.py ----
@echo.
exit /b 0
