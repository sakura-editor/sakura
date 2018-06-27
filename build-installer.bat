set platform=%1
set configuration=%2
set INSTALLER_RESOURCES=installer\temp
set INSTALLER_WORK=installer\sakura
set INSTALLER_OUTPUT=installer\Output

set INSTALLER_RESOURCES_SINT=installer\sinst_src
set INSTALLER_RESOURCES_BRON=installer\temp\bron

if exist "%INSTALLER_RESOURCES%" rmdir /s /q "%INSTALLER_RESOURCES%"
if exist "%INSTALLER_WORK%"      rmdir /s /q "%INSTALLER_WORK%"
if exist "%INSTALLER_OUTPUT%"    rmdir /s /q "%INSTALLER_OUTPUT%"

if exist "%INSTALLER_RESOURCES_BRON%" rmdir /s /q "%INSTALLER_RESOURCES_BRON%"

7z x installer\externals\bregonig\bron412.zip  -O"%INSTALLER_RESOURCES_BRON%" || (echo error && exit /b 1)

mkdir %INSTALLER_WORK%
mkdir %INSTALLER_WORK%\keyword

copy %INSTALLER_RESOURCES_SINT%\sakura.exe.manifest.x %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_SINT%\sakura.exe.manifest.v %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_SINT%\sakura.exe.ini        %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES_SINT%\keyword\*.*           %INSTALLER_WORK%\keyword\

if "%platform%" == "x64" (
	copy %INSTALLER_RESOURCES_BRON%\x64\*.dll         %INSTALLER_WORK%\
) else (
	copy %INSTALLER_RESOURCES_BRON%\*.dll             %INSTALLER_WORK%\
)
copy %INSTALLER_RESOURCES_BRON%\*.txt                 %INSTALLER_WORK%\

copy help\sakura\sakura.chm                      %INSTALLER_WORK%\
copy help\plugin\plugin.chm                      %INSTALLER_WORK%\
copy help\macro\macro.chm                        %INSTALLER_WORK%\

copy %platform%\%configuration%\*.exe            %INSTALLER_WORK%\
copy %platform%\%configuration%\*.dll            %INSTALLER_WORK%\

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" installer\sakura.iss || (echo error && exit /b 1)

if exist "%INSTALLER_RESOURCES%" rmdir /s /q "%INSTALLER_RESOURCES%"
if exist "%INSTALLER_WORK%"      rmdir /s /q "%INSTALLER_WORK%"
