/*!	@file
	@brief CEditViewクラスのコマンド処理系関数群

	@author Norio Nakatani
	@date	1998/07/17 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, みつ
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, すなふき, Moca, ai
	Copyright (C) 2003, MIK, genta, かろと, zenryaku, Moca, ryoji, naoh, KEITA, じゅうじ
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, みちばな, Kazika
	Copyright (C) 2005, genta, novice, かろと, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, かろと, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
// 2007.10.25 kobake CViewCommanderクラスに分離

#include "StdAfx.h"
#include <htmlhelp.h>
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "debug/CRunningTimer.h"
#include "_main/CControlTray.h"
#include "_main/CAppMode.h"
#include "CWaitCursor.h"
//@@@ 2002.2.2 YAZAKI マクロはCSMacroMgrに統一
#include "macro/CSMacroMgr.h"
#include "typeprop/CDlgTypeList.h"
#include "dlg/CDlgProperty.h"
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "dlg/CDlgExec.h"
#include "dlg/CDlgAbout.h"	//Dec. 24, 2000 JEPRO 追加
#include "dlg/CDlgTagJumpList.h"
#include "dlg/CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "doc/CDocReader.h"	//  Command_PROPERTY_FILE for _DEBUG
#include "doc/CDocLine.h"/// 2002/2/3 aroka 追加
#include "COpeBlk.h"/// 2002/2/3 aroka 追加
#include "window/CEditWnd.h"/// 2002/2/3 aroka 追加
#include "outline/CFuncInfoArr.h"
#include "CMarkMgr.h"/// 2002/2/3 aroka 追加
#include "CPrintPreview.h"
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"
#include "io/CBinaryStream.h"
#include "CEditApp.h"
#include "recent/CMRU.h"
#include "util/window.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/os.h"
#include "charset/CCodeFactory.h"
#include "io/CFileLoad.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
#include "plugin/CJackManager.h"
//#include "plugin/COutlineIfObj.h"
#include "plugin/CSmartIndentIfObj.h"
#include "CPropertyManager.h"
#include "CGrepAgent.h"
#include "CWriteManager.h"
#include "sakura_rc.h"

//外部依存
CEditDoc* CViewCommander::GetDocument()
{
	return m_pCommanderView->m_pcEditDoc;
}
CEditWnd* CViewCommander::GetEditWindow()
{
	return m_pCommanderView->m_pcEditWnd;
}
HWND CViewCommander::GetMainWindow()
{
	return ::GetParent( m_pCommanderView->m_hwndParent );
}
COpeBlk* CViewCommander::GetOpeBlk()
{
	return m_pCommanderView->m_pcOpeBlk;
}
void CViewCommander::ClearOpeBlk()
{
	delete m_pCommanderView->m_pcOpeBlk;
	m_pCommanderView->m_pcOpeBlk=NULL;
}
void CViewCommander::SetOpeBlk(COpeBlk* p)
{
	m_pCommanderView->m_pcOpeBlk = p;
}
CLayoutRange& CViewCommander::GetSelect()
{
	return m_pCommanderView->GetSelectionInfo().m_sSelect;
}
CCaret& CViewCommander::GetCaret()
{
	return m_pCommanderView->GetCaret();
}



/* コマンドコードによる処理振り分け */
BOOL CViewCommander::HandleCommand(
	EFunctionCode	nCommand,
	bool			bRedraw,
	LPARAM			lparam1,
	LPARAM			lparam2,
	LPARAM			lparam3,
	LPARAM			lparam4
)
{
	BOOL	bRet = TRUE;
	bool	bRepeat = false;
	int		nFuncID;

	//	May. 19, 2006 genta 上位16bitに送信元の識別子が入るように変更したので
	//	下位16ビットのみを取り出す
	//	Jul.  7, 2007 genta 定数と比較するためにシフトしないで使う
	int nCommandFrom = nCommand & ~0xffff;
	nCommand = (EFunctionCode)LOWORD( nCommand );


	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call message translators
	// -------------------------------------
	m_pCommanderView->TranslateCommand_grep( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );
	m_pCommanderView->TranslateCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );

	//	Aug, 14. 2000 genta
	if( GetDocument()->IsModificationForbidden( nCommand ) ){
		return TRUE;
	}

	++GetDocument()->m_nCommandExecNum;		/* コマンド実行回数 */
//	if( nCommand != F_COPY ){
		/* 辞書Tipを消す */
		m_pCommanderView->m_cTipWnd.Hide();
		m_pCommanderView->m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
//	}
	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( pCEditWnd->m_pPrintPreview && F_PRINT_PREVIEW != nCommand ){
		ErrorBeep();
		return -1;
	}
	/* キーリピート状態 */
	if( m_bPrevCommand == nCommand ){
		bRepeat = TRUE;
	}
	m_bPrevCommand = nCommand;
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow() &&	/* キーボードマクロを記録中のウィンドウ */
		( nCommandFrom & FA_NONRECORD ) != FA_NONRECORD	/* 2007.07.07 genta 記録抑制フラグ off */
	){
		/* キーリピート状態をなくする */
		bRepeat = FALSE;
		/* キーマクロに記録可能な機能かどうかを調べる */
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		//F_EXECEXTMACROコマンドはファイルを選択した後にマクロ文が確定するため個別に記録する。
		if( CSMacroMgr::CanFuncIsKeyMacro( nCommand ) &&
			nCommand != F_EXECEXTMACRO	//F_EXECEXTMACROは個別で記録します
		){
			/* キーマクロのバッファにデータ追加 */
			//@@@ 2002.1.24 m_CKeyMacroMgrをCEditDocへ移動
			CEditApp::getInstance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparam1, m_pCommanderView );
		}
	}

	//	2007.07.07 genta マクロ実行中フラグの設定
	//	マクロからのコマンドかどうかはnCommandFromでわかるが
	//	nCommandFromを引数で浸透させるのが大変なので，従来のフラグにも値をコピーする
	m_pCommanderView->m_bExecutingKeyMacro = ( nCommandFrom & FA_FROMMACRO ) ? true : false;

	/* キーボードマクロの実行中 */
	if( m_pCommanderView->m_bExecutingKeyMacro ){
		/* キーリピート状態をなくする */
		bRepeat = FALSE;
	}

	//	From Here Sep. 29, 2001 genta マクロの実行機能追加
	if( F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO ){
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一（インターフェースの変更）
		if( !CEditApp::getInstance()->m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, G_AppInstance(), m_pCommanderView,
			nCommandFrom & FA_NONRECORD )){
			InfoMessage(
				this->m_pCommanderView->m_hwndParent,
				_T("マクロ %d (%ts) の実行に失敗しました。"),
				nCommand - F_USERMACRO_0,
				CEditApp::getInstance()->m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
			);
		}
		return TRUE;
	}
	//	To Here Sep. 29, 2001 genta マクロの実行機能追加

	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call mode basis message handler
	// -------------------------------------
	m_pCommanderView->PreprocessCommand_hokan(nCommand);
	if( m_pCommanderView->ProcessCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 ))
		return TRUE;

	// -------------------------------------
	//	Jan. 10, 2005 genta コメント
	//	ここより前ではUndoバッファの準備ができていないので
	//	文書の操作を行ってはいけない
	//@@@ 2002.2.2 YAZAKI HandleCommand内でHandleCommandを呼び出せない問題に対処（何か副作用がある？）
	if( NULL == GetOpeBlk() ){	/* 操作ブロック */
		SetOpeBlk(new COpeBlk);
	}
	GetOpeBlk()->AddRef();	//参照カウンタ増加
	
	//	Jan. 10, 2005 genta コメント
	//	ここより後ではswitchの後ろでUndoを正しく登録するため，
	//	途中で処理の打ち切りを行ってはいけない
	// -------------------------------------

	switch( nCommand ){
	case F_WCHAR:	/* 文字入力 */
		{
			Command_WCHAR( (wchar_t)lparam1 );
		}
		break;

	/* ファイル操作系 */
	case F_FILENEW:				Command_FILENEW();break;			/* 新規作成 */
	case F_FILENEW_NEWWINDOW:	Command_FILENEW_NEWWINDOW();break;
	//	Oct. 2, 2001 genta マクロ用機能拡張
	case F_FILEOPEN:			Command_FILEOPEN((const WCHAR*)lparam1);break;			/* ファイルを開く */
	case F_FILEOPEN_DROPDOWN:	Command_FILEOPEN((const WCHAR*)lparam1);break;			/* ファイルを開く(ドロップダウン) */	//@@@ 2002.06.15 MIK
	case F_FILESAVE:			bRet = Command_FILESAVE();break;	/* 上書き保存 */
	case F_FILESAVEAS_DIALOG:	bRet = Command_FILESAVEAS_DIALOG();break;	/* 名前を付けて保存 */
	case F_FILESAVEAS:			bRet = Command_FILESAVEAS((const WCHAR*)lparam1,(EEolType)lparam3);break;	/* 名前を付けて保存 */
	case F_FILESAVEALL:			bRet = Command_FILESAVEALL();break;	/* 全ての編集ウィンドウで上書き保存 */ // Jan. 23, 2005 genta
	case F_FILESAVE_QUIET:		bRet = Command_FILESAVE(false,false); break;	/* 静かに上書き保存 */ // Jan. 24, 2005 genta
	case F_FILESAVECLOSE:
		//	Feb. 28, 2004 genta 保存＆閉じる
		//	保存が不要なら単に閉じる
		{	// Command_FILESAVE()とは別に保存不要をチェック	//### Command_FILESAVE() は実際に保存した場合だけ true を返すようになった（仕様変更？）
			if( !GetDllShareData().m_Common.m_sFile.m_bEnableUnmodifiedOverwrite && !GetDocument()->m_cDocEditor.IsModified() ){
				Command_WINCLOSE();
				break;
			}
		}
		if( Command_FILESAVE( false, true )){
			Command_WINCLOSE();
		}
		break;
	case F_FILECLOSE:										//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
		Command_FILECLOSE();
		break;
	case F_FILECLOSE_OPEN:	/* 閉じて開く */
		Command_FILECLOSE_OPEN();
		break;
	case F_FILE_REOPEN:				Command_FILE_REOPEN( GetDocument()->GetDocumentEncoding(), lparam1!=0 );break;//	Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS:		Command_FILE_REOPEN( CODE_SJIS, lparam1!=0 );break;		//SJISで開き直す
	case F_FILE_REOPEN_JIS:			Command_FILE_REOPEN( CODE_JIS, lparam1!=0 );break;		//JISで開き直す
	case F_FILE_REOPEN_EUC:			Command_FILE_REOPEN( CODE_EUC, lparam1!=0 );break;		//EUCで開き直す
	case F_FILE_REOPEN_UNICODE:		Command_FILE_REOPEN( CODE_UNICODE, lparam1!=0 );break;	//Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE: 	Command_FILE_REOPEN( CODE_UNICODEBE, lparam1!=0 );break;	//UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8:		Command_FILE_REOPEN( CODE_UTF8, lparam1!=0 );break;		//UTF-8で開き直す
	case F_FILE_REOPEN_CESU8:		Command_FILE_REOPEN( CODE_CESU8, lparam1!=0 );break;	//CESU-8で開きなおす
	case F_FILE_REOPEN_UTF7:		Command_FILE_REOPEN( CODE_UTF7, lparam1!=0 );break;		//UTF-7で開き直す
	case F_PRINT:				Command_PRINT();break;					/* 印刷 */
	case F_PRINT_PREVIEW:		Command_PRINT_PREVIEW();break;			/* 印刷プレビュー */
	case F_PRINT_PAGESETUP:		Command_PRINT_PAGESETUP();break;		/* 印刷ページ設定 */	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	case F_OPEN_HfromtoC:		bRet = Command_OPEN_HfromtoC( (BOOL)lparam1 );break;	/* 同名のC/C++ヘッダ(ソース)を開く */	//Feb. 7, 2001 JEPRO 追加
//	case F_OPEN_HHPP:			bRet = Command_OPEN_HHPP( (BOOL)lparam1, TRUE );break;		/* 同名のC/C++ヘッダファイルを開く */	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
//	case F_OPEN_CCPP:			bRet = Command_OPEN_CCPP( (BOOL)lparam1, TRUE );break;		/* 同名のC/C++ソースファイルを開く */	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
	case F_ACTIVATE_SQLPLUS:	Command_ACTIVATE_SQLPLUS();break;		/* Oracle SQL*Plusをアクティブ表示 */
	case F_PLSQL_COMPILE_ON_SQLPLUS:									/* Oracle SQL*Plusで実行 */
		Command_PLSQL_COMPILE_ON_SQLPLUS();
		break;
	case F_BROWSE:				Command_BROWSE();break;				/* ブラウズ */
	case F_VIEWMODE:			Command_VIEWMODE();break;			/* ビューモード */
	case F_PROPERTY_FILE:		Command_PROPERTY_FILE();break;		/* ファイルのプロパティ */
	case F_EXITALLEDITORS:		Command_EXITALLEDITORS();break;		/* 編集の全終了 */	// 2007.02.13 ryoji 追加
	case F_EXITALL:				Command_EXITALL();break;			/* サクラエディタの全終了 */	//Dec. 26, 2000 JEPRO 追加
	case F_PUTFILE:				Command_PUTFILE((LPCWSTR)lparam1, (ECodeType)lparam2, (int)lparam3);break;	/* 作業中ファイルの一時出力 */ //maru 2006.12.10
	case F_INSFILE:				Command_INSFILE((LPCWSTR)lparam1, (ECodeType)lparam2, (int)lparam3);break;	/* キャレット位置にファイル挿入 */ //maru 2006.12.10

	/* 編集系 */
	case F_UNDO:				Command_UNDO();break;				/* 元に戻す(Undo) */
	case F_REDO:				Command_REDO();break;				/* やり直し(Redo) */
	case F_DELETE:				Command_DELETE(); break;			//削除
	case F_DELETE_BACK:			Command_DELETE_BACK(); break;		//カーソル前を削除
	case F_WordDeleteToStart:	Command_WordDeleteToStart(); break;	//単語の左端まで削除
	case F_WordDeleteToEnd:		Command_WordDeleteToEnd(); break;	//単語の右端まで削除
	case F_WordDelete:			Command_WordDelete(); break;		//単語削除
	case F_WordCut:				Command_WordCut(); break;			//単語切り取り
	case F_LineCutToStart:		Command_LineCutToStart(); break;	//行頭まで切り取り(改行単位)
	case F_LineCutToEnd:		Command_LineCutToEnd(); break;		//行末まで切り取り(改行単位)
	case F_LineDeleteToStart:	Command_LineDeleteToStart(); break;	//行頭まで削除(改行単位)
	case F_LineDeleteToEnd:		Command_LineDeleteToEnd(); break;	//行末まで削除(改行単位)
	case F_CUT_LINE:			Command_CUT_LINE();break;			//行切り取り(折り返し単位)
	case F_DELETE_LINE:			Command_DELETE_LINE();break;		//行削除(折り返し単位)
	case F_DUPLICATELINE:		Command_DUPLICATELINE();break;		//行の二重化(折り返し単位)
	case F_INDENT_TAB:			Command_INDENT( WCODE::TAB, INDENT_TAB );break;	//TABインデント
	case F_UNINDENT_TAB:		Command_UNINDENT( WCODE::TAB );break;		//逆TABインデント
	case F_INDENT_SPACE:		Command_INDENT( WCODE::SPACE, INDENT_SPACE );break;	//SPACEインデント
	case F_UNINDENT_SPACE:			Command_UNINDENT( WCODE::SPACE );break;	//逆SPACEインデント
//	case F_WORDSREFERENCE:			Command_WORDSREFERENCE();break;		/* 単語リファレンス */
	case F_LTRIM:					Command_TRIM(TRUE);break;			// 2001.12.03 hor
	case F_RTRIM:					Command_TRIM(FALSE);break;			// 2001.12.03 hor
	case F_SORT_ASC:				Command_SORT(TRUE);break;			// 2001.12.06 hor
	case F_SORT_DESC:				Command_SORT(FALSE);break;			// 2001.12.06 hor
	case F_MERGE:					Command_MERGE();break;				// 2001.12.06 hor
	case F_RECONVERT:				Command_Reconvert();break;			/* メニューからの再変換対応 minfu 2002.04.09 */ 

	/* カーソル移動系 */
	case F_IME_CHAR:		Command_IME_CHAR( (WORD)lparam1 ); break;					//全角文字入力
	case F_MOVECURSOR:			Command_MOVECURSOR(CLogicPoint(CLogicInt((int)lparam2), CLogicInt((int)lparam1)), (int)lparam3); break;
	case F_MOVECURSORLAYOUT:	Command_MOVECURSORLAYOUT(CLayoutPoint(CLayoutInt((int)lparam2), CLayoutInt((int)lparam1)), (int)lparam3); break;
	case F_UP:				Command_UP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, bRepeat ); break;				//カーソル上移動
	case F_DOWN:			Command_DOWN( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, bRepeat ); break;			//カーソル下移動
	case F_LEFT:			Command_LEFT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, bRepeat ); break;			//カーソル左移動
	case F_RIGHT:			Command_RIGHT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, FALSE, bRepeat ); break;	//カーソル右移動
	case F_UP2:				Command_UP2( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;						//カーソル上移動(２行づつ)
	case F_DOWN2:			Command_DOWN2( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//カーソル下移動(２行づつ)
	case F_WORDLEFT:		Command_WORDLEFT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				/* 単語の左端に移動 */
	case F_WORDRIGHT:		Command_WORDRIGHT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				/* 単語の右端に移動 */
	//	0ct. 29, 2001 genta マクロ向け機能拡張
	case F_GOLINETOP:		Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, lparam1  ); break;		//行頭に移動(折り返し単位)
	case F_GOLINEEND:		Command_GOLINEEND( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, FALSE ); break;		//行末に移動(折り返し単位)
//	case F_ROLLDOWN:		Command_ROLLDOWN( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//スクロールダウン
//	case F_ROLLUP:			Command_ROLLUP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//スクロールアップ
	case F_HalfPageUp:		Command_HalfPageUp( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown:	Command_HalfPageDown( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;			//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp:			Command_1PageUp( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown:		Command_1PageDown( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_GOFILETOP:		Command_GOFILETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//ファイルの先頭に移動
	case F_GOFILEEND:		Command_GOFILEEND( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//ファイルの最後に移動
	case F_CURLINECENTER:	Command_CURLINECENTER(); break;								/* カーソル行をウィンドウ中央へ */
	case F_JUMPHIST_PREV:	Command_JUMPHIST_PREV(); break;								//移動履歴: 前へ
	case F_JUMPHIST_NEXT:	Command_JUMPHIST_NEXT(); break;								//移動履歴: 次へ
	case F_JUMPHIST_SET:	Command_JUMPHIST_SET(); break;								//現在位置を移動履歴に登録
	case F_WndScrollDown:	Command_WndScrollDown(); break;								//テキストを１行下へスクロール	// 2001/06/20 asa-o
	case F_WndScrollUp:		Command_WndScrollUp(); break;								//テキストを１行上へスクロール	// 2001/06/20 asa-o
	case F_GONEXTPARAGRAPH:	Command_GONEXTPARAGRAPH( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;			//次の段落へ進む
	case F_GOPREVPARAGRAPH:	Command_GOPREVPARAGRAPH( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;			//前の段落へ戻る

	/* 選択系 */
	case F_SELECTWORD:		Command_SELECTWORD( );break;					//現在位置の単語選択
	case F_SELECTALL:		Command_SELECTALL();break;						//すべて選択
	case F_SELECTLINE:		Command_SELECTLINE( lparam1 );break;			//1行選択	// 2007.10.13 nasukoji
	case F_BEGIN_SEL:		Command_BEGIN_SELECT();break;					/* 範囲選択開始 */
	case F_UP_SEL:			Command_UP( TRUE, bRepeat, lparam1 ); break;	//(範囲選択)カーソル上移動
	case F_DOWN_SEL:		Command_DOWN( TRUE, bRepeat ); break;			//(範囲選択)カーソル下移動
	case F_LEFT_SEL:		Command_LEFT( TRUE, bRepeat ); break;			//(範囲選択)カーソル左移動
	case F_RIGHT_SEL:		Command_RIGHT( TRUE, FALSE, bRepeat ); break;	//(範囲選択)カーソル右移動
	case F_UP2_SEL:			Command_UP2( TRUE ); break;						//(範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL:		Command_DOWN2( TRUE );break;					//(範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL:	Command_WORDLEFT( TRUE );break;					//(範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL:	Command_WORDRIGHT( TRUE );break;				//(範囲選択)単語の右端に移動
	case F_GOLINETOP_SEL:	Command_GOLINETOP( TRUE, 0 );break;				//(範囲選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_SEL:	Command_GOLINEEND( TRUE, FALSE );break;			//(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL:	Command_ROLLDOWN( TRUE ); break;				//(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL:		Command_ROLLUP( TRUE ); break;					//(範囲選択)スクロールアップ
	case F_HalfPageUp_Sel:	Command_HalfPageUp( TRUE ); break;				//(範囲選択)半ページアップ
	case F_HalfPageDown_Sel:Command_HalfPageDown( TRUE ); break;			//(範囲選択)半ページダウン
	case F_1PageUp_Sel:		Command_1PageUp( TRUE ); break;					//(範囲選択)１ページアップ
	case F_1PageDown_Sel:	Command_1PageDown( TRUE ); break;				//(範囲選択)１ページダウン
	case F_GOFILETOP_SEL:	Command_GOFILETOP( TRUE );break;				//(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL:	Command_GOFILEEND( TRUE );break;				//(範囲選択)ファイルの最後に移動
	case F_GONEXTPARAGRAPH_SEL:	Command_GONEXTPARAGRAPH( TRUE ); break;			//次の段落へ進む
	case F_GOPREVPARAGRAPH_SEL:	Command_GOPREVPARAGRAPH( TRUE ); break;			//前の段落へ戻る

	/* 矩形選択系 */
//	case F_BOXSELALL:		Command_BOXSELECTALL();break;		//矩形ですべて選択
	case F_BEGIN_BOX:		Command_BEGIN_BOXSELECT();break;	/* 矩形範囲選択開始 */
//	case F_UP_BOX:			Command_UP_BOX( bRepeat ); break;			//(矩形選択)カーソル上移動
//	case F_DOWN_BOX:		Command_DOWN( TRUE, bRepeat ); break;		//(矩形選択)カーソル下移動
//	case F_LEFT_BOX:		Command_LEFT( TRUE, bRepeat ); break;		//(矩形選択)カーソル左移動
//	case F_RIGHT_BOX:		Command_RIGHT( TRUE, FALSE, bRepeat ); break;//(矩形選択)カーソル右移動
//	case F_UP2_BOX:			Command_UP2( TRUE ); break;					//(矩形選択)カーソル上移動(２行ごと)
//	case F_DOWN2_BOX:		Command_DOWN2( TRUE );break;				//(矩形選択)カーソル下移動(２行ごと)
//	case F_WORDLEFT_BOX:	Command_WORDLEFT( TRUE );break;				//(矩形選択)単語の左端に移動
//	case F_WORDRIGHT_BOX:	Command_WORDRIGHT( TRUE );break;			//(矩形選択)単語の右端に移動
//	case F_GOLINETOP_BOX:	Command_GOLINETOP( TRUE, FALSE );break;		//(矩形選択)行頭に移動(折り返し単位)
//	case F_GOLINEEND_BOX:	Command_GOLINEEND( TRUE, FALSE );break;		//(矩形選択)行末に移動(折り返し単位)
//	case F_HalfPageUp_Box:	Command_HalfPageUp( TRUE ); break;			//(矩形選択)半ページアップ
//	case F_HalfPageDown_Box:Command_HalfPageDown( TRUE ); break;		//(矩形選択)半ページダウン
//	case F_1PageUp_Box:		Command_1PageUp( TRUE ); break;				//(矩形選択)１ページアップ
//	case F_1PageDown_Box:	Command_1PageDown( TRUE ); break;			//(矩形選択)１ページダウン
//	case F_GOFILETOP_Box:	Command_GOFILETOP( TRUE );break;			//(矩形選択)ファイルの先頭に移動
//	case F_GOFILEEND_Box:	Command_GOFILEEND( TRUE );break;			//(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	case F_CUT:						Command_CUT();break;					//切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:					Command_COPY( false, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy );break;			//コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF:			Command_COPY( false, true );break;		//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:				Command_COPY( false, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy, EOL_CRLF );break;	//CRLF改行でコピー(選択範囲をクリップボードにコピー)
	case F_PASTE:					Command_PASTE( (int)lparam1 );break;				//貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:				Command_PASTEBOX( (int)lparam1 );break;				//矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSBOXTEXT:				Command_INSBOXTEXT((const wchar_t*)lparam1, (int)lparam2 );break;				//矩形テキスト挿入
	case F_INSTEXT_W:				Command_INSTEXT( bRedraw, (const wchar_t*)lparam1, CLogicInt(-1), lparam2!=0 );break;/* テキストを貼り付け */ // 2004.05.14 Moca 長さを示す引数追加(-1は\0終端まで)
	case F_ADDTAIL_W:				Command_ADDTAIL( (const wchar_t*)lparam1, (int)lparam2 );break;	/* 最後にテキストを追加 */
	case F_COPYFNAME:				Command_COPYFILENAME();break;			//このファイル名をクリップボードにコピー / /2002/2/3 aroka
	case F_COPYPATH:				Command_COPYPATH();break;				//このファイルのパス名をクリップボードにコピー
	case F_COPYTAG:					Command_COPYTAG();break;				//このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	case F_COPYLINES:				Command_COPYLINES();break;				//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		Command_COPYLINESASPASSAGE();break;		//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	Command_COPYLINESWITHLINENUMBER();break;//選択範囲内全行行番号付きコピー
	case F_CREATEKEYBINDLIST:		Command_CREATEKEYBINDLIST();break;		//キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE:				Command_INS_DATE();break;	//日付挿入
	case F_INS_TIME:				Command_INS_TIME();break;	//時刻挿入
    case F_CTRL_CODE_DIALOG:		Command_CtrlCode_Dialog();break;	/* コントロールコードの入力(ダイアログ) */	//@@@ 2002.06.02 MIK

	/* 変換 */
	case F_TOLOWER:					Command_TOLOWER();break;				/* 小文字 */
	case F_TOUPPER:					Command_TOUPPER();break;				/* 大文字 */
	case F_TOHANKAKU:				Command_TOHANKAKU();break;				/* 全角→半角 */
	case F_TOHANKATA:				Command_TOHANKATA();break;				/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI:					Command_TOZENEI();break;				/* 全角→半角 */					//July. 30, 2001 Misaka
	case F_TOHANEI:					Command_TOHANEI();break;				/* 半角→全角 */
	case F_TOZENKAKUKATA:			Command_TOZENKAKUKATA();break;			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA:			Command_TOZENKAKUHIRA();break;			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKATA:	Command_HANKATATOZENKAKUKATA();break;	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENHIRA:	Command_HANKATATOZENKAKUHIRA();break;	/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE:				Command_TABTOSPACE();break;				/* TAB→空白 */
	case F_SPACETOTAB:				Command_SPACETOTAB();break;				/* 空白→TAB */  //---- Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS:		Command_CODECNV_AUTO2SJIS();break;		/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL:			Command_CODECNV_EMAIL();break;			/* E-Mail(JIS→SJIS)コード変換 */
	case F_CODECNV_EUC2SJIS:		Command_CODECNV_EUC2SJIS();break;		/* EUC→SJISコード変換 */
	case F_CODECNV_UNICODE2SJIS:	Command_CODECNV_UNICODE2SJIS();break;	/* Unicode→SJISコード変換 */
	case F_CODECNV_UNICODEBE2SJIS:	Command_CODECNV_UNICODEBE2SJIS();break;	/* UnicodeBE→SJISコード変換 */
	case F_CODECNV_UTF82SJIS:		Command_CODECNV_UTF82SJIS();break;		/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:		Command_CODECNV_UTF72SJIS();break;		/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:		Command_CODECNV_SJIS2JIS();break;		/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC:		Command_CODECNV_SJIS2EUC();break;		/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8:		Command_CODECNV_SJIS2UTF8();break;		/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7:		Command_CODECNV_SJIS2UTF7();break;		/* SJIS→UTF-7コード変換 */
	case F_BASE64DECODE:			Command_BASE64DECODE();break;			/* Base64デコードして保存 */
	case F_UUDECODE:				Command_UUDECODE();break;				/* uudecodeして保存 */	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
	case F_SEARCH_DIALOG:		Command_SEARCH_DIALOG();break;												//検索(単語検索ダイアログ)
	case F_SEARCH_BOX:			Command_SEARCH_BOX();break;		// Jan. 13, 2003 MIK					//検索(ボックス)	// 2006.06.04 yukihane Command_SEARCH_BOX()
	case F_SEARCH_NEXT:			Command_SEARCH_NEXT( true, bRedraw, (HWND)lparam1, (const WCHAR*)lparam2 );break;	//次を検索
	case F_SEARCH_PREV:			Command_SEARCH_PREV( bRedraw, (HWND)lparam1 );break;						//前を検索
	case F_REPLACE_DIALOG:	//置換(置換ダイアログ)
		/* 再帰処理対策 */
		ClearOpeBlk();
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI ダイアログ呼び出しと、実行を分離
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//置換実行 @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//すべて置換実行(通常) 2002.2.8 hor 2006.04.02 かろと
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//検索マークのクリア
	case F_GREP_DIALOG:	//Grepダイアログの表示
		/* 再帰処理対策 */
		ClearOpeBlk();
		Command_GREP_DIALOG();
		break;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//指定行ヘジャンプダイアログの表示
	case F_JUMP:			Command_JUMP();break;							//指定行ヘジャンプ
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1, OUTLINE_DEFAULT );break;	//アウトライン解析
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE, OUTLINE_DEFAULT );break;	//アウトライン解析(toggle) // 20060201 aroka
	case F_TAGJUMP:			Command_TAGJUMP(lparam1 != 0);break;			/* タグジャンプ機能 */ //	Apr. 03, 2003 genta 引数追加
	case F_TAGJUMP_CLOSE:	Command_TAGJUMP(true);break;					/* タグジャンプ(元ウィンドウClose) *///	Apr. 03, 2003 genta
	case F_TAGJUMPBACK:		Command_TAGJUMPBACK();break;					/* タグジャンプバック機能 */
	case F_TAGS_MAKE:		Command_TagsMake();break;						//タグファイルの作成	//@@@ 2003.04.13 MIK
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFileMsg( true );break;				/* ダイレクトタグジャンプ機能 */	//@@@ 2003.04.15 MIK
	case F_TAGJUMP_KEYWORD:	Command_TagJumpByTagsFileKeyword( (const wchar_t*)lparam1 );break;	/* @@ 2005.03.31 MIK キーワードを指定してダイレクトタグジャンプ機能 */
	case F_COMPARE:			Command_COMPARE();break;						/* ファイル内容比較 */
	case F_DIFF_DIALOG:		Command_Diff_Dialog();break;					/* DIFF差分表示(ダイアログ) */	//@@@ 2002.05.25 MIK
	case F_DIFF:			Command_Diff( (const WCHAR*)lparam1, (int)lparam2 );break;		/* DIFF差分表示 */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	case F_DIFF_NEXT:		Command_Diff_Next();break;						/* DIFF差分表示(次へ) */		//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:		Command_Diff_Prev();break;						/* DIFF差分表示(前へ) */		//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:		Command_Diff_Reset();break;						/* DIFF差分表示(全解除) */		//@@@ 2002.05.25 MIK
	case F_BRACKETPAIR:		Command_BRACKETPAIR();	break;					//対括弧の検索
// From Here 2001.12.03 hor
	case F_BOOKMARK_SET:	Command_BOOKMARK_SET();break;					/* ブックマーク設定・解除 */
	case F_BOOKMARK_NEXT:	Command_BOOKMARK_NEXT();break;					/* 次のブックマークへ */
	case F_BOOKMARK_PREV:	Command_BOOKMARK_PREV();break;					/* 前のブックマークへ */
	case F_BOOKMARK_RESET:	Command_BOOKMARK_RESET();break;					/* ブックマークの全解除 */
	case F_BOOKMARK_VIEW:	bRet = Command_FUNCLIST( (BOOL)lparam1 ,OUTLINE_BOOKMARK );break;	//アウトライン解析
// To Here 2001.12.03 hor
	case F_BOOKMARK_PATTERN:Command_BOOKMARK_PATTERN();break;				// 2002.01.16 hor 指定パターンに一致する行をマーク
	case F_JUMP_SRCHSTARTPOS:	Command_JUMP_SRCHSTARTPOS();break;			// 検索開始位置へ戻る 02/06/26 ai
	
	/* モード切り替え系 */
	case F_CHGMOD_INS:		Command_CHGMOD_INS();break;		//挿入／上書きモード切り替え
	// From Here 2003.06.23 Moca
	// F_CHGMOD_EOL_xxx はマクロに記録されないが、F_CHGMOD_EOLはマクロに記録されるので、マクロ関数を統合できるという手はず
	case F_CHGMOD_EOL_CRLF:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_CRLF, 0, 0, 0 );break;	//入力する改行コードをCRLFに設定
	case F_CHGMOD_EOL_LF:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_LF, 0, 0, 0 );break;	//入力する改行コードをLFに設定
	case F_CHGMOD_EOL_CR:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_CR, 0, 0, 0 );break;	//入力する改行コードをCRに設定
	// 2006.09.03 Moca F_CHGMOD_EOLで break 忘れの修正
	case F_CHGMOD_EOL:		Command_CHGMOD_EOL( (EEolType)lparam1 );break;	//入力する改行コードを設定
	// To Here 2003.06.23 Moca
	case F_CANCEL_MODE:		Command_CANCEL_MODE();break;	//各種モードの取り消し

	/* 設定系 */
	case F_SHOWTOOLBAR:		Command_SHOWTOOLBAR();break;	/* ツールバーの表示/非表示 */
	case F_SHOWFUNCKEY:		Command_SHOWFUNCKEY();break;	/* ファンクションキーの表示/非表示 */
	case F_SHOWTAB:			Command_SHOWTAB();break;		/* タブの表示/非表示 */	//@@@ 2003.06.10 MIK
	case F_SHOWSTATUSBAR:	Command_SHOWSTATUSBAR();break;	/* ステータスバーの表示/非表示 */
	case F_TYPE_LIST:		Command_TYPE_LIST();break;		/* タイプ別設定一覧 */
	case F_OPTION_TYPE:		Command_OPTION_TYPE();break;	/* タイプ別設定 */
	case F_OPTION:			Command_OPTION();break;			/* 共通設定 */
	case F_FONT:			Command_FONT();break;			/* フォント設定 */
	case F_WRAPWINDOWWIDTH:	Command_WRAPWINDOWWIDTH();break;/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	case F_FAVORITE:		Command_Favorite();break;		//履歴の管理	//@@@ 2003.04.08 MIK
	//	Jan. 29, 2005 genta 引用符の設定
	case F_SET_QUOTESTRING:	Command_SET_QUOTESTRING((const WCHAR*)lparam1);	break;
	case F_TMPWRAPNOWRAP:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_NO_TEXT_WRAP, 0, 0, 0 );break;	// 折り返さない（一時設定）			// 2008.05.30 nasukoji
	case F_TMPWRAPSETTING:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_SETTING_WIDTH, 0, 0, 0 );break;	// 指定桁で折り返す（一時設定）		// 2008.05.30 nasukoji
	case F_TMPWRAPWINDOW:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_WINDOW_WIDTH, 0, 0, 0 );break;	// 右端で折り返す（一時設定）		// 2008.05.30 nasukoji
	case F_TEXTWRAPMETHOD:	Command_TEXTWRAPMETHOD( (int)lparam1 );break;		// テキストの折り返し方法		// 2008.05.30 nasukoji
	case F_SELECT_COUNT_MODE:	Command_SELECT_COUNT_MODE( (int)lparam1 );break;	// 文字カウントの方法		// 2009.07.06 syat

	/* マクロ系 */
	case F_RECKEYMACRO:		Command_RECKEYMACRO();break;	/* キーマクロの記録開始／終了 */
	case F_SAVEKEYMACRO:	Command_SAVEKEYMACRO();break;	/* キーマクロの保存 */
	case F_LOADKEYMACRO:	Command_LOADKEYMACRO();break;	/* キーマクロの読み込み */
	case F_EXECKEYMACRO:									/* キーマクロの実行 */
		/* 再帰処理対策 */
		ClearOpeBlk();
		Command_EXECKEYMACRO();break;
	case F_EXECEXTMACRO:
		/* 再帰処理対策 */
		ClearOpeBlk();
		/* 名前を指定してマクロ実行 */
		Command_EXECEXTMACRO( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* 外部コマンド実行 */
	case F_EXECMD_DIALOG:
		/* 再帰処理対策 */// 2001/06/23 N.Nakatani
		ClearOpeBlk();
		//Command_EXECCOMMAND_DIALOG((const char*)lparam1);	/* 外部コマンド実行 */
		Command_EXECCOMMAND_DIALOG();	/* 外部コマンド実行 */	//	引数つかってないみたいなので
		break;
	//	To Here Sept. 20, 2000
	case F_EXECMD:
		//Command_EXECCOMMAND((const char*)lparam1);
		Command_EXECCOMMAND((LPCWSTR)lparam1, (int)lparam2);	//	2006.12.03 maru 引数の拡張のため
		break;

	/* カスタムメニュー */
	case F_MENU_RBUTTON:	/* 右クリックメニュー */
		/* 再帰処理対策 */
		ClearOpeBlk();
		Command_MENU_RBUTTON();
		break;
	case F_CUSTMENU_1:  /* カスタムメニュー1 */
	case F_CUSTMENU_2:  /* カスタムメニュー2 */
	case F_CUSTMENU_3:  /* カスタムメニュー3 */
	case F_CUSTMENU_4:  /* カスタムメニュー4 */
	case F_CUSTMENU_5:  /* カスタムメニュー5 */
	case F_CUSTMENU_6:  /* カスタムメニュー6 */
	case F_CUSTMENU_7:  /* カスタムメニュー7 */
	case F_CUSTMENU_8:  /* カスタムメニュー8 */
	case F_CUSTMENU_9:  /* カスタムメニュー9 */
	case F_CUSTMENU_10: /* カスタムメニュー10 */
	case F_CUSTMENU_11: /* カスタムメニュー11 */
	case F_CUSTMENU_12: /* カスタムメニュー12 */
	case F_CUSTMENU_13: /* カスタムメニュー13 */
	case F_CUSTMENU_14: /* カスタムメニュー14 */
	case F_CUSTMENU_15: /* カスタムメニュー15 */
	case F_CUSTMENU_16: /* カスタムメニュー16 */
	case F_CUSTMENU_17: /* カスタムメニュー17 */
	case F_CUSTMENU_18: /* カスタムメニュー18 */
	case F_CUSTMENU_19: /* カスタムメニュー19 */
	case F_CUSTMENU_20: /* カスタムメニュー20 */
	case F_CUSTMENU_21: /* カスタムメニュー21 */
	case F_CUSTMENU_22: /* カスタムメニュー22 */
	case F_CUSTMENU_23: /* カスタムメニュー23 */
	case F_CUSTMENU_24: /* カスタムメニュー24 */
		/* 再帰処理対策 */
		ClearOpeBlk();
		nFuncID = Command_CUSTMENU( nCommand - F_CUSTMENU_1 + 1 );
		if( 0 != nFuncID ){
			/* コマンドコードによる処理振り分け */
//			HandleCommand( nFuncID, TRUE, 0, 0, 0, 0 );
			::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nFuncID, 0 ), (LPARAM)NULL );
		}
		break;

	/* ウィンドウ系 */
	case F_SPLIT_V:			Command_SPLIT_V();break;	/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	case F_SPLIT_H:			Command_SPLIT_H();break;	/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	case F_SPLIT_VH:		Command_SPLIT_VH();break;	/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
	case F_WINCLOSE:		Command_WINCLOSE();break;	//ウィンドウを閉じる
	case F_WIN_CLOSEALL:	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」を左記のように変更
		//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
		Command_FILECLOSEALL();
		break;
	case F_BIND_WINDOW:		Command_BIND_WINDOW();break;	//結合して表示 2004.07.14 Kazika 新規追加
	case F_CASCADE:			Command_CASCADE();break;		//重ねて表示
	case F_TILE_V:			Command_TILE_V();break;			//上下に並べて表示
	case F_TILE_H:			Command_TILE_H();break;			//左右に並べて表示
	case F_MAXIMIZE_V:		Command_MAXIMIZE_V();break;		//縦方向に最大化
	case F_MAXIMIZE_H:		Command_MAXIMIZE_H();break;		//横方向に最大化 //2001.02.10 by MIK
	case F_MINIMIZE_ALL:	Command_MINIMIZE_ALL();break;	/* すべて最小化 */	//	Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	case F_REDRAW:			Command_REDRAW();break;			/* 再描画 */
	case F_WIN_OUTPUT:		Command_WIN_OUTPUT();break;		//アウトプットウィンドウ表示
	case F_TRACEOUT:		Command_TRACEOUT((const wchar_t*)lparam1, (int)lparam2);break;		//マクロ用アウトプットウィンドウに表示 maru 2006.04.26
	case F_TOPMOST:			Command_WINTOPMOST( lparam1 );break;	//常に手前に表示 Moca
	case F_WINLIST:			Command_WINLIST( nCommandFrom );break;		/* ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon // 2006.05.19 genta 引数追加
	case F_GROUPCLOSE:		Command_GROUPCLOSE();break;		/* グループを閉じる */		// 2007.06.20 ryoji 追加
	case F_NEXTGROUP:		Command_NEXTGROUP();break;		/* 次のグループ */			// 2007.06.20 ryoji 追加
	case F_PREVGROUP:		Command_PREVGROUP();break;		/* 前のグループ */			// 2007.06.20 ryoji 追加
	case F_TAB_MOVERIGHT:	Command_TAB_MOVERIGHT();break;	/* タブを右に移動 */		// 2007.06.20 ryoji 追加
	case F_TAB_MOVELEFT:	Command_TAB_MOVELEFT();break;	/* タブを左に移動 */		// 2007.06.20 ryoji 追加
	case F_TAB_SEPARATE:	Command_TAB_SEPARATE();break;	/* 新規グループ */			// 2007.06.20 ryoji 追加
	case F_TAB_JOINTNEXT:	Command_TAB_JOINTNEXT();break;	/* 次のグループに移動 */	// 2007.06.20 ryoji 追加
	case F_TAB_JOINTPREV:	Command_TAB_JOINTPREV();break;	/* 前のグループに移動 */	// 2007.06.20 ryoji 追加
	case F_TAB_CLOSEOTHER:	Command_TAB_CLOSEOTHER();break;	/* このタブ以外を閉じる */	// 2008.11.22 syat 追加
	case F_TAB_CLOSELEFT:	Command_TAB_CLOSELEFT();break;	/* 左をすべて閉じる */		// 2008.11.22 syat 追加
	case F_TAB_CLOSERIGHT:	Command_TAB_CLOSERIGHT();break;	/* 右をすべて閉じる */		// 2008.11.22 syat 追加

	/* 支援 */
	case F_HOKAN:			Command_HOKAN();break;			//入力補完
	case F_HELP_CONTENTS:	Command_HELP_CONTENTS();break;	/* ヘルプ目次 */				//Nov. 25, 2000 JEPRO 追加
	case F_HELP_SEARCH:		Command_HELP_SEARCH();break;	/* ヘルプトキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch();break;	/* キャレット位置の単語を辞書検索する機能ON-OFF */	// 2006.03.24 fon
	case F_MENU_ALLFUNC:									/* コマンド一覧 */
		/* 再帰処理対策 */
		ClearOpeBlk();
		Command_MENU_ALLFUNC();break;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* 外部ヘルプ１ */
	case F_EXTHTMLHELP:	/* 外部HTMLヘルプ */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	case F_ABOUT:	Command_ABOUT();break;				/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */
//	case F_SENDMAIL:	Command_SENDMAIL();break;		/* メール送信 */

	case F_0: break; // F_0でプラグインが実行されるバグ対策	// ← rev1886 の問題は呼び元で対策したが安全弁として残す

	default:
		//プラグインコマンドを実行する
		{
			CPlug::Array plugs;
			CJackManager::getInstance()->GetUsablePlug( PP_COMMAND, nCommand, &plugs );

			if( plugs.size() > 0 ){
				assert_warning( 1 == plugs.size() );
				//インタフェースオブジェクト準備
				CWSHIfObj::List params;
				//プラグイン呼び出し
				( *plugs.begin() )->Invoke( m_pCommanderView, params );

				/* フォーカス移動時の再描画 */
				m_pCommanderView->RedrawAll();
				break;
			}
		}

	}

	/* アンドゥバッファの処理 */
	m_pCommanderView->SetUndoBuffer( true );

	return bRet;
}



/**	選択範囲をクリップボードにコピー

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CViewCommander::Command_COPY(
	bool		bIgnoreLockAndDisable,	//!< [in] 選択範囲を解除するか？
	bool		bAddCRLFWhenCopy,		//!< [in] 折り返し位置に改行コードを挿入するか？
	EEolType	neweol					//!< [in] コピーするときのEOL。
)
{
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect = false;

	/* クリップボードに入れるべきテキストデータを、cmemBufに格納する */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 非選択時は、カーソル行をコピーする */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		m_pCommanderView->CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
		);
	}
	else{
		/* テキストが選択されているときは、選択範囲のデータを取得 */

		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			bBeginBoxSelect = TRUE;
		}
		/* 選択範囲のデータを取得 */
		/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
		if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, bAddCRLFWhenCopy, neweol ) ){
			ErrorBeep();
			return;
		}

		/* クリップボードにデータcmemBufの内容を設定 */
		if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect, FALSE ) ){
			ErrorBeep();
			return;
		}
	}

	/* 選択範囲の後片付け */
	if( !bIgnoreLockAndDisable ){
		/* 選択状態のロック */
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;
			m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
				GetCaret().m_cUnderLine.CaretUnderLineON(true);
			}
		}
	}
	if( GetDllShareData().m_Common.m_sEdit.m_bCopyAndDisablSelection ){	/* コピーしたら選択解除 */
		/* テキストが選択されているか */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	return;
}




/** 切り取り(選択範囲をクリップボードにコピーして削除)

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CViewCommander::Command_CUT( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CNativeW	cmemBuf;
	bool	bBeginBoxSelect;
	/* 範囲選択がされていない */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 非選択時は、カーソル行を切り取り */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		//行切り取り(折り返し単位)
		Command_CUT_LINE();
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		bBeginBoxSelect = true;
	}else{
		bBeginBoxSelect = false;
	}

	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
	if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}
	/* クリップボードにデータを設定 */
	if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect ) ){
		ErrorBeep();
		return;
	}

	/* カーソル位置または選択エリアを削除 */
	m_pCommanderView->DeleteData( TRUE );
	return;
}



/* すべて選択 */
void CViewCommander::Command_SELECTALL( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	/* 先頭へカーソルを移動 */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	//	Jul. 29, 2006 genta 選択位置の末尾を正確に取得する
	//	マクロから取得した場合に正しい範囲が取得できないため
	//int nX, nY;
	CLayoutRange sRange;
	sRange.SetFrom(CLayoutPoint(0,0));
	GetDocument()->m_cLayoutMgr.GetEndLayoutPos(sRange.GetToPointer());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* 選択領域描画 */
	m_pCommanderView->GetSelectionInfo().DrawSelectArea();
}




/*!	1行選択
	@brief カーソル位置を1行選択する
	@param lparam [in] マクロから使用する拡張フラグ（拡張用に予約）

	note 改行単位で選択を行う。

	@date 2007.11.15 nasukoji	新規作成
*/
void CViewCommander::Command_SELECTLINE( int lparam )
{
	// 改行単位で1行選択する
	Command_GOLINETOP( FALSE, 0x9 );	// 物理行頭に移動

	m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect = TRUE;		// 行単位選択中

	CLayoutPoint ptCaret;

	// 最下行（物理行）でない
	if(GetCaret().GetCaretLogicPos().y < GetDocument()->m_cDocLineMgr.GetLineCount() ){
		// 1行先の物理行からレイアウト行を求める
		GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

		// カーソルを次の物理行頭へ移動する
		m_pCommanderView->MoveCursorSelecting( ptCaret, TRUE );
		
		// 移動後のカーソル位置を取得する
		ptCaret = GetCaret().GetCaretLayoutPos().Get();
	}else{
		// カーソルを最下行（レイアウト行）へ移動する
		m_pCommanderView->MoveCursorSelecting( CLayoutPoint(CLayoutInt(0), GetDocument()->m_cLayoutMgr.GetLineCount()), TRUE );
		Command_GOLINEEND( TRUE, FALSE );	// 行末に移動

		// 選択するものが無い（[EOF]のみの行）時は選択状態としない
		if(( ! m_pCommanderView->GetSelectionInfo().IsTextSelected() )&&
		   ( GetCaret().GetCaretLogicPos().y >= GetDocument()->m_cDocLineMgr.GetLineCount() ))
		{
			// 現在の選択範囲を非選択状態に戻す
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}

	if( m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect ){
		// 範囲選択開始行・カラムを記憶
		m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo( ptCaret );
		m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo( ptCaret );
	}

	return;
}




/* 現在位置の単語選択 */
bool CViewCommander::Command_SELECTWORD( void )
{
	CLayoutRange sRange;
	CLogicInt	nIdx;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		return false;	//	単語選択に失敗
	}
	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* 現在位置の単語の範囲を調べる */
	if( GetDocument()->m_cLayoutMgr.WhereCurrentWord(	GetCaret().GetCaretLayoutPos().GetY2(), nIdx, &sRange, NULL, NULL ) ){

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetFrom().GetY2() );
		sRange.SetFromX( m_pCommanderView->LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetTo().GetY2() );
		sRange.SetToX( m_pCommanderView->LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
		*/

		/* 選択範囲の変更 */
		//	2005.06.24 Moca
		m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

		/* 単語の先頭にカーソルを移動 */
		GetCaret().MoveCursor( sRange.GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		/* 選択領域描画 */
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
		return true;	//	単語選択に成功。
	}
	else {
		return false;	//	単語選択に失敗
	}
}




/** 貼り付け(クリップボードから貼り付け)
	@date 2007.10.04 ryoji MSDEVLineSelect形式の行コピー対応処理を追加（VS2003/2005のエディタと類似の挙動に）
*/
void CViewCommander::Command_PASTE( int option )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//砂時計
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// クリップボードからデータを取得 -> cmemClip, bColmnSelect
	CNativeW	cmemClip;
	bool		bColmnSelect;
	bool		bLineSelect = false;
	bool		bLineSelectOption = 
		((option & 0x04) == 0x04) ? true :
		((option & 0x08) == 0x08) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste;

	if( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColmnSelect, bLineSelectOption ? &bLineSelect: NULL ) ){
		ErrorBeep();
		return;
	}

	// クリップボードデータ取得 -> pszText, nTextLen
	CLogicInt		nTextLen;
	const wchar_t*	pszText = cmemClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bConvertEOLPaste;

	bool bAutoColmnPaste = 
		((option & 0x10) == 0x10) ? true :
		((option & 0x20) == 0x20) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bAutoColmnPaste != FALSE;

	// 矩形コピーのテキストは常に矩形貼り付け
	if( bAutoColmnPaste ){
		// 矩形コピーのデータなら矩形貼り付け
		if( bColmnSelect ){
			if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){
				ErrorBeep();
				return;
			}
			if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){
				return;
			}
			Command_PASTEBOX(pszText, nTextLen);
			m_pCommanderView->AdjustScrollBars();
			m_pCommanderView->Redraw();
			return;
		}
	}

	// 2007.10.04 ryoji
	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if( bLineSelect ){
		if( pszText[nTextLen - 1] != WCODE::CR && pszText[nTextLen - 1] != WCODE::LF ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	if( bConvertEol ){
		wchar_t	*pszConvertedText = new wchar_t[nTextLen * 2]; // 全文字\n→\r\n変換で最大の２倍になる
		CLogicInt nConvertedTextLen = ConvertEol( pszText, nTextLen, pszConvertedText );
		// テキストを貼り付け
		Command_INSTEXT( true, pszConvertedText, nConvertedTextLen, true, bLineSelect );	// 2010.09.17 ryoji
		delete [] pszConvertedText;
	}else{
		// テキストを貼り付け
		Command_INSTEXT( true, pszText, nTextLen, true, bLineSelect );	// 2010.09.17 ryoji
	}

	return;
}

CLogicInt CViewCommander::ConvertEol(const wchar_t* pszText, CLogicInt nTextLen, wchar_t* pszConvertedText)
{
	// original by 2009.02.28 salarm
	CLogicInt nConvertedTextLen;
	CEol eol = GetDocument()->m_cDocEditor.GetNewLineCode();

	nConvertedTextLen = 0;
	for( int i = 0; i < nTextLen; i++ ){
		if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
			if( pszText[i] == WCODE::CR ){
				if( i + 1 < nTextLen && pszText[i + 1] == WCODE::LF ){
					i++;
				}
			}
			wmemcpy( &pszConvertedText[nConvertedTextLen], eol.GetValue2(), eol.GetLen() );
			nConvertedTextLen += eol.GetLen();
		} else {
			pszConvertedText[nConvertedTextLen++] = pszText[i];
		}
	}
	return nConvertedTextLen;
}

/*! テキストを貼り付け
	@date 2004.05.14 Moca '\\0'を受け入れるように、引数に長さを追加
	@date 2010.09.17 ryoji ラインモード貼り付けオプションを追加して以前の Command_PASTE() との重複部を整理・統合
*/
void CViewCommander::Command_INSTEXT(
	bool			bRedraw,		//!< 
	const wchar_t*	pszText,		//!< [in] 貼り付ける文字列。
	CLogicInt		nTextLen,		//!< [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	bool			bNoWaitCursor,	//!< 
	bool			bLinePaste		//!< [in] ラインモード貼り付け
)
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CWaitCursor*	pcWaitCursor;
	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_pCommanderView->GetHwnd() );
	}

	if( nTextLen < 0 ){
		nTextLen = CLogicInt(wcslen( pszText ));
	}

	GetDocument()->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	// テキストが選択されているか
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// 矩形範囲選択中か
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			//改行までを抜き出す
			CLogicInt i;
			for( i = CLogicInt(0); i < nTextLen; i++ ){
				if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
					break;
				}
			}
			Command_INDENT( pszText, i );
			goto end_of_func;
		}
		else{
			//	Jun. 23, 2000 genta
			//	同一行の行末以降のみが選択されている場合には選択無しと見なす
			CLogicInt		len;
			int pos;
			const wchar_t	*line;
			const CLayout* pcLayout;
			line = GetDocument()->m_cLayoutMgr.GetLineStr( GetSelect().GetFrom().GetY2(), &len, &pcLayout );

			pos = ( line == NULL ) ? 0 : m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() );

			//	開始位置が行末より後ろで、終了位置が同一行
			if( pos >= len && GetSelect().IsLineOne()){
				GetCaret().SetCaretLayoutPos(CLayoutPoint(GetSelect().GetFrom().x, GetCaret().GetCaretLayoutPos().y)); //キャレットX変更
				m_pCommanderView->GetSelectionInfo().DisableSelectArea(false);
			}
			else{
				// データ置換 削除&挿入にも使える
				// 行コピーの貼り付けでは選択範囲は削除（後で行頭に貼り付ける）	// 2007.10.04 ryoji
				m_pCommanderView->ReplaceData_CEditView(
					GetSelect(),				// 選択範囲
					NULL,					// 削除されたデータのコピー(NULL可能)
					bLinePaste? L"": pszText,	// 挿入するデータ
					bLinePaste? CLogicInt(0): nTextLen,	// 挿入するデータの長さ
					bRedraw,
					m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
				);
#ifdef _DEBUG
				gm_ProfileOutput = FALSE;
#endif
				if( !bLinePaste )	// 2007.10.04 ryoji
					goto end_of_func;
			}
		}
	}

	{	// 非選択時の処理 or ラインモード貼り付け時の残りの処理
		CLogicInt nPosX_PHY_Delta;
		if( bLinePaste ){	// 2007.10.04 ryoji
			/* 挿入ポイント（折り返し単位行頭）にカーソルを移動 */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	// 操作前のキャレット位置
			Command_GOLINETOP( FALSE, 1 );								// 行頭に移動(折り返し単位)
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	// 操作後のキャレット位置

			// 挿入ポイントと元の位置との差分文字数
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			//UNDO用記録
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* 操作前のキャレット位置 */
						ptCaretAfter	/* 操作後のキャレット位置 */
					)
				);
			}
		}

		// 現在位置にデータを挿入
		CLayoutPoint ptLayoutNew; //挿入された部分の次の位置
		m_pCommanderView->InsertData_CEditView(
			GetCaret().GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			bRedraw
		);

		// 挿入データの最後へカーソルを移動
		GetCaret().MoveCursor( ptLayoutNew, bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( bLinePaste ){	// 2007.10.04 ryoji
			/* 元の位置へカーソルを移動 */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	//操作前のキャレット位置
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptCaretBefore + CLogicPoint(nPosX_PHY_Delta, CLogicInt(0)),
				&ptLayout
			);
			GetCaret().MoveCursor( ptLayout, bRedraw );					//カーソル移動
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	//操作後のキャレット位置
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

			//UNDO用記録
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* 操作前のキャレット位置Ｘ */
						ptCaretAfter	/* 操作後のキャレット位置Ｘ */
					)
				);
			}
		}
	}

end_of_func:
	delete pcWaitCursor;

	return;
}

//<< 2002/03/28 Azumaiya
// メモリデータを矩形貼り付け用のデータと解釈して処理する。
//  なお、この関数は Command_PASTEBOX(void) と、
// 2769 : GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
// から、
// 3057 : m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
// 間まで、一緒です。
//  ですが、コメントを削ったり、#if 0 のところを削ったりしていますので、Command_PASTEBOX(void) は
// 残すようにしました(下にこの関数を使った使ったバージョンをコメントで書いておきました)。
//  なお、以下にあげるように Command_PASTEBOX(void) と違うところがあるので注意してください。
// > 呼び出し側が責任を持って、
// ・マウスによる範囲選択中である。
// ・現在のフォントは固定幅フォントである。
// の 2 点をチェックする。
// > 再描画を行わない
// です。
//  なお、これらを呼び出し側に期待するわけは、「すべて置換」のような何回も連続で呼び出す
// ときに、最初に一回チェックすればよいものを何回もチェックするのは無駄と判断したためです。
// @note 2004.06.30 現在、すべて置換では使用していない
void CViewCommander::Command_PASTEBOX( const wchar_t *szPaste, int nPasteSize )
{
	/* これらの動作は残しておきたいのだが、呼び出し側で責任を持ってやってもらうことに変更。
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}
	if( !GetDllShareData().m_Common.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}
	*/

	int				nBgn;
	int				nPos;
	CLayoutInt		nCount;
	CLayoutPoint	ptLayoutNew;	//挿入された部分の次の位置
	BOOL			bAddLastCR;
	CLayoutInt		nInsPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor

	// とりあえず選択範囲を削除
	// 2004.06.30 Moca m_pCommanderView->GetSelectionInfo().IsTextSelected()がないと未選択時、一文字消えてしまう
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		m_pCommanderView->DeleteData( FALSE/*TRUE 2002.01.25 hor*/ );
	}

	CLayoutPoint ptCurOld = GetCaret().GetCaretLayoutPos();

	nCount = CLayoutInt(0);

	// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無い場合の対策
	//	データの最後まで処理 i.e. nBgnがnPasteSizeを超えたら終了
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無いと
		//	最終行のPaste処理が動かないので，
		//	データの末尾に来た場合は強制的に処理するようにする
		if( szPaste[nPos] == WCODE::CR || szPaste[nPos] == WCODE::LF || nPos == nPasteSize )
		{
			/* 現在位置にデータを挿入 */
			if( nPos - nBgn > 0 ){
				m_pCommanderView->InsertData_CEditView(
					ptCurOld + CLayoutPoint(CLayoutInt(0), nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			/* この行の挿入位置へカーソルを移動 */
			GetCaret().MoveCursor( ptCurOld + CLayoutPoint(CLayoutInt(0), nCount), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* カーソル行が最後の行かつ行末に改行が無く、挿入すべきデータがまだある場合 */
			bAddLastCR = FALSE;
			const CLayout*	pcLayout;
			CLogicInt		nLineLen = CLogicInt(0);
			const wchar_t*	pLine;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

			if( NULL != pLine && 1 <= nLineLen )
			{
				if( pLine[nLineLen - 1] == WCODE::CR || pLine[nLineLen - 1] == WCODE::LF )
				{
				}
				else
				{
					bAddLastCR = TRUE;
				}
			}
			else
			{ // 2001/10/02 novice
				bAddLastCR = TRUE;
			}

			if( bAddLastCR )
			{
//				MYTRACE_A( " カーソル行が最後の行かつ行末に改行が無く、\n挿入すべきデータがまだある場合は行末に改行を挿入。\n" );
				nInsPosX = m_pCommanderView->LineIndexToColmn( pcLayout, nLineLen );

				m_pCommanderView->InsertData_CEditView(
					CLayoutPoint(nInsPosX, GetCaret().GetCaretLayoutPos().GetY2()),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2(),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetLen(),
					&ptLayoutNew,
					false
				);
			}

			if(
				(nPos + 1 < nPasteSize ) &&
				 ( szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
			  )
			{
				nBgn = nPos + 2;
			}
			else
			{
				nBgn = nPos + 1;
			}

			nPos = nBgn;
			++nCount;
		}
		else
		{
			++nPos;
		}
	}

	/* 挿入データの先頭位置へカーソルを移動 */
	GetCaret().MoveCursor( ptCurOld, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if( !m_pCommanderView->m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
	{
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe( 
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}

	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	return;
}

// 一応、Command_PASTEBOX(char *, int) を使った Command_PASTEBOX(void) を書いておきます。
/* 矩形貼り付け(クリップボードから矩形貼り付け) */
// 2004.06.29 Moca 未使用だったものを有効にする
//	オリジナルのCommand_PASTEBOX(void)はばっさり削除 (genta)
void CViewCommander::Command_PASTEBOX( int option )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}


	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}

	// クリップボードからデータを取得
	CNativeW	cmemClip;
	if( !m_pCommanderView->MyGetClipboardData( cmemClip, NULL ) ){
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0コピー対策
	int nstrlen;
	const wchar_t *lptstr = cmemClip.GetStringPtr( &nstrlen );

	Command_PASTEBOX(lptstr, nstrlen);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}

//>> 2002/03/29 Azumaiya

/*! 矩形文字列挿入
*/
void CViewCommander::Command_INSBOXTEXT( const wchar_t *pszPaste, int nPasteSize )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}

	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}

	Command_PASTEBOX(pszPaste, nPasteSize);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}



/* 範囲選択開始 */
void CViewCommander::Command_BEGIN_SELECT( void )
{
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在のカーソル位置から選択を開始する */
		m_pCommanderView->GetSelectionInfo().BeginSelectArea();
	}
	
	//	ロックの解除切り替え
	if ( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ) {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;	/* 選択状態のロック解除 */
	}
	else {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = TRUE;	/* 選択状態のロック */
	}
	if( GetSelect().IsOne() ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	}
	m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	return;
}




/* 矩形範囲選択開始 */
void CViewCommander::Command_BEGIN_BOXSELECT( void )
{
	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		return;
	}

//@@@ 2002.01.03 YAZAKI 範囲選択中にShift+F6を実行すると選択範囲がクリアされない問題に対処
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	/* 現在のカーソル位置から選択を開始する */
	m_pCommanderView->GetSelectionInfo().BeginSelectArea();

	m_pCommanderView->GetSelectionInfo().m_bSelectingLock = true;	/* 選択状態のロック */
	m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);	/* 矩形範囲選択中 */

	m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	return;
}




/* 新規作成 */
void CViewCommander::Command_FILENEW( void )
{
	/* 新たな編集ウィンドウを起動 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, NULL, false, NULL, false );
	return;
}

/* 新規作成（新しいウインドウで開く） */
void CViewCommander::Command_FILENEW_NEWWINDOW( void )
{
	/* 新たな編集ウィンドウを起動 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_DEFAULT;
	sLoadInfo.bViewMode = false;
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo,
		NULL,
		false,
		NULL,
		true
		);
	return;
}


/*! @brief ファイルを開く

	@date 2003.03.30 genta 「閉じて開く」から利用するために引数追加
	@date 2004.10.09 genta 実装をCEditDocへ移動
*/
void CViewCommander::Command_FILEOPEN( const WCHAR* filename, ECodeType nCharCode, bool bViewMode )
{
	//ロード情報
	SLoadInfo sLoadInfo(filename?to_tchar(filename):_T(""), nCharCode, bViewMode);
	std::vector<std::tstring> files;

	//必要であれば「ファイルを開く」ダイアログ
	if(!sLoadInfo.cFilePath.IsValidPath()){
		bool bDlgResult = GetDocument()->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::getInstance()->GetHwnd(),	//[in]  オーナーウィンドウ
			NULL,								//[in]  フォルダ
			&sLoadInfo,							//[out] ロード情報受け取り
			files								//[out] ファイル名
		);
		if(!bDlgResult)return;

		sLoadInfo.cFilePath = files[0].c_str();
		// 他のファイルは新規ウィンドウ
		for( size_t i = 1; i < files.size(); i++ ){
			SLoadInfo sFilesLoadInfo = sLoadInfo;
			sFilesLoadInfo.cFilePath = files[i].c_str();
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				CEditWnd::getInstance()->GetHwnd(),
				sFilesLoadInfo,
				NULL,
				true
			);
		}
	}

	//開く
	GetDocument()->m_cDocFileOperation.FileLoad( &sLoadInfo );
}




/* 閉じて(無題) */	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
void CViewCommander::Command_FILECLOSE( void )
{
	GetDocument()->m_cDocFileOperation.FileClose();
}




/*! @brief 閉じて開く

	@date 2003.03.30 genta 開くダイアログでキャンセルしたとき元のファイルが残るように。
				ついでにFILEOPENと同じように引数を追加しておく
*/
void CViewCommander::Command_FILECLOSE_OPEN( LPCWSTR filename, ECodeType nCharCode, bool bViewMode )
{
	GetDocument()->m_cDocFileOperation.FileCloseOpen( SLoadInfo(to_tchar(filename), nCharCode, bViewMode) );
}




/*! 上書き保存

	F_FILESAVEALLとの組み合わせのみで使われるコマンド．
	@param warnbeep [in] true: 保存不要 or 保存禁止のときに警告を出す
	@param askname	[in] true: ファイル名未設定の時に入力を促す

	@date 2004.02.28 genta 引数warnbeep追加
	@date 2005.01.24 genta 引数askname追加

*/
bool CViewCommander::Command_FILESAVE( bool warnbeep, bool askname )
{
	CEditDoc* pcDoc = GetDocument();

	//ファイル名が指定されていない場合は「名前を付けて保存」のフローへ遷移
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		if( !askname )
			return false;	// 保存しない
		return pcDoc->m_cDocFileOperation.FileSaveAs();
	}

	//セーブ情報
	SSaveInfo sSaveInfo;
	pcDoc->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //改行コード無変換
	sSaveInfo.bOverwriteMode = true; //上書き要求

	//上書き処理
	if(!warnbeep)CEditApp::getInstance()->m_cSoundSet.MuteOn();
	bool bRet = pcDoc->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
	if(!warnbeep)CEditApp::getInstance()->m_cSoundSet.MuteOff();

	return bRet;
}

/* 名前を付けて保存ダイアログ */
bool CViewCommander::Command_FILESAVEAS_DIALOG()
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs();
}


/* 名前を付けて保存
	filenameで保存。NULLは厳禁。
*/
BOOL CViewCommander::Command_FILESAVEAS( const WCHAR* filename, EEolType eEolType )
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs(filename, eEolType);
}

/*!	全て上書き保存

	編集中の全てのウィンドウで上書き保存を行う．
	ただし，上書き保存の指示を出すのみで実行結果の確認は行わない．

	上書き禁止及びファイル名未設定のウィンドウでは何も行わない．

	@date 2005.01.24 genta 新規作成
*/
BOOL CViewCommander::Command_FILESAVEALL( void )
{
	CAppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG( F_FILESAVE_QUIET, 0 ),
		0,
		NULL
	);
	return TRUE;
}


/*!	現在編集中のファイル名をクリップボードにコピー
	2002/2/3 aroka
*/
void CViewCommander::Command_COPYFILENAME( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* クリップボードにデータを設定 */
		const WCHAR* pszFile = to_wchar(GetDocument()->m_cDocFile.GetFileName());
		m_pCommanderView->MySetClipboardData( pszFile , wcslen( pszFile ), false );
	}
	else{
		ErrorBeep();
	}
}




/* 現在編集中のファイルのパス名をクリップボードにコピー */
void CViewCommander::Command_COPYPATH( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* クリップボードにデータを設定 */
		const TCHAR* szPath = GetDocument()->m_cDocFile.GetFilePath();
		m_pCommanderView->MySetClipboardData( szPath, _tcslen(szPath), false );
	}
	else{
		ErrorBeep();
	}
}




//	May 9, 2000 genta
/* 現在編集中のファイルのパス名とカーソル位置をクリップボードにコピー */
void CViewCommander::Command_COPYTAG( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		wchar_t	buf[ MAX_PATH + 20 ];

		CLogicPoint ptColLine;

		//	論理行番号を得る
		GetDocument()->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptColLine );

		/* クリップボードにデータを設定 */
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument()->m_cDocFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		m_pCommanderView->MySetClipboardData( buf, wcslen( buf ), false );
	}
	else{
		ErrorBeep();
	}
}



/* 小文字 */
void CViewCommander::Command_TOLOWER( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOLOWER );
	return;
}




/* 大文字 */
void CViewCommander::Command_TOUPPER( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOUPPER );
	return;
}




/* 全角→半角 */
void CViewCommander::Command_TOHANKAKU( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOHANKAKU );
	return;
}


/* 全角カタカナ→半角カタカナ */		//Aug. 29, 2002 ai
void CViewCommander::Command_TOHANKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOHANKATA );
	return;
}


/*! 半角英数→全角英数 */			//July. 30, 2001 Misaka
void CViewCommander::Command_TOZENEI( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOZENEI );
	return;
}

/*! 全角英数→半角英数 */
void CViewCommander::Command_TOHANEI( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOHANEI );
	return;
}


/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
void CViewCommander::Command_TOZENKAKUKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOZENKAKUKATA );
	return;
}




/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
void CViewCommander::Command_TOZENKAKUHIRA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TOZENKAKUHIRA );
	return;
}




/* 半角カタカナ→全角カタカナ */
void CViewCommander::Command_HANKATATOZENKAKUKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_HANKATATOZENKATA );
	return;
}




/* 半角カタカナ→全角ひらがな */
void CViewCommander::Command_HANKATATOZENKAKUHIRA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_HANKATATOZENHIRA );
	return;
}




/* TAB→空白 */
void CViewCommander::Command_TABTOSPACE( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_TABTOSPACE );
	return;
}

/* 空白→TAB */ //---- Stonee, 2001/05/27
void CViewCommander::Command_SPACETOTAB( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_SPACETOTAB );
	return;
}



//#define F_HANKATATOZENKATA	30557	/* 半角カタカナ→全角カタカナ */
//#define F_HANKATATOZENHIRA	30558	/* 半角カタカナ→全角ひらがな */




/* E-Mail(JIS→SJIS)コード変換 */
void CViewCommander::Command_CODECNV_EMAIL( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_EMAIL );
	return;
}




/* EUC→SJISコード変換 */
void CViewCommander::Command_CODECNV_EUC2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_EUC2SJIS );
	return;
}




/* Unicode→SJISコード変換 */
void CViewCommander::Command_CODECNV_UNICODE2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UNICODE2SJIS );
	return;
}




/* UnicodeBE→SJISコード変換 */
void CViewCommander::Command_CODECNV_UNICODEBE2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UNICODEBE2SJIS );
	return;
}




/* SJIS→JISコード変換 */
void CViewCommander::Command_CODECNV_SJIS2JIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2JIS );
	return;
}




/* SJIS→EUCコード変換 */
void CViewCommander::Command_CODECNV_SJIS2EUC( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2EUC );
	return;
}




/* UTF-8→SJISコード変換 */
void CViewCommander::Command_CODECNV_UTF82SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UTF82SJIS );
	return;
}




/* UTF-7→SJISコード変換 */
void CViewCommander::Command_CODECNV_UTF72SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UTF72SJIS );
	return;
}




/* SJIS→UTF-7コード変換 */
void CViewCommander::Command_CODECNV_SJIS2UTF7( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2UTF7 );
	return;
}




/* SJIS→UTF-8コード変換 */
void CViewCommander::Command_CODECNV_SJIS2UTF8( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2UTF8 );
	return;
}




/* 自動判別→SJISコード変換 */
void CViewCommander::Command_CODECNV_AUTO2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_AUTO2SJIS );
	return;
}




/* 右クリックメニュー */
void CViewCommander::Command_MENU_RBUTTON( void )
{
	int			nId;
	int			nLength;
//	HGLOBAL		hgClip;
//	char*		pszClip;
	int			i;
	/* ポップアップメニュー(右クリック) */
	nId = m_pCommanderView->CreatePopUpMenu_R();
	if( 0 == nId ){
		return;
	}
	switch( nId ){
	case IDM_COPYDICINFO:
		const TCHAR*	pszStr;
		pszStr = m_pCommanderView->m_cTipWnd.m_cInfo.GetStringPtr( &nLength );

		TCHAR*		pszWork;
		pszWork = new TCHAR[nLength + 1];
		auto_memcpy( pszWork, pszStr, nLength );
		pszWork[nLength] = _T('\0');

		// 見た目と同じように、\n を CR+LFへ変換する
		for( i = 0; i < nLength ; ++i){
			if( pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')){
				pszWork[i] =     WCODE::CR;
				pszWork[i + 1] = WCODE::LF;
			}
		}
		/* クリップボードにデータを設定 */
		m_pCommanderView->MySetClipboardData( pszWork, nLength, false );
		delete[] pszWork;

		break;

	case IDM_JUMPDICT:
		/* キーワード辞書ファイルを開く */
		if(GetDocument()->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp){		/* キーワード辞書セレクトを使用する */	// 2006.04.10 fon
			//	Feb. 17, 2007 genta 相対パスを実行ファイル基準で開くように
			m_pCommanderView->TagJumpSub(
				GetDocument()->m_cDocType.GetDocumentType()->m_KeyHelpArr[m_pCommanderView->m_cTipWnd.m_nSearchDict].m_szPath,
				CMyPoint(1, m_pCommanderView->m_cTipWnd.m_nSearchLine),
				0,
				true
			);
		}
		break;

	default:
		/* コマンドコードによる処理振り分け */
//		HandleCommand( nId, TRUE, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ),  (LPARAM)NULL );
		break;
	}
	return;
}




/* 最後にテキストを追加 */
void CViewCommander::Command_ADDTAIL(
	const wchar_t*	pszData,	//!< 追加するテキスト
	int				nDataLen	//!< 追加するテキストの長さ。文字単位。-1を指定すると、テキスト終端まで。
)
{
	//テキスト長自動計算
	if(nDataLen==-1 && pszData!=NULL)nDataLen=wcslen(pszData);

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	/*ファイルの最後に移動 */
	Command_GOFILEEND( FALSE );

	/* 現在位置にデータを挿入 */
	CLayoutPoint ptLayoutNew;	// 挿入された部分の次の位置
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	/* 挿入データの最後へカーソルを移動 */
	// Sep. 2, 2002 すなふき アンダーラインの表示が残ってしまう問題を修正
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
}



/* C/C++ヘッダファイル オープン機能 */		//Feb. 10, 2001 jepro	説明を「インクルードファイル」から変更
//BOOL CViewCommander::Command_OPENINCLUDEFILE( BOOL bCheckOnly )
BOOL CViewCommander::Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca ヘッダ・ソースのコードを統合＆削除
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		_T("C/C++ヘッダファイルのオープンに失敗しました。") );
}




/* C/C++ソースファイル オープン機能 */
//BOOL CViewCommander::Command_OPENCCPP( BOOL bCheckOnly )	//Feb. 10, 2001 JEPRO	コマンド名を若干変更
BOOL CViewCommander::Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca ヘッダ・ソースのコードを統合＆削除
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		_T("C/C++ソースファイルのオープンに失敗しました。"));
}




//From Here Feb. 10, 2001 JEPRO 追加
/* C/C++ヘッダファイルまたはソースファイル オープン機能 */
BOOL CViewCommander::Command_OPEN_HfromtoC( BOOL bCheckOnly )
{
	if ( Command_OPEN_HHPP( bCheckOnly, FALSE ) )	return TRUE;
	if ( Command_OPEN_CCPP( bCheckOnly, FALSE ) )	return TRUE;
	ErrorBeep();
	return FALSE;
// 2002/03/24 YAZAKI コードの重複を削減
// 2003.06.28 Moca コメントとして残っていたコードを削除
}



/* Base64デコードして保存 */
void CViewCommander::Command_BASE64DECODE( void )
{
	/* テキストが選択されているか */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
	CNativeW	ctextBuf;
	if( !m_pCommanderView->GetSelectedData( &ctextBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}

	/* Base64デコード */
	CMemory cmemBuf;
	bool bret = CDecode_Base64Decode().CallDecode(ctextBuf, &cmemBuf);
	if( !bret ){
		return;
	}

	/* 保存ダイアログ モーダルダイアログの表示 */
	TCHAR		szPath[_MAX_PATH] = _T("");
	if( !GetDocument()->m_cDocFileOperation.SaveFileDialog( szPath ) ){
		return;
	}

	//データ
	int nDataLen;
	const void* pData = cmemBuf.GetRawPtr(&nDataLen);

	//カキコ
	CBinaryOutputStream out(szPath);
	if(!out)goto err;
	if( nDataLen != out.Write(pData, nDataLen) )goto err;

	return;

err:
	ErrorBeep();
	ErrorMessage( m_pCommanderView->GetHwnd(), _T("ファイルの書き込みに失敗しました。\n\n%ts"), szPath );
}




/* uudecodeして保存 */
void CViewCommander::Command_UUDECODE( void )
{
	/* テキストが選択されているか */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	// 選択範囲のデータを取得 -> cmemBuf
	// 正常時はTRUE,範囲未選択の場合はFALSEを返す
	CNativeW	ctextBuf;
	if( !m_pCommanderView->GetSelectedData( &ctextBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}

	// uudecode(デコード)  ctextBuf -> cmemBin, szPath
	CMemory cmemBin;
	TCHAR szPath[_MAX_PATH]=_T("");
	CDecode_UuDecode decoder;
	if( !decoder.CallDecode(ctextBuf, &cmemBin) ){
		return;
	}
	decoder.CopyFilename( szPath );

	/* 保存ダイアログ モーダルダイアログの表示 */
	if( !GetDocument()->m_cDocFileOperation.SaveFileDialog( szPath ) ){
		return;
	}

	//データ
	int nDataLen;
	const void* pData = cmemBin.GetRawPtr(&nDataLen);

	//カキコ
	CBinaryOutputStream out(szPath);
	if( !out )goto err;
	if( nDataLen != out.Write(pData,nDataLen) )goto err;

	//完了
	return;

err:
	ErrorBeep();
	ErrorMessage( m_pCommanderView->GetHwnd(), _T("ファイルの書き込みに失敗しました。\n\n%ts"), szPath );
}




/* Oracle SQL*Plusで実行 */
void CViewCommander::Command_PLSQL_COMPILE_ON_SQLPLUS( void )
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	HWND		hwndSQLPLUS;
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];
	BOOL		bResult;

	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plusで実行します。\n\n\nOracle SQL*Plusが起動されていません。\n") );
		return;
	}
	/* テキストが変更されている場合 */
	if( GetDocument()->m_cDocEditor.IsModified() ){
		nRet = ::MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			_T("%ts\nは変更されています。 Oracle SQL*Plusで実行する前に保存しますか？"),
			GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ? GetDocument()->m_cDocFile.GetFilePath() : _T("(無題)")
		);
		switch( nRet ){
		case IDYES:
			if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
				//nBool = HandleCommand( F_FILESAVE, TRUE, 0, 0, 0, 0 );
				nBool = Command_FILESAVE();
			}else{
				//nBool = HandleCommand( F_FILESAVEAS_DIALOG, TRUE, 0, 0, 0, 0 );
				nBool = Command_FILESAVEAS_DIALOG();
			}
			if( !nBool ){
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* ファイルパスに空白が含まれている場合はダブルクォーテーションで囲む */
		//	2003.10.20 MIK コード簡略化
		if( _tcschr( GetDocument()->m_cDocFile.GetFilePath(), TCODE::SPACE ) ? TRUE : FALSE ){
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}else{
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}
		/* クリップボードにデータを設定 */
		m_pCommanderView->MySetClipboardData( szPath, _tcslen( szPath ), false );

		/* Oracle SQL*Plusをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( hwndSQLPLUS );

		/* Oracle SQL*Plusにペーストのコマンドを送る */
		DWORD_PTR	dwResult;
		bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG( 201, 0 ),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if( !bResult ){
			TopErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plusからの反応がありません。\nしばらく待ってから再び実行してください。") );
		}
	}else{
		ErrorBeep();
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("SQLをファイルに保存しないとOracle SQL*Plusで実行できません。\n") );
		return;
	}
	return;
}




/* Oracle SQL*Plusをアクティブ表示 */
void CViewCommander::Command_ACTIVATE_SQLPLUS( void )
{
	HWND		hwndSQLPLUS;
	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plusをアクティブ表示します。\n\n\nOracle SQL*Plusが起動されていません。\n") );
		return;
	}
	/* Oracle SQL*Plusをアクティブにする */
	/* アクティブにする */
	ActivateFrameWindow( hwndSQLPLUS );
	return;
}




/* ビューモード */
void CViewCommander::Command_VIEWMODE( void )
{
	//ビューモードを反転
	CAppMode::getInstance()->SetViewMode(!CAppMode::getInstance()->IsViewMode());

	// 排他制御の切り替え
	// ※ビューモード ON 時は排他制御 OFF、ビューモード OFF 時は排他制御 ON の仕様（>>data:5262）を即時反映する
	GetDocument()->m_cDocFileOperation.DoFileUnlock();	// ファイルの排他ロック解除
	GetDocument()->m_cDocLocker.CheckWritable(!CAppMode::getInstance()->IsViewMode());	// ファイル書込可能のチェック
	if( GetDocument()->m_cDocLocker.IsDocWritable() ){
		GetDocument()->m_cDocFileOperation.DoFileLock();	// ファイルの排他ロック
	}

	// 親ウィンドウのタイトルを更新
	this->GetEditWindow()->UpdateCaption();
}

/* ファイルのプロパティ */
void CViewCommander::Command_PROPERTY_FILE( void )
{
#ifdef _DEBUG
	{
		/* 全行データを返すテスト */
		wchar_t*	pDataAll;
		int		nDataAllLen;
		CRunningTimer cRunningTimer( "CViewCommander::Command_PROPERTY_FILE 全行データを返すテスト" );
		cRunningTimer.Reset();
		pDataAll = CDocReader(GetDocument()->m_cDocLineMgr).GetAllData( &nDataAllLen );
//		MYTRACE_A( "全データ取得             (%dバイト) 所要時間(ミリ秒) = %d\n", nDataAllLen, cRunningTimer.Read() );
		free( pDataAll );
		pDataAll = NULL;
//		MYTRACE_A( "全データ取得のメモリ開放 (%dバイト) 所要時間(ミリ秒) = %d\n", nDataAllLen, cRunningTimer.Read() );
	}
#endif


	CDlgProperty	cDlgProperty;
//	cDlgProperty.Create( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument() );
	cDlgProperty.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() );
	return;
}




/* 編集の全終了 */	// 2007.02.13 ryoji 追加
void CViewCommander::Command_EXITALLEDITORS( void )
{
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, 0 );
	return;
}

/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加
void CViewCommander::Command_EXITALL( void )
{
	CControlTray::TerminateApplication( GetMainWindow() );	// 2006.12.25 ryoji 引数追加
	return;
}



/* カスタムメニュー表示 */
int CViewCommander::Command_CUSTMENU( int nMenuIdx )
{
	HMENU		hMenu;
	int			nId;
	POINT		po;
	int			i;
	UINT		uFlags;

	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();
	
	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	if( nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx ){
		return 0;
	}
	if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] ){
		return 0;
	}
	hMenu = ::CreatePopupMenu();
	for( i = 0; i < GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( F_0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			// 2010.07.24 メニュー配列に入れる
			pCEditWnd->GetMenuDrawer().MyAppendMenuSep( hMenu, MF_SEPARATOR, F_0 , _T("") );
		}else{
			//	Oct. 3, 2001 genta
			WCHAR		szLabel[300];
			WCHAR		szLabel2[300];
			const WCHAR*	pszMenuLabel = szLabel2;
			FuncLookup.Funccode2Name( GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* キー */
			if( L'\0' == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				pszMenuLabel = szLabel;
			}else{
				auto_sprintf( szLabel2, LTEXT("%ls (&%hc)"),
					szLabel,
					GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i]
				);
			}
			/* 機能が利用可能か調べる */
			if( IsFuncEnable( GetDocument(), &GetDllShareData(), GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
			pCEditWnd->GetMenuDrawer().MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , pszMenuLabel, L"" );
		}
	}
	po.x = 0;
	po.y = 0;
	//2002/04/08 YAZAKI カスタムメニューもマウスカーソルの位置に表示するように変更。
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow(),
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}




//選択範囲内全行コピー
void CViewCommander::Command_COPYLINES( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* 引用符 */
		FALSE	/* 行番号を付与する */
	);
	return;
}




//選択範囲内全行引用符付きコピー
void CViewCommander::Command_COPYLINESASPASSAGE( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	m_pCommanderView->CopySelectedAllLines(
		GetDllShareData().m_Common.m_sFormat.m_szInyouKigou,	/* 引用符 */
		FALSE 									/* 行番号を付与する */
	);
	return;
}




//選択範囲内全行行番号付きコピー
void CViewCommander::Command_COPYLINESWITHLINENUMBER( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* 引用符 */
		TRUE	/* 行番号を付与する */
	);
	return;
}




////キー割り当て一覧をコピー
	//Dec. 26, 2000 JEPRO //Jan. 24, 2001 JEPRO debug version (directed by genta)
void CViewCommander::Command_CREATEKEYBINDLIST( void )
{
	CNativeW		cMemKeyList;

	CKeyBind::CreateKeyBindList(
		G_AppInstance(),
		GetDllShareData().m_Common.m_sKeyBind.m_nKeyNameArrNum,
		GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr,
		cMemKeyList,
		&GetDocument()->m_cFuncLookup,	//	Oct. 31, 2001 genta 追加
		FALSE	// 2007.02.22 ryoji 追加
	);

	// Windowsクリップボードにコピー
	//2004.02.17 Moca 関数化
	SetClipboardText( CEditWnd::getInstance()->m_cSplitterWnd.GetHwnd(), cMemKeyList.GetStringPtr(), cMemKeyList.GetStringLength() );
}


/* 印刷 */
void CViewCommander::Command_PRINT( void )
{
	// 使っていない処理を削除 2003.05.04 かろと
	Command_PRINT_PREVIEW();
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* 印刷実行 */
	pCEditWnd->m_pPrintPreview->OnPrint();
}




/* 印刷プレビュー */
void CViewCommander::Command_PRINT_PREVIEW( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* 印刷プレビューモードのオン/オフ */
	pCEditWnd->PrintPreviewModeONOFF();
	return;
}




/* 印刷のページレイアウトの設定 */
void CViewCommander::Command_PRINT_PAGESETUP( void )
{
	BOOL		bRes;
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* 印刷ページ設定 */
	bRes = pCEditWnd->OnPrintPageSetting();
	return;
}




/* ブラウズ */
void CViewCommander::Command_BROWSE( void )
{
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		ErrorBeep();
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	auto_sprintf( szURL, L"%ls", GetDocument()->m_cDocFile.GetFilePath() );
	/* URLを開く */
//	::ShellExecuteEx( NULL, L"open", szURL, NULL, NULL, SW_SHOW );

    SHELLEXECUTEINFO info; 
    info.cbSize =sizeof(info);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = GetDocument()->m_cDocFile.GetFilePath();
    info.lpParameters = NULL;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWNORMAL;
    info.hInstApp = 0;
    info.lpIDList = NULL;
    info.lpClass = NULL;
    info.hkeyClass = 0; 
    info.dwHotKey = 0;
    info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}




/* キーマクロの記録開始／終了 */
void CViewCommander::Command_RECKEYMACRO( void )
{
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro ){									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;							/* キーボードマクロを記録中のウィンドウ */
		//@@@ 2002.1.24 YAZAKI キーマクロをマクロ用フォルダに「RecKey.mac」という名で保存
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca 記録用キーマクロのフルパスをCShareData経由で取得
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			ErrorMessage( m_pCommanderView->GetHwnd(), _T("マクロファイルを作成できませんでした。\nファイル名の取得エラー nRet=%d"), nRet );
			return;
		}else{
			_tcscpy( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		int nSaveResult=CEditApp::getInstance()->m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName
		);
		if ( !nSaveResult ){
			ErrorMessage(	m_pCommanderView->GetHwnd(), _T("マクロファイルを作成できませんでした。\n\n%ts"), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
	}else{
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = GetMainWindow();;	/* キーボードマクロを記録中のウィンドウ */
		/* キーマクロのバッファをクリアする */
		//@@@ 2002.1.24 m_CKeyMacroMgrをCEditDocへ移動
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		CEditApp::getInstance()->m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	/* 親ウィンドウのタイトルを更新 */
	this->GetEditWindow()->UpdateCaption();

	/* キャレットの行桁位置を表示する */
	GetCaret().ShowCaretPosInfo();
}




/* キーマクロの保存 */
void CViewCommander::Command_SAVEKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	//	Jun. 16, 2002 genta
	if( !CEditApp::getInstance()->m_pcSMacroMgr->IsSaveOk() ){
		//	保存不可
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("保存可能なマクロがありません．キーボードマクロ以外は保存できません．") );
	}

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	_tcscpy( szPath, _T("") );
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER ) ){
		GetInidirOrExedir( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );
	}else{
		_tcscpy( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, NULL );
//	wcscat( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, L"\\" );

	/* キーボードマクロの保存 */
	//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
	//@@@ 2002.1.24 YAZAKI
	if ( !CEditApp::getInstance()->m_pcSMacroMgr->Save( STAND_KEYMACRO, G_AppInstance(), szPath ) ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("マクロファイルを作成できませんでした。\n\n%ts"), szPath );
	}
	return;
}




/* キーマクロの実行 */
void CViewCommander::Command_EXECKEYMACRO( void )
{
	//@@@ 2002.1.24 YAZAKI 記録中は終了してから実行
	if (GetDllShareData().m_sFlags.m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	/* キーボードマクロの実行 */
	//@@@ 2002.1.24 YAZAKI
	if ( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName[0] ){
		//	ファイルが保存されていたら
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		BOOL bLoadResult = CEditApp::getInstance()->m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName,
			NULL
		);
		if ( !bLoadResult ){
			ErrorMessage( m_pCommanderView->GetHwnd(), _T("ファイルを開けませんでした。\n\n%ts"), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
		else {
			//	2007.07.20 genta : flagsオプション追加
			CEditApp::getInstance()->m_pcSMacroMgr->Exec( STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0 );
		}
	}

	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();

	return;
}




/*! キーマクロの読み込み
	@date 2005/02/20 novice デフォルトの拡張子変更
 */
void CViewCommander::Command_LOADKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	const TCHAR*		pszFolder;
	_tcscpy( szPath, _T("") );
	pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( pszFolder ) ){
		GetInidirOrExedir( szInitDir, pszFolder );
	}else{
		_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
// 2005/02/20 novice デフォルトの拡張子変更
// 2005/07/13 novice 多様なマクロをサポートしているのでデフォルトは全て表示にする
		_T("*.*"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* キーボードマクロの読み込み */
	//@@@ 2002.1.24 YAZAKI 読み込みといいつつも、ファイル名をコピーするだけ。実行直前に読み込む
	_tcscpy(GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro( G_AppInstance(), m_pCommanderView->GetHwnd(), szPath );
	return;
}


/*! 名前を指定してマクロ実行
	@param pszPath	マクロのファイルパス、またはマクロのコード。
	@param pszType	種別。NULLの場合ファイル指定、それ以外の場合は言語の拡張子を指定

	@date 2008.10.23 syat 新規作成
	@date 2008.12.21 syat 引数「種別」を追加
 */
void CViewCommander::Command_EXECEXTMACRO( const WCHAR* pszPathW, const WCHAR* pszTypeW )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];	//ファイル選択ダイアログの初期フォルダ
	const TCHAR*	pszFolder;					//マクロフォルダ
	const TCHAR*	pszPath = NULL;				//第1引数をTCHAR*に変換した文字列
	const TCHAR*	pszType = NULL;				//第2引数をTCHAR*に変換した文字列
	HWND			hwndRecordingKeyMacro = NULL;

	if ( pszPathW != NULL ) {
		//to_tchar()で取得した文字列はdeleteしないこと。
		pszPath = to_tchar( pszPathW );
		pszType = to_tchar( pszTypeW );

	} else {
		// ファイルが指定されていない場合、ダイアログを表示する
		_tcscpy( szPath, _T("") );
		pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;

		if( _IS_REL_PATH( pszFolder ) ){
			GetInidirOrExedir( szInitDir, pszFolder );
		}else{
			_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
		}
		/* ファイルオープンダイアログの初期化 */
		cDlgOpenFile.Create(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	//キーマクロ記録中の場合、追加する
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow()	/* キーボードマクロを記録中のウィンドウ */
	){
		CEditApp::getInstance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, (LPARAM)pszPath, m_pCommanderView );

		//キーマクロの記録を一時停止する
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */
	}

	//古い一時マクロの退避
	CMacroManagerBase* oldMacro = CEditApp::getInstance()->m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = CEditApp::getInstance()->m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("マクロの読み込みに失敗しました。\n\n%ts"), pszPath );
	}
	else {
		CEditApp::getInstance()->m_pcSMacroMgr->Exec( TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO );
	}

	// 終わったら開放
	CEditApp::getInstance()->m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		CEditApp::getInstance()->m_pcSMacroMgr->SetTempMacro( oldMacro );
	}

	// キーマクロ記録中だった場合は再開する
	if ( hwndRecordingKeyMacro != NULL ) {
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
	}

	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();

	return;
}


//! ファイルの再オープン
void CViewCommander::Command_FILE_REOPEN(
	ECodeType	nCharCode,	//!< [in] 開き直す際の文字コード
	bool		bNoConfirm	//!< [in] ファイルが更新された場合に確認を行わ「ない」かどうか。true:確認しない false:確認する
)
{
	CEditDoc* pcDoc = GetDocument();
	if( !bNoConfirm && fexist(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocEditor.IsModified() ){
		int nDlgResult = MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\n\nこのファイルは変更されています。\n再ロードを行うと変更が失われますが、よろしいですか?"),
			pcDoc->m_cDocFile.GetFilePath()
		);
		if( IDOK == nDlgResult ){
			//継続。下へ進む
		}else{
			return; //中断
		}
	}

	// 同一ファイルの再オープン
	pcDoc->m_cDocFileOperation.ReloadCurrentFile( nCharCode );
}




/*! 外部コマンド実行ダイアログ表示
	@date 2002.02.02 YAZAKI.
*/
void CViewCommander::Command_EXECCOMMAND_DIALOG( void )
{
	CDlgExec cDlgExec;

	/* モードレスダイアログの表示 */
	if( !cDlgExec.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ){
		return;
	}

	m_pCommanderView->AddToCmdArr( cDlgExec.m_szCommand );
	const WCHAR* cmd_string = to_wchar(cDlgExec.m_szCommand);

	//HandleCommand( F_EXECMD, TRUE, (LPARAM)cmd_string, 0, 0, 0);	//	外部コマンド実行コマンドの発行
	HandleCommand( F_EXECMD, TRUE, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().m_nExecFlgOpt), 0, 0);	//	外部コマンド実行コマンドの発行	
}

//外部コマンド実行
//	Sept. 20, 2000 JEPRO  名称CMMANDをCOMMANDに変更
//	Oct. 9, 2001   genta  マクロ対応のため引数追加
//  2002.2.2       YAZAKI ダイアログ呼び出し部とコマンド実行部を分離
//void CEditView::Command_EXECCOMMAND( const char *cmd_string )
void CViewCommander::Command_EXECCOMMAND( LPCWSTR cmd_string, const int nFlgOpt)	//	2006.12.03 maru 引数の拡張
{
	//	From Here Aug. 21, 2001 genta
	//	パラメータ置換 (超暫定)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	CSakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);
	
	// 子プロセスの標準出力をリダイレクトする
	m_pCommanderView->ExecCmd( to_tchar(buf), nFlgOpt );
	//	To Here Aug. 21, 2001 genta
	return;
}






/*!	@brief 編集中の内容を別名保存

	主に編集中の一時ファイル出力などの目的に使用する．
	現在開いているファイル(m_szFilePath)には影響しない．

	@retval	TRUE 正常終了
	@retval	FALSE ファイル作成に失敗

	@author	maru
	@date	2006.12.10 maru 新規作成
*/
BOOL CViewCommander::Command_PUTFILE(
	LPCWSTR		filename,	//!< [in] filename 出力ファイル名
	ECodeType	nCharCode,	//!< [in] nCharCode 文字コード指定
							//!<  @li CODE_xxxxxxxxxx:各種文字コード
							//!<  @li CODE_AUTODETECT:現在の文字コードを維持
	int			nFlgOpt		//!< [in] nFlgOpt 動作オプション
							//!<  @li 0x01:選択範囲を出力 (非選択状態でも空ファイルを出力する)
)
{
	BOOL		bResult = TRUE;
	ECodeType	nSaveCharCode = nCharCode;
	if(filename[0] == L'\0') {
		return FALSE;
	}
	
	if(nSaveCharCode == CODE_AUTODETECT) nSaveCharCode = GetDocument()->GetDocumentEncoding();
	
	//	2007.09.08 genta CEditDoc::FileWrite()にならって砂時計カーソル
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	std::auto_ptr<CCodeBase> pcSaveCode( CCodeFactory::CreateCodeBase(nSaveCharCode,0) );

	bool bBom = false;
	switch( nSaveCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
		case CODE_UTF8:
		case CODE_CESU8:
		case CODE_UTF7:
			bBom = GetDocument()->m_cDocFile.IsBomExist();;
			break;
	}
	
	if(nFlgOpt & 0x01)
	{	/* 選択範囲を出力 */
		try
		{
			CBinaryOutputStream out(to_tchar(filename),true);

			//BOM出力
			if( bBom ){
				CMemory cmemBom;
				pcSaveCode->GetBom(&cmemBom);
				if(cmemBom.GetRawLength()>0)
					out.Write(cmemBom.GetRawPtr(),cmemBom.GetRawLength());
				else
					PleaseReportToAuthor( NULL, _T("CEditView::Command_PUTFILE/BOM Error\nSaveCharCode=%d"), nSaveCharCode );
			}

			// 選択範囲の取得 -> cMem
			CNativeW cMem;
			m_pCommanderView->GetSelectedData(&cMem, FALSE, NULL, FALSE, FALSE);

			// 書き込み時のコード変換 -> cDst
			CMemory cDst;
			pcSaveCode->UnicodeToCode(cMem, &cDst);

			//書込
			if( 0 < cDst.GetRawLength() )
				out.Write(cDst.GetRawPtr(),cDst.GetRawLength());
		}
		catch(CError_FileOpen)
		{
			WarningMessage(
				NULL,
				_T("\'%ls\'\n")
				_T("ファイルを保存できません。\n")
				_T("パスが存在しないか、他のアプリケーションで使用されている可能性があります。"),
				filename
			);
			bResult = FALSE;
		}
		catch(CError_FileWrite)
		{
			WarningMessage(
				NULL,
				_T("ファイルの書き込み中にエラーが発生しました。")
			);
			bResult = FALSE;
		}
	}
	else {	/* ファイル全体を出力 */
		HWND		hwndProgress;
		CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;
		
		if( NULL != pCEditWnd ){
			hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		}else{
			hwndProgress = NULL;
		}
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}

		// 一時ファイル出力
		EConvertResult eRet = CWriteManager().WriteFile_From_CDocLineMgr(
			GetDocument()->m_cDocLineMgr,
			SSaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EOL_NONE,
				bBom
			)
		);
		bResult = (eRet != RESULT_FAILURE);

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}
	return bResult;
}

/*!	@brief カーソル位置にファイルを挿入

	現在のカーソル位置に指定のファイルを読み込む．

	@param[in] filename 入力ファイル名
	@param[in] nCharCode 文字コード指定
		@li	CODE_xxxxxxxxxx:各種文字コード
		@li	CODE_AUTODETECT:前回文字コードもしくは自動判別の結果による
	@param[in] nFlgOpt 動作オプション（現在は未定義．0を指定のこと）

	@retval	TRUE 正常終了
	@retval	FALSE ファイルオープンに失敗

	@author	maru
	@date	2006.12.10 maru 新規作成
*/
BOOL CViewCommander::Command_INSFILE( LPCWSTR filename, ECodeType nCharCode, int nFlgOpt )
{
	CFileLoad	cfl(GetDocument()->m_cDocType.GetDocumentAttribute().m_encoding);
	CEol cEol;
	int			nLineNum = 0;

	CDlgCancel*	pcDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	BOOL		bResult = TRUE;

	if(filename[0] == L'\0') {
		return FALSE;
	}

	//	2007.09.08 genta CEditDoc::FileLoad()にならって砂時計カーソル
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// 範囲選択中なら挿入後も選択状態にするため	/* 2007.04.29 maru */
	BOOL	bBeforeTextSelected = m_pCommanderView->GetSelectionInfo().IsTextSelected();
	CLayoutPoint ptFrom;
	/*
	int	nLineFrom, nColmFrom;
	*/
	if (bBeforeTextSelected){
		ptFrom = m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom();
		/*
		nLineFrom = m_nSelectLineFrom;
		nColmFrom = m_nSelectColmFrom;
		*/
	}


	ECodeType	nSaveCharCode = nCharCode;
	if(nSaveCharCode == CODE_AUTODETECT) {
		EditInfo    fi;
		const CMRUFile  cMRU;
		if ( cMRU.GetEditInfo( to_tchar(filename), &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = GetDocument()->GetDocumentEncoding();
		}
	}
	
	/* ここまできて文字コードが決定しないならどこかおかしい */
	if( !IsValidCodeType(nSaveCharCode) ) nSaveCharCode = CODE_SJIS;
	
	try{
		// ファイルを開く
		cfl.FileOpen( to_tchar(filename), nSaveCharCode, 0 );

		/* ファイルサイズが65KBを越えたら進捗ダイアログ表示 */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				Progress_SetRange( hwndProgress, 0, 100 );
				Progress_SetPos( hwndProgress, 0);
			}
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow CError_FileRead を投げます
		CNativeW cBuf;
		while( RESULT_FAILURE != cfl.ReadLine( &cBuf, &cEol ) ){

			const wchar_t*	pLine = cBuf.GetStringPtr();
			int			nLineLen = cBuf.GetStringLength();

			++nLineNum;
			Command_INSTEXT(false, pLine, CLogicInt(nLineLen), true);

			/* 進捗ダイアログ有無 */
			if( NULL == pcDlgCancel ){
				continue;
			}
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				break;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				break;
			}
			if( 0 == ( nLineNum & 0xFF ) ){
				Progress_SetPos( hwndProgress, cfl.GetPercent() );
				m_pCommanderView->Redraw();
			}
		}
		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		WarningMessage( NULL, _T("file open error [%ls]"), filename );
		bResult = FALSE;
	}
	catch( CError_FileRead ){
		WarningMessage( NULL, _T("ファイルの読み込み中にエラーが発生しました。") );
		bResult = FALSE;
	} // 例外処理終わり

	delete pcDlgCancel;

	if (bBeforeTextSelected){	// 挿入された部分を選択状態に
		m_pCommanderView->GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				ptFrom,
				/*
				nLineFrom, nColmFrom,
				*/
				GetCaret().GetCaretLayoutPos()
				/*
				m_nCaretPosY, m_nCaretPosX
				*/
			)
		);
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
	}
	m_pCommanderView->Redraw();
	return bResult;
}


/*!
	@brief 検索で見つからないときの警告（メッセージボックス／サウンド）

	@date 2010.04.21 ryoji	新規作成（数カ所で用いられていた類似コードの共通化）
*/
void CViewCommander::AlertNotFound(HWND hwnd, PCTSTR format, ...)
{
	if( GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND
		&& m_pCommanderView->GetDrawSwitch()	// ← たぶん「全て置換」実行中判定の代用品（もとは Command_SEARCH_NEXT() の中でだけ使用されていた）
	){
		if( NULL == hwnd ){
			hwnd = m_pCommanderView->GetHwnd();
		}
		//InfoMessage(hwnd, format, __VA_ARGS__);
		va_list p;
		va_start(p, format);
		VMessageBoxF(hwnd, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, format, p);
		va_end(p);
	}else{
		DefaultBeep();
	}
}
