call sakura\githash.bat
call tools\hhc\find-hhc.bat

if exist html rmdir /s /q html

set PROJECT_NUMBER=%GIT_SHORT_COMMIT_HASH%
set HHC_LOCATION=%CMD_HHC%
doxygen doxygen.conf
