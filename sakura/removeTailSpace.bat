@echo off

set SED=C:\Program Files\Git\usr\bin\sed.exe
set UNIX2DOS=C:\Program Files\Git\usr\bin\unix2dos.exe

set TARGET=%1

if exist "%SED%" (
	@echo ---- removing tail spaces ----
	"%SED%" --regexp-extended --in-place s/\s+$// "%TARGET%"
)

if exist "%UNIX2DOS%" (
	@echo ---- converting EOL to DOS/Windows Format ----
	"%UNIX2DOS%" -q "%TARGET%"
)

ENDLOCAL
rem exit 0
