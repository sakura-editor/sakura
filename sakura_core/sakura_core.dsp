# Microsoft Developer Studio Project File - Name="sakura_core" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sakura_core - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "sakura_core.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "sakura_core.mak" CFG="sakura_core - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "sakura_core - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "sakura_core - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "sakura_core"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sakura_core - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../sakura/Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SAKURA_CORE_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SAKURA_CORE_EXPORTS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ole32.lib comctl32.lib Imm32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "sakura_core - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../sakura/Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SAKURA_CORE_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SAKURA_CORE_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ole32.lib comctl32.lib Imm32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sakura_core - Win32 Release"
# Name "sakura_core - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CAutoSave.cpp
# End Source File
# Begin Source File

SOURCE=.\CBregexp.cpp
# End Source File
# Begin Source File

SOURCE=.\CCommandLine.cpp
# End Source File
# Begin Source File

SOURCE=.\CControlProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\CDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CDicMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgAbout.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgCancel.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgCompare.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgExec.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgFind.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgFuncList.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgGrep.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgInput1.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgJump.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgOpenFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgPrintPage.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgPrintSetting.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgProperty.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgReplace.cpp
# End Source File
# Begin Source File

SOURCE=.\CDlgTypeList.cpp
# End Source File
# Begin Source File

SOURCE=.\CDllHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\CDocLine.cpp
# End Source File
# Begin Source File

SOURCE=.\CDocLineMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CDocLineMgr_New.cpp
# End Source File
# Begin Source File

SOURCE=.\CDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditApp.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditDoc_new.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditView_Command.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditView_Command_New.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditView_New.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditView_New2.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditView_New3.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CEol.cpp
# End Source File
# Begin Source File

SOURCE=.\CFuncInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\CFuncInfoArr.cpp
# End Source File
# Begin Source File

SOURCE=.\CFuncKeyWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CFuncLookup.cpp
# End Source File
# Begin Source File

SOURCE=.\CHokanMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CHtmlHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\CImageListMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CKeyBind.cpp
# End Source File
# Begin Source File

SOURCE=.\CKeyMacroMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CKeyWordSetMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\CLayoutMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CLayoutMgr_New.cpp
# End Source File
# Begin Source File

SOURCE=.\CLayoutMgr_New2.cpp
# End Source File
# Begin Source File

SOURCE=.\CMacro.cpp
# End Source File
# Begin Source File

SOURCE=.\CMarkMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\CMenuDrawer.cpp
# End Source File
# Begin Source File

SOURCE=.\CMRU.cpp
# End Source File
# Begin Source File

SOURCE=.\CMRUFolder.cpp
# End Source File
# Begin Source File

SOURCE=.\CNormalProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\COpe.cpp
# End Source File
# Begin Source File

SOURCE=.\COpeBlk.cpp
# End Source File
# Begin Source File

SOURCE=.\COpeBuf.cpp
# End Source File
# Begin Source File

SOURCE=.\CPrint.cpp
# End Source File
# Begin Source File

SOURCE=.\CPrintPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\CProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\CProcessFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\CProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComBackup.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComCustmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComGrep.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComKeybind.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComKeyword.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComMacro.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComToolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComUrl.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropComWin.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\CPropTypesRegex.cpp
# End Source File
# Begin Source File

SOURCE=.\CRegexKeyword.cpp
# End Source File
# Begin Source File

SOURCE=.\CRunningTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\CShareData.cpp
# End Source File
# Begin Source File

SOURCE=.\CShareData_new.cpp
# End Source File
# Begin Source File

SOURCE=.\CShareData_new2.cpp
# End Source File
# Begin Source File

SOURCE=.\CSMacroMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CSplitBoxWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CSplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CTipWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CWaitCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\CWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\etc_uty.cpp
# End Source File
# Begin Source File

SOURCE=.\Funccode.cpp
# End Source File
# Begin Source File

SOURCE=.\my_icmp.cpp
# End Source File
# Begin Source File

SOURCE=.\sakura_core.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\sakura_core\WinMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CAutoSave.h
# End Source File
# Begin Source File

SOURCE=.\CBregexp.h
# End Source File
# Begin Source File

SOURCE=.\CDialog.h
# End Source File
# Begin Source File

SOURCE=.\CDicMgr.h
# End Source File
# Begin Source File

SOURCE=.\CDlgAbout.h
# End Source File
# Begin Source File

SOURCE=.\CDlgCancel.h
# End Source File
# Begin Source File

SOURCE=.\CDlgCompare.h
# End Source File
# Begin Source File

SOURCE=.\CDlgDebug.h
# End Source File
# Begin Source File

SOURCE=.\CDlgExec.h
# End Source File
# Begin Source File

SOURCE=.\CDlgFind.h
# End Source File
# Begin Source File

SOURCE=.\CDlgFuncList.h
# End Source File
# Begin Source File

SOURCE=.\CDlgGrep.h
# End Source File
# Begin Source File

SOURCE=.\CDlgInput1.h
# End Source File
# Begin Source File

SOURCE=.\CDlgJump.h
# End Source File
# Begin Source File

SOURCE=.\CDlgOpenFile.h
# End Source File
# Begin Source File

SOURCE=.\CDlgPrintPage.h
# End Source File
# Begin Source File

SOURCE=.\CDlgPrintSetting.h
# End Source File
# Begin Source File

SOURCE=.\CDlgProperty.h
# End Source File
# Begin Source File

SOURCE=.\CDlgReplace.h
# End Source File
# Begin Source File

SOURCE=.\CDlgTypeList.h
# End Source File
# Begin Source File

SOURCE=.\CDllHandler.h
# End Source File
# Begin Source File

SOURCE=.\CDocLine.h
# End Source File
# Begin Source File

SOURCE=.\CDocLineMgr.h
# End Source File
# Begin Source File

SOURCE=.\CDropTarget.h
# End Source File
# Begin Source File

SOURCE=.\CEditApp.h
# End Source File
# Begin Source File

SOURCE=.\CEditDoc.h
# End Source File
# Begin Source File

SOURCE=.\CEditView.h
# End Source File
# Begin Source File

SOURCE=.\CEditWnd.h
# End Source File
# Begin Source File

SOURCE=.\CEol.h
# End Source File
# Begin Source File

SOURCE=.\CFuncInfo.h
# End Source File
# Begin Source File

SOURCE=.\CFuncInfoArr.h
# End Source File
# Begin Source File

SOURCE=.\CFuncKeyWnd.h
# End Source File
# Begin Source File

SOURCE=.\CFuncLookup.h
# End Source File
# Begin Source File

SOURCE=.\charcode.h
# End Source File
# Begin Source File

SOURCE=.\CHokanMgr.h
# End Source File
# Begin Source File

SOURCE=.\CHtmlHelp.h
# End Source File
# Begin Source File

SOURCE=.\CImageListMgr.h
# End Source File
# Begin Source File

SOURCE=.\CKeyBind.h
# End Source File
# Begin Source File

SOURCE=.\CKeyMacroMgr.h
# End Source File
# Begin Source File

SOURCE=.\CKeyWordSetMgr.h
# End Source File
# Begin Source File

SOURCE=.\CLayout.h
# End Source File
# Begin Source File

SOURCE=.\CLayoutMgr.h
# End Source File
# Begin Source File

SOURCE=.\CMacro.h
# End Source File
# Begin Source File

SOURCE=.\CMarkMgr.h
# End Source File
# Begin Source File

SOURCE=.\CMemory.h
# End Source File
# Begin Source File

SOURCE=.\CMenuDrawer.h
# End Source File
# Begin Source File

SOURCE=.\CMRU.h
# End Source File
# Begin Source File

SOURCE=.\CMRUFolder.h
# End Source File
# Begin Source File

SOURCE=.\COpe.h
# End Source File
# Begin Source File

SOURCE=.\COpeBlk.h
# End Source File
# Begin Source File

SOURCE=.\COpeBuf.h
# End Source File
# Begin Source File

SOURCE=.\CPrint.h
# End Source File
# Begin Source File

SOURCE=.\CProfile.h
# End Source File
# Begin Source File

SOURCE=.\CPropCommon.h
# End Source File
# Begin Source File

SOURCE=.\CPropTypes.h
# End Source File
# Begin Source File

SOURCE=.\CRegexKeyword.h
# End Source File
# Begin Source File

SOURCE=.\CRunningTimer.h
# End Source File
# Begin Source File

SOURCE=.\CShareData.h
# End Source File
# Begin Source File

SOURCE=.\CSMacroMgr.h
# End Source File
# Begin Source File

SOURCE=.\CSplitBoxWnd.h
# End Source File
# Begin Source File

SOURCE=.\CSplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\CTipWnd.h
# End Source File
# Begin Source File

SOURCE=.\CWaitCursor.h
# End Source File
# Begin Source File

SOURCE=.\CWnd.h
# End Source File
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\etc_uty.h
# End Source File
# Begin Source File

SOURCE=.\Funccode.h
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\Keycode.h
# End Source File
# Begin Source File

SOURCE=.\my_icmp.h
# End Source File
# Begin Source File

SOURCE=.\mymessage.h
# End Source File
# Begin Source File

SOURCE=.\sakura_core.h
# End Source File
# Begin Source File

SOURCE=.\sakura_rc.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\VC98\Lib\MPR.LIB"
# End Source File
# End Target
# End Project
