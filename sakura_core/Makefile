# Makefile for MinGW32/MinGW-W64

ifndef PREFIX
PREFIX=
RCPREFIX=
else
ifeq ($(PREFIX),x86_64-w64-mingw32-)
RCPREFIX=$(PREFIX)
else
ifeq ($(PREFIX),i686-w64-mingw32-)
ifeq ($(OS),Windows_NT)
RCPREFIX=
else
RCPREFIX=$(PREFIX)
endif
endif
endif
endif

CC= $(PREFIX)gcc
CXX= $(PREFIX)g++
RC= $(RCPREFIX)windres

DEFINES= \
 -DWIN32 \
 -DWINVER=0x500 \
 -D_WIN32_WINNT=0x500 \
 -D_WIN32_IE=0x501 \
 -DMINGW_HAS_SECURE_API=1 \
 -DNOMINMAX \
 -D_UNICODE \
 -DUNICODE \
 -DNDEBUG
CFLAGS= -O2 \
 -finput-charset=utf-8 -fexec-charset=cp932 \
 -I. \
 $(DEFINES) $(MYCFLAGS)
CXXFLAGS= $(CFLAGS) $(MYCXXFLAGS)
LIBS= \
 -lkernel32 -luser32 -lgdi32 \
 -lwinspool -lcomdlg32 -ladvapi32 \
 -lshell32 -lole32 -loleaut32 \
 -luuid -lcomctl32 -limm32 \
 -lmpr -limagehlp -lshlwapi \
 -static-libgcc -static-libstdc++ -mwindows -s

exe= sakura.exe

# ls *.cpp */*.cpp */*/*.cpp | sed -E -e "s/([[:alnum:]_]+)\.[[:alnum:]]+/\1.o \\\\/"
OBJS= \
CAutoReloadAgent.o \
CAutoSaveAgent.o \
CBackupAgent.o \
CCodeChecker.o \
CDataProfile.o \
CDicMgr.o \
CEditApp.o \
CEol.o \
CFileExt.o \
CGrepAgent.o \
CHokanMgr.o \
CKeyWordSetMgr.o \
CLoadAgent.o \
CMarkMgr.o \
COpe.o \
COpeBlk.o \
COpeBuf.o \
CProfile.o \
CPropertyManager.o \
CReadManager.o \
CRegexKeyword.o \
CSaveAgent.o \
CSearchAgent.o \
CSelectLang.o \
CSortedTagJumpList.o \
CWriteManager.o \
StdAfx.o \
apiwrap/StdApi.o \
apiwrap/StdControl.o \
basis/CLaxInteger.o \
basis/CMyPoint.o \
basis/CMyRect.o \
basis/CMySize.o \
basis/CMyString.o \
basis/CStrictInteger.o \
basis/CStrictPoint.o \
basis/CStrictRange.o \
basis/CStrictRect.o \
basis/SakuraBasis.o \
charset/CCesu8.o \
charset/CCodeBase.o \
charset/CCodeFactory.o \
charset/CCodeMediator.o \
charset/CCodePage.o \
charset/CESI.o \
charset/CEuc.o \
charset/charcode.o \
charset/charset.o \
charset/CJis.o \
charset/CLatin1.o \
charset/codechecker.o \
charset/codeutil.o \
charset/CShiftJis.o \
charset/CUnicode.o \
charset/CUnicodeBe.o \
charset/CUtf7.o \
charset/CUtf8.o \
cmd/CViewCommander.o \
cmd/CViewCommander_Bookmark.o \
cmd/CViewCommander_Clipboard.o \
cmd/CViewCommander_Convert.o \
cmd/CViewCommander_Cursor.o \
cmd/CViewCommander_CustMenu.o \
cmd/CViewCommander_Diff.o \
cmd/CViewCommander_Edit.o \
cmd/CViewCommander_Edit_advanced.o \
cmd/CViewCommander_Edit_word_line.o \
cmd/CViewCommander_File.o \
cmd/CViewCommander_Grep.o \
cmd/CViewCommander_Insert.o \
cmd/CViewCommander_Macro.o \
cmd/CViewCommander_ModeChange.o \
cmd/CViewCommander_Outline.o \
cmd/CViewCommander_Search.o \
cmd/CViewCommander_Select.o \
cmd/CViewCommander_Settings.o \
cmd/CViewCommander_Support.o \
cmd/CViewCommander_TagJump.o \
cmd/CViewCommander_Window.o \
config/build_config.o \
convert/CConvert.o \
convert/CConvert_HaneisuToZeneisu.o \
convert/CConvert_HankataToZenhira.o \
convert/CConvert_HankataToZenkata.o \
convert/CConvert_SpaceToTab.o \
convert/CConvert_TabToSpace.o \
convert/CConvert_ToHankaku.o \
convert/CConvert_ToLower.o \
convert/CConvert_ToUpper.o \
convert/CConvert_ToZenhira.o \
convert/CConvert_ToZenkata.o \
convert/CConvert_Trim.o \
convert/CConvert_ZeneisuToHaneisu.o \
convert/CConvert_ZenkataToHankata.o \
convert/CDecode_Base64Decode.o \
convert/CDecode_UuDecode.o \
convert/convert_util.o \
convert/convert_util2.o \
debug/CRunningTimer.o \
debug/Debug1.o \
debug/Debug2.o \
debug/Debug3.o \
dlg/CDialog.o \
dlg/CDlgAbout.o \
dlg/CDlgCancel.o \
dlg/CDlgCompare.o \
dlg/CDlgCtrlCode.o \
dlg/CDlgDiff.o \
dlg/CDlgExec.o \
dlg/CDlgFavorite.o \
dlg/CDlgFileUpdateQuery.o \
dlg/CDlgFind.o \
dlg/CDlgGrep.o \
dlg/CDlgGrepReplace.o \
dlg/CDlgInput1.o \
dlg/CDlgJump.o \
dlg/CDlgOpenFile.o \
dlg/CDlgPluginOption.o \
dlg/CDlgPrintSetting.o \
dlg/CDlgProfileMgr.o \
dlg/CDlgProperty.o \
dlg/CDlgReplace.o \
dlg/CDlgSetCharSet.o \
dlg/CDlgTagJumpList.o \
dlg/CDlgTagsMake.o \
dlg/CDlgWindowList.o \
dlg/CDlgWinSize.o \
doc/CBlockComment.o \
doc/CDocEditor.o \
doc/CDocFile.o \
doc/CDocFileOperation.o \
doc/CDocListener.o \
doc/CDocLocker.o \
doc/CDocOutline.o \
doc/CDocReader.o \
doc/CDocType.o \
doc/CDocTypeSetting.o \
doc/CDocVisitor.o \
doc/CEditDoc.o \
doc/CLineComment.o \
doc/layout/CLayout.o \
doc/layout/CLayoutMgr.o \
doc/layout/CLayoutMgr_DoLayout.o \
doc/layout/CLayoutMgr_New.o \
doc/layout/CLayoutMgr_New2.o \
doc/layout/CTsvModeInfo.o \
doc/logic/CDocLine.o \
doc/logic/CDocLineMgr.o \
docplus/CBookmarkManager.o \
docplus/CDiffManager.o \
docplus/CFuncListManager.o \
docplus/CModifyManager.o \
env/CAppNodeManager.o \
env/CDocTypeManager.o \
env/CFileNameManager.o \
env/CFormatManager.o \
env/CHelpManager.o \
env/CommonSetting.o \
env/CSakuraEnvironment.o \
env/CSearchKeywordManager.o \
env/CShareData.o \
env/CShareData_IO.o \
env/CTagJumpManager.o \
env/DLLSHAREDATA.o \
extmodule/CBregexp.o \
extmodule/CBregexpDll2.o \
extmodule/CDllHandler.o \
extmodule/CHtmlHelp.o \
extmodule/CMigemo.o \
extmodule/CUxTheme.o \
func/CFuncKeyWnd.o \
func/CFuncLookup.o \
func/CKeyBind.o \
func/Funccode.o \
io/CBinaryStream.o \
io/CFile.o \
io/CFileLoad.o \
io/CIoBridge.o \
io/CStream.o \
io/CTextStream.o \
io/CZipFile.o \
macro/CCookieManager.o \
macro/CEditorIfObj.o \
macro/CIfObj.o \
macro/CKeyMacroMgr.o \
macro/CMacro.o \
macro/CMacroFactory.o \
macro/CMacroManagerBase.o \
macro/CPPA.o \
macro/CPPAMacroMgr.o \
macro/CSMacroMgr.o \
macro/CWSH.o \
macro/CWSHIfObj.o \
macro/CWSHManager.o \
mem/CMemory.o \
mem/CNative.o \
mem/CNativeA.o \
mem/CNativeW.o \
mem/CRecycledBuffer.o \
mfclike/CMyWnd.o \
outline/CDlgFileTree.o \
outline/CDlgFuncList.o \
outline/CFuncInfo.o \
outline/CFuncInfoArr.o \
parse/CWordParse.o \
plugin/CDllPlugin.o \
plugin/CJackManager.o \
plugin/CPlugin.o \
plugin/CPluginManager.o \
plugin/CWSHPlugin.o \
print/CPrint.o \
print/CPrintPreview.o \
prop/CPropComBackup.o \
prop/CPropComCustmenu.o \
prop/CPropComEdit.o \
prop/CPropComFile.o \
prop/CPropComFileName.o \
prop/CPropComFormat.o \
prop/CPropComGeneral.o \
prop/CPropComGrep.o \
prop/CPropComHelper.o \
prop/CPropComKeybind.o \
prop/CPropComKeyword.o \
prop/CPropComMacro.o \
prop/CPropComMainMenu.o \
prop/CPropCommon.o \
prop/CPropComPlugin.o \
prop/CPropComStatusbar.o \
prop/CPropComTab.o \
prop/CPropComToolbar.o \
prop/CPropComWin.o \
recent/CMRUFile.o \
recent/CMRUFolder.o \
recent/CMruListener.o \
recent/CRecent.o \
recent/CRecentCmd.o \
recent/CRecentCurDir.o \
recent/CRecentEditNode.o \
recent/CRecentExceptMru.o \
recent/CRecentFile.o \
recent/CRecentFolder.o \
recent/CRecentGrepFile.o \
recent/CRecentGrepFolder.o \
recent/CRecentImp.o \
recent/CRecentReplace.o \
recent/CRecentSearch.o \
recent/CRecentTagjumpKeyword.o \
typeprop/CDlgKeywordSelect.o \
typeprop/CDlgSameColor.o \
typeprop/CDlgTypeAscertain.o \
typeprop/CDlgTypeList.o \
typeprop/CImpExpManager.o \
typeprop/CPropTypes.o \
typeprop/CPropTypesColor.o \
typeprop/CPropTypesKeyHelp.o \
typeprop/CPropTypesRegex.o \
typeprop/CPropTypesScreen.o \
typeprop/CPropTypesSupport.o \
typeprop/CPropTypesWindow.o \
types/CType.o \
types/CTypeSupport.o \
types/CType_Asm.o \
types/CType_Awk.o \
types/CType_Basis.o \
types/CType_Cobol.o \
types/CType_CorbaIdl.o \
types/CType_Cpp.o \
types/CType_Dos.o \
types/CType_Erlang.o \
types/CType_Html.o \
types/CType_Ini.o \
types/CType_Java.o \
types/CType_Others.o \
types/CType_Pascal.o \
types/CType_Perl.o \
types/CType_Python.o \
types/CType_Rich.o \
types/CType_Sql.o \
types/CType_Tex.o \
types/CType_Text.o \
types/CType_Vb.o \
uiparts/CGraphics.o \
uiparts/CImageListMgr.o \
uiparts/CMenuDrawer.o \
uiparts/CSoundSet.o \
uiparts/CVisualProgress.o \
uiparts/CWaitCursor.o \
util/file.o \
util/format.o \
util/input.o \
util/MessageBoxF.o \
util/module.o \
util/ole_convert.o \
util/os.o \
util/other_util.o \
util/relation_tool.o \
util/shell.o \
util/string_ex.o \
util/string_ex2.o \
util/tchar_convert.o \
util/tchar_printf.o \
util/tchar_receive.o \
util/tchar_template.o \
util/window.o \
view/CCaret.o \
view/CEditView.o \
view/CEditView_Cmdgrep.o \
view/CEditView_CmdHokan.o \
view/CEditView_Cmdisrch.o \
view/CEditView_Command.o \
view/CEditView_Command_New.o \
view/CEditView_Diff.o \
view/CEditView_ExecCmd.o \
view/CEditView_Ime.o \
view/CEditView_Mouse.o \
view/CEditView_Paint.o \
view/CEditView_Paint_Bracket.o \
view/CEditView_Scroll.o \
view/CEditView_Search.o \
view/CRuler.o \
view/CTextArea.o \
view/CTextDrawer.o \
view/CTextMetrics.o \
view/CViewCalc.o \
view/CViewFont.o \
view/CViewParser.o \
view/CViewSelect.o \
view/DispPos.o \
view/colors/CColorStrategy.o \
view/colors/CColor_Comment.o \
view/colors/CColor_Found.o \
view/colors/CColor_Heredoc.o \
view/colors/CColor_KeywordSet.o \
view/colors/CColor_Numeric.o \
view/colors/CColor_Quote.o \
view/colors/CColor_RegexKeyword.o \
view/colors/CColor_Url.o \
view/figures/CFigureManager.o \
view/figures/CFigureStrategy.o \
view/figures/CFigure_Comma.o \
view/figures/CFigure_CtrlCode.o \
view/figures/CFigure_Eol.o \
view/figures/CFigure_HanSpace.o \
view/figures/CFigure_Tab.o \
view/figures/CFigure_ZenSpace.o \
window/CAutoScrollWnd.o \
window/CEditWnd.o \
window/CMainStatusBar.o \
window/CMainToolBar.o \
window/CSplitBoxWnd.o \
window/CSplitterWnd.o \
window/CTabWnd.o \
window/CTipWnd.o \
window/CWnd.o \
_main/CAppMode.o \
_main/CCommandLine.o \
_main/CControlProcess.o \
_main/CControlTray.o \
_main/CNormalProcess.o \
_main/CProcess.o \
_main/CProcessFactory.o \
_main/global.o \
_main/WinMain.o \
_os/CClipboard.o \
_os/CDropTarget.o \
_os/COsVersionInfo.o \
sakura_rc.o \

RCTOOLDIR=../btool
RCTOOL=$(RCTOOLDIR)/mrc2grc.exe
HEADERMAKETOOLDIR= ../HeaderMake
HEADERMAKE= $(HEADERMAKETOOLDIR)/HeaderMake.exe

all: $(exe)

$(exe): githash $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

Funccode_define.h: $(HEADERMAKE) Funccode_x.hsrc
	$(HEADERMAKE) -in=../sakura_core/Funccode_x.hsrc -out=../sakura_core/Funccode_define.h -mode=define

Funccode_enum.h: $(HEADERMAKE) Funccode_x.hsrc
	$(HEADERMAKE) -in=../sakura_core/Funccode_x.hsrc -out=../sakura_core/Funccode_enum.h -mode=enum -enum=EFunctionCode

githash:
	cmd /c $(CURDIR)/../sakura/githash.bat ../sakura_core

stdafx: Funccode_enum.h StdAfx.h
	$(CXX) $(CXXFLAGS) -c StdAfx.h

.cpp.o: stdafx
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(HEADERMAKE): $(HEADERMAKETOOLDIR)/HeaderMake.cpp
	$(CXX) $(CXXFLAGS) $(HEADERMAKETOOLDIR)/HeaderMake.cpp -o $@ -static-libgcc

sakura_rc.o: Funccode_define.h githash sakura_rc.rc
	$(RC) -c utf-8 --language=0411 $(DEFINES) sakura_rc.rc -o $@

clean:
	$(RM) $(exe) $(OBJS) $(HEADERMAKE) StdAfx.h.gch

depend: githash
	$(CXX) -E -MM -w $(DEFINES) $(CXXFLAGS) *.cpp */*.cpp */*/*.cpp > depend.mak

.SUFFIXES: .cpp .o .rc
.PHONY: all clean depend

-include depend.mak
