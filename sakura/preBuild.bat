@echo off

HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_define.h -mode=define
HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_enum.h -mode=enum -enum=EFunctionCode
MakefileMake -file=..\sakura_core\Makefile -dir=..\sakura_core

set GITREV_H=..\sakura_core\gitrev.h

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
