@echo off
@echo ---- start python check_encoding.py ----
python src\main\py\check_encoding.py %1 || (echo error check_encoding.py  && exit /b 1)
@echo ---- end   python check_encoding.py ----
@echo.
exit /b 0
