# Microsoft Developer Studio Project File - Name="sakura" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=sakura - Win32 Release
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "sakura.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "sakura.mak" CFG="sakura - Win32 Release"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "sakura - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "sakura - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sakura - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\sakura__"
# PROP BASE Intermediate_Dir ".\sakura__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GR /GX /O1 /I "../sakura_core" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Release/sakura_core.lib comctl32.lib user32.lib /nologo /subsystem:windows /pdb:none /machine:I386
# SUBTRACT LINK32 /verbose /profile /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "sakura - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\sakura_0"
# PROP BASE Intermediate_Dir ".\sakura_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "../sakura_core" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /WX /Fr /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 debug/sakura_core.lib RASAPI32.LIB ole32.lib comctl32.lib Imm32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib htmlhelp.lib /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "sakura - Win32 Release"
# Name "sakura - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\sakura_core\sakura_rc.rc

!IF  "$(CFG)" == "sakura - Win32 Release"

!ELSEIF  "$(CFG)" == "sakura - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinMain.cpp

!IF  "$(CFG)" == "sakura - Win32 Release"

!ELSEIF  "$(CFG)" == "sakura - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\sakura_core\sakura_rc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\resource\cursor_copy.cur
# End Source File
# Begin Source File

SOURCE=..\resource\cursor_hand.cur
# End Source File
# Begin Source File

SOURCE=..\resource\cursor_move.cur
# End Source File
# Begin Source File

SOURCE=..\resource\icon_debug.ico
# End Source File
# Begin Source File

SOURCE=..\resource\icon_grep.ico
# End Source File
# Begin Source File

SOURCE=..\resource\icon_hana.ico
# End Source File
# Begin Source File

SOURCE=..\resource\icon_std.ico
# End Source File
# Begin Source File

SOURCE=..\resource\mytool.bmp
# End Source File
# Begin Source File

SOURCE=..\resource\opened.bmp
# End Source File
# Begin Source File

SOURCE=..\resource\opened_this.bmp
# End Source File
# Begin Source File

SOURCE=..\resource\printer.bmp
# End Source File
# End Group
# End Target
# End Project
