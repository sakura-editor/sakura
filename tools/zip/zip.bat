set DSTZIP=%1
set SRCDIR=%2

if exist "%DSTZIP%" del "%DSTZIP%"
powershell -ExecutionPolicy RemoteSigned -File %~dp0zip.ps1 %DSTZIP% %SRCDIR%
