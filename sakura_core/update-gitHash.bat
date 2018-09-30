@echo off

set OUTDIR=%1

if "%OUTDIR%" == "" (
	set OUTDIR=.
)
set GITREV_H=%OUTDIR%\gitrev.h

for /f "usebackq" %%s in (`git show -s --format^=%%H`) do (
    set COMMITID=%%s
)
for /f "usebackq" %%s in (`git show -s --format^=%%h`) do (
    set SHORT_COMMITID=%%s
)

type nul                                  > %GITREV_H%
if "%COMMITID%" == "" (
	type nul                                  >> %GITREV_H%
) else (
	echo #define GIT_COMMIT_HASH "%COMMITID%" >> %GITREV_H%
)
if "%SHORT_COMMITID%" == "" (
	type nul                                              >> %GITREV_H%
) else (
	echo #define GIT_SHORT_COMMIT_HASH "%SHORT_COMMITID%" >> %GITREV_H%
)

ENDLOCAL
rem exit 0
