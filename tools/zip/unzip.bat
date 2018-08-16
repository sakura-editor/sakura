set SRCZIP=%1
set OUTDIR=%2

if exist "%OUTDIR%" rmdir /s /q "%OUTDIR%"
powershell -ExecutionPolicy RemoteSigned -File %~dp0unzip.ps1 %SRCZIP% %OUTDIR%
