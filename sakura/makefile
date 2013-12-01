# Makefile for Sakura Editor
# 	Output - sakura.exe
# 	Compiler - borland C++ free compiler 5.5.1 (with borland make)
# 	Compiler - MS Visual C++ Toolkit 2003 + MS Platform SDK (with nmake)
# 	Option - '-DDEBUG'	'-DTRACE'
#
#	2006.03.25 Directory auto creation (Thanks to Nam san)

!undef BCC

## Directory and files
OUTDIR=..\sakura
SRCDIR=..\sakura_core
TOOLDIR=..\btool

#
#	Compile options
#

!if "$(CC)"=="bcc32"
!MESSAGE Compile for Borland C++
#------------------------------------------
# Definitions for Borland C++
#------------------------------------------
# BCC: Flag for bcc
BCC=1
RC=brcc32.exe
LINK=ilink32.exe

# compatibility option
CC_OBJFILEOPT=-o
CC_EXEFILEOPT=-e

## (BCC) CPP Options
CPPFLAGS=-I$(SRCDIR)  -DWINVER=0x400 -D_WIN32_WINNT=0 -D_WIN32_IE=0x400 -D_MBCS -D__MT__ -tWM
RFLAGS=-dDS_SETFOREGROUND=0x200L -i$(SRCDIR)
WARNINGS=-wamb -wamp -wbbf -wcln -wdef -wncf -wnak -wnod -wsig -wstu -wstv -wucp -wuse

## (BCC) Optimizing Options	// DEBUG,TRACE,RELEASE
!ifdef DEBUG
OPTFLAGS=-Od -v -y -N -D_DEBUG $(WARNINGS)
OBJDIR=.\Debug_bcc
!else
OPTFLAGS=-5 -O1 -Octv -d
OBJDIR=.\Release_bcc
!ifdef TRACE
OPTFLAGS=-v -y $(OPTFLAGS) $(WARNINGS)
!else
OPTFLAGS=$(OPTFLAGS) -DNDEBUG -w-8004  -w-8057
!endif
!endif

## (BCC) LINK Options
LFLAGS=-aa -V4.0 -c -Gn -m -s
STDOBJS=c0w32.obj
STDLIBS=import32.lib cw32mt.lib

!elseif "$(CC)"=="cl"
#------------------------------------------
# Micrsoft Optimizing Compiler
#------------------------------------------
!ifdef DEBUG
OPTFLAGS=/W3 /GR /GX /GS /Od /MTd /D "DEBUG" /D "_DEBUG" /Zi
OBJDIR=.\Debug_one
OPTLIBS=libcmtd.lib libcpmtd.lib
!else
OPTFLAGS=/W3 /GR /GX /O1 /MT /D "NDEBUG" 
OBJDIR=.\Release_one
OPTLIBS=libcmt.lib libcpmt.lib
!endif

RC=rc.exe
CPPFLAGS=/nologo /I "$(SRCDIR)" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /Fp$(OBJDIR)\sakura.pch /Fd$(OBJDIR)\sakura.pdb
RFLAGS=/l 0x411 /d "NDEBUG" /I "$(SRCDIR)"
LFLAGS=/subsystem:windows /machine:x86 /map

# compatibility option
CC_OBJFILEOPT=/Fo
CC_EXEFILEOPT=/Fe

!else
#other compiler
!ERROR Compiler $(CC) is not supported.

!endif

LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib Imm32.lib mpr.lib imagehlp.lib

OBJS =\
	$(OBJDIR)\CAutoSave.obj \
	$(OBJDIR)\CBlockComment.obj \
	$(OBJDIR)\CCommandLine.obj \
	$(OBJDIR)\CControlProcess.obj \
	$(OBJDIR)\CControlTray.obj \
	$(OBJDIR)\CDialog.obj \
	$(OBJDIR)\CDicMgr.obj \
	$(OBJDIR)\CDlgAbout.obj \
	$(OBJDIR)\CDlgCancel.obj \
	$(OBJDIR)\CDlgCompare.obj \
	$(OBJDIR)\CDlgCtrlCode.obj \
	$(OBJDIR)\CDlgDiff.obj \
	$(OBJDIR)\CDlgExec.obj \
	$(OBJDIR)\CDlgFavorite.obj \
	$(OBJDIR)\CDlgFileUpdateQuery.obj \
	$(OBJDIR)\CDlgFind.obj \
	$(OBJDIR)\CDlgFuncList.obj \
	$(OBJDIR)\CDlgGrep.obj \
	$(OBJDIR)\CDlgInput1.obj \
	$(OBJDIR)\CDlgJump.obj \
	$(OBJDIR)\CDlgKeywordSelect.obj \
	$(OBJDIR)\CDlgOpenFile.obj \
	$(OBJDIR)\CDlgPrintSetting.obj \
	$(OBJDIR)\CDlgProperty.obj \
	$(OBJDIR)\CDlgReplace.obj \
	$(OBJDIR)\CDlgSameColor.obj \
	$(OBJDIR)\CDlgTagJumpList.obj \
	$(OBJDIR)\CDlgTagsMake.obj \
	$(OBJDIR)\CDlgTypeList.obj \
	$(OBJDIR)\CDlgWinSize.obj \
	$(OBJDIR)\CDllHandler.obj \
	$(OBJDIR)\CDocLine.obj \
	$(OBJDIR)\CDocLineMgr.obj \
	$(OBJDIR)\CDocLineMgr_New.obj \
	$(OBJDIR)\CDropTarget.obj \
	$(OBJDIR)\CEditApp.obj \
	$(OBJDIR)\CEditDoc.obj \
	$(OBJDIR)\CEditDoc_FuncList1.obj \
	$(OBJDIR)\CEditDoc_FuncList2.obj \
	$(OBJDIR)\CEditDoc_FuncList3.obj \
	$(OBJDIR)\CEditDoc_new.obj \
	$(OBJDIR)\CEditView.obj \
	$(OBJDIR)\CEditView_Cmdgrep.obj \
	$(OBJDIR)\CEditView_CmdHokan.obj \
	$(OBJDIR)\CEditView_Cmdisrch.obj \
	$(OBJDIR)\CEditView_Command.obj \
	$(OBJDIR)\CEditView_Command_New.obj \
	$(OBJDIR)\CEditView_Diff.obj \
	$(OBJDIR)\CEditView_ExecCmd.obj \
	$(OBJDIR)\CEditView_Ime.obj \
	$(OBJDIR)\CEditView_Mouse.obj \
	$(OBJDIR)\CEditView_New.obj \
	$(OBJDIR)\CEditView_New2.obj \
	$(OBJDIR)\CEditWnd.obj \
	$(OBJDIR)\CEol.obj \
	$(OBJDIR)\CESI.obj \
	$(OBJDIR)\CFileExt.obj \
	$(OBJDIR)\CFileWrite.obj \
	$(OBJDIR)\CFileLoad.obj \
	$(OBJDIR)\CFuncInfo.obj \
	$(OBJDIR)\CFuncInfoArr.obj \
	$(OBJDIR)\CFuncKeyWnd.obj \
	$(OBJDIR)\CFuncLookup.obj \
	$(OBJDIR)\charcode.obj \
	$(OBJDIR)\CHokanMgr.obj \
	$(OBJDIR)\CHtmlHelp.obj \
	$(OBJDIR)\CImageListMgr.obj \
	$(OBJDIR)\CKeyBind.obj \
	$(OBJDIR)\CKeyMacroMgr.obj \
	$(OBJDIR)\CKeyWordSetMgr.obj \
	$(OBJDIR)\CLayout.obj \
	$(OBJDIR)\CLayoutMgr.obj \
	$(OBJDIR)\CLayoutMgr_New.obj \
	$(OBJDIR)\CLayoutMgr_New2.obj \
	$(OBJDIR)\CLineComment.obj \
	$(OBJDIR)\CMacro.obj \
	$(OBJDIR)\CMacroFactory.obj \
	$(OBJDIR)\CMacroManagerBase.obj \
	$(OBJDIR)\CMarkMgr.obj \
	$(OBJDIR)\CMemory.obj \
	$(OBJDIR)\CMenuDrawer.obj \
	$(OBJDIR)\CMigemo.obj \
	$(OBJDIR)\CMRUFile.obj \
	$(OBJDIR)\CMRUFolder.obj \
	$(OBJDIR)\CNormalProcess.obj \
	$(OBJDIR)\COpe.obj \
	$(OBJDIR)\COpeBlk.obj \
	$(OBJDIR)\COpeBuf.obj \
	$(OBJDIR)\COsVersionInfo.obj \
	$(OBJDIR)\CPPA.obj \
	$(OBJDIR)\CPPAMacroMgr.obj \
	$(OBJDIR)\CPrint.obj \
	$(OBJDIR)\CPrintPreview.obj \
	$(OBJDIR)\CProcess.obj \
	$(OBJDIR)\CProcessFactory.obj \
	$(OBJDIR)\CProfile.obj \
	$(OBJDIR)\CPropComBackup.obj \
	$(OBJDIR)\CPropComCustmenu.obj \
	$(OBJDIR)\CPropComEdit.obj \
	$(OBJDIR)\CPropComFile.obj \
	$(OBJDIR)\CPropComFileName.obj \
	$(OBJDIR)\CPropComFormat.obj \
	$(OBJDIR)\CPropComGeneral.obj \
	$(OBJDIR)\CPropComGrep.obj \
	$(OBJDIR)\CPropComHelper.obj \
	$(OBJDIR)\CPropComKeybind.obj \
	$(OBJDIR)\CPropComKeyword.obj \
	$(OBJDIR)\CPropCommon.obj \
	$(OBJDIR)\CPropComMacro.obj \
	$(OBJDIR)\CPropComTab.obj \
	$(OBJDIR)\CPropComToolbar.obj \
	$(OBJDIR)\CPropComWin.obj \
	$(OBJDIR)\CPropertyManager.obj \
	$(OBJDIR)\CPropTypes.obj \
	$(OBJDIR)\CPropTypesColor.obj \
	$(OBJDIR)\CPropTypesKeyHelp.obj \
	$(OBJDIR)\CPropTypesRegex.obj \
	$(OBJDIR)\CPropTypesScreen.obj \
	$(OBJDIR)\CPropTypesSupport.obj \
	$(OBJDIR)\CRunningTimer.obj \
	$(OBJDIR)\CRecent.obj \
	$(OBJDIR)\CRegexKeyword.obj \
	$(OBJDIR)\CShareData_new.obj \
	$(OBJDIR)\CShareData.obj \
	$(OBJDIR)\CSMacroMgr.obj \
	$(OBJDIR)\CSortedTagJumpList.obj \
	$(OBJDIR)\CSplitBoxWnd.obj \
	$(OBJDIR)\CSplitterWnd.obj \
	$(OBJDIR)\CTabWnd.obj \
	$(OBJDIR)\CTipWnd.obj \
	$(OBJDIR)\CUxTheme.obj \
	$(OBJDIR)\CViewFont.obj \
	$(OBJDIR)\CWnd.obj \
	$(OBJDIR)\CWaitCursor.obj \
	$(OBJDIR)\CWSH.obj \
	$(OBJDIR)\CWSHManager.obj \
	$(OBJDIR)\CBregexp.obj \
	$(OBJDIR)\Debug.obj \
	$(OBJDIR)\etc_uty.obj \
	$(OBJDIR)\File.obj \
	$(OBJDIR)\Funccode.obj \
	$(OBJDIR)\global.obj \
	$(OBJDIR)\my_icmp.obj \
	$(OBJDIR)\os.obj \
	$(OBJDIR)\StdAfx.obj \
	$(OBJDIR)\WinMain.obj \

## Targets
!ifdef BCC
all : tool tall

## Tool for Borland C++
tool : "$(OBJDIR)" $(OBJDIR)\mrc2brc.exe

$(OBJDIR)\mrc2brc.exe : $(OBJDIR)\mrc2brc.obj $(OBJDIR)\RcCvt.obj
	$(CC) $(CPPFLAGS) -M $(OPTFLAGS) -WC -e$@ $**

$(OBJDIR)\sakurabc.rc : $(SRCDIR)\sakura_rc.rc
	$(OBJDIR)\mrc2brc	$** $@ > $(OBJDIR)\mrc2brc.log

# TEMPRC: input file for resource compiler
TEMPRC=$(OBJDIR)\sakurabc.rc

# bcc32 needs response file due to limitation of commandline length
# bcc32 doesn't accept resource file


!else
all : tall

TEMPRC=$(SRCDIR)\sakura_rc.rc
!endif


## Editor includes sakura_core
tall : "$(OBJDIR)" $(OBJDIR)\sakura.exe

!ifdef BCC
$(OBJDIR)\sakurabc.res : $(TEMPRC)
	$(RC) $(RFLAGS) $** $@

$(OBJDIR)\sakura.exe : $(OBJDIR)\sakurabc.res $(OBJS)
	$(CC) $(CPPFLAGS) -c $(OPTFLAGS) @&&%
$(OBJS)
%
	$(LINK) @&&%
$(LFLAGS) $(STDOBJS) $(OBJS),$@,$(@:exe=map),$(STDLIBS),,$(OBJDIR)\sakurabc.res
%

!else
$(OBJDIR)\sakura_rc.res : $(TEMPRC)
	$(RC) $(RFLAGS) /fo$@ $** 

$(OBJDIR)\sakura.exe : $(OBJDIR)\sakura_rc.res $(OBJS)
	$(CC) @<<
$(CPPFLAGS) $(OPTFLAGS) $(CC_EXEFILEOPT)$@ $** /link $(LFLAGS) $(LIBS) $(OPTLIBS)
<<
!endif

"$(OBJDIR)":
    if not exist "$(OBJDIR)/" mkdir "$(OBJDIR)"

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj :
	$(CC) $(CPPFLAGS) $(OPTFLAGS) $(CC_OBJFILEOPT)$(OBJDIR)\$(@F) -c $<

{$(SRCDIR)}.c{$(OBJDIR)}.obj :
	$(CC) $(CPPFLAGS) $(OPTFLAGS) $(CC_OBJFILEOPT)$(OBJDIR)\$(@F) -c $<

{$(TOOLDIR)}.cpp{$(OBJDIR)}.obj :
	$(CC) $(CPPFLAGS) $(OPTFLAGS) $(CC_OBJFILEOPT)$(OBJDIR)\$(@F) -c $<

cleanall : clean
	-erase $(OBJDIR)\mrc2brc.exe
	-erase $(OBJDIR)\mrc2brc.map
	-erase $(OBJDIR)\mrc2brc.tds
	-erase $(OBJDIR)\mrc2brc.log

clean : clobber
	-erase $(OBJDIR)\*.exe

clobber :
	-erase $(OBJDIR)\*.res
	-erase $(OBJDIR)\sakurabc.rc
	-erase $(OBJDIR)\*.obj
	-erase $(OBJDIR)\*.map
	-erase $(OBJDIR)\*.tds
	-erase $(OBJDIR)\*.pch
	-erase $(OBJDIR)\*.idb
	-erase $(OBJDIR)\*.pdb
	-erase $(OBJDIR)\*.ilk

#test : .\aXX
#	@echo "*$* @@$@@ @$@ **$** ?$? @D$(@D) @B$(@B) "
#
#.\aXX : 
#	@echo "aaa"
