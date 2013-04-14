@echo off

HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_define.h -mode=define
HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_enum.h -mode=enum -enum=EFunctionCode
MakefileMake -file=..\sakura_core\Makefile -dir=..\sakura_core"

SubWCRev.exe "..\\" "..\sakura_core\svnrev_template.h" "..\sakura_core\svnrev.h"
if %ERRORLEVEL% NEQ 0 (
  echo Automatic revision update unavailable, using generic template instead.
  echo You can safely ignore this message - see svnrev.h for details.
  copy /Y "..\sakura_core\svnrev_unknown.h" "..\sakura_core\svnrev.h"
)

ENDLOCAL
:: Always return an errorlevel of 0 -- this allows compilation to continue if SubWCRev failed.
rem exit 0
