# Microsoft Developer Studio Project File - Name="sakura" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=sakura - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "sakura.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "sakura.mak" CFG="sakura - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "sakura - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "sakura - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "sakura"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sakura - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_one"
# PROP Intermediate_Dir "Release_one"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\sakura_core" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "SAKURA_NO_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib Imm32.lib htmlhelp.lib mpr.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "sakura - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_one"
# PROP Intermediate_Dir "Debug_one"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\sakura_core" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "SAKURA_NO_DLL" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib Imm32.lib htmlhelp.lib mpr.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sakura - Win32 Release"
# Name "sakura - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\sakura_core\CAutoSave.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CBregexp.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CCommandLine.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CControlProcess.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDicMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgAbout.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgCancel.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgCompare.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgExec.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgFind.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgFuncList.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgGrep.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgInput1.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgJump.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgOpenFile.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgPrintPage.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\cdlgprintsetting.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgProperty.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgReplace.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgTypeList.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDllHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDocLine.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDocLineMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDocLineMgr_New.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditApp.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditDoc.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditDoc_new.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\ceditview.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\ceditview_command.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\ceditview_command_new.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditView_New.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditView_New2.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditView_New3.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEol.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncInfoArr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncKeyWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncLookup.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CHokanMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CHtmlHelp.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CImageListMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CKeyBind.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CKeyMacroMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CKeyWordSetMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CLayout.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CLayoutMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CLayoutMgr_New.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CLayoutMgr_New2.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMacro.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMacroFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMacroManagerBase.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMarkMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMemory.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMenuDrawer.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMRU.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMRUFolder.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CNormalProcess.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\COpe.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\COpeBlk.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\COpeBuf.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPPA.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPPAMacroMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPrint.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPrintPreview.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CProcess.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CProcessFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CProfile.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComBackup.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComCustmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComFile.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComGrep.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComKeybind.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComKeyword.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComMacro.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropCommon.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComToolbar.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComUrl.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropComWin.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropTypesRegex.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CRegexKeyword.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CRunningTimer.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CShareData.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CShareData_new.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CSMacroMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CSplitBoxWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CSplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CTipWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CWaitCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\Debug.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\etc_uty.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\Funccode.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\global.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\my_icmp.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\sakura_rc.rc
# End Source File
# Begin Source File

SOURCE=..\sakura_core\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\sakura_core\WinMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\sakura_core\CAutoSave.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CBregexp.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CBuffer.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDialog.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDicMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgAbout.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgCancel.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgCompare.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgDebug.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgExec.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgFind.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgFuncList.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgGrep.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgInput1.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgJump.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgOpenFile.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgPrintPage.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgPrintSetting.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgProperty.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgReplace.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDlgTypeList.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDllHandler.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDocLine.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDocLineMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CDropTarget.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditApp.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditDoc.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditView.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEditWnd.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CEol.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncInfo.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncInfoArr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncKeyWnd.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CFuncLookup.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\charcode.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CHokanMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CHtmlHelp.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CImageListMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CKeyBind.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CKeyMacroMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CKeyWordSetMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CLayout.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CLayoutMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMacro.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMarkMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMemory.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMenuDrawer.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMRU.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CMRUFolder.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\COpe.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\COpeBlk.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\COpeBuf.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPrint.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPrintPreview.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CProfile.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropCommon.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CPropTypes.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CRegexKeyword.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CRunningTimer.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CShareData.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CSMacroMgr.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CSplitBoxWnd.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CSplitterWnd.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CTipWnd.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CWaitCursor.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\CWnd.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\Debug.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\etc_uty.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\Funccode.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\global.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\Keycode.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\my_icmp.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\mymessage.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\sakura_core.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\sakura_rc.h
# End Source File
# Begin Source File

SOURCE=..\sakura_core\sakura_rc.hm
# End Source File
# Begin Source File

SOURCE=..\sakura_core\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
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

SOURCE=..\resource\icon_moji.ico
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
# Begin Source File

SOURCE=..\sakura_core\sakura.hh
# End Source File
# End Target
# End Project
