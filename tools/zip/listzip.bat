set SRCZIP=%1
powershell -ExecutionPolicy RemoteSigned -File %~dp0listzip.ps1 %SRCZIP%
