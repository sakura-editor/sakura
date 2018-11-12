@echo off
setlocal ENABLEDELAYEDEXPANSION

set NL=^

 
::: Blank lines above are parts of the NL (New Line) definition. Do not compact them.

::: About `appveyor` command line program:
::: https://www.appveyor.com/docs/build-worker-api/
set AppVeyor=appveyor
where /Q "%AppVeyor%"
if errorlevel 1 (
	echo>&2 %~nx0: `%AppVeyor%` command was not found. This filter does nothing effective.
	set AppVeyor=
)

set fileName=
set testName=
set testMsg=
::: for each line read from stdin.
for /F "usebackq delims=" %%L in (`FINDSTR /B "^"`) do (
	echo %%L

	if not "%AppVeyor%" == "" (
		for /F "tokens=1,2,3,5 delims=[]() " %%A in ("%%L") do (
			if "%%A" == "----------" (
				if not "%%D" == "" (
					set fileName=%%D
				)
			) else if "%%A" == "RUN" (
				set testName=%%B
				set testMsg=
			) else if "%%A" == "OK" (
				if not "!testName!,!fileName!" == "," (
					rem `start` does not wait for `%AppVeyor%` to return, so the main loop goes immediately.
					start "" "%AppVeyor%" AddTest !testName! -Framework xUnit -FileName !fileName! -Outcome Passed -Duration %%C -StdOut "!testMsg!"
				)
			) else if "%%A" == "FAILED" (
				if not "!testName!,!fileName!" == "," (
					rem `start` does not wait for `%AppVeyor%` to return, so the main loop goes immediately.
					start "" "%AppVeyor%" AddTest !testName! -Framework xUnit -FileName !fileName! -Outcome Failed -Duration %%C -ErrorMessage "!testMsg!"
				)
			) else if "%%A" == "PASSED" (
				rem
			) else if "%%A" == "==========" (
				rem
			) else (
				set testMsg=!testMsg!%%L!NL!
			)
		)
	)
)
