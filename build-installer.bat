set platform=%1
set configuration=%2
set INSTALLER_RESOURCES=installer\temp
set INSTALLER_WORK=installer\sakura
set INSTALLER_OUTPUT=installer\Output

if exist "%INSTALLER_RESOURCES%" rmdir /s /q "%INSTALLER_RESOURCES%"
if exist "%INSTALLER_WORK%"      rmdir /s /q "%INSTALLER_WORK%"
if exist "%INSTALLER_OUTPUT%"    rmdir /s /q "%INSTALLER_OUTPUT%"

7z x installer\sinst_src.zip -O"%INSTALLER_RESOURCES%" || (echo error && exit /b 1)
7z x installer\bron412.zip   -O"%INSTALLER_RESOURCES%" || (echo error && exit /b 1)

mkdir %INSTALLER_WORK%
mkdir %INSTALLER_WORK%\keyword

copy %INSTALLER_RESOURCES%\sakura.exe.manifest.x %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES%\sakura.exe.manifest.v %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES%\sakura.exe.ini        %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES%\keyword\*.*           %INSTALLER_WORK%\keyword\

copy %INSTALLER_RESOURCES%\*.dll                 %INSTALLER_WORK%\
copy %INSTALLER_RESOURCES%\*.txt                 %INSTALLER_WORK%\

copy help\sakura\sakura.chm                      %INSTALLER_WORK%\
copy help\plugin\plugin.chm                      %INSTALLER_WORK%\
copy help\macro\macro.chm                        %INSTALLER_WORK%\

copy %platform%\%configuration%\*.exe            %INSTALLER_WORK%\

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" installer\sakura.iss || (echo error && exit /b 1)

if exist "%INSTALLER_RESOURCES%" rmdir /s /q "%INSTALLER_RESOURCES%"
if exist "%INSTALLER_WORK%"      rmdir /s /q "%INSTALLER_WORK%"
