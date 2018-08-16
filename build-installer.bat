set platform=%1
set configuration=%2
set INSTALLER_WORK=installer\sakura
set INSTALLER_OUTPUT=installer\Output-%platform%

set INSTALLER_RESOURCES_SINT=installer\sinst_src
set INSTALLER_RESOURCES_BRON=installer\temp\bron

if exist "%INSTALLER_WORK%"      rmdir /s /q "%INSTALLER_WORK%"
if exist "%INSTALLER_OUTPUT%"    rmdir /s /q "%INSTALLER_OUTPUT%"

mkdir %INSTALLER_WORK%
mkdir %INSTALLER_WORK%\keyword

copy /Y %INSTALLER_RESOURCES_SINT%\sakura.exe.manifest.x %INSTALLER_WORK%\
copy /Y %INSTALLER_RESOURCES_SINT%\sakura.exe.manifest.v %INSTALLER_WORK%\
copy /Y %INSTALLER_RESOURCES_SINT%\sakura.exe.ini        %INSTALLER_WORK%\
copy /Y %INSTALLER_RESOURCES_SINT%\keyword\*.*           %INSTALLER_WORK%\keyword\
copy /Y %INSTALLER_RESOURCES_BRON%\*.txt                 %INSTALLER_WORK%\

copy /Y /B help\sakura\sakura.chm                           %INSTALLER_WORK%\
copy /Y /B help\plugin\plugin.chm                           %INSTALLER_WORK%\
copy /Y /B help\macro\macro.chm                             %INSTALLER_WORK%\

copy /Y /B %platform%\%configuration%\*.exe                 %INSTALLER_WORK%\
copy /Y /B %platform%\%configuration%\*.dll                 %INSTALLER_WORK%\

set SAKURA_ISS=installer\sakura-%platform%.iss
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" %SAKURA_ISS% || (echo error && exit /b 1)
