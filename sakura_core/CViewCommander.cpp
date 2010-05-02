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

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
// 2007.10.25 kobake CViewCommanderクラスに分離

#include "StdAfx.h"
#include "CViewCommander.h"

#include <stdlib.h>
#include <io.h>
#include <mbstring.h>
#include "sakura_rc.h"
#include "view/CEditView.h"
#include "debug/Debug.h"
#include "func/Funccode.h"
#include "debug/CRunningTimer.h"
#include "charset/charcode.h"
#include "CControlTray.h"
#include "CWaitCursor.h"
#include "window/CSplitterWnd.h"
//@@@ 2002.2.2 YAZAKI マクロはCSMacroMgrに統一
#include "macro/CSMacroMgr.h"
#include "typeprop/CDlgTypeList.h"
#include "dlg/CDlgProperty.h"
#include "dlg/CDlgCompare.h"
#include "global.h"
#include <htmlhelp.h>
#include "debug/CRunningTimer.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgAbout.h"	//Dec. 24, 2000 JEPRO 追加
#include "COpe.h"/// 2002/2/3 aroka 追加 from here
#include "COpeBlk.h"///
#include "doc/CLayout.h"///
#include "window/CEditWnd.h"///
#include "outline/CFuncInfoArr.h"
#include "CMarkMgr.h"///
#include "doc/CDocLine.h"///
#include "macro/CSMacroMgr.h"///
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "CPrintPreview.h"
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "dlg/CDlgCancel.h"
#include "dlg/CDlgTagJumpList.h"
#include "dlg/CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "COsVersionInfo.h"
#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"
#include "io/CBinaryStream.h"
#include "CEditApp.h"
#include "util/window.h"
#include "util/file.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/os.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "charset/CCodeFactory.h"
#include "io/CFileLoad.h"
#include "env/CSakuraEnvironment.h"
#include "plugin/CJackManager.h"
#include "plugin/COutlineIfObj.h"

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
			CEditApp::Instance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparam1, m_pCommanderView );
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
		if( !CEditApp::Instance()->m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, G_AppInstance(), m_pCommanderView,
			nCommandFrom & FA_NONRECORD )){
			InfoMessage(
				this->m_pCommanderView->m_hwndParent,
				_T("マクロ %d (%ts) の実行に失敗しました。"),
				nCommand - F_USERMACRO_0,
				CEditApp::Instance()->m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
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
	
	//	Jan. 10, 2005 genta コメント
	//	ここより後ではswitchの後ろでUndoを正しく登録するため，
	//	途中で処理の打ち切りを行ってはいけない
	// -------------------------------------

	switch( nCommand ){
	case F_WCHAR:	/* 文字入力 */
		{
			/* コントロールコード入力禁止 */
			if(WCODE::IsControlCode((wchar_t)lparam1)){
				ErrorBeep();
			}else{
				Command_WCHAR( (wchar_t)lparam1 );
			}
		}
		break;

	/* ファイル操作系 */
	case F_FILENEW:		Command_FILENEW();break;			/* 新規作成 */
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
	case F_PASTE:					Command_PASTE();break;					//貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:				Command_PASTEBOX();break;				//矩形貼り付け(クリップボードから矩形貼り付け)
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
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI ダイアログ呼び出しと、実行を分離
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//置換実行 @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//すべて置換実行(通常) 2002.2.8 hor 2006.04.02 かろと
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//検索マークのクリア
	case F_GREP_DIALOG:	//Grepダイアログの表示
		/* 再帰処理対策 */
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
		Command_GREP_DIALOG();
		break;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//指定行ヘジャンプダイアログの表示
	case F_JUMP:			Command_JUMP();break;							//指定行ヘジャンプ
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1 );break;	//アウトライン解析
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE );break;	//アウトライン解析(toggle) // 20060201 aroka
	case F_TAGJUMP:			Command_TAGJUMP(lparam1 != 0);break;			/* タグジャンプ機能 */ //	Apr. 03, 2003 genta 引数追加
	case F_TAGJUMP_CLOSE:	Command_TAGJUMP(true);break;					/* タグジャンプ(元ウィンドウClose) *///	Apr. 03, 2003 genta
	case F_TAGJUMPBACK:		Command_TAGJUMPBACK();break;					/* タグジャンプバック機能 */
	case F_TAGS_MAKE:		Command_TagsMake();break;						//タグファイルの作成	//@@@ 2003.04.13 MIK
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFile();break;				/* ダイレクトタグジャンプ機能 */	//@@@ 2003.04.15 MIK
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
	case F_CHGMOD_EOL:		Command_CHGMOD_EOL( (enumEOLType)lparam1 );break;	//入力する改行コードを設定
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
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
		Command_EXECKEYMACRO();break;
	case F_EXECEXTMACRO:
		/* 再帰処理対策 */
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
		/* 名前を指定してマクロ実行 */
		Command_EXECEXTMACRO( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* 外部コマンド実行 */
	case F_EXECMD_DIALOG:
		/* 再帰処理対策 */// 2001/06/23 N.Nakatani
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
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
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
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
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
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
		if( NULL != GetOpeBlk() ){	/* 操作ブロック */
			ClearOpeBlk();
		}
		Command_MENU_ALLFUNC();break;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* 外部ヘルプ１ */
	case F_EXTHTMLHELP:	/* 外部HTMLヘルプ */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	case F_ABOUT:	Command_ABOUT();break;				/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */
//	case F_SENDMAIL:	Command_SENDMAIL();break;		/* メール送信 */

	default:
		//プラグインコマンドを実行する
		{
			CPlug::Array plugs;
			CJackManager::Instance()->GetUsablePlug( PP_COMMAND, nCommand, &plugs );

			if( plugs.size() > 0 ){
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
	if( NULL != GetOpeBlk() ){
		if( 0 < GetOpeBlk()->GetNum() ){	/* 操作の数を返す */
			/* 操作の追加 */
			GetDocument()->m_cDocEditor.m_cOpeBuf.AppendOpeBlk( GetOpeBlk() );

			if( GetDocument()->m_cDocEditor.m_cOpeBuf.GetCurrentPointer() == 1 )	// 全Undo状態からの変更か？	// 2009.03.26 ryoji
				m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER, false );	// 自ペインの行番号（変更行）表示を更新 ← 変更行のみの表示更新で済ませている場合があるため

			if( !GetEditWindow()->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
				GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	他のペインの表示を更新
		}
		else{
			delete GetOpeBlk();
		}
		SetOpeBlk(NULL);
	}

	return bRet;
}



/////////////////////////////////// 以下はコマンド群 (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

/*! カーソル上移動 */
int CViewCommander::Command_UP( bool bSelect, bool bRepeat, int lines )
{
	//	From Here Oct. 24, 2001 genta
	if( lines != 0 ){
		GetCaret().Cursor_UPDOWN( CLayoutInt(lines), FALSE );
		return 1;
	}
	//	To Here Oct. 24, 2001 genta


	int		nRepeat = 0;

	/* キーリピート時のスクロールを滑らかにするか */
	if( !GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		CLayoutInt i;
		if( !bRepeat ){
			i = CLayoutInt(-1);
		}else{
			i = -1 * GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* キーリピート時のスクロール行数 */
		}
		GetCaret().Cursor_UPDOWN( i, bSelect );
		nRepeat = -1 * (Int)i;
	}
	else{
		++nRepeat;
		if( GetCaret().Cursor_UPDOWN( CLayoutInt(-1), bSelect )!=0 && bRepeat ){
			for( int i = 0; i < GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){		/* キーリピート時のスクロール行数 */
				::UpdateWindow( m_pCommanderView->GetHwnd() );	//	YAZAKI
				GetCaret().Cursor_UPDOWN( CLayoutInt(-1), bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}




/* カーソル下移動 */
int CViewCommander::Command_DOWN( bool bSelect, bool bRepeat )
{
	int		nRepeat;
	nRepeat = 0;
	/* キーリピート時のスクロールを滑らかにするか */
	if( !GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		CLayoutInt i;
		if( !bRepeat ){
			i = CLayoutInt(1);
		}else{
			i = GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* キーリピート時のスクロール行数 */
		}
		GetCaret().Cursor_UPDOWN( i, bSelect );
		nRepeat = (Int)i;
	}else{
		++nRepeat;
		if( GetCaret().Cursor_UPDOWN(CLayoutInt(1),bSelect)!=0 && bRepeat ){
			for( int i = 0; i < GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){	/* キーリピート時のスクロール行数 */
				//	ここで再描画。
				::UpdateWindow( m_pCommanderView->GetHwnd() );	//	YAZAKI
				GetCaret().Cursor_UPDOWN( CLayoutInt(1), bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}




/*! @brief カーソル左移動

	@date 2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正．
			pcLayout == NULLかつキャレット位置が行頭以外の場合は
			2つのifのどちらにも当てはまらないが，そのあとのMoveCursorにて適正な
			位置に移動させられる．
*/
int CViewCommander::Command_LEFT( bool bSelect, bool bRepeat )
{
	int		nRepCount;
	int		nRepeat;
	int		nRes;
	if( bRepeat ){
		nRepeat = 2;
	}else{
		nRepeat = 1;
	}
	for( nRepCount = 0; nRepCount < nRepeat; ++nRepCount ){
		CLayoutPoint ptPos(CLayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2());
		const CLayout*	pcLayout;
		if( bSelect ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在のカーソル位置から選択を開始する */
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
		}else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 矩形範囲選択中か */
				if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
					/* 2点を対角とする矩形を求める */
					CLayoutRange rcSel;
					TwoPointToRange(
						&rcSel,
						GetSelect().GetFrom(),	// 範囲選択開始
						GetSelect().GetTo()		// 範囲選択終了
					);
					/* 現在の選択範囲を非選択状態に戻す */
					m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
					/* カーソルを選択開始位置に移動 */
					GetCaret().MoveCursor( rcSel.GetFrom(), TRUE );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
				else{
					ptPos = GetSelect().GetFrom();
					/* 現在の選択範囲を非選択状態に戻す */
					m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
					/* カーソルを選択開始位置に移動 */
					GetCaret().MoveCursor( ptPos, TRUE );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
				nRes = 1;
				goto end_of_func;
			}
		}
		/* 現在行のデータを取得 */
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		/* カーソルが左端にある */
		if( GetCaret().GetCaretLayoutPos().GetX2() == (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0))){
			if( GetCaret().GetCaretLayoutPos().GetY2() > 0 ){
				pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() - CLayoutInt(1) );
				CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
				while( !it.end() ){
					it.scanNext();
					if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			}else{
				nRes = 0;
				goto end_of_func;
			}
		}
		//  2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正
		else if( pcLayout ){
			CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > GetCaret().GetCaretLayoutPos().GetX2() - 1 ){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			//	Oct. 18, 2002 YAZAKI
			if( it.getIndex() >= pcLayout->GetLengthWithEOL() ){
				ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() - CLayoutInt(1);
			}
		}
		GetCaret().MoveCursor( ptPos, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/*	現在のカーソル位置によって選択範囲を変更．
			
				2004.04.02 Moca 
				キャレット位置が不正だった場合にMoveCursorの移動結果が
				引数で与えた座標とは異なることがあるため，
				ptPosの代わりに実際の移動結果を使うように．
			*/
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}
		nRes = 1;
		goto end_of_func;
end_of_func:;
	}
	return nRes;
}




/* カーソル右移動 */
void CViewCommander::Command_RIGHT( bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat )
{
	int nRepeat; //押した回数
	if( bRepeat ){
		nRepeat = 2;
	}else{
		nRepeat = 1;
	}
	for( int nRepCount = 0; nRepCount < nRepeat; ++nRepCount ){
		CLayoutPoint ptPos;
		ptPos.y = GetCaret().GetCaretLayoutPos().GetY2();
		const CLayout*	pcLayout;

		// 2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に
		// 対応するため、現在行のデータを取得を移動
		if( !bIgnoreCurrentSelection ){
			if( bSelect ){
				if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
					/* 現在のカーソル位置から選択を開始する */
					m_pCommanderView->GetSelectionInfo().BeginSelectArea();
				}
			}
			else{
				if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
					/* 矩形範囲選択中か */
					if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
						/* 2点を対角とする矩形を求める */
						CLayoutRange rcSel;
						TwoPointToRange(
							&rcSel,
							GetSelect().GetFrom(),	// 範囲選択開始
							GetSelect().GetTo()		// 範囲選択終了
						);
						
						/* 現在の選択範囲を非選択状態に戻す */
						m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

						/* カーソルを選択終了位置に移動 */
						GetCaret().MoveCursor( rcSel.GetFrom(), TRUE );
						GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
					}
					else{
						ptPos = GetSelect().GetTo();

						/* 現在の選択範囲を非選択状態に戻す */
						m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
						if( ptPos.y >= GetDocument()->m_cLayoutMgr.GetLineCount() ){
							/* ファイルの最後に移動 */
							Command_GOFILEEND(FALSE);
						}
						else{
							/* カーソルを選択終了位置に移動 */
							GetCaret().MoveCursor( ptPos, TRUE );
							GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
						}
					}
					goto end_of_func;
				}
			}
		}
//		2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に対応

		/* 現在行のデータを取得 */
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		//	2004.04.02 EOF以降にカーソルがあったときに右を押しても何も起きなかったのを、EOFに移動するように
		if( pcLayout )
		{
			int nIndex = 0;
			CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() > GetCaret().GetCaretLayoutPos().GetX2() ){
					break;
				}
				if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
					nIndex += it.getIndexDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x = it.getColumn(); //新しいキャレット位置
			nIndex += it.getIndex();
			if( nIndex >= pcLayout->GetLengthWithEOL() ){
				/* フリーカーソルモードか */
				if( (
					GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode
				 || m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().IsBoxSelecting()	/* 矩形範囲選択中 */
					)
				 &&
					/* 改行で終わっているか */
					( EOL_NONE != pcLayout->GetLayoutEol() )
				){
					/*-- フリーカーソルモードの場合 --*/
					if( ptPos.x <= GetCaret().GetCaretLayoutPos().GetX2() ){
						/* 最終行か */
						if( GetCaret().GetCaretLayoutPos().GetY2() + 1 == GetDocument()->m_cLayoutMgr.GetLineCount() ){
							/* 改行で終わっているか */
							if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){
								ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() + 1;
							}else{
								ptPos.x = GetCaret().GetCaretLayoutPos().GetX2();
							}
						}else{
							ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() + 1;
						}
					}else{
						ptPos.x = ptPos.x;
					}
				}
				else{
					/*-- フリーカーソルモードではない場合 --*/
					/* 最終行か */
					if( GetCaret().GetCaretLayoutPos().GetY2() + 1 == GetDocument()->m_cLayoutMgr.GetLineCount() ){
						/* 改行で終わっているか */
						if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){
							ptPos.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
							++ptPos.y;
						}
						else{
						}
					}
					else{
						if( ptPos.x <= GetCaret().GetCaretLayoutPos().GetX2()
							&& EOL_NONE == pcLayout->GetLayoutEol()
							&& pcLayout->GetNextLayout()
						){
							nRepeat++;	// レイアウト行の右端は次の行の先頭と論理的に同等なのでさらに右へ	// 2007.02.19 ryoji
						}
						ptPos.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
						++ptPos.y;
					}
				}
				//	キャレット位置が折り返し位置より右側だった場合の処理
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( ptPos.x >= GetDocument()->m_cLayoutMgr.GetMaxLineKetas() ){
					if( GetCaret().GetCaretLayoutPos().GetX2() >= GetDocument()->m_cLayoutMgr.GetMaxLineKetas()
						&& pcLayout->GetNextLayout()
					){
						nRepeat++;	// レイアウト行の右端は次の行の先頭と論理的に同等なのでさらに右へ	// 2007.02.19 ryoji
					}
					ptPos.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
					++ptPos.y;
				}
			}
		}else{
			// pcLayoutがNULLの場合はptPos.x=0に調整
			ptPos.x = CLayoutInt(0);
		}
		GetCaret().MoveCursor( ptPos, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}

end_of_func:;
	}
	return;
}




//	From Here Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL/UP/DOWN→PAGE/DOWN/UP)するために以下をコメントアウト
///* スクロールアップ */
//void CViewCommander::Command_ROLLUP( bool bSelect )
//{
//	GetCaret().Cursor_UPDOWN( ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
//	return;
//}
//
//
//
//
///* スクロールダウン */
//void CViewCommander::Command_ROLLDOWN( bool bSelect )
//{
//	GetCaret().Cursor_UPDOWN( - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
//	return;
//}
//	To Here Oct. 6, 2000




/* 半ページアップ */	//Oct. 6, 2000 JEPRO added (実は従来のスクロールダウンそのもの)
void CViewCommander::Command_HalfPageUp( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
	return;
}




/* 半ページダウン */	//Oct. 6, 2000 JEPRO added (実は従来のスクロールアップそのもの)
void CViewCommander::Command_HalfPageDown( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
	return;
}




/*! １ページアップ

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページアップに動作変更
*/	//Oct. 10, 2000 JEPRO added
void CViewCommander::Command_1PageUp( bool bSelect )
{
//	GetCaret().Cursor_UPDOWN( - m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページアップ
	if(m_pCommanderView->GetTextArea().GetViewTopLine()>=m_pCommanderView->GetTextArea().m_nViewRowNum-1){
		m_pCommanderView->SetDrawSwitch(false);
		CLayoutInt nViewTopLine=GetCaret().GetCaretLayoutPos().GetY2()-m_pCommanderView->GetTextArea().GetViewTopLine();
		GetCaret().Cursor_UPDOWN( -m_pCommanderView->GetTextArea().m_nViewRowNum+1, bSelect );
		//	Sep. 11, 2004 genta 同期スクロール処理のため
		//	m_pCommanderView->RedrawAllではなくScrollAtを使うように
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( GetCaret().GetCaretLayoutPos().GetY2()-nViewTopLine ));
		m_pCommanderView->SetDrawSwitch(true);
		m_pCommanderView->RedrawAll();
		
	}else{
		GetCaret().Cursor_UPDOWN( -m_pCommanderView->GetTextArea().m_nViewRowNum+1, bSelect );
	}
	return;
}




/*!	１ページダウン

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページダウンに動作変更
*/
void CViewCommander::Command_1PageDown( bool bSelect )
{
//	GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページダウン
	if(m_pCommanderView->GetTextArea().GetViewTopLine()+m_pCommanderView->GetTextArea().m_nViewRowNum <= GetDocument()->m_cLayoutMgr.GetLineCount() ){ //- m_pCommanderView->GetTextArea().m_nViewRowNum){
		m_pCommanderView->SetDrawSwitch(false);
		CLayoutInt nViewTopLine=GetCaret().GetCaretLayoutPos().GetY2()-m_pCommanderView->GetTextArea().GetViewTopLine();
		GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum-1, bSelect );
		//	Sep. 11, 2004 genta 同期スクロール処理のため
		//	m_pCommanderView->RedrawAllではなくScrollAtを使うように
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( GetCaret().GetCaretLayoutPos().GetY2()-nViewTopLine ));
		m_pCommanderView->SetDrawSwitch(true);
		m_pCommanderView->RedrawAll();
	}else{
		GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum , bSelect );
		Command_DOWN( bSelect, TRUE );
	}

	return;
}




/* カーソル上移動(２行づつ) */
void CViewCommander::Command_UP2( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( CLayoutInt(-2), bSelect );
	return;
}




/* カーソル下移動(２行づつ) */
void CViewCommander::Command_DOWN2( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( CLayoutInt(2), bSelect );
	return;
}




/*! @brief 行頭に移動

	@date Oct. 29, 2001 genta マクロ用機能拡張(パラメータ追加) + goto排除
	@date May. 15, 2002 oak   改行単位移動
	@date Oct.  7, 2002 YAZAKI 冗長な引数 bLineTopOnly を削除
	@date Jun. 18, 2007 maru 行頭判定に全角空白のインデント設定も考慮する
*/
void CViewCommander::Command_GOLINETOP(
	bool	bSelect,	//!< [in] 選択の有無。true: 選択しながら移動。false: 選択しないで移動。
	int		lparam		/*!< [in] マクロから使用する拡張フラグ
								  @li 0: キー操作と同一(default)
								  @li 1: カーソル位置に関係なく行頭に移動。
								  @li 4: 選択して移動(合成可)
								  @li 8: 改行単位で先頭に移動(合成可)
						*/
)
{
	using namespace WCODE;

	// lparamの解釈
	bool	bLineTopOnly = ((lparam & 1) != 0);
	if( lparam & 4 ){
		bSelect = true;
	}


	CLayoutPoint ptCaretPos;
	if ( lparam & 8 ){
		/* 改行単位指定の場合は、物理行頭位置から目的論理位置を求める */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0,GetCaret().GetCaretLogicPos().y),
			&ptCaretPos
		);
	}
	else{
		const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		ptCaretPos.x = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
		ptCaretPos.y = GetCaret().GetCaretLayoutPos().GetY2();
	}
	if( !bLineTopOnly ){
		/* 目的行のデータを取得 */
		/* 改行単位指定で、先頭から空白が1折り返し行以上続いている場合は次の行データを取得 */
		CLayoutInt nPosY_Layout;
		CLogicInt  nPosX_Logic;

		nPosY_Layout = ptCaretPos.y - 1;
		const CLayout*	pcLayout;
		bool			bZenSpace = GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
		
		CLogicInt		nLineLen;
		do {
			++nPosY_Layout;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( nPosY_Layout, &nLineLen, &pcLayout );
			if( !pLine ){
				return;
			}
			for( nPosX_Logic = 0; nPosX_Logic < nLineLen; ++nPosX_Logic ){
				if(WCODE::IsIndentChar(pLine[nPosX_Logic],bZenSpace!=0))continue;
				
				if(WCODE::IsLineDelimiter(pLine[nPosX_Logic]) ){
					nPosX_Logic = 0;	// 空白またはタブおよび改行だけの行だった
				}
				break;
			}
		}
		while (( lparam & 8 ) && (nPosX_Logic >= nLineLen) && (GetDocument()->m_cLayoutMgr.GetLineCount() - 1 > nPosY_Layout) );
		
		if( nPosX_Logic >= nLineLen ){
			/* 折り返し単位の行頭を探して物理行末まで到達した
			または、最終行のため改行コードに遭遇せずに行末に到達した */
			nPosX_Logic = 0;
		}
		
		if(0 == nPosX_Logic) nPosY_Layout = ptCaretPos.y;	/* 物理行の移動なし */
		
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		CLayoutInt nPosX_Layout = m_pCommanderView->LineIndexToColmn( pcLayout, nPosX_Logic );
		CLayoutPoint ptPos(nPosX_Layout, nPosY_Layout);
		if( GetCaret().GetCaretLayoutPos() != ptPos ){
			ptCaretPos = ptPos;
		}
	}

	//	2006.07.09 genta 新規関数にまとめた
	m_pCommanderView->MoveCursorSelecting( ptCaretPos, bSelect );
}




// 行末に移動(折り返し単位)
void CViewCommander::Command_GOLINEEND( bool bSelect, int bIgnoreCurrentSelection )
{
	if( !bIgnoreCurrentSelection ){
		if( bSelect ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在のカーソル位置から選択を開始する */
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
		}else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
			}
		}
	}

	// 現在行のデータから、そのレイアウト幅を取得
	CLayoutPoint	nPosXY = GetCaret().GetCaretLayoutPos();
	nPosXY.x = CLayoutInt(0);
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nPosXY.y );
	if(pcLayout)
		nPosXY.x = pcLayout->GetIndent() + pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr);

	// キャレット移動
	GetCaret().MoveCursor( nPosXY, true );
	GetCaret().m_nCaretPosX_Prev = nPosXY.x;
	if( bSelect ){
		// 現在のカーソル位置によって選択範囲を変更
		m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( nPosXY );
	}
}




/* ファイルの先頭に移動 */
void CViewCommander::Command_GOFILETOP( bool bSelect )
{
	/* 先頭へカーソルを移動 */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();

	//	2006.07.09 genta 新規関数にまとめた
	CLayoutPoint pt(
		!m_pCommanderView->GetSelectionInfo().IsBoxSelecting()? CLayoutInt(0): GetCaret().GetCaretLayoutPos().GetX2(),
		CLayoutInt(0)
	);
	m_pCommanderView->MoveCursorSelecting( pt, bSelect );	//	通常は、(0, 0)へ移動。ボックス選択中は、(GetCaret().GetCaretLayoutPos().GetX2(), 0)へ移動
}




/* ファイルの最後に移動 */
void CViewCommander::Command_GOFILEEND( bool bSelect )
{
// 2001.12.13 hor BOX選択中にファイルの最後にジャンプすると[EOF]の行が反転したままになるの修正
	if( !bSelect && m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );	// 2001.12.21 hor Add
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().Cursor_UPDOWN( GetDocument()->m_cLayoutMgr.GetLineCount() , bSelect );
	Command_DOWN( bSelect, TRUE );
	if ( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			改行のない最終行で選択肢ながら文書末へ移動した場合に
			選択範囲が正しくない場合がある問題に対応
		*/
		Command_GOLINEEND( bSelect, FALSE );				// 2001.12.21 hor Add
	}
	GetCaret().MoveCursor( GetCaret().GetCaretLayoutPos(), TRUE );	// 2001.12.21 hor Add
	// 2002.02.16 hor 矩形選択中を除き直前のカーソル位置をリセット
	if( !(m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().IsBoxSelecting()) ) GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	// 選択範囲情報メッセージを表示する	// 2009.05.06 ryoji 追加
	if( bSelect ){
		m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	}
}




/* 単語の左端に移動 */
void CViewCommander::Command_WORDLEFT( bool bSelect )
{
	CLogicInt		nIdx;
	if( bSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){		/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}

	const CLayout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		bool bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* フリーカーソルモードか */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* カーソル左移動 */
		Command_LEFT( bSelect, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* フリーカーソルモードか */
		return;
	}

	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* 現在位置の左の単語の先頭位置を調べる */
	CLayoutPoint ptLayoutNew;
	int nResult=GetDocument()->m_cLayoutMgr.PrevWord(
		GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&ptLayoutNew,
		GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* 行が変わった */
		if( ptLayoutNew.y != GetCaret().GetCaretLayoutPos().GetY2() ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptLayoutNew.GetY2() );
			if( NULL == pcLayout ){
				return;
			}
		}

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColmn( pcLayout, ptLayoutNew.x );
		*/

		/* カーソル移動 */
		GetCaret().MoveCursor( ptLayoutNew, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptLayoutNew );
		}
	}else{
		bool bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* フリーカーソルモードか */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* カーソル左移動 */
		Command_LEFT( bSelect, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* フリーカーソルモードか */
	}
	return;
}




/* 単語の右端に移動 */
void CViewCommander::Command_WORDRIGHT( bool bSelect )
{
	CLogicInt	nIdx;
	CLayoutInt	nCurLine;
	if( bSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	bool	bTryAgain = false;
try_again:;
	nCurLine = GetCaret().GetCaretLayoutPos().GetY2();
	const CLayout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nCurLine );
	if( NULL == pcLayout ){
		return;
	}
	if( bTryAgain ){
		const wchar_t*	pLine = pcLayout->GetPtr();
		if( pLine[0] != L' ' && pLine[0] != WCODE::TAB ){
			return;
		}
	}
	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* 現在位置の右の単語の先頭位置を調べる */
	CLayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* 行が変わった */
		if( ptLayoutNew.y != nCurLine ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptLayoutNew.GetY2() );
			if( NULL == pcLayout ){
				return;
			}
		}
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColmn( pcLayout, ptLayoutNew.x );
		*/
		// カーソル移動
		GetCaret().MoveCursor( ptLayoutNew, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptLayoutNew );
		}
	}
	else{
		bool	bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* フリーカーソルモードか */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* カーソル右移動 */
		Command_RIGHT( bSelect, false, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* フリーカーソルモードか */
		if( !bTryAgain ){
			bTryAgain = true;
			goto try_again;
		}
	}
	return;
}



/**	選択範囲をクリップボードにコピー
 	@param bIgnoreLockAndDisable [in] 選択範囲を解除するか？
 	@param bAddCRLFWhenCopy [in] 折り返し位置に改行コードを挿入するか？
 	@param neweol [in] コピーするときのEOL。

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CViewCommander::Command_COPY(
	bool		bIgnoreLockAndDisable,	//!< [in] 選択範囲を解除するか？
	bool		bAddCRLFWhenCopy,		//!< [in] 折り返し位置に改行コードを挿入するか？
	enumEOLType	neweol					//!< [in] コピーするときのEOL。
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




//カーソル位置または選択エリアを削除
void CViewCommander::Command_DELETE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){		/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		// 2008.08.03 nasukoji	選択範囲なしでDELETEを実行した場合、カーソル位置まで半角スペースを挿入した後改行を削除して次行と連結する
		if( GetDocument()->m_cLayoutMgr.GetLineCount() > GetCaret().GetCaretLayoutPos().GetY2() ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIndex;
				nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen != 0 ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){	// 行終端は改行コードか?
						Command_INSTEXT( TRUE, L"", CLogicInt(0), FALSE );	// カーソル位置まで半角スペース挿入
					}else{	// 行終端が折り返し
						// 折り返し行末ではスペース挿入後、次の文字を削除する	// 2009.02.19 ryoji

						// フリーカーソル時の折り返し越え位置での削除はどうするのが妥当かよくわからないが
						// 非フリーカーソル時（ちょうどカーソルが折り返し位置にある）には次の行の先頭文字を削除したい

						if( nLineLen < GetCaret().GetCaretLayoutPos().GetX2() ){	// 折り返し行末とカーソルの間に隙間がある
							Command_INSTEXT( TRUE, L"", CLogicInt(0), FALSE );	// カーソル位置まで半角スペース挿入
							pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
							nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
						}
						if( nLineLen != 0 ){	// （スペース挿入後も）折り返し行末なら次文字を削除するために次行の先頭に移動する必要がある
							if( pcLayout->GetNextLayout() != NULL ){	// 最終行末ではない
								CLayoutPoint ptLay;
								CLogicPoint ptLog(pcLayout->GetLogicOffset() + nIndex, pcLayout->GetLogicLineNo());
								GetDocument()->m_cLayoutMgr.LogicToLayout( ptLog, &ptLay );
								GetCaret().MoveCursor( ptLay, TRUE );
								GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
							}
						}
					}
				}
			}
		}
	}
	m_pCommanderView->DeleteData( TRUE );
	return;
}




//カーソル前を削除
void CViewCommander::Command_DELETE_BACK( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//	May 29, 2004 genta 実際に削除された文字がないときはフラグをたてないように
	//GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){				/* テキストが選択されているか */
		m_pCommanderView->DeleteData( TRUE );
	}
	else{
		CLayoutPoint	ptLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		CLogicPoint		ptLogicPos_Old = GetCaret().GetCaretLogicPos();
		BOOL	bBool = Command_LEFT( FALSE, FALSE );
		if( bBool ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIdx = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen == 0 ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					// 右からの移動では折り返し末尾文字は削除するが改行は削除しない
					// 下から（下の行の行頭から）の移動では改行も削除する
					if( nIdx < pcLayout->GetLengthWithoutEOL() || GetCaret().GetCaretLayoutPos().GetY2() < ptLayoutPos_Old.GetY2() ){
						if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
							/* 操作の追加 */
							GetOpeBlk()->AppendOpe(
								new CMoveCaretOpe(
									ptLogicPos_Old,
									GetCaret().GetCaretLogicPos()
								)
							);
						}
						m_pCommanderView->DeleteData( TRUE );
					}
				}
			}
		}
	}
	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}




//単語の右端まで削除
void CViewCommander::Command_WordDeleteToEnd( void )
{
	CMemory	cmemData;

	/* 矩形選択状態では実行不能((★★もろ手抜き★★)) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}
	/* 単語の右端に移動 */
	CViewCommander::Command_WORDRIGHT( TRUE );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		CMoveCaretOpe*	pcOpe = new CMoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe( pcOpe );
	}
	/* 削除 */
	m_pCommanderView->DeleteData( TRUE );
}




//単語の左端まで削除
void CViewCommander::Command_WordDeleteToStart( void )
{
	/* 矩形選択状態では実行不能(★★もろ手抜き★★) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}

	// 単語の左端に移動
	CViewCommander::Command_WORDLEFT( TRUE );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		//$$ 動作上は問題が無いらしいですが。
		//   ここの COpe だけ、意味付け(EOpeCode指定)がされていないので、
		//   せめて何らかの名前付けをしてあげないと、意図が読み取れません。
		COpe*	pcOpe = new COpe;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置

		// 操作の追加
		GetOpeBlk()->AppendOpe( pcOpe );
	}

	// 削除
	m_pCommanderView->DeleteData( TRUE );
}




//単語切り取り
void CViewCommander::Command_WordCut( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	//現在位置の単語選択
	Command_SELECTWORD();
	/* 切り取り(選択範囲をクリップボードにコピーして削除) */
	if ( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		//	単語選択で選択できなかったら、次の文字を選ぶことに挑戦。
		Command_RIGHT( TRUE, FALSE, FALSE );
	}
	Command_CUT();
	return;
}




//単語削除
void CViewCommander::Command_WordDelete( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 削除 */
		m_pCommanderView->DeleteData( TRUE );
		return;
	}
	//現在位置の単語選択
	Command_SELECTWORD();
	/* 削除 */
	m_pCommanderView->DeleteData( TRUE );
	return;
}




//行頭まで切り取り(改行単位)
void CViewCommander::Command_LineCutToStart( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*切り取り(選択範囲をクリップボードにコピーして削除) */
	Command_CUT();
}




//行末まで切り取り(改行単位)
void CViewCommander::Command_LineCutToEnd( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* 改行コードの種類 */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}
	else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange(GetCaret().GetCaretLayoutPos(),ptPos);
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*切り取り(選択範囲をクリップボードにコピーして削除) */
	Command_CUT();
}




//行頭まで削除(改行単位)
void CViewCommander::Command_LineDeleteToStart( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		m_pCommanderView->DeleteData( TRUE );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* 選択領域削除 */
	m_pCommanderView->DeleteData( TRUE );
}




//行末まで削除(改行単位)
void CViewCommander::Command_LineDeleteToEnd( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		m_pCommanderView->DeleteData( TRUE );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* 改行コードの種類 */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	CLayoutRange sRange( GetCaret().GetCaretLayoutPos(), ptPos );
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* 選択領域削除 */
	m_pCommanderView->DeleteData( TRUE );
}




//行切り取り(折り返し単位)
void CViewCommander::Command_CUT_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		ErrorBeep();
		return;
	}

	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().y );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji 処理簡素化
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy,
		EOL_UNKNOWN,
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}




/* 行削除(折り返し単位) */
void CViewCommander::Command_DELETE_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	const CLayout*	pcLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		ErrorBeep();
		return;
	}
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2()    ));	//範囲選択開始位置
	GetSelect().SetTo  (CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2() + 1));	//範囲選択終了位置

	CLayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL != pcLayout ){
		// 2003-04-30 かろと
		// 行削除した後、フリーカーソルでないのにカーソル位置が行端より右になる不具合対応
		// フリーカーソルモードでない場合は、カーソル位置を調整する
		if( !GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode ) {
			CLogicInt nIndex;

			CLayoutInt tmp;
			nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, ptCaretPos_OLD.GetX2(), &tmp );
			ptCaretPos_OLD.x=tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			} else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColmn( pcLayout, nIndex );
			}
		}
		/* 操作前の位置へカーソルを移動 */
		GetCaret().MoveCursor( ptCaretPos_OLD, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
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
void CViewCommander::Command_PASTE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//砂時計
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	BOOL		bBox;
	int			i;

	// クリップボードからデータを取得 -> cmemClip, bColmnSelect
	CNativeW	cmemClip;
	bool		bColmnSelect;
	bool		bLineSelect = false;
	if( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColmnSelect, GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: NULL ) ){
		ErrorBeep();
		return;
	}

	// 矩形コピーのテキストは常に矩形貼り付け
	if( GetDllShareData().m_Common.m_sEdit.m_bAutoColmnPaste ){
		// 矩形コピーのデータなら矩形貼り付け
		if( bColmnSelect ){
			Command_PASTEBOX();
			return;
		}
	}

	// クリップボードデータ取得 -> pszText, nTextLen
	CLogicInt		nTextLen;
	const wchar_t*	pszText = cmemClip.GetStringPtr( &nTextLen );

	// 2007.10.04 ryoji
	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if( bLineSelect ){
		// ※CRやLFは2バイト文字の2バイト目として扱われることはないので末尾だけで判定（CMemory::GetSizeOfChar()参照）
		if( pszText[nTextLen - 1] != WCODE::CR && pszText[nTextLen - 1] != WCODE::LF ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	// テキストが選択されているか
	bBox = FALSE;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// 矩形範囲選択中か
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			bBox = TRUE;
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
					/*
					pszText,				// 挿入するデータ
					nTextLen,				// 挿入するデータの長さ
					*/
					bLineSelect? L"": pszText,	// 挿入するデータ
					bLineSelect? CLogicInt(0): nTextLen,	// 挿入するデータの長さ
					true,
					m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
				);
#ifdef _DEBUG
				gm_ProfileOutput = FALSE;
#endif
				if( !bLineSelect )	// 2007.10.04 ryoji
					return;
			}
		}
	}
	if( bBox ){
		//改行までを抜き出す pszText -> szPaste
		for( i = 0; i < nTextLen; i++  ){
			if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
				break;
			}
		}
		wchar_t szPaste[1024];
		wmemcpy( szPaste, pszText, i );
		szPaste[i] = L'\0';

		//szPasteを挿入
		Command_INDENT( szPaste, CLogicInt(i) );
	}
	else{
		GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

		CLogicInt nPosX_PHY_Delta;
		if( bLineSelect ){	// 2007.10.04 ryoji
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
			true
		);

		//		::GlobalUnlock(hglb);
		// 挿入データの最後へカーソルを移動
		GetCaret().MoveCursor( ptLayoutNew, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( bLineSelect ){	// 2007.10.04 ryoji
			/* 元の位置へカーソルを移動 */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	//操作前のキャレット位置
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptCaretBefore + CLogicPoint(nPosX_PHY_Delta, CLogicInt(0)),
				&ptLayout
			);
			GetCaret().MoveCursor( ptLayout, true );					//カーソル移動
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
	return;

}


/*! テキストを貼り付け
	@date 2004.05.14 Moca '\\0'を受け入れるように、引数に長さを追加
*/
void CViewCommander::Command_INSTEXT(
	bool			bRedraw,		//!< 
	const wchar_t*	pszText,		//!< [in] 貼り付ける文字列。
	CLogicInt		nTextLen,		//!< [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	bool			bNoWaitCursor	//!< 
)
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CWaitCursor*	pcWaitCursor;

	GetDocument()->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_pCommanderView->GetHwnd() );
	}

	if( nTextLen < 0 ){
		nTextLen = CLogicInt(wcslen( pszText ));
	}

	/* テキストが選択されているか */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			// i = strcspn(pszText, CRLF);
			// 2004.05.14 Moca strcspnでは'\0'が扱えないので
			CLogicInt i;
			for( i = CLogicInt(0); i < nTextLen; i++ ){
				if( pszText[i] == WCODE::CR 
				 || pszText[i] == WCODE::LF ){
					break;
				}
			}
			Command_INDENT( pszText, i );
		}
		else{
			/* データ置換 削除&挿入にも使える */
			m_pCommanderView->ReplaceData_CEditView(
				GetSelect(),			//選択範囲
				NULL,				// 削除されたデータのコピー(NULL可能)
				pszText,			// 挿入するデータ
				nTextLen,			// 挿入するデータの長さ
				bRedraw,
				m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
			);
#ifdef _DEBUG
				gm_ProfileOutput = FALSE;
#endif
		}
	}
	else
	{
		GetDocument()->m_cDocEditor.SetModified(true,true);	/* 変更フラグ */
		
		//	Jun. 13, 2004 genta 不要なチェック？
		if( nTextLen < 0 ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONWARNING, GSTR_APPNAME,
				_T("バグじゃないの？ @Command_INSTEXT") );
			nTextLen = CLogicInt(wcslen( pszText ));
		}

		/* 現在位置にデータを挿入 */
		CLayoutPoint ptLayoutNew;	// 挿入された部分の次の位置
		m_pCommanderView->InsertData_CEditView( GetCaret().GetCaretLayoutPos(), pszText, nTextLen, &ptLayoutNew, true );

		/* 挿入データの最後へカーソルを移動 */
		GetCaret().MoveCursor( ptLayoutNew, bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
	}

	if( NULL != pcWaitCursor ){
		delete pcWaitCursor;
	}
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
				(
// 2004.06.30 Moca WORD*では非x86で境界不整列の可能性あり
				 ( szPaste[nPos] == L'\n' && szPaste[nPos + 1] == L'\r') ||
				 ( szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
//				 ((WORD *)(szPaste + nPos))[0] == MAKEWORD('\n', L'\r') ||
//				 ((WORD *)(szPaste + nPos))[0] == MAKEWORD('\r', L'\n')
				)
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
void CViewCommander::Command_PASTEBOX( void )
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

/* wchar_t1個分の文字を入力 */
void CViewCommander::Command_WCHAR( wchar_t wcChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CLogicInt		nPos;
	CLogicInt		nCharChars;
	CLogicInt		nIdxTo;
	CLayoutInt		nPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	/* 現在位置にデータを挿入 */
	nPosX = CLayoutInt(-1);
	CNativeW cmemDataW2;
	cmemDataW2 = wcChar;
	if( WCODE::IsLineDelimiter(wcChar) ){ 
		/* 現在、Enterなどで挿入する改行コードの種類を取得 */
		// enumEOLType nWorkEOL;
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemDataW2.SetString( cWork.GetValue2(), cWork.GetLen() );

		/* テキストが選択されているか */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			m_pCommanderView->DeleteData( TRUE );
		}
		if( GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent ){	/* オートインデント */
			const CLayout* pCLayout;
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pCLayout );
			if( NULL != pCLayout ){
				const CDocLine* pcDocLine;
				pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( pCLayout->GetLogicLineNo() );
				pLine = pcDocLine->GetDocLineStrWithEOL( &nLineLen );
				if( NULL != pLine ){
					/*
					  カーソル位置変換
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					  →
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					*/
					CLogicPoint ptXY;
					GetDocument()->m_cLayoutMgr.LayoutToLogic(
						GetCaret().GetCaretLayoutPos(),
						&ptXY
					);

					/* 指定された桁に対応する行のデータ内の位置を調べる */
					nIdxTo = m_pCommanderView->LineColmnToIndex( pcDocLine, GetCaret().GetCaretLayoutPos().GetX2() );
					for( nPos = CLogicInt(0); nPos < nLineLen && nPos < ptXY.GetX2(); ){
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );

						/* その他のインデント文字 */
						if( 0 < nCharChars
						 && pLine[nPos] != L'\0'	// その他のインデント文字に L'\0' は含まれない	// 2009.02.04 ryoji L'\0'がインデントされてしまう問題修正
						 && 0 < (int)wcslen( GetDocument()->m_cDocType.GetDocumentAttribute().m_szIndentChars )
						){
							wchar_t szCurrent[10];
							wmemcpy( szCurrent, &pLine[nPos], nCharChars );
							szCurrent[nCharChars] = L'\0';
							/* その他のインデント対象文字 */
							if( NULL != wcsstr(
								GetDocument()->m_cDocType.GetDocumentAttribute().m_szIndentChars,
								szCurrent
							) ){
								goto end_of_for;
							}
						}

						bool bZenSpace=GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
						if(nCharChars==1 && WCODE::IsIndentChar(pLine[nPos],bZenSpace))
						{
							//下へ進む
						}
						else break;

end_of_for:;
						nPos += nCharChars;
					}
					if( nPos > 0 ){
						nPosX = m_pCommanderView->LineIndexToColmn( pcDocLine, nPos );
					}

					//インデント取得
					CNativeW cmemIndent;
					cmemIndent.SetString( pLine, nPos );

					//インデント付加
					cmemDataW2.AppendNativeData(cmemIndent);
				}
			}
		}
	}
	else{
		/* テキストが選択されているか */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* 矩形範囲選択中か */
			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				Command_INDENT( wcChar );
				return;
			}else{
				m_pCommanderView->DeleteData( TRUE );
			}
		}
		else{
			if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
				DelCharForOverwrite();	// 上書き用の一文字削除	// 2009.04.11 ryoji
			}
		}
	}

	//UNICODEに変換する
	CNativeW cmemUnicode;
	cmemUnicode.SetNativeData(cmemDataW2);

	//本文に挿入する
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemUnicode.GetStringPtr(),
		cmemUnicode.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* 挿入データの最後へカーソルを移動 */
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	/* スマートインデント */
	ESmartIndentType nSIndentType = GetDocument()->m_cDocType.GetDocumentAttribute().m_eSmartIndent;
	switch( nSIndentType ){	/* スマートインデント種別 */
	case SMARTINDENT_NONE:
		break;
	case SMARTINDENT_CPP:
		/* C/C++スマートインデント処理 */
		m_pCommanderView->SmartIndent_CPP( wcChar );
		break;
	default:
		//プラグインから検索する
		{
			CPlug::Array plugs;
			CJackManager::Instance()->GetUsablePlug( PP_SMARTINDENT, nSIndentType, &plugs );

			if( plugs.size() > 0 ){
				//インタフェースオブジェクト準備
				CWSHIfObj::List params;
				//プラグイン呼び出し
				( *plugs.begin() )->Invoke( m_pCommanderView, params );				break;
			}
		}
		break;
	}

	/* 2005.10.11 ryoji 改行時に末尾の空白を削除 */
	if( WCODE::CR == wcChar && GetDocument()->m_cDocType.GetDocumentAttribute().m_bRTrimPrevLine ){	/* 改行時に末尾の空白を削除 */
		/* 前の行にある末尾の空白を削除する */
		m_pCommanderView->RTrimPrevLine();
	}

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}




/*!
	@brief 2バイト文字入力
	
	WM_IME_CHARで送られてきた文字を処理する．
	ただし，挿入モードではWM_IME_CHARではなくWM_IME_COMPOSITIONで文字列を
	取得するのでここには来ない．

	@param wChar [in] SJIS漢字コード．上位が1バイト目，下位が2バイト目．
	
	@date 2002.10.06 genta 引数の上下バイトの意味を逆転．
		WM_IME_CHARのwParamに合わせた．
*/
void CViewCommander::Command_IME_CHAR( WORD wChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CMemory			cmemData;

	//	Oct. 6 ,2002 genta 上下逆転
	if( 0 == (wChar & 0xff00) ){
		Command_WCHAR( wChar & 0xff );
		return;
	}
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	// Oct. 6 ,2002 genta バッファに格納する
	// Aug. 15, 2007 kobake WCHARバッファに変換する
	wchar_t szWord[2]={wChar,0};
	CLogicInt nWord=CLogicInt(1);

	/* テキストが選択されているか */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			Command_INDENT( szWord, nWord );	//	Oct. 6 ,2002 genta 
			return;
		}else{
			m_pCommanderView->DeleteData( TRUE );
		}
	}
	else{
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			DelCharForOverwrite();	// 上書き用の一文字削除	// 2009.04.11 ryoji
		}
	}

	//	Oct. 6 ,2002 genta 
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView( GetCaret().GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true );

	/* 挿入データの最後へカーソルを移動 */
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}




/*! 挿入／上書きモード切り替え

	@date 2005.10.02 genta InsMode関数化
*/
void CViewCommander::Command_CHGMOD_INS( void )
{
	/* 挿入モードか？ */
	if( m_pCommanderView->IsInsMode() ){
		m_pCommanderView->SetInsMode( false );
	}else{
		m_pCommanderView->SetInsMode( true );
	}
	/* キャレットの表示・更新 */
	GetCaret().ShowEditCaret();
	/* キャレットの行桁位置を表示する */
	GetCaret().ShowCaretPosInfo();
}


/*!
検索(ボックス)コマンド実行.
ツールバーの検索ボックスにフォーカスを移動する.
	@date 2006.06.04 yukihane 新規作成
*/
void CViewCommander::Command_SEARCH_BOX( void )
{
	const CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;
	pCEditWnd->m_cToolbar.SetFocusSearchBox();
}

/* 検索(単語検索ダイアログ) */
void CViewCommander::Command_SEARCH_DIALOG( void )
{
//	int			nRet;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	CNativeW		cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* 検索文字列を初期化 */
	wcscpy( GetEditWindow()->m_cDlgFind.m_szText, cmemCurText.GetStringPtr() );

	/* 検索ダイアログの表示 */
	if( NULL == GetEditWindow()->m_cDlgFind.GetHwnd() ){
		GetEditWindow()->m_cDlgFind.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)&GetEditWindow()->GetActiveView() );
	}
	else{
		/* アクティブにする */
		ActivateFrameWindow( GetEditWindow()->m_cDlgFind.GetHwnd() );
		::DlgItem_SetText( GetEditWindow()->m_cDlgFind.GetHwnd(), IDC_COMBO_TEXT, cmemCurText.GetStringT() );
	}
	return;
}



/* 前を検索 */
void CViewCommander::Command_SEARCH_PREV( bool bReDraw, HWND hwndParent )
{
	bool		bSelecting;
	bool		bSelectingLock_Old;
	bool		bFound = false;
	bool		bRedo = false;			//	hor
	CLayoutInt	nLineNumOld;
	CLogicInt	nIdxOld;
	const CLayout* pcLayout = NULL;
	CLayoutInt nLineNum;
	CLogicInt nIdx;

	CLayoutRange sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;

	//	bFlag1 = FALSE;
	bSelecting = FALSE;
	if( L'\0' == GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0][0] ){
		goto end_of_func;
	}
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		sSelectBgn_Old = m_pCommanderView->GetSelectionInfo().m_sSelectBgn; //範囲選択(原点)
		sSelect_Old = GetSelect();
		
		bSelectingLock_Old = m_pCommanderView->GetSelectionInfo().m_bSelectingLock;
		
		/* 矩形範囲選択中か */
		if( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && TRUE == m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){	/* 選択状態のロック */
			bSelecting = TRUE;
		}
		else{
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bReDraw );
		}
	}

	nLineNum = GetCaret().GetCaretLayoutPos().GetY2();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );


	if( NULL == pcLayout ){
		// pcLayoutはNULLとなるのは、[EOF]から前検索した場合
		// １行前に移動する処理
		nLineNum--;
		if( nLineNum < 0 ){
			goto end_of_func;
		}
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		if( NULL == pcLayout ){
			goto end_of_func;
		}
		// カーソル左移動はやめて nIdxは行の長さとしないと[EOF]から改行を前検索した時に最後の改行を検索できない 2003.05.04 かろと
		CLayout* pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		nIdx = CLogicInt(pCLayout->GetDocLineRef()->GetLengthWithEOL() + 1);		// 行末のヌル文字(\0)にマッチさせるために+1 2003.05.16 かろと
	} else {
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
	}
	// 2002.01.16 hor
	// 共通部分のくくりだし
	if(!m_pCommanderView->ChangeCurRegexp())return;

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;							//	hor
	/* 現在位置より前の位置を検索する */
	if( GetDocument()->m_cLayoutMgr.SearchWord(
		nLineNum,								// 検索開始レイアウト行
		nIdx,									// 検索開始データ位置
		m_pCommanderView->m_szCurSrchKey,							// 検索条件
		SEARCH_BACKWARD,						// 0==前方検索 1==後方検索
		m_pCommanderView->m_sCurSearchOption,						// 検索オプション
		&sRangeA,								// マッチレイアウト範囲
		&m_pCommanderView->m_CurRegexp							// 正規表現コンパイルデータ
	) ){
		if( bSelecting ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRangeA.GetFrom() );
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
		}else{
			/* 選択範囲の変更 */
			//	2005.06.24 Moca
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );

			if( bReDraw ){
				/* 選択領域描画 */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}
		/* カーソル移動 */
		//	Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		GetCaret().MoveCursor( sRangeA.GetFrom(), bReDraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}else{
		/* フォーカス移動時の再描画 */
//		m_pCommanderView->RedrawAll();	hor コメント化
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bReDraw );
		}
		if( bSelecting ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
			/* 選択範囲の変更 */
			m_pCommanderView->GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
			GetSelect() = sSelect_Old;

			/* カーソル移動 */
			GetCaret().MoveCursor( sRangeA.GetFrom(), bReDraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* 選択領域描画 */
			m_pCommanderView->GetSelectionInfo().DrawSelectArea();
		}
	}
end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		){
			nLineNum=GetDocument()->m_cLayoutMgr.GetLineCount()-CLayoutInt(1);
			nIdx=CLogicInt(MAXLINEKETAS);
			bRedo=FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if(bFound){
		if((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			m_pCommanderView->SendStatusMessage(_T("▲末尾から再検索しました"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("△見つかりませんでした"));
//	if( !bFound ){
// To Here 2002.01.26 hor
		ErrorBeep();
		if( bReDraw	&&
			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND 	/* 検索／置換  見つからないときメッセージを表示 */
		){
			if( NULL == hwndParent ){
				hwndParent = m_pCommanderView->GetHwnd();
			}
			InfoMessage(
				hwndParent,
				_T("前方(↑) に文字列 '%ls' が１つも見つかりません。"),	//Jan. 25, 2001 jepro メッセージを若干変更
				m_pCommanderView->m_szCurSrchKey
			);
		}
	}
	return;
}




/*! 次を検索
	@param bChangeCurRegexp 共有データの検索文字列を使う
	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理見直し．
	@date 2004.05.30 Moca bChangeCurRegexp=trueで従来通り。falseで、CEditViewの現在設定されている検索パターンを使う
*/
void CViewCommander::Command_SEARCH_NEXT(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	HWND			hwndParent,
	const WCHAR*	pszNotFoundMessage
)
{
	bool		bSelecting;
	bool		bFlag1;
	bool		bSelectingLock_Old;
	bool		bFound = false;
	const CLayout* pcLayout;
	bool b0Match = false;		//!< 長さ０でマッチしているか？フラグ by かろと
	const wchar_t *pLine;
	CLogicInt nIdx;
	CLogicInt	nLineLen;
	CLayoutInt	nLineNum;

	CLayoutRange sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;
	CLayoutInt  nLineNumOld;

	bSelecting = FALSE;
	//	2004.05.30 Moca bChangeCurRegexpに応じて対象文字列を変える
	if( bChangeCurRegexp  && L'\0' == GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0][0] 
	 || !bChangeCurRegexp && L'\0' == m_pCommanderView->m_szCurSrchKey[0] ){
		goto end_of_func;
	}

	// 検索開始位置を調整
	bFlag1 = FALSE;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 矩形範囲選択中でない & 選択状態のロック */
		if( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			bSelecting = TRUE;
			bSelectingLock_Old = m_pCommanderView->GetSelectionInfo().m_bSelectingLock;

			sSelectBgn_Old = m_pCommanderView->GetSelectionInfo().m_sSelectBgn; //範囲選択(原点)
			sSelect_Old = GetSelect();

			if( PointCompare(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom(),GetCaret().GetCaretLayoutPos()) >= 0 ){
				// カーソル移動
				GetCaret().SetCaretLayoutPos(GetSelect().GetFrom());
				if (GetSelect().IsOne()) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
				bFlag1 = TRUE;
			}
			else{
				// カーソル移動
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				if (GetSelect().IsOne()) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
			}
		}
		else{
			/* カーソル移動 */
			GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
			if (GetSelect().IsOne()) {
				// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
				b0Match = true;
			}

			/* 現在の選択範囲を非選択状態に戻す */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bRedraw );
		}
	}
	nLineNum = GetCaret().GetCaretLayoutPos().GetY2();
	nLineLen = CLogicInt(0); // 2004.03.17 Moca NULL == pLineのとき、nLineLenが未設定になり落ちるバグ対策
	pLine = GetDocument()->m_cLayoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);

	/* 指定された桁に対応する行のデータ内の位置を調べる */
// 2002.02.08 hor EOFのみの行からも次検索しても再検索可能に (2/2)
	nIdx = pcLayout ? m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() ) : CLogicInt(0);
	if( b0Match ) {
		// 現在、長さ０でマッチしている場合は物理行で１文字進める(無限マッチ対策)
		if( nIdx < nLineLen ) {
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nIdx += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
		} else {
			// 念のため行末は別処理
			++nIdx;
		}
	}

	// 2002.01.16 hor
	// 共通部分のくくりだし
	// 2004.05.30 Moca CEditViewの現在設定されている検索パターンを使えるように
	if(bChangeCurRegexp && !m_pCommanderView->ChangeCurRegexp())return;

	bool bRedo			= TRUE;		//	hor
	nLineNumOld = nLineNum;	//	hor
	int  nIdxOld		= nIdx;		//	hor

re_do:;
	 /* 現在位置より後ろの位置を検索する */
	// 2004.05.30 Moca 引数をGetShareData()からメンバ変数に変更。他のプロセス/スレッドに書き換えられてしまわないように。
	int nSearchResult=GetDocument()->m_cLayoutMgr.SearchWord(
		nLineNum,						// 検索開始レイアウト行
		nIdx,							// 検索開始データ位置
		m_pCommanderView->m_szCurSrchKey,					// 検索条件
		SEARCH_FORWARD,					// 0==前方検索 1==後方検索
		m_pCommanderView->m_sCurSearchOption,				// 検索オプション
		&sRangeA,						// マッチレイアウト範囲
		&m_pCommanderView->m_CurRegexp					// 正規表現コンパイルデータ
	);
	if( nSearchResult ){
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		if( bFlag1 && sRangeA.GetFrom()==GetCaret().GetCaretLayoutPos() ){
			CLogicRange sRange_Logic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(sRangeA,&sRange_Logic);

			nLineNum = sRangeA.GetTo().GetY2();
			nIdx     = sRange_Logic.GetTo().GetX2();
			if( sRange_Logic.GetFrom() == sRange_Logic.GetTo() ) { // 幅0マッチでの無限ループ対策。
				nIdx += 1; // wchar_t一個分進めるだけでは足りないかもしれないが。
			}
			goto re_do;
		}

		if( bSelecting ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRangeA.GetTo() );
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
		}
		else{
			/* 選択範囲の変更 */
			//	2005.06.24 Moca
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );

			if( bRedraw ){
				/* 選択領域描画 */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}

		/* カーソル移動 */
		//	Sep. 8, 2000 genta
		if ( m_pCommanderView->GetDrawSwitch() ) m_pCommanderView->AddCurrentLineToHistory();	// 2002.02.16 hor すべて置換のときは不要
		GetCaret().MoveCursor( sRangeA.GetFrom(), bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}
	else{
		if( bSelecting ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */

			/* 選択範囲の変更 */
			m_pCommanderView->GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old; //範囲選択(原点)
			GetSelect().SetFrom(sSelect_Old.GetFrom());
			GetSelect().SetTo(sRangeA.GetFrom());

			/* カーソル移動 */
			GetCaret().MoveCursor( sRangeA.GetFrom(), bRedraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if( bRedraw ){
				/* 選択領域描画 */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// 見つからなかった
			bRedo	&&		// 最初の検索
			m_pCommanderView->GetDrawSwitch()	// 全て置換の実行中じゃない
		){
			nLineNum=CLayoutInt(0);
			nIdx=CLogicInt(0);
			bRedo=FALSE;
			goto re_do;		// 先頭から再検索
		}
	}

	if(bFound){
		if((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx))
			m_pCommanderView->SendStatusMessage(_T("▼先頭から再検索しました"));
	}
	else{
		GetCaret().ShowEditCaret();	// 2002/04/18 YAZAKI
		GetCaret().ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		m_pCommanderView->SendStatusMessage(_T("▽見つかりませんでした"));
// To Here 2002.01.26 hor

		/* 検索／置換  見つからないときメッセージを表示 */
		ErrorBeep();
		if( bRedraw	&& GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND ){
			if( NULL == hwndParent ){
				hwndParent = m_pCommanderView->GetHwnd();
			}
			if( NULL == pszNotFoundMessage ){
				InfoMessage(
					hwndParent,
					_T("後方(↓) に文字列 '%ls' が１つも見つかりません。"),
					m_pCommanderView->m_szCurSrchKey
				);
			}
			else{
				InfoMessage(hwndParent, _T("%ls"),pszNotFoundMessage);
			}
		}
	}
}




/* 各種モードの取り消し */
void CViewCommander::Command_CANCEL_MODE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}
	m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;	/* 選択状態のロック */
	return;
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
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo );
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

	//必要であれば「ファイルを開く」ダイアログ
	if(!sLoadInfo.cFilePath.IsValidPath()){
		bool bDlgResult = GetDocument()->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::Instance()->GetHwnd(),	//[in]  オーナーウィンドウ
			NULL,								//[in]  フォルダ
			&sLoadInfo							//[out] ロード情報受け取り
		);
		if(!bDlgResult)return;
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
	if(!warnbeep)CEditApp::Instance()->m_cSoundSet.MuteOn();
	bool bRet = pcDoc->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
	if(!warnbeep)CEditApp::Instance()->m_cSoundSet.MuteOff();

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

/*! 指定行へジャンプダイアログの表示
	2002.2.2 YAZAKI
*/
void CViewCommander::Command_JUMP_DIALOG( void )
{
	if( !GetEditWindow()->m_cDlgJump.DoModal(
		G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument()
	) ){
		return;
	}
}



/* 指定行ヘジャンプ */
void CViewCommander::Command_JUMP( void )
{
	const wchar_t*	pLine;
	int			nMode;
	int			bValidLine;
	int			nCurrentLine;
	int			nCommentBegin;

	if( 0 == GetDocument()->m_cLayoutMgr.GetLineCount() ){
		ErrorBeep();
		return;
	}

	/* 行番号 */
	int	nLineNum; //$$ 単位混在
	nLineNum = GetEditWindow()->m_cDlgJump.m_nLineNum;

	if( !GetEditWindow()->m_cDlgJump.m_bPLSQL ){	/* PL/SQLソースの有効行か */
		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if( GetDllShareData().m_bLineNumIsCRLF_ForJump ){
			if( CLogicInt(0) >= nLineNum ){
				nLineNum = CLogicInt(1);
			}
			/*
			  カーソル位置変換
			  ロジック位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			CLayoutPoint ptPosXY;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineNum - 1),
				&ptPosXY
			);
			nLineNum = (Int)ptPosXY.y + 1;
		}
		else{
			if( 0 >= nLineNum ){
				nLineNum = 1;
			}
			if( nLineNum > GetDocument()->m_cLayoutMgr.GetLineCount() ){
				nLineNum = (Int)GetDocument()->m_cLayoutMgr.GetLineCount();
			}
		}
		//	Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		//	2006.07.09 genta 選択状態を解除しないように
		m_pCommanderView->MoveCursorSelecting( CLayoutPoint(0, nLineNum - 1), m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3 );
		return;
	}
	if( 0 >= nLineNum ){
		nLineNum = 1;
	}
	nMode = 0;
	nCurrentLine = GetEditWindow()->m_cDlgJump.m_nPLSQL_E2 - 1;

	int	nLineCount; //$$ 単位混在
	nLineCount = GetEditWindow()->m_cDlgJump.m_nPLSQL_E1 - 1;

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( !GetDocument()->m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF ){ //レイアウト単位
		/*
		  カーソル位置変換
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		  →
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		CLogicPoint ptPosXY;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(0,nLineCount),
			&ptPosXY
		);
		nLineCount = ptPosXY.y;
	}

	for( ; nLineCount <  GetDocument()->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		CLogicInt	nLineLen;
		CLogicInt	nBgn = CLogicInt(0);
		CLogicInt	i;
		pLine = GetDocument()->m_cDocLineMgr.GetLine(CLogicInt(nLineCount))->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = FALSE;
		for( i = CLogicInt(0); i < nLineLen; ++i ){
			if( L' ' != pLine[i] &&
				WCODE::TAB != pLine[i]
			){
				break;
			}
		}
		nBgn = i;
		for( i = nBgn; i < nLineLen; ++i ){
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				bValidLine = TRUE;
				if( L'\'' == pLine[i] ){
					if( i > 0 && L'\\' == pLine[i - 1] ){
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				bValidLine = TRUE;
				if( L'"' == pLine[i] ){
					if( i > 0 && L'\\' == pLine[i - 1] ){
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					if( /*nCommentBegin != nLineCount &&*/ nCommentBegin != 0){
						bValidLine = TRUE;
					}
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					bValidLine = TRUE;
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					nCommentBegin = nLineCount;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L'"' == pLine[i] ){
					nMode = 21;
					continue;
				}else{
					bValidLine = TRUE;
				}
			}
		}
		/* コメント読み込み中 */
		if( 8 == nMode ){
			if( nCommentBegin != 0){
				bValidLine = TRUE;
			}
			/* コメントブロック内の改行だけの行 */
			if( WCODE::CR == pLine[nBgn] ||
				WCODE::LF == pLine[nBgn] ){
				bValidLine = FALSE;
			}
		}
		if( bValidLine ){
			++nCurrentLine;
			if( nCurrentLine >= nLineNum ){
				break;
			}
		}
	}
	/*
	  カーソル位置変換
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	  →
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	*/
	CLayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(0, nLineCount),
		&ptPos
	);
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	//	2006.07.09 genta 選択状態を解除しないように
	m_pCommanderView->MoveCursorSelecting( ptPos, m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3 );
}




/* フォント設定 */
void CViewCommander::Command_FONT( void )
{
	HWND	hwndFrame;
	hwndFrame = GetMainWindow();

	/* フォント設定ダイアログ */
	LOGFONT cLogfont = GetDllShareData().m_Common.m_sView.m_lf;
	INT nPointSize;
	if( MySelectFont( &cLogfont, &nPointSize, CEditWnd::Instance()->m_cSplitterWnd.GetHwnd() )  ){
		GetDllShareData().m_Common.m_sView.m_lf = cLogfont;
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;

		// 文字幅キャッシュの初期化	// 2008/5/15 Uchi
		InitCharWidthCache(cLogfont);
		InitCharWidthCacheCommon();

//		/* 変更フラグ フォント */
//		GetDllShareData().m_bFontModify = TRUE;

		if( GetDllShareData().m_Common.m_sView.m_lf.lfPitchAndFamily & FIXED_PITCH  ){
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;	/* 現在のフォントは固定幅フォントである */
		}else{
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = FALSE;	/* 現在のフォントは固定幅フォントである */
		}
		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)hwndFrame, hwndFrame
		);

		/* キャレットの表示 */
//		::HideCaret( GetHwnd() );
//		::ShowCaret( GetHwnd() );

//		/* アクティブにする */
//		/* アクティブにする */
//		ActivateFrameWindow( hwndFrame );
	}
	return;
}




/* 共通設定 */
void CViewCommander::Command_OPTION( void )
{
	/* 設定プロパティシート テスト用 */
	CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheet( -1/*, -1*/ );
}




/* タイプ別設定 */
void CViewCommander::Command_OPTION_TYPE( void )
{
	CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheetTypes( -1, GetDocument()->m_cDocType.GetDocumentType() );
}




/* タイプ別設定一覧 */
void CViewCommander::Command_TYPE_LIST( void )
{
	CDlgTypeList			cDlgTypeList;
	CDlgTypeList::SResult	sResult;
	sResult.cDocumentType = GetDocument()->m_cDocType.GetDocumentType();
	if( cDlgTypeList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), &sResult ) ){
		//	Nov. 29, 2000 genta
		//	一時的な設定適用機能を無理矢理追加
		if( sResult.bTempChange ){
			GetDocument()->m_cDocType.SetDocumentType( sResult.cDocumentType, true );
			GetDocument()->m_cDocType.LockDocumentType();
			/* 設定変更を反映させる */
			GetDocument()->m_bTextWrapMethodCurTemp = false;	// 折り返し方法の一時設定適用中を解除	// 2008.06.08 ryoji
			GetDocument()->OnChangeSetting();

			// 2006.09.01 ryoji タイプ変更後自動実行マクロを実行する
			GetDocument()->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnTypeChanged );
		}
		else{
			/* タイプ別設定 */
			CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheetTypes( -1, sResult.cDocumentType );
		}
	}
	return;
}




/* 行の二重化(折り返し単位) */
void CViewCommander::Command_DUPLICATELINE( void )
{
	int				bCRLF;
	int				bAddCRLF;
	CNativeW		cmemBuf;
	const CLayout*	pcLayout;

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos() + CLayoutPoint(0,1);

	//行頭に移動(折り返し単位)
	Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, 0x1 /* カーソル位置に関係なく行頭に移動 */ );

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}



	/* 二重化したい行を調べる
	||	・改行で終わっている
	||	・改行で終わっていない
	||	・最終行である
	||	→折り返しでない
	||	・最終行でない
	||	→折り返しである
	*/
	bCRLF = ( EOL_NONE == pcLayout->GetLayoutEol() ) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if( !bCRLF ){
		if( GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_cLayoutMgr.GetLineCount() - 1 ){
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen() );	//	※pcLayout->GetLengthWithEOL()は、EOLの長さを必ず1にするので使えない。
	if( bAddCRLF ){
		/* 現在、Enterなどで挿入する改行コードの種類を取得 */
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemBuf.AppendString( cWork.GetValue2(), cWork.GetLen() );
	}

	/* 現在位置にデータを挿入 */
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* カーソルを移動 */
	GetCaret().MoveCursor( ptCaretPosOld, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();


	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		/* 操作の追加 */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
				GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
			)
		);
	}
	return;
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




/*!	アウトライン解析
	
	2002/3/13 YAZAKI nOutlineTypeとnListTypeを統合。
*/
// トグル用のフラグに変更 20060201 aroka
BOOL CViewCommander::Command_FUNCLIST(
	int nAction,
	int _nOutlineType
)
{
	static bool bIsProcessing = false;	//アウトライン解析処理中フラグ

	//アウトラインプラグイン内でのEditor.Outline呼び出しによる再入を禁止する
	if( bIsProcessing )return FALSE;

	bIsProcessing = true;

	EOutlineType nOutlineType = (EOutlineType)_nOutlineType; //2007.11.29 kobake

//	if( bCheckOnly ){
//		return TRUE;
//	}

	static CFuncInfoArr	cFuncInfoArr;
//	int		nLine;
//	int		nListType;
	tstring sTitleOverride;				//プラグインによるダイアログタイトル上書き

	//	2001.12.03 hor & 2002.3.13 YAZAKI
	if( nOutlineType == OUTLINE_DEFAULT ){
		/* タイプ別に設定されたアウトライン解析方法 */
		nOutlineType = GetDocument()->m_cDocType.GetDocumentAttribute().m_eDefaultOutline;
	}

	if( NULL != GetEditWindow()->m_cDlgFuncList.GetHwnd() && nAction != SHOW_RELOAD ){
		switch(nAction ){
		case SHOW_NORMAL: // アクティブにする
			//	開いているものと種別が同じならActiveにするだけ．異なれば再解析
			if( GetEditWindow()->m_cDlgFuncList.CheckListType( nOutlineType )){
				ActivateFrameWindow( GetEditWindow()->m_cDlgFuncList.GetHwnd() );
				bIsProcessing = false;
				return TRUE;
			}
			break;
		case SHOW_TOGGLE: // 閉じる
			//	開いているものと種別が同じなら閉じる．異なれば再解析
			if( GetEditWindow()->m_cDlgFuncList.CheckListType( nOutlineType )){
				::SendMessageAny( GetEditWindow()->m_cDlgFuncList.GetHwnd(), WM_CLOSE, 0, 0 );
				bIsProcessing = false;
				return TRUE;
			}
			break;
		default:
			break;
		}
	}

	/* 解析結果データを空にする */
	cFuncInfoArr.Empty();

	switch( nOutlineType ){
//	case OUTLINE_C:			GetDocument()->MakeFuncList_C( &cFuncInfoArr );break;
	case OUTLINE_CPP:
		GetDocument()->m_cDocOutline.MakeFuncList_C( &cFuncInfoArr );
		/* C言語標準保護委員会勧告特別処理実装箇所(嘘) */
		if( CheckEXT( GetDocument()->m_cDocFile.GetFilePath(), _T("c") ) ){
			nOutlineType = OUTLINE_C;	/* これでC関数一覧リストビューになる */
		}
		break;
	case OUTLINE_PLSQL:		GetDocument()->m_cDocOutline.MakeFuncList_PLSQL( &cFuncInfoArr );break;
	case OUTLINE_JAVA:		GetDocument()->m_cDocOutline.MakeFuncList_Java( &cFuncInfoArr );break;
	case OUTLINE_COBOL:		GetDocument()->m_cDocOutline.MakeTopicList_cobol( &cFuncInfoArr );break;
	case OUTLINE_ASM:		GetDocument()->m_cDocOutline.MakeTopicList_asm( &cFuncInfoArr );break;
	case OUTLINE_PERL:		GetDocument()->m_cDocOutline.MakeFuncList_Perl( &cFuncInfoArr );break;	//	Sep. 8, 2000 genta
	case OUTLINE_VB:		GetDocument()->m_cDocOutline.MakeFuncList_VisualBasic( &cFuncInfoArr );break;	//	June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		GetDocument()->m_cDocOutline.MakeTopicList_wztxt(&cFuncInfoArr);break;		// 2003.05.20 zenryaku 階層付テキスト アウトライン解析
	case OUTLINE_HTML:		GetDocument()->m_cDocOutline.MakeTopicList_html(&cFuncInfoArr);break;		// 2003.05.20 zenryaku HTML アウトライン解析
	case OUTLINE_TEX:		GetDocument()->m_cDocOutline.MakeTopicList_tex(&cFuncInfoArr);break;		// 2003.07.20 naoh TeX アウトライン解析
	case OUTLINE_BOOKMARK:	GetDocument()->m_cDocOutline.MakeFuncList_BookMark( &cFuncInfoArr );break;	//	2001.12.03 hor
	case OUTLINE_FILE:		GetDocument()->m_cDocOutline.MakeFuncList_RuleFile( &cFuncInfoArr );break;	//	2002.04.01 YAZAKI アウトライン解析にルールファイルを導入
//	case OUTLINE_UNKNOWN:	//Jul. 08, 2001 JEPRO 使わないように変更
	case OUTLINE_PYTHON:	GetDocument()->m_cDocOutline.MakeFuncList_python(&cFuncInfoArr);break;		// 2007.02.08 genta
	case OUTLINE_ERLANG:	GetDocument()->m_cDocOutline.MakeFuncList_Erlang(&cFuncInfoArr);break;		// 2009.08.10 genta
	case OUTLINE_TEXT:
		//	fall though
		//	ここには何も入れてはいけない 2007.02.28 genta 注意書き
	default:
		//プラグインから検索する
		{
			CPlug::Array plugs;
			CJackManager::Instance()->GetUsablePlug( PP_OUTLINE, nOutlineType, &plugs );

			if( plugs.size() > 0 ){
				//インタフェースオブジェクト準備
				CWSHIfObj::List params;
				COutlineIfObj* objOutline = new COutlineIfObj( cFuncInfoArr );
				objOutline->AddRef();
				params.push_back( objOutline );
				//プラグイン呼び出し
				( *plugs.begin() )->Invoke( m_pCommanderView, params );

				nOutlineType = objOutline->m_nListType;			//ダイアログの表示方法をを上書き
				sTitleOverride = objOutline->m_sOutlineTitle;	//ダイアログタイトルを上書き

				objOutline->Release();
				break;
			}
		}

		//それ以外
		GetDocument()->m_cDocOutline.MakeTopicList_txt( &cFuncInfoArr );
		break;
	}

	/* 解析対象ファイル名 */
	_tcscpy( cFuncInfoArr.m_szFilePath, GetDocument()->m_cDocFile.GetFilePath() );

	/* アウトライン ダイアログの表示 */
	CLayoutPoint poCaret = GetCaret().GetCaretLayoutPos();
	if( NULL == GetEditWindow()->m_cDlgFuncList.GetHwnd() ){
		GetEditWindow()->m_cDlgFuncList.DoModeless(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			(LPARAM)m_pCommanderView,
			&cFuncInfoArr,
			poCaret.GetY2() + CLayoutInt(1),
			poCaret.GetX2() + CLayoutInt(1),
			nOutlineType,
			GetDocument()->m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		);
	}else{
		/* アクティブにする */
		GetEditWindow()->m_cDlgFuncList.Redraw( nOutlineType, &cFuncInfoArr, poCaret.GetY2() + 1, poCaret.GetX2() + 1 );
		ActivateFrameWindow( GetEditWindow()->m_cDlgFuncList.GetHwnd() );
	}

	// ダイアログタイトルを上書き
	if( ! sTitleOverride.empty() ){
		GetEditWindow()->m_cDlgFuncList.SetWindowText( sTitleOverride.c_str() );
	}

	bIsProcessing = false;
	return TRUE;
}




/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
void CViewCommander::Command_SPLIT_V( void )
{
	GetEditWindow()->m_cSplitterWnd.VSplitOnOff();
	return;
}




/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
void CViewCommander::Command_SPLIT_H( void )
{
	GetEditWindow()->m_cSplitterWnd.HSplitOnOff();
	return;
}




/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
void CViewCommander::Command_SPLIT_VH( void )
{
	GetEditWindow()->m_cSplitterWnd.VHSplitOnOff();
	return;
}




//From Here Nov. 25, 2000 JEPRO
/* ヘルプ目次 */
void CViewCommander::Command_HELP_CONTENTS( void )
{
	ShowWinHelpContents( m_pCommanderView->GetHwnd(), CEditApp::Instance()->GetHelpFilePath() );	//	目次を表示する
	return;
}




/* ヘルプキーワード検索 */
void CViewCommander::Command_HELP_SEARCH( void )
{
	MyWinHelp( m_pCommanderView->GetHwnd(), CEditApp::Instance()->GetHelpFilePath(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return;
}
//To Here Nov. 25, 2000




/*! キャレット位置の単語を辞書検索ON-OFF

	@date 2006.03.24 fon 新規作成
*/
void CViewCommander::Command_ToggleKeySearch( void )
{	/* 共通設定ダイアログの設定をキー割り当てでも切り替えられるように */
	if( GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord ){
		GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = FALSE;
	}else{
		GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = TRUE;
	}
}




/* コマンド一覧 */
void CViewCommander::Command_MENU_ALLFUNC( void )
{

	UINT	uFlags;
	POINT	po;
	HMENU	hMenu;
	HMENU	hMenuPopUp;
	int		i;
	int		j;
	int		nId;

//	From Here Sept. 15, 2000 JEPRO
//	サブメニュー、特に「その他」のコマンドに対してステータスバーに表示されるキーアサイン情報が
//	メニューで隠れないように右にずらした
//	(本当はこの「コマンド一覧」メニューをダイアログに変更しバーをつまんで自由に移動できるようにしたい)
//	po.x = 0;
	po.x = 540;
//	To Here Sept. 15, 2000 (Oct. 7, 2000 300→500; Nov. 3, 2000 500→540)
	po.y = 0;
	::ClientToScreen( m_pCommanderView->GetHwnd(), &po );

	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更したことで1番(カーソル移動系)が前にシフトされた(この変更によって i=1→i=0 と変更)
	//	Oct. 3, 2001 genta
	for( i = 0; i < FuncLookup.GetCategoryCount(); i++ ){
		hMenuPopUp = ::CreatePopupMenu();
		for( j = 0; j < FuncLookup.GetItemCount(i); j++ ){
			//	Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode( i, j, false );	// 2007.11.02 ryoji 未登録マクロ非表示を明示指定
			if( code != 0 ){
				WCHAR	szLabel[300];
				FuncLookup.Pos2FuncName( i, j, szLabel, 256 );
				uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				//	Oct. 3, 2001 genta
				pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenuPopUp, uFlags, code, szLabel );
			}
		}
		//	Oct. 3, 2001 genta
		pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , FuncLookup.Category2Name(i) );
//		pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i] );
	}

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
		GetMainWindow()/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu( hMenu );
	if( 0 != nId ){
		/* コマンドコードによる処理振り分け */
//		HandleCommand( nFuncID, TRUE, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ), (LPARAM)NULL );
	}
	return;
}




/* 外部ヘルプ１ */
void CViewCommander::Command_EXTHELP1( void )
{
retry:;
	if( CHelpManager().ExtWinHelpIsSet( GetDocument()->m_cDocType.GetDocumentType() ) == false){
//	if( 0 == wcslen( GetDllShareData().m_Common.m_szExtHelp1 ) ){
		ErrorBeep();
//From Here Sept. 15, 2000 JEPRO
//		[Esc]キーと[x]ボタンでも中止できるように変更
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
//To Here Sept. 15, 2000
			_T("外部ヘルプ１が設定されていません。\n今すぐ設定しますか?")
		) ){
			/* 共通設定 プロパティシート */
			if( !CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHELP1*/ ) ){
				return;
			}
			goto retry;
		}
		//	Jun. 15, 2000 genta
		else{
			return;
		}
	}

	CNativeW		cmemCurText;
	const TCHAR*	helpfile = CHelpManager().GetExtWinHelp( GetDocument()->m_cDocType.GetDocumentType() );

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	m_pCommanderView->GetCurrentTextForSearch( cmemCurText );
	if( _IS_REL_PATH( helpfile ) ){
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		TCHAR path[_MAX_PATH];
		GetInidirOrExedir( path, helpfile );
		::WinHelp( m_pCommanderView->m_hwndParent, path, HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr() );
		return;
	}
	::WinHelp( m_pCommanderView->m_hwndParent, helpfile , HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr() );
	return;
}




/*!
	外部HTMLヘルプ
	
	@param helpfile [in] HTMLヘルプファイル名．NULLのときはタイプ別に設定されたファイル．
	@param kwd [in] 検索キーワード．NULLのときはカーソル位置or選択されたワード
	@date 2002.07.05 genta 任意のファイル・キーワードの指定ができるよう引数追加
*/
void CViewCommander::Command_EXTHTMLHELP( const WCHAR* _helpfile, const WCHAR* kwd )
{
	const TCHAR* helpfile = to_tchar(_helpfile);

	HWND		hwndHtmlHelp;
	int			nLen;

	DBPRINT_A("helpfile=%ls\n",helpfile);

	//	From Here Jul. 5, 2002 genta
	const TCHAR *filename = NULL;
	if ( helpfile == NULL || helpfile[0] == _T('\0') ){
		while( !CHelpManager().ExtHTMLHelpIsSet( GetDocument()->m_cDocType.GetDocumentType()) ){
			ErrorBeep();
	//	From Here Sept. 15, 2000 JEPRO
	//		[Esc]キーと[x]ボタンでも中止できるように変更
			if( IDYES != ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
	//	To Here Sept. 15, 2000
				_T("外部HTMLヘルプが設定されていません。\n今すぐ設定しますか?")
			) ){
				return;
			}
			/* 共通設定 プロパティシート */
			if( !CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHTMLHELP*/ ) ){
				return;
			}
		}
		filename = CHelpManager().GetExtHTMLHelp( GetDocument()->m_cDocType.GetDocumentType() );
	}
	else {
		filename = helpfile;
	}
	//	To Here Jul. 5, 2002 genta

	//	Jul. 5, 2002 genta
	//	キーワードの外部指定を可能に
	CNativeW	cmemCurText;
	if( kwd != NULL && kwd[0] != _T('\0') ){
		cmemCurText.SetString( kwd );
	}
	else {
		/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
		m_pCommanderView->GetCurrentTextForSearch( cmemCurText );
	}

	/* HtmlHelpビューアはひとつ */
	if( CHelpManager().HTMLHelpIsSingle( GetDocument()->m_cDocType.GetDocumentType() ) ){
		// タスクトレイのプロセスにHtmlHelpを起動させる
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		TCHAR* pWork=GetDllShareData().m_sWorkBuffer.GetWorkBuffer<TCHAR>();
		if( _IS_REL_PATH( filename ) ){
			GetInidirOrExedir( pWork, filename );
		}else{
			_tcscpy( pWork, filename ); //	Jul. 5, 2002 genta
		}
		nLen = _tcslen( pWork );
		_tcscpy( &pWork[nLen + 1], cmemCurText.GetStringT() );
		hwndHtmlHelp = (HWND)::SendMessageAny(
			GetDllShareData().m_sHandles.m_hwndTray,
			MYWM_HTMLHELP,
			(WPARAM)GetMainWindow(),
			0
		);
	}
	else{
		/* 自分でHtmlHelpを起動させる */
		HH_AKLINK	link;
		link.cbStruct = sizeof( link ) ;
		link.fReserved = FALSE ;
		link.pszKeywords = cmemCurText.GetStringT();
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszMsgTitle = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;

		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( filename ) ){
			TCHAR path[_MAX_PATH];
			GetInidirOrExedir( path, filename );
			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_sHandles.m_hwndTray*/,
				path, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else{
			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_sHandles.m_hwndTray*/,
				filename, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}

	//	Jul. 6, 2001 genta hwndHtmlHelpのチェックを追加
	if( hwndHtmlHelp != NULL ){
		::BringWindowToTop( hwndHtmlHelp );
	}

	return;
}




//From Here Dec. 25, 2000 JEPRO
/* バージョン情報 */
void CViewCommander::Command_ABOUT( void )
{
	CDlgAbout cDlgAbout;
	cDlgAbout.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd() );
	return;
}
//To Here Dec. 25, 2000




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




/* インデント ver1 */
void CViewCommander::Command_INDENT( wchar_t wcChar, EIndentType eIndent )
{
	using namespace WCODE;

#if 1	// ↓ここを残せば選択幅ゼロを最大にする（従来互換挙動）。無くても Command_INDENT() ver0 が適切に動作するように変更されたので、削除しても特に不都合にはならない。
	// From Here 2001.12.03 hor
	/* SPACEorTABインンデントで矩形選択桁がゼロの時は選択範囲を最大にする */
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( INDENT_NONE != eIndent && m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && GetSelect().GetFrom().x==GetSelect().GetTo().x ){
		GetSelect().SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		m_pCommanderView->RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
#endif
	Command_INDENT( &wcChar, CLogicInt(1), eIndent );
	return;
}




/* インデント ver0 */
/*
	選択された各行の範囲の直前に、与えられた文字列( pData )を挿入する。
	@param eIndent インデントの種別
*/
void CViewCommander::Command_INDENT( const wchar_t* const pData, const CLogicInt nDataLen, EIndentType eIndent )
{
	if( nDataLen <= 0 ) return;

	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );
	CLayoutRange sSelectOld;		//範囲選択
	CLayoutPoint ptInserted;		//挿入後の挿入位置
	const struct {
		bool operator()( const wchar_t ch ) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SSoftTabData {
		SSoftTabData( CLayoutInt nTab ) : m_szTab(NULL), m_nTab((Int)nTab) {}
		~SSoftTabData() { delete []m_szTab; }
		operator const wchar_t* ()
		{
			if( !m_szTab ){
				m_szTab = new wchar_t[m_nTab];
				wmemset( m_szTab, WCODE::SPACE, m_nTab );
			}
			return m_szTab;
		}
		int Len( CLayoutInt nCol ) { return m_nTab - ((Int)nCol % m_nTab); }
		wchar_t* m_szTab;
		int m_nTab;
	} stabData( GetDocument()->m_cLayoutMgr.GetTabSpace() );

	const bool bSoftTab = ( eIndent == INDENT_TAB && GetDocument()->m_cDocType.GetDocumentAttribute().m_bInsSpace );
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* テキストが選択されているか */
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR( pData[0] );	// 1文字入力
		}
		else{
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			if( bSoftTab && !m_pCommanderView->IsInsMode() ){
				DelCharForOverwrite();
			}
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				true
			);
			GetCaret().MoveCursor( ptInserted, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		return;
	}
	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor
	/* 矩形範囲選択中か */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		// From Here 2001.12.03 hor
		/* 上書モードのときは選択範囲削除 */
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */){
			sSelectOld = GetSelect();
			m_pCommanderView->DeleteData( FALSE );
			GetSelect() = sSelectOld;
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
		}
		// To Here 2001.12.03 hor

		/* 2点を対角とする矩形を求める */
		CLayoutRange rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// 範囲選択開始
			GetSelect().GetTo()		// 範囲選択終了
		);
		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( FALSE/*TRUE 2002.01.25 hor*/ );

		/*
			文字を直前に挿入された文字が、それにより元の位置からどれだけ後ろにずれたか。
			これに従い矩形選択範囲を後ろにずらす。
		*/
		CLayoutInt minOffset( -1 );
		/*
			■全角文字の左側の桁揃えについて
			(1) eIndent == INDENT_TAB のとき
				選択範囲がタブ境界にあるときにタブを入力すると、全角文字の前半が選択範囲から
				はみ出している行とそうでない行でタブの幅が、1から設定された最大までと大きく異なり、
				最初に選択されていた文字を選択範囲内にとどめておくことができなくなる。
				最初は矩形選択範囲内にきれいに収まっている行にはタブを挿入せず、ちょっとだけはみ
				出している行にだけタブを挿入することとし、それではどの行にもタブが挿入されない
				とわかったときはやり直してタブを挿入する。
			(2) eIndent == INDENT_SPACE のとき（※従来互換的な動作）
				幅1で選択している場合のみ全角文字の左側を桁揃えする。
				最初は矩形選択範囲内にきれいに収まっている行にはスペースを挿入せず、ちょっとだけはみ
				出している行にだけスペースを挿入することとし、それではどの行にもスペースが挿入されない
				とわかったときはやり直してスペースを挿入する。
		*/
		bool alignFullWidthChar = eIndent == INDENT_TAB && 0 == rcSel.GetFrom().x % this->GetDocument()->m_cLayoutMgr.GetTabSpace();
#if 1	// ↓ここを残せば選択幅1のSPACEインデントで全角文字を揃える機能(2)が追加される。
		alignFullWidthChar = alignFullWidthChar || (eIndent == INDENT_SPACE && 1 == rcSel.GetTo().x - rcSel.GetFrom().x);
#endif
		for( bool insertionWasDone = false; ; alignFullWidthChar = false ) {
			minOffset = CLayoutInt( -1 );
			for( CLayoutInt nLineNum = rcSel.GetFrom().y; nLineNum <= rcSel.GetTo().y; ++nLineNum ){
				const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
				//	Nov. 6, 2002 genta NULLチェック追加
				//	これがないとEOF行を含む矩形選択中の文字列入力で落ちる
				CLogicInt nIdxFrom, nIdxTo;
				CLayoutInt xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if( pcLayout ) {
					/* 指定された桁に対応する行のデータ内の位置を調べる */
					const struct {
						CLayoutInt keta;
						CLogicInt* outLogicX;
						CLayoutInt* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ CLayoutInt(-1), 0, 0 }
					};
					CMemoryIterator it( pcLayout, this->GetDocument()->m_cLayoutMgr.GetTabSpace() );
					for( int i = 0; 0 <= sortedKetas[i].keta; ++i ) {
						for( ; ! it.end(); it.addDelta() ) {
							if( sortedKetas[i].keta == it.getColumn() ) {
								break;
							}
							it.scanNext();
							if( sortedKetas[i].keta < it.getColumn() + it.getColumnDelta() ) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = it.getIndex();
						*sortedKetas[i].outLayoutX = it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else{
					nIdxFrom = nIdxTo = CLogicInt(0);
					xLayoutFrom = xLayoutTo = CLayoutInt(0);
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pcLayout || 0 == pcLayout->GetLengthWithoutEOL();
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pcLayout && pcLayout->GetLayoutEol() != EOL_NONE) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// 入力文字の挿入位置
				const CLayoutPoint ptInsert( selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum );

				/* TABやスペースインデントの時 */
				if( INDENT_NONE != eIndent ) {
					if( emptyLine || selectionIsOutOfLine ) {
						continue; // インデント文字をインデント対象が存在しない部分(改行文字の後ろや空行)に挿入しない。
					}
					/*
						入力がインデント用の文字のとき、ある条件で入力文字を挿入しないことで
						インデントを揃えることができる。
						http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=dev&v=4103
					*/
					if( nIdxFrom == nIdxTo // 矩形選択範囲の右端までに範囲の左端にある文字の末尾が含まれておらず、
						&& ! selectionIsOutOfLine && pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] ) // その、末尾の含まれていない文字がインデント文字であり、
						&& rcSel.GetFrom().x < rcSel.GetTo().x // 幅0矩形選択ではない(<<互換性とインデント文字挿入の使い勝手のために除外する)とき。
					) {
						continue;
					}
					/*
						全角文字の左側の桁揃え
					*/
					if( alignFullWidthChar
						&& (ptInsert.x == rcSel.GetFrom().x || (pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] )))
					) {	// 文字の左側が範囲にぴったり収まっている
						minOffset = CLayoutInt(0);
						continue;
					}
				}

				/* 現在位置にデータを挿入 */
				m_pCommanderView->InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = std::min(
					0 <= minOffset ? minOffset : this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas(),
					ptInsert.x <= ptInserted.x ? ptInserted.x - ptInsert.x : std::max( CLayoutInt(0), this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() - ptInsert.x)
				);

				GetCaret().MoveCursor( ptInserted, FALSE );
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}
			if( insertionWasDone || !alignFullWidthChar ) {
				break; // ループの必要はない。(1.文字の挿入が行われたから。2.そうではないが文字の挿入を控えたせいではないから)
			}
		}

		// 挿入された文字の分だけ選択範囲を後ろにずらし、rcSelにセットする。
		if( 0 < minOffset ) {
			rcSel.GetFromPointer()->x = std::min( rcSel.GetFrom().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
			rcSel.GetToPointer()->x = std::min( rcSel.GetTo().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		}

		/* カーソルを移動 */
		GetCaret().MoveCursor( rcSel.GetFrom(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
		GetSelect().SetFrom(rcSel.GetFrom());	//範囲選択開始位置
		GetSelect().SetTo(rcSel.GetTo());		//範囲選択終了位置
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
	}
	else if( GetSelect().IsLineOne() ){	// 通常選択(1行内)
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR( pData[0] );	// 1文字入力
		}
		else{
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			m_pCommanderView->DeleteData( false );
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				false
			);
			GetCaret().MoveCursor( ptInserted, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}
	else{	// 通常選択(複数行)
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		// 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( FALSE );

		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout ||						//	テキストが無いEOLの行は無視
				pcLayout->GetLogicOffset() > 0 ||				//	折り返し行は無視
				pcLayout->GetLengthWithoutEOL() == 0 ){	//	改行のみの行は無視する。
				continue;
			}

			/* カーソルを移動 */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			/* 現在位置にデータを挿入 */
			m_pCommanderView->InsertData_CEditView(
				CLayoutPoint(CLayoutInt(0),i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(CLayoutInt(0)),
				&ptInserted,
				false
			);
			/* カーソルを移動 */
			GetCaret().MoveCursor( ptInserted, FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	行数が変化した!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}

		GetSelect() = sSelectOld;

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	/* 再描画 */
	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	m_pCommanderView->RedrawAll();			// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}




/* 逆インデント */
void CViewCommander::Command_UNINDENT( wchar_t wcChar )
{
	//	Aug. 9, 2003 genta
	//	選択されていない場合に逆インデントした場合に
	//	注意メッセージを出す
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* wchar_t1個分の文字入力 */
		Command_WCHAR( wcChar );	//	2003.10.09 zenryaku警告を出すが，動作は以前のままにする 
		m_pCommanderView->SendStatusMessage(_T("★逆インデントは選択時のみ"));
		return;
	}

	//砂時計
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CMemory		cmemBuf;

	/* 矩形範囲選択中か */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		ErrorBeep();
//**********************************************
//	 箱型逆インデントについては、保留とする (1998.10.22)
//**********************************************
	}
	else{
		GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

		CLayoutRange sSelectOld;	//範囲選択
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		/* 現在の選択範囲を非選択状態に戻す */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( FALSE );

		CLogicInt		nDelLen;
		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();

			const CLayout*	pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( i, &nLineLen, &pcLayout );
			if( NULL == pcLayout || pcLayout->GetLogicOffset() > 0 ){ //折り返し以降の行はインデント処理を行わない
				continue;
			}

			if( WCODE::TAB == wcChar ){
				if( pLine[0] == wcChar ){
					nDelLen = CLogicInt(1);
				}
				else{
					//削り取る半角スペース数 (1～タブ幅分) -> nDelLen
					CLogicInt i;
					CLogicInt nTabSpaces = CLogicInt((Int)GetDocument()->m_cLayoutMgr.GetTabSpace());
					for( i = CLogicInt(0); i < nLineLen; i++ ){
						if( WCODE::SPACE != pLine[i] ){
							break;
						}
						//	Sep. 23, 2002 genta LayoutMgrの値を使う
						if( i >= nTabSpaces ){
							break;
						}
					}
					if( 0 == i ){
						continue;
					}
					nDelLen = i;
				}
			}
			else{
				if( pLine[0] != wcChar ){
					continue;
				}
				nDelLen = CLogicInt(1);
			}

			/* カーソルを移動 */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			

			CNativeW	pcMemDeleted;
			/* 指定位置の指定長データ削除 */
			m_pCommanderView->DeleteData2(
				CLayoutPoint(CLayoutInt(0),i),
				nDelLen,	// 2001.12.03 hor
				&pcMemDeleted
			);
			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	行数が変化した!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}
		GetSelect() = sSelectOld;	//範囲選択

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// 操作前のキャレット位置
					GetCaret().GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}

	/* 再描画 */
	m_pCommanderView->RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
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




/*! タグジャンプ

	@param bClose [in] true:元ウィンドウを閉じる

	@date 2003.04.03 genta 元ウィンドウを閉じるかどうかの引数を追加
	@date 2004.05.13 Moca 行桁位置の指定が無い場合は、行桁を移動しない
*/
bool CViewCommander::Command_TAGJUMP( bool bClose )
{
	//	2004.05.13 Moca 初期値を1ではなく元の位置を継承するように
	// 0以下は未指定扱い。(1開始)
	int			nJumpToLine;
	int			nJumpToColm;
	nJumpToLine = 0;
	nJumpToColm = 0;

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	CLogicPoint ptXY;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&ptXY
	);
	/* 現在行のデータを取得 */
	CLogicInt		nLineLen;
	const wchar_t*	pLine;
	pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
	if( NULL == pLine ){
		goto can_not_tagjump_end;
	}

	//ファイル名バッファ
	wchar_t		szJumpToFile[1024];
	int			nBgn;
	int			nPathLen;
	wmemset( szJumpToFile, 0, _countof(szJumpToFile) );

	/* WZ風のタグリストか */
	if( 0 == wmemcmp( pLine, L"■\"", 2 ) ){
		if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
			wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
			GetLineColm( &pLine[2] + nPathLen, &nJumpToLine, &nJumpToColm );
		}
		else{
			goto can_not_tagjump;
		}
	}
	else if(0 == wmemcmp( pLine, L"・", 1 )){
		GetLineColm( &pLine[1], &nJumpToLine, &nJumpToColm );
		ptXY.y--;

		for( ; 0 <= ptXY.y; ptXY.y-- ){
			pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
			if( NULL == pLine ){
				goto can_not_tagjump;
			}
			if( 0 == wmemcmp( pLine, L"・", 1 ) ){
				continue;
			}
			else if( 0 == wmemcmp( pLine, L"■\"", 2 ) ){
				if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
					wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
					break;
				}
				else{
					goto can_not_tagjump;
				}
			}
			else{
				goto can_not_tagjump;
			}
		}
	}
	else{
		//@@@ 2001.12.31 YAZAKI
		const wchar_t *p = pLine;
		const wchar_t *p_end = p + nLineLen;

		//	From Here Aug. 27, 2001 genta
		//	Borland 形式のメッセージからのTAG JUMP
		while( p < p_end ){
			//	skip space
			for( ; p < p_end && ( *p == L' ' || *p == L'\t' || *p == L'\n' ); ++p )
				;
			if( p >= p_end )
				break;
		
			//	Check Path
			if( IsFilePath( p, &nBgn, &nPathLen ) ){
				wmemcpy( szJumpToFile, &p[nBgn], nPathLen );
				GetLineColm( &p[nBgn + nPathLen], &nJumpToLine, &nJumpToColm );
				break;
			}
			//	Jan. 04, 2001 genta Directoryを対象外にしたので文字列には柔軟に対応
			//	break;	//@@@ 2001.12.31 YAZAKI 「working ...」問題に対処
			//	skip non-space
			for( ; p < p_end && ( *p != L' ' && *p != L'\t' ); ++p )
				;
		}
		if( szJumpToFile[0] == L'\0' ){
			if( !Command_TagJumpByTagsFile() )	//@@@ 2003.04.13
				goto can_not_tagjump;
			return true;
		}
		//	From Here Aug. 27, 2001 genta
	}

	//	Apr. 21, 2003 genta bClose追加
	if( !m_pCommanderView->TagJumpSub( to_tchar(szJumpToFile), CMyPoint(nJumpToColm, nJumpToLine), bClose ) )	//@@@ 2003.04.13
		goto can_not_tagjump;

	return true;

can_not_tagjump:;
can_not_tagjump_end:;
	m_pCommanderView->SendStatusMessage(_T("タグジャンプできません"));	//@@@ 2003.04.13
	return false;
}




/* タグジャンプバック */
void CViewCommander::Command_TAGJUMPBACK( void )
{
// 2004/06/21 novice タグジャンプ機能追加
	TagJump tagJump;

	/* タグジャンプ情報の参照 */
	if( !CTagJumpManager().PopTagJump(&tagJump) || !IsSakuraMainWindow(tagJump.hwndReferer) ){
		m_pCommanderView->SendStatusMessage(_T("タグジャンプバックできません"));
		// 2004.07.10 Moca m_TagJumpNumを0にしなくてもいいと思う
		// GetDllShareData().m_TagJumpNum = 0;
		return;
	}

	/* アクティブにする */
	ActivateFrameWindow( tagJump.hwndReferer );

	/* カーソルを移動させる */
	memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &(tagJump.point), sizeof( tagJump.point ) );
	::SendMessageAny( tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0 );

	return;
}

/*
	ダイレクトタグジャンプ

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.05.12	フォルダ階層も考慮して探す
*/
bool CViewCommander::Command_TagJumpByTagsFile( void )
{
	CNativeW	cmemKey;
	int		i;
	TCHAR	szCurrentPath[1024];	//カレントフォルダ
	TCHAR	szTagFile[1024];		//タグファイル
	TCHAR	szLineData[1024];		//行バッファ
	TCHAR	s[5][1024];
	int		n2;
	int		nRet;
	int		nMatch;						//一致数
	CDlgTagJumpList	cDlgTagJumpList;	//タグジャンプリスト
	FILE	*fp;
	bool	bNoTag = true;
	int		nLoop;

	//現在カーソル位置のキーを取得する。
	m_pCommanderView->GetCurrentTextForSearch( cmemKey );
	if( 0 == cmemKey.GetStringLength() ) return false;	//キーがないなら終わり

	if( ! GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ) return false;

	// ファイル名に応じて探索回数を決定する
	_tcscpy( szCurrentPath, GetDocument()->m_cDocFile.GetFilePath() );
	nLoop = CalcDirectoryDepth( szCurrentPath );

	if( nLoop <  0 ) nLoop =  0;
	if( nLoop > (_MAX_PATH/2) ) nLoop = (_MAX_PATH/2);	//\A\B\C...のようなとき1フォルダで2文字消費するので...

		//パス名のみ取り出す。
		cDlgTagJumpList.SetFileName( szCurrentPath );
		szCurrentPath[ _tcslen( szCurrentPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');

		for( i = 0; i <= nLoop; i++ )
		{
			//タグファイル名を作成する。
			auto_sprintf( szTagFile, _T("%ts%ls"), szCurrentPath, TAG_FILENAME );

			//タグファイルを開く。
			fp = _tfopen( szTagFile, _T("r") );
			if( fp )
			{
				bNoTag = false;
				nMatch = 0;
				while( _fgetts( szLineData, _countof( szLineData ), fp ) )
				{
					if( szLineData[0] <= _T('!') ) goto next_line;	//コメントならスキップ
					//chop( szLineData );

					s[0][0] = s[1][0] = s[2][0] = s[3][0] = s[4][0] = _T('\0');
					n2 = 0;
					//	2004.06.04 Moca ファイル名/パスにスペースが含まれているときに
					//	ダイレクトタグジャンプに失敗していた
					//	sscanf の%[^\t\r\n] でスペースを読みとるように変更
					//	@@ 2005.03.31 MIK TAG_FORMAT定数化
					nRet = _stscanf(
						szLineData, 
						TAG_FORMAT,	//tagsフォーマット
						s[0], s[1], &n2, s[3], s[4]
					);
					if( nRet < 4 ) goto next_line;
					if( n2 <= 0 ) goto next_line;	//行番号不正(-excmd=nが指定されてないかも)

					if( 0 != wcscmp( to_wchar(s[0]), cmemKey.GetStringPtr() ) ) goto next_line;

					//	@@ 2005.03.31 MIK 階層パラメータ追加
					cDlgTagJumpList.AddParam( s[0], s[1], n2, s[3], s[4], i );
					nMatch++;
					continue;

next_line:
					if( nMatch ) break;
				}

				//ファイルを閉じる。
				fclose( fp );

				//複数あれば選択してもらう。
				if( nMatch > 1 )
				{
					if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)0 ) ) 
					{
						nMatch = 0;
						return true;	//キャンセル
					}
				}

				//タグジャンプする。
				if( nMatch > 0 )
				{
					//	@@ 2005.03.31 MIK 階層パラメータ追加
					int depth;
					if( false == cDlgTagJumpList.GetSelectedParam( s[0], s[1], &n2, s[3], s[4], &depth ) )
					{
						return false;
					}

					/*
					 * s[0] キー
					 * s[1] ファイル名
					 * n2   行番号
					 * s[3] タイプ
					 * s[4] コメント
					 * depth (さかのぼる)階層数
					 */

					//完全パス名を作成する。
					TCHAR	*p;
					p = s[1];
					if( p[0] == _T('\\') )	//ドライブなし絶対パスか？
					{
						//	2003.09.20 Moca パスが変
						if( p[1] == _T('\\') )	//ネットワークパスか？
						{
							_tcscpy( szTagFile, p );	//何も加工しない。
						}
						else
						{
							//ドライブ加工したほうがよい？
							_tcscpy( szTagFile, p );	//何も加工しない。
						}
					}
					else if( isalpha( p[0] ) && p[1] == L':' )	//絶対パスか？
					{
						_tcscpy( szTagFile, p );	//何も加工しない。
					}
					else
					{
						auto_sprintf( szTagFile, _T("%ts%ls"), szCurrentPath, p );
					}

					return m_pCommanderView->TagJumpSub( szTagFile, CMyPoint(0, n2) );
				}
			}	//fp

			//カレントパスを1階層上へ。
			_tcscat( szCurrentPath, _T("..\\") );
		}

	return false;
}

/*
	タグファイルを作成する。

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.05.12	ダイアログ表示でフォルダ等を細かく指定できるようにした。
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CViewCommander::Command_TagsMake( void )
{
#define	CTAGS_COMMAND	_T("ctags.exe")

	TCHAR	szTargetPath[1024 /*_MAX_PATH+1*/ ];
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() )
	{
		_tcscpy( szTargetPath, GetDocument()->m_cDocFile.GetFilePath() );
		szTargetPath[ _tcslen( szTargetPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');
	}
	else
	{
		TCHAR	szTmp[1024];
		::GetModuleFileName(
			NULL,
			szTmp, _countof( szTmp )
		);
		/* ファイルのフルパスを、フォルダとファイル名に分割 */
		/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
		::SplitPath_FolderAndFile( szTmp, szTargetPath, NULL );
	}

	//ダイアログを表示する
	CDlgTagsMake	cDlgTagsMake;
	if( !cDlgTagsMake.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0, szTargetPath ) ) return false;

	TCHAR	cmdline[1024];
	/* exeのあるフォルダ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	GetExedir( cmdline, CTAGS_COMMAND );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//ctags.exeの存在チェック
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T( "タグ作成コマンド実行は失敗しました。\n\nCTAGS.EXE が見つかりません。" ) );
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_pCommanderView->GetHwnd() );

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(sa) );
	sa.nLength              = sizeof(sa);
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = NULL;
	hStdOutRead = hStdOutWrite = 0;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE )
	{
		//エラー
		return false;
	}

	//継承不能にする
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS );

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb          = sizeof(sui);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//	To Here Dec. 28, 2002 MIK

	TCHAR	options[1024];
	_tcscpy( options, _T("--excmd=n") );	//デフォルトのオプション
	if( cDlgTagsMake.m_nTagsOpt & 0x0001 ) _tcscat( options, _T(" -R") );	//サブフォルダも対象
	if( _tcslen( cDlgTagsMake.m_szTagsCmdLine ) )	//個別指定のコマンドライン
	{
		_tcscat( options, _T(" ") );
		_tcscat( options, cDlgTagsMake.m_szTagsCmdLine );
	}
	_tcscat( options, _T(" *") );	//配下のすべてのファイル

	//OSバージョン取得
	COsVersionInfo cOsVer;
	//コマンドライン文字列作成(MAX:1024)
	if (cOsVer.IsWin32NT())
	{
		//	2006.08.04 genta add /D to disable autorun
		auto_sprintf( cmdline, _T("cmd.exe /D /C \"\"%ts\\%ts\" %ts\""),
				szExeFolder,	//sakura.exeパス
				CTAGS_COMMAND,	//ctags.exe
				options			//ctagsオプション
			);
	}
	else
	{
		auto_sprintf( cmdline, _T("command.com /C \"%ts\\%ts\" %ts"),
				szExeFolder,	//sakura.exeパス
				CTAGS_COMMAND,	//ctags.exe
				options			//ctagsオプション
			);
	}

	//コマンドライン実行
	BOOL bProcessResult = CreateProcess(
		NULL, cmdline, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, cDlgTagsMake.m_szPath, &sui, &pi
	);
	if( !bProcessResult)
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T("タグ作成コマンド実行は失敗しました。\n\n%ts"), cmdline );
		goto finish;
	}

	{
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		bool	bLoopFlag = true;

		//中断ダイアログ表示
		HWND	hwndCancel;
		HWND	hwndMsg;
		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->m_hwndParent, IDD_EXECRUNNING );
		hwndMsg = ::GetDlgItem( hwndCancel, IDC_STATIC_CMD );
		::SendMessage( hwndMsg, WM_SETTEXT, 0, (LPARAM)L"タグファイルを作成中です。" );

		//実行結果の取り込み
		do {
			// Jun. 04, 2003 genta CPU消費を減らすために200msec待つ
			// その間メッセージ処理が滞らないように待ち方をWaitForSingleObjectから
			// MsgWaitForMultipleObjectに変更
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 200, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//終了していればループフラグをFALSEとする
					//ただしループの終了条件は プロセス終了 && パイプが空
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//処理中のユーザー操作を可能にする
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						break;
					}
					break;
				default:
					break;
			}

			//中断ボタン押下チェック
			if( cDlgCancel.IsCanceled() )
			{
				//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess( pi.hProcess, 0 );
				break;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )	//パイプの中の読み出し待機中の文字数を取得
			{
				if( new_cnt > 0 )												//待機中のものがある
				{
					if( new_cnt >= _countof(work) - 2 )							//パイプから読み出す量を調整
					{
						new_cnt = _countof(work) - 2;
					}
					::ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					if( read_cnt == 0 )
					{
						continue;
					}
					// 2003.11.09 じゅうじ
					//	正常終了の時はメッセージが出力されないので
					//	何か出力されたらエラーメッセージと見なす．
					else {
						//終了処理
						CloseHandle( hStdOutWrite );
						CloseHandle( hStdOutRead  );
						if( pi.hProcess ) CloseHandle( pi.hProcess );
						if( pi.hThread  ) CloseHandle( pi.hThread  );

						cDlgCancel.CloseDialog( TRUE );

						work[ read_cnt ] = L'\0';	// Nov. 15, 2003 genta 表示用に0終端する
						WarningMessage( m_pCommanderView->GetHwnd(), _T("タグ作成コマンド実行は失敗しました。\n\n%hs"), work ); // 2003.11.09 じゅうじ

						return true;
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );

	}


finish:
	//終了処理
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	cDlgCancel.CloseDialog( TRUE );

	InfoMessage(m_pCommanderView->GetHwnd(), _T("タグファイルの作成が終了しました。"));

	return true;
}

/*!
	キーワードを指定してタグジャンプ

	@author MIK
	@date 2005.03.31 新規作成
*/
bool CViewCommander::Command_TagJumpByTagsFileKeyword( const wchar_t* keyword )
{
	CMemory	cmemKey;
	CDlgTagJumpList	cDlgTagJumpList;
	TCHAR	s[5][1024];
	int		n2;
	int depth;
	TCHAR	szTagFile[1024];		//タグファイル
	TCHAR	szCurrentPath[1024];

	if( ! GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ) return false;
	_tcscpy( szCurrentPath, GetDocument()->m_cDocFile.GetFilePath() );

	cDlgTagJumpList.SetFileName( szCurrentPath );
	cDlgTagJumpList.SetKeyword( keyword );

	szCurrentPath[ _tcslen( szCurrentPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');

	if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)1 ) ) 
	{
		return true;	//キャンセル
	}

	//タグジャンプする。
	if( false == cDlgTagJumpList.GetSelectedParam( s[0], s[1], &n2, s[3], s[4], &depth ) )
	{
		return false;
	}

	/*
	 * s[0] キー
	 * s[1] ファイル名
	 * n2   行番号
	 * s[3] タイプ
	 * s[4] コメント
	 * depth (さかのぼる)階層数
	 */

	//完全パス名を作成する。
	TCHAR	*p;
	p = s[1];
	if( p[0] == _T('\\') )	//ドライブなし絶対パスか？
	{
		if( p[1] == _T('\\') )	//ネットワークパスか？
		{
			_tcscpy( szTagFile, p );	//何も加工しない。
		}
		else
		{
			//ドライブ加工したほうがよい？
			_tcscpy( szTagFile, p );	//何も加工しない。
		}
	}
	else if( _istalpha( p[0] ) && p[1] == _T(':') )	//絶対パスか？
	{
		_tcscpy( szTagFile, p );	//何も加工しない。
	}
	else
	{
		for( int i = 0; i < depth; i++ )
		{
			_tcscat( szCurrentPath, _T("..\\") );
		}
		auto_sprintf( szTagFile, _T("%ls%ls"), szCurrentPath, p );
	}

	return m_pCommanderView->TagJumpSub( szTagFile, CMyPoint(0, n2) );
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

//Start 2004.07.14 Kazika 追加
/*!	@brief 結合して表示

	タブウィンドウの結合、非結合を切り替えるコマンドです。
	[共通設定]->[ウィンドウ]->[タブ表示 まとめない]の切り替えと同じです。
	@author Kazika
	@date 2004.07.14 Kazika 新規作成
	@date 2007.06.20 ryoji GetDllShareData().m_TabWndWndplの廃止，グループIDリセット
*/
void CViewCommander::Command_BIND_WINDOW( void )
{
	//タブモードであるならば
	if (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd)
	{
		//タブウィンドウの設定を変更
		GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin = !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin;

		// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
		if( !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			GetDocument()->m_pcEditWnd->WindowTopMost(
				( (DWORD)::GetWindowLongPtr( GetDocument()->m_pcEditWnd->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
			);
		}

		//Start 2004.08.27 Kazika 変更
		//タブウィンドウの設定を変更をブロードキャストする
		CAppNodeManager::Instance()->ResetGroupId();
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,						//タブウィンドウイベント
			(WPARAM)((GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin) ? TWNT_MODE_DISABLE : TWNT_MODE_ENABLE),//タブモード有効/無効化イベント
			(LPARAM)GetDocument()->m_pcEditWnd->GetHwnd(),	//CEditWndのウィンドウハンドル
			m_pCommanderView->GetHwnd());									//自分自身
		//End 2004.08.27 Kazika
	}
}
//End 2004.07.14 Kazika

/*!	@brief 重ねて表示

	@date 2002.01.08 YAZAKI 「左右に並べて表示」すると、
		裏で最大化されているエクスプローラが「元の大きさ」になるバグ修正。
	@date 2003.06.12 MIK タブウインドウ時は動作しないように
	@date 2004.03.19 crayonzen カレントウィンドウを最後に配置．
		ウィンドウが多い場合に2周目以降は右にずらして配置．
	@date 2004.03.20 genta Z-Orderの上から順に並べていくように．(SetWindowPosを利用)
	@date 2007.06.20 ryoji タブモードは解除せずグループ単位で並べる
*/
void CViewCommander::Command_CASCADE( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		struct WNDARR {
			HWND	hWnd;
			int		newX;
			int		newY;
		};

		WNDARR*	pWndArr = new WNDARR[nRowNum];
		int		count = 0;	//	処理対象ウィンドウカウント
		// Mar. 20, 2004 genta 現在のウィンドウを末尾に持っていくのに使う
		int		current_win_index = -1;

		// -----------------------------------------
		//	ウィンドウ(ハンドル)リストの作成
		// -----------------------------------------

		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	Mar. 20, 2004 genta
			//	現在のウィンドウを末尾に持っていくためここではスキップ
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd() ){
				current_win_index = i;
				continue;
			}
			pWndArr[count].hWnd = pEditNodeArr[i].GetHwnd();
			count++;
		}

		//	Mar. 20, 2004 genta
		//	現在のウィンドウを末尾に挿入 inspired by crayonzen
		if( current_win_index >= 0 ){
			pWndArr[count].hWnd = pEditNodeArr[current_win_index].GetHwnd();
			count++;
		}

		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		
		int width = (rcDesktop.right - rcDesktop.left ) * 4 / 5; // Mar. 9, 2003 genta 整数演算のみにする
		int height = (rcDesktop.bottom - rcDesktop.top ) * 4 / 5;
		int w_delta = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXSIZE);
		int h_delta = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CYSIZE);
		int w_offset = rcDesktop.left; //Mar. 19, 2004 crayonzen 絶対値だとエクスプローラーのウィンドウに重なるので
		int h_offset = rcDesktop.top; //初期値をデスクトップ内に収める。

		// -----------------------------------------
		//	座標計算
		//
		//	Mar. 19, 2004 crayonzen
		//		左上をデスクトップ領域に合わせる(タスクバーが上・左にある場合のため)．
		//		ウィンドウが右下からはみ出たら左上に戻るが，
		//		2周目以降は開始位置を右にずらしてアイコンが見えるようにする．
		//
		//	Mar. 20, 2004 genta ここでは計算値を保管するだけでウィンドウの再配置は行わない
		// -----------------------------------------

		int roundtrip = 0; //２度目の描画以降で使用するカウント
		int sw_offset = w_delta; //右スライドの幅

		for(i = 0; i < count; ++i ){
			if (w_offset + width > rcDesktop.right || h_offset + height > rcDesktop.bottom){
				++roundtrip;
				if ((rcDesktop.right - rcDesktop.left) - sw_offset * roundtrip < width){
					//	これ以上右にずらせないときはしょうがないから左上に戻る
					roundtrip = 0;
				}
				//	ウィンドウ領域の左上にセット
				//	craonzen 初期値修正(２度目以降の描画で少しづつスライド)
				w_offset = rcDesktop.left + sw_offset * roundtrip;
				h_offset = rcDesktop.top;
			}
			
			pWndArr[i].newX = w_offset;
			pWndArr[i].newY = h_offset;

			w_offset += w_delta;
			h_offset += h_delta;
		}

		// -----------------------------------------
		//	最大化/非表示解除
		//	最大化されたウィンドウを元に戻す．これがないと，最大化ウィンドウが
		//	最大化状態のまま並び替えられてしまい，その後最大化動作が変になる．
		//
		//	Sep. 04, 2004 genta
		// -----------------------------------------
		for( i = 0; i < count; i++ ){
			::ShowWindow( pWndArr[i].hWnd, SW_RESTORE | SW_SHOWNA );
		}

		// -----------------------------------------
		//	ウィンドウ配置
		//
		//	Mar. 20, 2004 genta APIを素直に使ってZ-Orderの上から下の順で並べる．
		// -----------------------------------------

		// まずカレントを最前面に
		i = count - 1;
		
		::SetWindowPos(
			pWndArr[i].hWnd, HWND_TOP,
			pWndArr[i].newX, pWndArr[i].newY,
			width, height,
			0
		);

		// 残りを1つずつ下に入れていく
		while( --i >= 0 ){
			::SetWindowPos(
				pWndArr[i].hWnd, pWndArr[i + 1].hWnd,
				pWndArr[i].newX, pWndArr[i].newY,
				width, height,
				SWP_NOACTIVATE
			);
		}

		delete [] pWndArr;
		delete [] pEditNodeArr;
	}
	return;
}




//左右に並べて表示
void CViewCommander::Command_TILE_H( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	現在のウィンドウを先頭に持ってくる
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd() ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = CEditWnd::Instance()->GetHwnd();
			}
			else {
				phwndArr[count] = pEditNodeArr[i].GetHwnd();
			}
			//	To Here Jul. 28, 2002 genta
			count++;
		}
		int width = (rcDesktop.right - rcDesktop.left ) / count;
		for(i = 0; i < count; ++i ){
			//	Jul. 21, 2002 genta
			::ShowWindow( phwndArr[i], SW_RESTORE );
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta タスクバーが左にある場合を考慮
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus( phwndArr[0] );	// Aug. 17, 2002 MIK
		delete [] phwndArr;
		delete [] pEditNodeArr;
	}
	return;
}




//上下に並べて表示
void CViewCommander::Command_TILE_V( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	現在のウィンドウを先頭に持ってくる
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd() ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = CEditWnd::Instance()->GetHwnd();
			}
			else {
				phwndArr[count] = pEditNodeArr[i].GetHwnd();
			}
			//	To Here Jul. 28, 2002 genta
			count++;
		}
		int height = (rcDesktop.bottom - rcDesktop.top ) / count;
		for(i = 0; i < count; ++i ){
			//	Jul. 21, 2002 genta
			::ShowWindow( phwndArr[i], SW_RESTORE );
			::SetWindowPos(
				phwndArr[i], 0,
				rcDesktop.left, rcDesktop.top + height * i, //Mar. 19, 2004 crayonzen 上端調整
				rcDesktop.right - rcDesktop.left, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus( phwndArr[0] );	// Aug. 17, 2002 MIK

		delete [] phwndArr;
		delete [] pEditNodeArr;
	}
	return;
}



//縦方向に最大化
void CViewCommander::Command_MAXIMIZE_V( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;
	hwndFrame = GetMainWindow();
	::GetWindowRect( hwndFrame, &rcOrg );
	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect( hwndFrame, &rcDesktop );
	::SetWindowPos(
		hwndFrame, 0,
		rcOrg.left, rcDesktop.top,
		rcOrg.right - rcOrg.left, rcDesktop.bottom - rcDesktop.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}




//2001.02.10 Start by MIK: 横方向に最大化
//横方向に最大化
void CViewCommander::Command_MAXIMIZE_H( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;

	hwndFrame = GetMainWindow();
	::GetWindowRect( hwndFrame, &rcOrg );
	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect( hwndFrame, &rcDesktop );
	::SetWindowPos(
		hwndFrame, 0,
		rcDesktop.left, rcOrg.top,
		rcDesktop.right - rcDesktop.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}
//2001.02.10 End: 横方向に最大化




/* すべて最小化 */	//	Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
void CViewCommander::Command_MINIMIZE_ALL( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = GetDllShareData().m_sNodes.m_nEditArrNum;
	if( 0 == j ){
		return;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = GetDllShareData().m_sNodes.m_pEditArr[i].GetHwnd();
	}
	for( i = 0; i < j; ++i ){
		if( IsSakuraMainWindow( phWndArr[i] ) )
		{
			if( ::IsWindowVisible( phWndArr[i] ) )
				::ShowWindow( phWndArr[i], SW_MINIMIZE );
		}
	}
	delete [] phWndArr;
	return;
}




//置換(置換ダイアログ)
void CViewCommander::Command_REPLACE_DIALOG( void )
{
	BOOL		bSelected = FALSE;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	CNativeW	cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* 検索文字列を初期化 */
	wcscpy( GetEditWindow()->m_cDlgReplace.m_szText, cmemCurText.GetStringPtr() );
	wcsncpy( GetEditWindow()->m_cDlgReplace.m_szText2, GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0], MAX_PATH - 1 );	// 2006.08.23 ryoji 前回の置換後文字列を引き継ぐ
	GetEditWindow()->m_cDlgReplace.m_szText2[MAX_PATH - 1] = L'\0';

	if ( m_pCommanderView->GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne() ) {
		bSelected = TRUE;	//選択範囲をチェックしてダイアログ表示
	}else{
		bSelected = FALSE;	//ファイル全体をチェックしてダイアログ表示
	}
	/* 置換オプションの初期化 */
	GetEditWindow()->m_cDlgReplace.m_nReplaceTarget=0;	/* 置換対象 */
	GetEditWindow()->m_cDlgReplace.m_nPaste=FALSE;		/* 貼り付ける？ */
// To Here 2001.12.03 hor

	/* 置換ダイアログの表示 */
	//	From Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	if( !::IsWindow( GetEditWindow()->m_cDlgReplace.GetHwnd() ) ){
		GetEditWindow()->m_cDlgReplace.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)m_pCommanderView, bSelected );
	}
	else {
		/* アクティブにする */
		ActivateFrameWindow( GetEditWindow()->m_cDlgReplace.GetHwnd() );
		::DlgItem_SetText( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_COMBO_TEXT, cmemCurText.GetStringT() );
	}
	//	To Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	return;
}

/*! 置換実行
	
	@date 2002/04/08 親ウィンドウを指定するように変更。
	@date 2003.05.17 かろと 長さ０マッチの無限置換回避など
*/
void CViewCommander::Command_REPLACE( HWND hwndParent )
{
	if ( hwndParent == NULL ){	//	親ウィンドウが指定されていなければ、CEditViewが親。
		hwndParent = m_pCommanderView->GetHwnd();
	}
	//2002.02.10 hor
	int nPaste			=	GetEditWindow()->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	=	GetEditWindow()->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		=	GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp;
	int nFlag			=	GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase ? 0x01 : 0x00;

	// From Here 2001.12.03 hor
	if( nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste()){
		OkMessage( hwndParent, _T("クリップボードに有効なデータがありません！") );
		::CheckDlgButton( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );
		return;	//	失敗return;
	}

	// 2002.01.09 hor
	// 選択エリアがあれば、その先頭にカーソルを移す
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			GetCaret().MoveCursor( GetSelect().GetFrom(), true );
		} else {
//			HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
			Command_LEFT( FALSE, FALSE );
		}
	}
	// To Here 2002.01.09 hor
	
	// 矩形選択？
//			bBeginBoxSelect = m_pCommanderView->GetSelectionInfo().IsBoxSelecting();

	/* カーソル左移動 */
	//HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );	//？？？
	// To Here 2001.12.03 hor

	/* テキスト選択解除 */
	/* 現在の選択範囲を非選択状態に戻す */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

	// 2004.06.01 Moca 検索中に、他のプロセスによってm_aReplaceKeysが書き換えられても大丈夫なように
	const CNativeW	cMemRepKey( GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0] );

	/* 次を検索 */
	Command_SEARCH_NEXT( true, TRUE, hwndParent, 0 );

	/* テキストが選択されているか */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// From Here 2001.12.03 hor
		CLayoutPoint ptTmp(0,0);
		if ( !bRegularExp ) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if(nReplaceTarget==1){	//挿入位置へ移動
				ptTmp = GetSelect().GetTo() - GetSelect().GetFrom();
				GetSelect().Clear(-1);
			}
			else if(nReplaceTarget==2){	//追加位置へ移動
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				GetSelect().Clear(-1);
			}
			else{
				// 位置指定ないので、何もしない
			}
		}
		/* コマンドコードによる処理振り分け */
		/* テキストを貼り付け */
		if(nPaste){
			Command_PASTE();
		} else if ( bRegularExp ) { /* 検索／置換  1==正規表現 */
			// 先読みに対応するために物理行末までを使うように変更 2005/03/27 かろと
			// 2002/01/19 novice 正規表現による文字列置換
			CMemory cmemory;
			CBregexp cRegexp;

			if( !InitRegexp( m_pCommanderView->GetHwnd(), cRegexp, true ) ){
				return;	//	失敗return;
			}

			// 物理行、物理行長、物理行での検索マッチ位置
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			CLogicInt nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() ) + pcLayout->GetLogicOffset();
			CLogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			// 正規表現で選択始点・終点への挿入を記述
			//	Jun. 6, 2005 かろと
			// →これでは「検索の後ろの文字が改行だったら次の行頭へ移動」が処理できない
			// → Oct. 30, 「検索の後ろの文字が改行だったら・・」の処理をやめる（誰もしらないみたいなので）
			// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
			CNativeW cMemMatchStr; cMemMatchStr.SetString(L"$&");
			CNativeW cMemRepKey2;
			if (nReplaceTarget == 1) {	//選択始点へ挿入
				cMemRepKey2 = cMemRepKey;
				cMemRepKey2 += cMemMatchStr;
			} else if (nReplaceTarget == 2) { // 選択終点へ挿入
				cMemRepKey2 = cMemMatchStr;
				cMemRepKey2 += cMemRepKey;
			} else {
				cMemRepKey2 = cMemRepKey;
			}
			cRegexp.Compile( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], cMemRepKey2.GetStringPtr(), nFlag);
			if( cRegexp.Replace(pLine, nLen, nIdx) ){
				// From Here Jun. 6, 2005 かろと
				// 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				// キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				// そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				CLayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;
				CLogicInt matchLen = cRegexp.GetMatchLen();
				CLogicInt nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				if (matchLen == 0) {
					// ０文字マッチの時(無限置換にならないように１文字進める)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// 無限置換しないように、１文字増やしたので１文字選択に変更
					// 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
					rLayoutMgr.LogicToLayout( CLogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 行位置も取得する
				}
				// 行末から検索文字列末尾までの文字数
				CLogicInt colDiff = nLen - nIdxTo;
				//	Oct. 22, 2005 Karoto
				//	\rを置換するとその後ろの\nが消えてしまう問題の対応
				if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					// 改行にかかっていたら、行全体をINSTEXTする。
					colDiff = CLogicInt(0);
					rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 追加
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 かろと
			}
		}else{
			//	HandleCommand( F_INSTEXT_W, FALSE, (LPARAM)GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0], FALSE, 0, 0 );
			Command_INSTEXT( FALSE, cMemRepKey.GetStringPtr(), cMemRepKey.GetStringLength(), TRUE );
		}

		// 挿入後の検索開始位置を調整
		if(nReplaceTarget==1){
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos()+ptTmp);
		}

		// To Here 2001.12.03 hor
		/* 最後まで置換した時にOK押すまで置換前の状態が表示されるので、
		** 置換後、次を検索する前に書き直す 2003.05.17 かろと
		*/
		m_pCommanderView->Redraw();

		/* 次を検索 */
		Command_SEARCH_NEXT( true, TRUE, hwndParent, LTEXT("最後まで置換しました。") );
	}
}

/*! すべて置換実行

	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理など見直し
	@date 2006.03.31 かろと 行置換機能追加
	@date 2007.01.16 ryoji 行置換機能を全置換のオプションに変更
	@date 2009.09.20 genta 左下～右上で矩形選択された領域の置換が行われない
*/
void CViewCommander::Command_REPLACE_ALL()
{




	//2002.02.10 hor
	int nPaste			= GetEditWindow()->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	= GetEditWindow()->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp;
	int bSelectedArea	= GetDllShareData().m_Common.m_sSearch.m_bSelectedArea;
	int bConsecutiveAll	= GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll;	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji

	GetEditWindow()->m_cDlgReplace.m_bCanceled=false;
	GetEditWindow()->m_cDlgReplace.m_nReplaceCnt=0;

	// From Here 2001.12.03 hor
	if( nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste() ){
		OkMessage( m_pCommanderView->GetHwnd(), _T("クリップボードに有効なデータがありません！") );
		::CheckDlgButton( m_pCommanderView->GetHwnd(), IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_pCommanderView->GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor

	bool		bBeginBoxSelect; // 矩形選択？
	if(m_pCommanderView->GetSelectionInfo().IsTextSelected()){
		bBeginBoxSelect=m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	}
	else{
		bSelectedArea=FALSE;
		bBeginBoxSelect=false;
	}

	/* 表示処理ON/OFF */
	bool bDisplayUpdate = false;

	m_pCommanderView->SetDrawSwitch(bDisplayUpdate);

	int	nAllLineNum = (Int)GetDocument()->m_cLayoutMgr.GetLineCount();

	/* 進捗表示&中止ダイアログの作成 */
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), IDD_REPLACERUNNING );
	::EnableWindow( m_pCommanderView->GetHwnd(), FALSE );
	::EnableWindow( ::GetParent( m_pCommanderView->GetHwnd() ), FALSE );
	::EnableWindow( ::GetParent( ::GetParent( m_pCommanderView->GetHwnd() ) ), FALSE );
	//<< 2002/03/26 Azumaiya
	// 割り算掛け算をせずに進歩状況を表せるように、シフト演算をする。
	int nShiftCount;
	for ( nShiftCount = 0; SHRT_MAX < nAllLineNum; nShiftCount++ )
	{
		nAllLineNum/=2;
	}
	//>> 2002/03/26 Azumaiya

	/* プログレスバー初期化 */
	HWND		hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
	::SendMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, nAllLineNum ) );
	int			nNewPos = 0;
 	::SendMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );

	/* 置換個数初期化 */
	int			nReplaceNum = 0;
	HWND		hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
	TCHAR szLabel[64];
	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	CLayoutRange sRangeA;	//選択範囲
	CLogicPoint ptColLineP;

	// From Here 2001.12.03 hor
	if (bSelectedArea){
		/* 選択範囲置換 */
		/* 選択範囲開始位置の取得 */
		sRangeA = GetSelect();

		//	From Here 2007.09.20 genta 矩形範囲の選択置換ができない
		//	左下～右上と選択した場合，m_nSelectColmTo < m_nSelectColmFrom となるが，
		//	範囲チェックで colFrom < colTo を仮定しているので，
		//	矩形選択の場合は左上～右下指定になるよう桁を入れ換える．
		if( bBeginBoxSelect && sRangeA.GetTo().x < sRangeA.GetFrom().x )
			t_swap(sRangeA.GetFromPointer()->x,sRangeA.GetToPointer()->x);
		//	To Here 2007.09.20 genta 矩形範囲の選択置換ができない

		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			sRangeA.GetTo(),
			&ptColLineP
		);
		//選択範囲開始位置へ移動
		GetCaret().MoveCursor( sRangeA.GetFrom(), bDisplayUpdate );
	}
	else{
		/* ファイル全体置換 */
		/* ファイルの先頭に移動 */
	//	HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
		Command_GOFILETOP(bDisplayUpdate);
	}

	CLayoutPoint ptLast = GetCaret().GetCaretLayoutPos();

	/* テキスト選択解除 */
	/* 現在の選択範囲を非選択状態に戻す */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( bDisplayUpdate );
	/* 次を検索 */
	Command_SEARCH_NEXT( true, bDisplayUpdate, 0, 0 );
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// 速く動かすことを最優先に組んでみました。
	// ループの外で文字列の長さを特定できるので、一時変数化。
	const wchar_t *szREPLACEKEY;		// 置換後文字列。
	bool		bColmnSelect;	// 矩形貼り付けを行うかどうか。
	CNativeW	cmemClip;		// 置換後文字列のデータ（データを格納するだけで、ループ内ではこの形ではデータを扱いません）。

	// クリップボードからのデータ貼り付けかどうか。
	if( nPaste != 0 )
	{
		// クリップボードからデータを取得。
		if ( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColmnSelect ) )
		{
			ErrorBeep();
			return;
		}

		// 矩形貼り付けが許可されていて、クリップボードのデータが矩形選択のとき。
		if ( GetDllShareData().m_Common.m_sEdit.m_bAutoColmnPaste && bColmnSelect )
		{
			// マウスによる範囲選択中
			if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )
			{
				ErrorBeep();
				return;
			}

			// 現在のフォントは固定幅フォントである
			if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )
			{
				return;
			}
		}
		else
		// クリップボードからのデータは普通に扱う。
		{
			bColmnSelect = false;
		}
	}
	else
	{
		// 2004.05.14 Moca 全置換の途中で他のウィンドウで置換されるとまずいのでコピーする
		cmemClip.SetString( GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0] );
	}

	CLogicInt nREPLACEKEY;			// 置換後文字列の長さ。
	szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);

	// 取得にステップがかかりそうな変数などを、一時変数化する。
	// とはいえ、これらの操作をすることによって得をするクロック数は合わせても 1 ループで数十だと思います。
	// 数百クロック毎ループのオーダーから考えてもそんなに得はしないように思いますけど・・・。
	BOOL bAddCRLFWhenCopy = GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy;
	BOOL &bCANCEL = cDlgCancel.m_bCANCEL;
	CDocLineMgr& rDocLineMgr = GetDocument()->m_cDocLineMgr;
	CLayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;

	//  クラス関係をループの中で宣言してしまうと、毎ループごとにコンストラクタ、デストラクタが
	// 呼ばれて遅くなるので、ここで宣言。
	CMemory cmemory;
	CBregexp cRegexp;
	// 初期化も同様に毎ループごとにやると遅いので、最初に済ましてしまう。
	if( bRegularExp )
	{
		if ( !InitRegexp( m_pCommanderView->GetHwnd(), cRegexp, true ) )
		{
			return;
		}

		const CNativeW	cMemRepKey( szREPLACEKEY, wcslen(szREPLACEKEY) );
		// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
		CNativeW cMemRepKey2;
		CNativeW cMemMatchStr;
		cMemMatchStr.SetString(L"$&");
		if (nReplaceTarget == 1 ) {	//選択始点へ挿入
			cMemRepKey2 = cMemRepKey;
			cMemRepKey2 += cMemMatchStr;
		} else if (nReplaceTarget == 2) { // 選択終点へ挿入
			cMemRepKey2 = cMemMatchStr;
			cMemRepKey2 += cMemRepKey;
		} else {
			cMemRepKey2 = cMemRepKey;
		}
		// 正規表現オプションの設定2006.04.01 かろと
		int nFlag = (GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase ? CBregexp::optCaseSensitive : CBregexp::optNothing);
		nFlag |= (bConsecutiveAll ? CBregexp::optNothing : CBregexp::optGlobal);	// 2007.01.16 ryoji
		cRegexp.Compile(GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], cMemRepKey2.GetStringPtr(), nFlag);
	}

	//$$ 単位混在
	CLayoutPoint	ptOld;						//検索後の選択範囲
	/*CLogicInt*/int		lineCnt;					//置換前の行数
	/*CLayoutInt*/int		linDif = (0);		//置換後の行調整
	/*CLayoutInt*/int		colDif = (0);		//置換後の桁調整
	/*CLayoutInt*/int		linPrev = (0);	//前回の検索行(矩形) @@@2001.12.31 YAZAKI warning退治
	/*CLogicInt*/int		linOldLen = (0);	//検査後の行の長さ
	/*CLayoutInt*/int		linNext;					//次回の検索行(矩形)

	/* テキストが選択されているか */
	while( m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		/* キャンセルされたか */
		if( bCANCEL )
		{
			break;
		}

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( hwndCancel ) )
		{
			return;// -1;
		}

		// 128 ごとに表示。
		if( 0 == (nReplaceNum & 0x7F ) )
		// 時間ごとに進歩状況描画だと時間取得分遅くなると思うが、そちらの方が自然だと思うので・・・。
		// と思ったけど、逆にこちらの方が自然ではないので、やめる。
		{
			nNewPos = (Int)GetSelect().GetFrom().GetY2() >> nShiftCount;
			::PostMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			_itot( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );
		}

		// From Here 2001.12.03 hor
		/* 検索後の位置を確認 */
		if( bSelectedArea )
		{
			// 矩形選択
			//	o レイアウト座標をチェックしながら置換する
			//	o 折り返しがあると変になるかも・・・
			//
			if ( bBeginBoxSelect )
			{
				// 検索時の行数を記憶
				lineCnt = (Int)rLayoutMgr.GetLineCount();
				// 検索後の範囲終端
				ptOld = GetSelect().GetTo();
				// 前回の検索行と違う？
				if(ptOld.y!=linPrev){
					colDif=(0);
				}
				linPrev=(Int)ptOld.GetY2();
				// 行は範囲内？
				if ((sRangeA.GetTo().y+linDif == ptOld.y && sRangeA.GetTo().GetX2()+colDif < ptOld.x) ||
					(sRangeA.GetTo().y+linDif <  ptOld.y)) {
					break;
				}
				// 桁は範囲内？
				if(!(sRangeA.GetFrom().x<=GetSelect().GetFrom().x && ptOld.GetX2()<=sRangeA.GetTo().GetX2()+colDif)){
					if(ptOld.x<sRangeA.GetTo().GetX2()+colDif){
						linNext=(Int)GetSelect().GetTo().GetY2();
					}else{
						linNext=(Int)GetSelect().GetTo().GetY2()+1;
					}
					//次の検索開始位置へシフト
					GetCaret().SetCaretLayoutPos(CLayoutPoint(sRangeA.GetFrom().x,CLayoutInt(linNext)));
					// 2004.05.30 Moca 現在の検索文字列を使って検索する
					Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
					colDif=(0);
					continue;
				}
			}
			// 普通の選択
			//	o 物理座標をチェックしながら置換する
			//
			else {
				// 検索時の行数を記憶
				lineCnt = rDocLineMgr.GetLineCount();

				// 検索後の範囲終端
				CLogicPoint ptOldTmp;
				rLayoutMgr.LayoutToLogic(
					GetSelect().GetTo(),
					&ptOldTmp
				);
				ptOld=CLayoutPoint(ptOldTmp); //$$ レイアウト型に無理やりロジック型を代入。気持ち悪い

				// 置換前の行の長さ(改行は１文字と数える)を保存しておいて、置換前後で行位置が変わった場合に使用
				linOldLen = rDocLineMgr.GetLine(ptOldTmp.GetY2())->GetLengthWithoutEOL() + CLogicInt(1);

				// 行は範囲内？
				// 2007.01.19 ryoji 条件追加: 選択終点が行頭(ptColLineP.x == 0)になっている場合は前の行の行末までを選択範囲とみなす
				// （選択始点が行頭ならその行頭は選択範囲に含み、終点が行頭ならその行頭は選択範囲に含まない、とする）
				// 論理的に少し変と指摘されるかもしれないが、実用上はそのようにしたほうが望ましいケースが多いと思われる。
				// ※行選択で行末までを選択範囲にしたつもりでも、UI上は次の行の行頭にカーソルが行く
				// ※終点の行頭を「^」にマッチさせたかったら１文字以上選択してね、ということで．．．
				// $$ 単位混在しまくりだけど、大丈夫？？
				if ((ptColLineP.y+linDif == (Int)ptOld.y && (ptColLineP.x+colDif < (Int)ptOld.x || ptColLineP.x == 0))
					|| ptColLineP.y+linDif < (Int)ptOld.y) {
					break;
				}
			}
		}


		CLayoutPoint ptTmp(0,0);

		if ( !bRegularExp ) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if( nReplaceTarget == 1 )	//挿入位置セット
			{
				ptTmp.x = GetSelect().GetTo().x - GetSelect().GetFrom().x;
				ptTmp.y = GetSelect().GetTo().y - GetSelect().GetFrom().y;
				GetSelect().Clear(-1);
			}
			else if( nReplaceTarget == 2 )	//追加位置セット
			{
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				GetSelect().Clear(-1);
		    }
			else {
				// 位置指定ないので、何もしない
			}
		}

		/* コマンドコードによる処理振り分け */
		/* テキストを貼り付け */
		if( nPaste )
		{
			if ( !bColmnSelect )
			{
				/* 本当は Command_INSTEXT を使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
				** →m_nSelectXXXが-1の時に m_pCommanderView->ReplaceData_CEditViewを直接たたくと動作不良となるため
				**   直接たたくのやめた。2003.05.18 by かろと
				*/
				Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			}
			else
			{
				Command_PASTEBOX(szREPLACEKEY, nREPLACEKEY);
				// 再描画を行わないとどんな結果が起きているのか分からずみっともないので・・・。
				m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
				m_pCommanderView->Redraw();
			}
		}
		// 2002/01/19 novice 正規表現による文字列置換
		else if( bRegularExp ) /* 検索／置換  1==正規表現 */
		{
			// 物理行、物理行長、物理行での検索マッチ位置
			const CLayout* pcLayout = rLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			CLogicInt nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() ) + pcLayout->GetLogicOffset();
			CLogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			CLogicInt colDiff = CLogicInt(0);
			if( !bConsecutiveAll ){	// 一括置換
				// 2007.01.16 ryoji
				// 選択範囲置換の場合は行内の選択範囲末尾まで置換範囲を縮め，
				// その位置を記憶する．
				if( bSelectedArea ){
					if( bBeginBoxSelect ){	// 矩形選択
						CLogicPoint ptWork;
						rLayoutMgr.LayoutToLogic(
							CLayoutPoint(sRangeA.GetTo().x,ptOld.y),
							&ptWork
						);
						ptColLineP.x = ptWork.x;
						if( nLen - pcLayout->GetDocLineRef()->GetEol().GetLen() > ptColLineP.x + colDif )
							nLen = ptColLineP.GetX2() + CLogicInt(colDif);
					} else {	// 通常の選択
						if( ptColLineP.y+linDif == (Int)ptOld.y ){ //$$ 単位混在
							if( nLen - pcLayout->GetDocLineRef()->GetEol().GetLen() > ptColLineP.x + colDif )
								nLen = ptColLineP.GetX2() + CLogicInt(colDif);
						}
					}
				}

				if(pcLayout->GetDocLineRef()->GetLengthWithoutEOL() < nLen)
					ptOld.x = (CLayoutInt)(Int)pcLayout->GetDocLineRef()->GetLengthWithoutEOL() + 1; //$$ 単位混在
				else
					ptOld.x = (CLayoutInt)(Int)nLen; //$$ 単位混在
			}

			if( int nReplace = cRegexp.Replace(pLine, nLen, nIdx) ){
				nReplaceNum += nReplace;
				if ( !bConsecutiveAll ) { // 2006.04.01 かろと	// 2007.01.16 ryoji
					// 行単位での置換処理
					// 選択範囲を物理行末までにのばす
					rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );
				} else {
				    // From Here Jun. 6, 2005 かろと
				    // 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				    // キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				    // そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				    CLogicInt matchLen = cRegexp.GetMatchLen();
				    CLogicInt nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				    if (matchLen == 0) {
					    // ０文字マッチの時(無限置換にならないように１文字進める)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // 無限置換しないように、１文字増やしたので１文字選択に変更
					    // 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
						rLayoutMgr.LogicToLayout( CLogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 行位置も取得する
				    }
				    // 行末から検索文字列末尾までの文字数
					colDiff =  nLen - nIdxTo;
					ptOld.x = (CLayoutInt)(Int)nIdxTo;	// 2007.01.19 ryoji 追加  // $$ 単位混在
				    //	Oct. 22, 2005 Karoto
				    //	\rを置換するとその後ろの\nが消えてしまう問題の対応
				    if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					    // 改行にかかっていたら、行全体をINSTEXTする。
					    colDiff = CLogicInt(0);
						rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji 追加
						ptOld.x = (CLayoutInt)(Int)pcLayout->GetDocLineRef()->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji 追加 //$$ 単位混在
				    }
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 かろと
			}
		}
		else
		{
			/* 本当は元コードを使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
			** →m_nSelectXXXが-1の時に m_pCommanderView->ReplaceData_CEditViewを直接たたくと動作不良となるため直接たたくのやめた。2003.05.18 かろと
			*/
			Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			++nReplaceNum;
		}

		// 挿入後の位置調整
		if( nReplaceTarget == 1 )
		{
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos()+ptTmp);
			if (!bBeginBoxSelect)
			{
				CLogicPoint p;
				rLayoutMgr.LayoutToLogic(
					GetCaret().GetCaretLayoutPos(),
					&p
				);
				GetCaret().SetCaretLogicPos(p);
			}
		}

		// 最後に置換した位置を記憶
		ptLast = GetCaret().GetCaretLayoutPos();

		/* 置換後の位置を確認 */
		if( bSelectedArea )
		{
			// 検索→置換の行補正値取得
			if( bBeginBoxSelect )
			{
				colDif += (Int)(ptLast.GetX2() - ptOld.GetX2());
				linDif += (Int)(rLayoutMgr.GetLineCount() - lineCnt);
			}
			else{
				// 置換前の検索文字列の最終位置は ptOld
				// 置換後のカーソル位置
				CLogicPoint ptTmp2 = GetCaret().GetCaretLogicPos();
				int linDif_thistime = rDocLineMgr.GetLineCount() - lineCnt;	// 今回置換での行数変化
				linDif += linDif_thistime;
				if( ptColLineP.y + linDif == ptTmp2.y)
				{
					// 最終行で置換した時、又は、置換の結果、選択エリア最終行まで到達した時
					// 最終行なので、置換前後の文字数の増減で桁位置を調整する
					colDif += (Int)ptTmp2.GetX2() - (Int)ptOld.GetX2(); //$$ 単位混在

					// 但し、以下の場合は置換前後で行が異なってしまうので、行の長さで補正する必要がある
					// １）最終行直前で行連結が起こり、行が減っている場合（行連結なので、桁位置は置換後のカーソル桁位置分増加する）
					// 　　ptTmp2.x-ptOld.xだと、\r\n → "" 置換で行連結した場合に、桁位置が負になり失敗する（負とは前行の後ろの方になることなので補正する）
					// 　　今回置換での行数の変化(linDif_thistime)で、最終行が行連結されたかどうかを見ることにする
					// ２）改行を置換した（ptTmp2.y!=ptOld.y）場合、改行を置換すると置換後の桁位置が次行の桁位置になっているため
					//     ptTmp2.x-ptOld.xだと、負の数となり、\r\n → \n や \n → "abc" などで桁位置がずれる
					//     これも前行の長さで調整する必要がある
					if (linDif_thistime < 0 || ptTmp2.y != (Int)ptOld.y) { //$$ 単位混在
						colDif += linOldLen;
					}
				}
			}
		}
		// To Here 2001.12.03 hor

		/* 次を検索 */
		// 2004.05.30 Moca 現在の検索文字列を使って検索する
		Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
	}

	if( 0 < nAllLineNum )
	{
		nNewPos = (Int)GetSelect().GetFrom().GetY2() >> nShiftCount;
		::SendMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	//>> 2002/03/26 Azumaiya

	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	if( !cDlgCancel.IsCanceled() ){
		nNewPos = nAllLineNum;
		::SendMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	cDlgCancel.CloseDialog( 0 );
	::EnableWindow( m_pCommanderView->GetHwnd(), TRUE );
	::EnableWindow( ::GetParent( m_pCommanderView->GetHwnd() ), TRUE );
	::EnableWindow( ::GetParent( ::GetParent( m_pCommanderView->GetHwnd() ) ), TRUE );

	// From Here 2001.12.03 hor

	/* テキスト選択解除 */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

	/* カーソル・選択範囲復元 */
	if((!bSelectedArea) ||			// ファイル全体置換
	   (cDlgCancel.IsCanceled())) {		// キャンセルされた
		// 最後に置換した文字列の右へ
		GetCaret().MoveCursor( ptLast, TRUE );
	}
	else{
		if (bBeginBoxSelect) {
			// 矩形選択
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			sRangeA.GetToPointer()->y += linDif;
			if(sRangeA.GetTo().y<0)sRangeA.SetToY(CLayoutInt(0));
		}
		else{
			// 普通の選択
			ptColLineP.x+=colDif;
			if(ptColLineP.x<0)ptColLineP.x=0;
			ptColLineP.y+=linDif;
			if(ptColLineP.y<0)ptColLineP.y=0;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptColLineP,
				sRangeA.GetToPointer()
			);
		}
		if(sRangeA.GetFrom().y<sRangeA.GetTo().y || sRangeA.GetFrom().x<sRangeA.GetTo().x){
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );	// 2009.07.25 ryoji
		}
		GetCaret().MoveCursor( sRangeA.GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	GetEditWindow()->m_cDlgReplace.m_bCanceled = (cDlgCancel.IsCanceled() != FALSE);
	GetEditWindow()->m_cDlgReplace.m_nReplaceCnt=nReplaceNum;
	m_pCommanderView->SetDrawSwitch(true);
	ActivateFrameWindow( GetMainWindow() );
}



/* カーソル行をウィンドウ中央へ */
void CViewCommander::Command_CURLINECENTER( void )
{
	CLayoutInt		nViewTopLine;
	nViewTopLine = GetCaret().GetCaretLayoutPos().GetY2() - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 );

	// sui 02/08/09
	if( 0 > nViewTopLine )	nViewTopLine = CLayoutInt(0);
	
	CLayoutInt nScrollLines = nViewTopLine - m_pCommanderView->GetTextArea().GetViewTopLine();	//Sep. 11, 2004 genta 同期用に行数を記憶
	m_pCommanderView->GetTextArea().SetViewTopLine( nViewTopLine );
	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();
	// sui 02/08/09

	//	Sep. 11, 2004 genta 同期スクロールの関数化
	m_pCommanderView->SyncScrollV( nScrollLines );
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




/* 再描画 */
void CViewCommander::Command_REDRAW( void )
{
	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();
	return;
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
	CAppMode::Instance()->SetViewMode(!CAppMode::Instance()->IsViewMode());

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




/* グループを閉じる */	// 2007.06.20 ryoji 追加
void CViewCommander::Command_GROUPCLOSE( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		int nGroup = CAppNodeManager::Instance()->GetEditNode( GetMainWindow() )->GetGroup();
		CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, nGroup );
	}
	return;
}

/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
void CViewCommander::Command_FILECLOSEALL( void )
{
	int nGroup = CAppNodeManager::Instance()->GetEditNode( GetMainWindow() )->GetGroup();
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), FALSE, nGroup );	// 2006.12.25, 2007.02.13 ryoji 引数追加
	return;
}




/* ウィンドウを閉じる */
void CViewCommander::Command_WINCLOSE( void )
{
	/* 閉じる */
	::PostMessageAny( GetMainWindow(), MYWM_CLOSE, FALSE, 0 );	// 2007.02.13 ryoji WM_CLOSE→MYWM_CLOSEに変更
	return;
}

//アウトプットウィンドウ表示
void CViewCommander::Command_WIN_OUTPUT( void )
{
	if( NULL == GetDllShareData().m_sHandles.m_hwndDebug || !IsSakuraMainWindow(GetDllShareData().m_sHandles.m_hwndDebug) ){
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_SJIS;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( NULL, m_pCommanderView->GetHwnd(), sLoadInfo, _T("-DEBUGMODE"), true );
	}else{
		/* 開いているウィンドウをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( GetDllShareData().m_sHandles.m_hwndDebug );
	}
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
		if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
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
				GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , pszMenuLabel );
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
	SetClipboardText( CEditWnd::Instance()->m_cSplitterWnd.GetHwnd(), cMemKeyList.GetStringPtr(), cMemKeyList.GetStringLength() );
}

/* ファイル内容比較 */
void CViewCommander::Command_COMPARE( void )
{
	HWND		hwndCompareWnd;
	TCHAR		szPath[_MAX_PATH + 1];
	CMyPoint	poDes;
	CDlgCompare	cDlgCompare;
	BOOL		bDefferent;
	const wchar_t*	pLineSrc;
	CLogicInt		nLineLenSrc;
	const wchar_t*	pLineDes;
	int			nLineLenDes;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK

	/* 比較後、左右に並べて表示 */
	cDlgCompare.m_bCompareAndTileHorz = GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz;
	BOOL bDlgCompareResult = cDlgCompare.DoModal(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		(LPARAM)GetDocument(),
		GetDocument()->m_cDocFile.GetFilePath(),
		GetDocument()->m_cDocEditor.IsModified(),
		szPath,
		&hwndCompareWnd
	);
	if( !bDlgCompareResult ){
		return;
	}
	/* 比較後、左右に並べて表示 */
	GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;

	//タブウインドウ時は禁止	//@@@ 2003.06.12 MIK
	if( TRUE  == GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd
	 && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		hwndMsgBox = m_pCommanderView->GetHwnd();
		GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = FALSE;
	}
	else
	{
		hwndMsgBox = hwndCompareWnd;
	}


	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	CLogicPoint	poSrc;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&poSrc
	);

	// カーソル位置取得 -> poDes
	{
		::SendMessageAny( hwndCompareWnd, MYWM_GETCARETPOS, 0, 0 );
		CLogicPoint* ppoCaretDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	bDefferent = TRUE;
	pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
	/* 行(改行単位)データの要求 */
	nLineLenDes = ::SendMessageAny( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	pLineDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
	while( 1 ){
		if( pLineSrc == NULL &&	0 == nLineLenDes ){
			bDefferent = FALSE;
			break;
		}
		if( pLineSrc == NULL || 0 == nLineLenDes ){
			break;
		}
		if( nLineLenDes > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
			TopErrorMessage( m_pCommanderView->GetHwnd(),
				_T("比較先のファイル\n%ts\n%d文字を超える行があります。\n")
				_T("比較できません。"),
				szPath,
				GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>()
			);
			return;
		}
		for( ; poSrc.x < nLineLenSrc; ){
			if( poDes.x >= nLineLenDes ){
				goto end_of_compare;
			}
			if( pLineSrc[poSrc.x] != pLineDes[poDes.x] ){
				goto end_of_compare;
			}
			poSrc.x++;
			poDes.x++;
		}
		if( poDes.x < nLineLenDes ){
			goto end_of_compare;
		}
		poSrc.x = 0;
		poSrc.y++;
		poDes.x = 0;
		poDes.y++;
		pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
		/* 行(改行単位)データの要求 */
		nLineLenDes = ::SendMessageAny( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	}
end_of_compare:;
	/* 比較後、左右に並べて表示 */
//From Here Oct. 10, 2000 JEPRO	チェックボックスをボタン化すれば以下の行(To Here まで)は不要のはずだが
//	うまくいかなかったので元に戻してある…
	if( GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz ){
		HWND* phwndArr = new HWND[2];
		phwndArr[0] = GetMainWindow();
		phwndArr[1] = hwndCompareWnd;
		
		int i;	// Jan. 28, 2002 genta ループ変数 intの宣言を前に出した．
				// 互換性対策．forの()内で宣言すると古い規格と新しい規格で矛盾するので．
		for( i = 0; i < 2; ++i ){
			if( ::IsZoomed( phwndArr[i] ) ){
				::ShowWindow( phwndArr[i], SW_RESTORE );
			}
		}
		//	デスクトップサイズを得る 2002.1.24 YAZAKI
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( phwndArr[0], &rcDesktop );
		int width = (rcDesktop.right - rcDesktop.left ) / 2;
		for( i = 1; i >= 0; i-- ){
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta タスクバーが左にある場合を考慮
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
		delete [] phwndArr;
	}
//To Here Oct. 10, 2000

	//	2002/05/11 YAZAKI 親ウィンドウをうまく設定してみる。
	if( !bDefferent ){
		TopInfoMessage( hwndMsgBox, _T("異なる箇所は見つかりませんでした。") );
	}
	else{
//		TopInfoMessage( hwndMsgBox, _T("異なる箇所が見つかりました。") );
		/* カーソルを移動させる
			比較相手は、別プロセスなのでメッセージを飛ばす。
		*/
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poDes, sizeof( poDes ) );
		::SendMessageAny( hwndCompareWnd, MYWM_SETCARETPOS, 0, 0 );

		/* カーソルを移動させる */
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poSrc, sizeof( poSrc ) );
		::PostMessageAny( GetMainWindow(), MYWM_SETCARETPOS, 0, 0 );
		TopWarningMessage( hwndMsgBox, _T("異なる箇所が見つかりました。") );	// 位置を変更してからメッセージ	2008/4/27 Uchi
	}

	/* 開いているウィンドウをアクティブにする */
	/* アクティブにする */
	ActivateFrameWindow( GetMainWindow() );
	return;
}




/*! ツールバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutToolBar(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CViewCommander::Command_SHOWTOOLBAR( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispTOOLBAR = ((NULL == pCEditWnd->m_cToolbar.GetToolbarHwnd())? TRUE: FALSE);	/* ツールバー表示 */
	pCEditWnd->LayoutToolBar();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TOOLBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}




/*! ステータスバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutStatusBar(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CViewCommander::Command_SHOWSTATUSBAR( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispSTATUSBAR = ((NULL == pCEditWnd->m_cStatusBar.GetStatusHwnd())? TRUE: FALSE);	/* ステータスバー表示 */
	pCEditWnd->LayoutStatusBar();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_STATUSBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}




/*! ファンクションキーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutFuncKey(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CViewCommander::Command_SHOWFUNCKEY( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispFUNCKEYWND = ((NULL == pCEditWnd->m_CFuncKeyWnd.GetHwnd())? TRUE: FALSE);	/* ファンクションキー表示 */
	pCEditWnd->LayoutFuncKey();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_FUNCKEY,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}

//@@@ From Here 2003.06.10 MIK
/*! タブ(ウインドウ)の表示/非表示

	@author MIK
	@date 2003.06.10 新規作成
	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutTabBar(), CEditWnd::EndLayoutBars() で行うように変更
	@date 2007.06.20 ryoji グループIDリセット
 */
void CViewCommander::Command_SHOWTAB( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd = ((NULL == pCEditWnd->m_cTabWnd.GetHwnd())? TRUE: FALSE);	/* タブバー表示 */
	pCEditWnd->LayoutTabBar();
	pCEditWnd->EndLayoutBars();

	// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		GetDocument()->m_pcEditWnd->WindowTopMost(
			( (DWORD)::GetWindowLongPtr( GetDocument()->m_pcEditWnd->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
		);
	}

	//全ウインドウに変更を通知する。
	CAppNodeManager::Instance()->ResetGroupId();
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TAB,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}
//@@@ To Here 2003.06.10 MIK



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
		int nSaveResult=CEditApp::Instance()->m_pcSMacroMgr->Save(
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
		CEditApp::Instance()->m_pcSMacroMgr->Clear(STAND_KEYMACRO);
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
	if( !CEditApp::Instance()->m_pcSMacroMgr->IsSaveOk() ){
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
	if ( !CEditApp::Instance()->m_pcSMacroMgr->Save( STAND_KEYMACRO, G_AppInstance(), szPath ) ){
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
		BOOL bLoadResult = CEditApp::Instance()->m_pcSMacroMgr->Load(
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
			CEditApp::Instance()->m_pcSMacroMgr->Exec( STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0 );
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
		CEditApp::Instance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, (LPARAM)pszPath, m_pCommanderView );

		//キーマクロの記録を一時停止する
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */
	}

	//古い一時マクロの退避
	CMacroManagerBase* oldMacro = CEditApp::Instance()->m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = CEditApp::Instance()->m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("マクロの読み込みに失敗しました。\n\n%ts"), pszPath );
	}
	else {
		CEditApp::Instance()->m_pcSMacroMgr->Exec( TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO );
	}

	// 終わったら開放
	CEditApp::Instance()->m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		CEditApp::Instance()->m_pcSMacroMgr->SetTempMacro( oldMacro );
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


/*! 現在のウィンドウ幅で折り返し

	@date 2002.01.14 YAZAKI 現在のウィンドウ幅で折り返されているときは、最大値にするように
	@date 2002.04.08 YAZAKI ときどきウィンドウ幅で折り返されないことがあるバグ修正。
	@date 2005.08.14 genta ここでの設定は共通設定に反映しない．
	@date 2005.10.22 aroka 現在のウィンドウ幅→最大値→文書タイプの初期値 をトグルにする

	@note 変更する順序を変更したときはCEditWnd::InitMenu()も変更すること
	@sa CEditWnd::InitMenu()
*/
void CViewCommander::Command_WRAPWINDOWWIDTH( void )	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
{
	// Jan. 8, 2006 genta 判定処理をm_pCommanderView->GetWrapMode()へ移動
	CEditView::TOGGLE_WRAP_ACTION nWrapMode;
	CLayoutInt newKetas;
	
	nWrapMode = m_pCommanderView->GetWrapMode( &newKetas );
	GetDocument()->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
	GetDocument()->m_bTextWrapMethodCurTemp = !( GetDocument()->m_nTextWrapMethodCur == GetDocument()->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod );
	if( nWrapMode == CEditView::TGWRAP_NONE ){
		return;	// 折り返し桁は元のまま
	}

	GetEditWindow()->ChangeLayoutParam( true, GetDocument()->m_cLayoutMgr.GetTabSpace(), newKetas );
	

	//	Aug. 14, 2005 genta 共通設定へは反映させない
//	GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas = m_nViewColNum;

	m_pCommanderView->GetTextArea().SetViewLeftCol( CLayoutInt(0) );		/* 表示域の一番左の桁(0開始) */

	/* フォーカス移動時の再描画 */
	m_pCommanderView->RedrawAll();
	return;
}




//検索マークの切替え	// 2001.12.03 hor クリア を 切替え に変更
void CViewCommander::Command_SEARCH_CLEARMARK( void )
{
// From Here 2001.12.03 hor

	//検索マークのセット

	if(m_pCommanderView->GetSelectionInfo().IsTextSelected()){

		// 検索文字列取得
		CNativeW	cmemCurText;
		m_pCommanderView->GetCurrentTextForSearch( cmemCurText );

		// 検索文字列設定
		int i,j;
		wcscpy( m_pCommanderView->m_szCurSrchKey, cmemCurText.GetStringPtr() );
		for( i = 0; i < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size(); ++i ){
			if( 0 == wcscmp( m_pCommanderView->m_szCurSrchKey, GetDllShareData().m_sSearchKeywords.m_aSearchKeys[i] ) ){
				break;
			}
		}
		if( i < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size() ){
			for( j = i; j > 0; j-- ){
				wcscpy( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j], GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j - 1] );
			}
		}
		else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				wcscpy( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j], GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j - 1] );
			}
			GetDllShareData().m_sSearchKeywords.m_aSearchKeys.resize( t_min<int>(MAX_SEARCHKEY, GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size()+1) );
		}
		wcscpy( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], cmemCurText.GetStringPtr() );

		// 検索オプション設定
		GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp=false;	//正規表現使わない
		GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bWordOnly=false;		//単語で検索しない
		m_pCommanderView->ChangeCurRegexp(); // 2002.11.11 Moca 正規表現で検索した後，色分けができていなかった

		// 再描画
		m_pCommanderView->RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	//検索マークのクリア

	m_pCommanderView->m_bCurSrchKeyMark = false;	/* 検索文字列のマーク */
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




//日付挿入
void CViewCommander::Command_INS_DATE( void )
{
	// 日付をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1 );

	// テキストを貼り付け ver1
	Command_INSTEXT( TRUE, to_wchar(szText), CLogicInt(-1), TRUE );
}




//時刻挿入
void CViewCommander::Command_INS_TIME( void )
{
	// 時刻をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );

	// テキストを貼り付け ver1
	Command_INSTEXT( TRUE, to_wchar(szText), CLogicInt(-1), TRUE );
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
	//ExecCmd( buf, GetDllShareData().m_bGetStdout );	//	2006.12.03 maru マクロからの呼び出しではオプションを保存させないため
	m_pCommanderView->ExecCmd( to_tchar(buf), nFlgOpt );
	//	To Here Aug. 21, 2001 genta
	return;
}






//	Jun. 16, 2000 genta
//	対括弧の検索
void CViewCommander::Command_BRACKETPAIR( void )
{
	CLayoutPoint ptColLine;
	//int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	if( m_pCommanderView->SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) ){	// 02/09/18 ai
		//	2005.06.24 Moca
		//	2006.07.09 genta 表示更新漏れ：新規関数にて対応
		m_pCommanderView->MoveCursorSelecting( ptColLine, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
	else{
		//	失敗した場合は nCol/nLineには有効な値が入っていない.
		//	何もしない
	}
}


//	現在位置を移動履歴に登録する
void CViewCommander::Command_JUMPHIST_SET( void )
{
	m_pCommanderView->AddCurrentLineToHistory();
}


//	From HERE Sep. 8, 2000 genta
//	移動履歴を前へたどる
//
void CViewCommander::Command_JUMPHIST_PREV( void )
{
	// 2001.12.13 hor
	// 移動履歴の最後に現在の位置を記憶する
	// ( 次の履歴が取得できないときは追加して戻る )
	if( !m_pCommanderView->m_cHistory->CheckNext() ){
		m_pCommanderView->AddCurrentLineToHistory();
		m_pCommanderView->m_cHistory->PrevValid();
	}

	if( m_pCommanderView->m_cHistory->CheckPrev() ){
		if( ! m_pCommanderView->m_cHistory->PrevValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK );
		}
		CLayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(m_pCommanderView->m_cHistory->GetCurrent().GetPos(), m_pCommanderView->m_cHistory->GetCurrent().GetLineNo()),
			&pt
		);
		//	2006.07.09 genta 選択を考慮
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
}

//	移動履歴を次へたどる
void CViewCommander::Command_JUMPHIST_NEXT( void )
{
	if( m_pCommanderView->m_cHistory->CheckNext() ){
		if( ! m_pCommanderView->m_cHistory->NextValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK );
		}

		CLayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(m_pCommanderView->m_cHistory->GetCurrent().GetPos(), m_pCommanderView->m_cHistory->GetCurrent().GetLineNo()),
			&pt );

		//	2006.07.09 genta 選択を考慮
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
}
//	To HERE Sep. 8, 2000 genta

/* 次のグループ */			// 2007.06.20 ryoji
void CViewCommander::Command_NEXTGROUP( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->NextGroup();
}

/* 前のグループ */			// 2007.06.20 ryoji
void CViewCommander::Command_PREVGROUP( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->PrevGroup();
}

/* タブを右に移動 */		// 2007.06.20 ryoji
void CViewCommander::Command_TAB_MOVERIGHT( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->MoveRight();
}

/* タブを左に移動 */		// 2007.06.20 ryoji
void CViewCommander::Command_TAB_MOVELEFT( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->MoveLeft();
}

/* 新規グループ */			// 2007.06.20 ryoji
void CViewCommander::Command_TAB_SEPARATE( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->Separate();
}

/* 次のグループに移動 */	// 2007.06.20 ryoji
void CViewCommander::Command_TAB_JOINTNEXT( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->JoinNext();
}

/* 前のグループに移動 */	// 2007.06.20 ryoji
void CViewCommander::Command_TAB_JOINTPREV( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->JoinPrev();
}

/* このタブ以外を閉じる */	// 2008.11.22 syat
// 2009.12.26 syat このウィンドウ以外を閉じるとの兼用化
void CViewCommander::Command_TAB_CLOSEOTHER( void )
{
	int nGroup = 0;

	// ウィンドウ一覧を取得する
	EditNode* pEditNode;
	int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNode, TRUE );
	if( 0 >= nCount )return;

	for( int i = 0; i < nCount; i++ ){
		if( pEditNode[i].m_hWnd == GetMainWindow() ){
			pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
			nGroup = pEditNode[i].m_nGroup;
		}
	}

	//終了要求を出す
	CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
	delete []pEditNode;
	return;
}

/* 左をすべて閉じる */		// 2008.11.22 syat
void CViewCommander::Command_TAB_CLOSELEFT( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// ウィンドウ一覧を取得する
		EditNode* pEditNode;
		int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == GetMainWindow() ){
				pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//右は閉じない
			}
		}

		//終了要求を出す
		CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
		delete []pEditNode;
	}
	return;
}

/* 右をすべて閉じる */		// 2008.11.22 syat
void CViewCommander::Command_TAB_CLOSERIGHT( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// ウィンドウ一覧を取得する
		EditNode* pEditNode;
		int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == GetMainWindow() ){
				pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( !bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//左は閉じない
			}
		}

		//終了要求を出す
		CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
		delete []pEditNode;
	}
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
	
	if(nFlgOpt & 0x01)
	{	/* 選択範囲を出力 */
		try
		{
			CBinaryOutputStream out(to_tchar(filename),true);

			//BOM出力
			if ( GetDocument()->m_cDocFile.IsBomExist() ) {
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
				GetDocument()->m_cDocFile.IsBomExist()
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
	CFileLoad	cfl;
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
		EditInfo		fi;
		CMRU			cMRU;
		if ( cMRU.GetEditInfo( to_tchar(filename), &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = GetDocument()->GetDocumentEncoding();
		}
	}
	
	/* ここまできて文字コードが決定しないならどこかおかしい */
	if( 0 > nSaveCharCode || nSaveCharCode > CODE_CODEMAX ) nSaveCharCode = CODE_SJIS;
	
	try{
		// ファイルを開く
		cfl.FileOpen( to_tchar(filename), nSaveCharCode, NULL );

		/* ファイルサイズが65KBを越えたら進捗ダイアログ表示 */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
				::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
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
				::PostMessage( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
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

	if( NULL != pcDlgCancel ){
		delete pcDlgCancel;
	}
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
	@brief テキストの折り返し方法を変更する
	
	@param[in] nWrapMethod 折り返し方法
		WRAP_NO_TEXT_WRAP  : 折り返さない
		WRAP_SETTING_WIDTH ; 指定桁で折り返す
		WRAP_WINDOW_WIDTH  ; 右端で折り返す
	
	@note ウィンドウが左右に分割されている場合、左側のウィンドウ幅を折り返し幅とする。
	
	@date 2008.05.31 nasukoji	新規作成
	@date 2009.08.28 nasukoji	テキストの最大幅を算出する
*/
void CViewCommander::Command_TEXTWRAPMETHOD( int nWrapMethod )
{
	CEditDoc* pcDoc = GetDocument();

	// 現在の設定値と同じなら何もしない
	if( pcDoc->m_nTextWrapMethodCur == nWrapMethod )
		return;

	int nWidth;

	switch( nWrapMethod ){
	case WRAP_NO_TEXT_WRAP:		// 折り返さない
		nWidth = MAXLINEKETAS;	// アプリケーションの最大幅で折り返し
		break;

	case WRAP_SETTING_WIDTH:	// 指定桁で折り返す
		nWidth = (Int)pcDoc->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas;
		break;

	case WRAP_WINDOW_WIDTH:		// 右端で折り返す
		// ウィンドウが左右に分割されている場合は左側のウィンドウ幅を使用する
		nWidth = (Int)m_pCommanderView->ViewColNumToWrapColNum( GetEditWindow()->m_pcEditViewArr[0]->GetTextArea().m_nViewColNum );
		break;

	default:
		return;		// 不正な値の時は何もしない
	}

	pcDoc->m_nTextWrapMethodCur = nWrapMethod;	// 設定を記憶

	// 折り返し方法の一時設定適用／一時設定適用解除	// 2008.06.08 ryoji
	pcDoc->m_bTextWrapMethodCurTemp = !( pcDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod == nWrapMethod );

	// 折り返し位置を変更
	GetEditWindow()->ChangeLayoutParam( false, pcDoc->m_cLayoutMgr.GetTabSpace(), (CLayoutInt)nWidth );

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if( pcDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		pcDoc->m_cLayoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
		pcDoc->m_pcEditWnd->RedrawAllViews( NULL );		// スクロールバーの更新が必要なので再表示を実行する
	}else{
		pcDoc->m_cLayoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
	}
}

/* 	上書き用の一文字削除	2009.04.11 ryoji */
void CViewCommander::DelCharForOverwrite( void )
{
	bool bEol = false;
	BOOL bDelete = TRUE;
	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL != pcLayout ){
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		CLogicInt nIdxTo = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
		if( nIdxTo >= pcLayout->GetLengthWithoutEOL() ){
			bEol = true;	// 現在位置は改行または折り返し以後
			if( pcLayout->GetLayoutEol() != EOL_NONE ){
				if( GetDllShareData().m_Common.m_sEdit.m_bNotOverWriteCRLF ){	/* 改行は上書きしない */
					/* 現在位置が改行ならば削除しない */
					bDelete = FALSE;
				}
			}
		}
	}
	if( bDelete ){
		/* 上書きモードなので、現在位置の文字を１文字消去 */
		if( bEol ){
			Command_DELETE();	//行数減では再描画が必要＆行末以後の削除を処理統一
		}else{
			m_pCommanderView->DeleteData( FALSE );
		}
	}
}

/*!
	@brief 文字カウント方法を変更する
	
	@param[in] nMode 文字カウント方法
		SELECT_COUNT_TOGGLE  : 文字カウント方法をトグル
		SELECT_COUNT_BY_CHAR ; 文字数でカウント
		SELECT_COUNT_BY_BYTE ; バイト数でカウント
*/
void CViewCommander::Command_SELECT_COUNT_MODE( int nMode )
{
	//設定には保存せず、View毎に持つフラグを設定
	//BOOL* pbDispSelCountByByte = &GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte;
	ESelectCountMode* pnSelectCountMode = &GetEditWindow()->m_nSelectCountMode;

	if( nMode == SELECT_COUNT_TOGGLE ){
		//文字数⇔バイト数トグル
		ESelectCountMode nCurrentMode;
		if( *pnSelectCountMode == SELECT_COUNT_TOGGLE ){
			nCurrentMode = ( GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte ?
								SELECT_COUNT_BY_BYTE :
								SELECT_COUNT_BY_CHAR );
		}else{
			nCurrentMode = *pnSelectCountMode;
		}
		*pnSelectCountMode = ( nCurrentMode == SELECT_COUNT_BY_BYTE ?
								SELECT_COUNT_BY_CHAR :
								SELECT_COUNT_BY_BYTE );
	}else if( nMode == SELECT_COUNT_BY_BYTE || nMode == SELECT_COUNT_BY_CHAR ){
		*pnSelectCountMode = ( ESelectCountMode )nMode;
	}
}
