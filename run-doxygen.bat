call sakura\githash.bat

set PROJECT_NUMBER=%GIT_SHORT_COMMIT_HASH%
doxygen doxygen.conf
