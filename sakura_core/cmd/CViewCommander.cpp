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
	Copyright (C) 2011, ryoji, nasukoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
// 2007.10.25 kobake CViewCommanderクラスに分離

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

//@@@ 2002.2.2 YAZAKI マクロはCSMacroMgrに統一
#include "macro/CSMacroMgr.h"
#include "CEditApp.h"
#include "plugin/CJackManager.h"

CViewCommander::CViewCommander(CEditView* pEditView) : m_pCommanderView(pEditView)
{
	m_bPrevCommand = 0;
	m_pcSMacroMgr = CEditApp::getInstance()->m_pcSMacroMgr;
}


/*!
	コマンドコードによる処理振り分け

	@param nCommand コマンドコード
	@param lparam1 parameter1(内容はコマンドコードによって変わります)
	@param lparam2 parameter2(内容はコマンドコードによって変わります)
	@param lparam3 parameter3(内容はコマンドコードによって変わります)
	@param lparam4 parameter4(内容はコマンドコードによって変わります)
*/
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


	if( m_pCommanderView->m_nAutoScrollMode && F_AUTOSCROLL != nCommand ){
		m_pCommanderView->AutoScrollExit();
	}
	m_pCommanderView->GetCaret().m_bClearStatus = true;
	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call message translators
	// -------------------------------------
	m_pCommanderView->TranslateCommand_grep( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );
	m_pCommanderView->TranslateCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );

	// 2013.09.23 novice 機能が利用可能か調べる
	if( !IsFuncEnable( GetDocument(), &GetDllShareData(), nCommand ) ){
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
	if( GetEditWindow()->m_pPrintPreview && F_PRINT_PREVIEW != nCommand ){
		ErrorBeep();
		return -1;
	}
	/* キーリピート状態 */
	if( m_bPrevCommand == nCommand ){
		bRepeat = true;
	}
	m_bPrevCommand = nCommand;
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow() &&	/* キーボードマクロを記録中のウィンドウ */
		( nCommandFrom & FA_NONRECORD ) != FA_NONRECORD	/* 2007.07.07 genta 記録抑制フラグ off */
	){
		/* キーリピート状態をなくする */
		bRepeat = false;
		/* キーマクロに記録可能な機能かどうかを調べる */
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		//F_EXECEXTMACROコマンドはファイルを選択した後にマクロ文が確定するため個別に記録する。
		if( CSMacroMgr::CanFuncIsKeyMacro( nCommand ) &&
			nCommand != F_EXECEXTMACRO	//F_EXECEXTMACROは個別で記録します
		){
			/* キーマクロのバッファにデータ追加 */
			//@@@ 2002.1.24 m_CKeyMacroMgrをCEditDocへ移動
			LPARAM lparams[] = {lparam1, lparam2, lparam3, lparam4};
			m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparams, m_pCommanderView );
		}
	}

	//	2007.07.07 genta マクロ実行中フラグの設定
	//	マクロからのコマンドかどうかはnCommandFromでわかるが
	//	nCommandFromを引数で浸透させるのが大変なので，従来のフラグにも値をコピーする
	m_pCommanderView->m_bExecutingKeyMacro = ( nCommandFrom & FA_FROMMACRO ) ? true : false;

	/* キーボードマクロの実行中 */
	if( m_pCommanderView->m_bExecutingKeyMacro ){
		/* キーリピート状態をなくする */
		bRepeat = false;
	}

	//	From Here Sep. 29, 2001 genta マクロの実行機能追加
	if( F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO ){
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一（インターフェースの変更）
		if( !m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, G_AppInstance(), m_pCommanderView,
			nCommandFrom & FA_NONRECORD )){
			InfoMessage(
				this->m_pCommanderView->m_hwndParent,
				LS(STR_ERR_MACRO1),
				nCommand - F_USERMACRO_0,
				m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
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
	case F_FILEOPEN2:			Command_FILEOPEN((const WCHAR*)lparam1, (ECodeType)lparam2, lparam3 != 0, (const WCHAR*)lparam4);break;	//ファイルを開く2
	case F_FILEOPEN_DROPDOWN:	Command_FILEOPEN((const WCHAR*)lparam1);break;			/* ファイルを開く(ドロップダウン) */	//@@@ 2002.06.15 MIK
	case F_FILESAVE:			bRet = Command_FILESAVE();break;	/* 上書き保存 */
	case F_FILESAVEAS_DIALOG:	bRet = Command_FILESAVEAS_DIALOG((const WCHAR*)lparam1, (ECodeType)lparam2, (EEolType)lparam3);break;	/* 名前を付けて保存 */
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
	case F_FILE_REOPEN_LATIN1:		Command_FILE_REOPEN( CODE_LATIN1, lparam1!=0 );break;	//Latin1で開きなおす	// 2010/3/20 Uchi
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
	case F_PROFILEMGR:			Command_PROFILEMGR();break;			// プロファイルマネージャ
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
	case F_RIGHT:			Command_RIGHT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, false, bRepeat ); break;	//カーソル右移動
	case F_UP2:				Command_UP2( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;						//カーソル上移動(２行づつ)
	case F_DOWN2:			Command_DOWN2( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//カーソル下移動(２行づつ)
	case F_WORDLEFT:		Command_WORDLEFT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				/* 単語の左端に移動 */
	case F_WORDRIGHT:		Command_WORDRIGHT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				/* 単語の右端に移動 */
	//	0ct. 29, 2001 genta マクロ向け機能拡張
	case F_GOLINETOP:		Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, lparam1  ); break;		//行頭に移動(折り返し単位/改行単位)
	case F_GOLINEEND:		Command_GOLINEEND( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, 0, lparam1 ); break;		//行末に移動(折り返し単位)
//	case F_ROLLDOWN:		Command_ROLLDOWN( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//スクロールダウン
//	case F_ROLLUP:			Command_ROLLUP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//スクロールアップ
	case F_HalfPageUp:		Command_HalfPageUp( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, CLayoutYInt(lparam1) ); break;				//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown:	Command_HalfPageDown( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, CLayoutYInt(lparam1) ); break;			//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp:			Command_1PageUp( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, CLayoutYInt(lparam1) ); break;					//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown:		Command_1PageDown( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, CLayoutYInt(lparam1) ); break;				//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
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
	case F_AUTOSCROLL:	Command_AUTOSCROLL(); break;	//オートスクロール
	case F_WHEELUP:			Command_WHEELUP(lparam1); break;
	case F_WHEELDOWN:		Command_WHEELDOWN(lparam1); break;
	case F_WHEELLEFT:		Command_WHEELLEFT(lparam1); break;
	case F_WHEELRIGHT:		Command_WHEELRIGHT(lparam1); break;
	case F_WHEELPAGEUP:		Command_WHEELPAGEUP(lparam1); break;
	case F_WHEELPAGEDOWN:	Command_WHEELPAGEDOWN(lparam1); break;
	case F_WHEELPAGELEFT:	Command_WHEELPAGELEFT(lparam1); break;
	case F_WHEELPAGERIGHT:	Command_WHEELPAGERIGHT(lparam1); break;
	case F_MODIFYLINE_NEXT:	Command_MODIFYLINE_NEXT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;	// 次の変更行へ
	case F_MODIFYLINE_PREV:	Command_MODIFYLINE_PREV( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;	// 前の変更行へ

	/* 選択系 */
	case F_SELECTWORD:		Command_SELECTWORD();break;						//現在位置の単語選択
	case F_SELECTALL:		Command_SELECTALL();break;						//すべて選択
	case F_SELECTLINE:		Command_SELECTLINE( lparam1 );break;			//1行選択	// 2007.10.13 nasukoji
	case F_BEGIN_SEL:		Command_BEGIN_SELECT();break;					/* 範囲選択開始 */
	case F_UP_SEL:			Command_UP( true, bRepeat, lparam1 ); break;	//(範囲選択)カーソル上移動
	case F_DOWN_SEL:		Command_DOWN( true, bRepeat ); break;			//(範囲選択)カーソル下移動
	case F_LEFT_SEL:		Command_LEFT( true, bRepeat ); break;			//(範囲選択)カーソル左移動
	case F_RIGHT_SEL:		Command_RIGHT( true, false, bRepeat ); break;	//(範囲選択)カーソル右移動
	case F_UP2_SEL:			Command_UP2( true ); break;						//(範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL:		Command_DOWN2( true );break;					//(範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL:	Command_WORDLEFT( true );break;					//(範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL:	Command_WORDRIGHT( true );break;				//(範囲選択)単語の右端に移動
	case F_GOLINETOP_SEL:	Command_GOLINETOP( true, lparam1 );break;		//(範囲選択)行頭に移動(折り返し単位/改行単位)
	case F_GOLINEEND_SEL:	Command_GOLINEEND( true, 0, lparam1 );break;	//(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL:	Command_ROLLDOWN( TRUE ); break;				//(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL:		Command_ROLLUP( TRUE ); break;					//(範囲選択)スクロールアップ
	case F_HalfPageUp_Sel:	Command_HalfPageUp( true, CLayoutYInt(lparam1) ); break;				//(範囲選択)半ページアップ
	case F_HalfPageDown_Sel:Command_HalfPageDown( true, CLayoutYInt(lparam1) ); break;			//(範囲選択)半ページダウン
	case F_1PageUp_Sel:		Command_1PageUp( true, CLayoutYInt(lparam1) ); break;					//(範囲選択)１ページアップ
	case F_1PageDown_Sel:	Command_1PageDown( true, CLayoutYInt(lparam1) ); break;				//(範囲選択)１ページダウン
	case F_GOFILETOP_SEL:	Command_GOFILETOP( true );break;				//(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL:	Command_GOFILEEND( true );break;				//(範囲選択)ファイルの最後に移動
	case F_GONEXTPARAGRAPH_SEL:	Command_GONEXTPARAGRAPH( true ); break;			//次の段落へ進む
	case F_GOPREVPARAGRAPH_SEL:	Command_GOPREVPARAGRAPH( true ); break;			//前の段落へ戻る
	case F_MODIFYLINE_NEXT_SEL:	Command_MODIFYLINE_NEXT( true ); break;			//(範囲選択)次の変更行へ
	case F_MODIFYLINE_PREV_SEL:	Command_MODIFYLINE_PREV( true ); break;			//(範囲選択)前の変更行へ

	/* 矩形選択系 */
//	case F_BOXSELALL:		Command_BOXSELECTALL();break;		//矩形ですべて選択
	case F_BEGIN_BOX:		Command_BEGIN_BOXSELECT( true );break;	/* 矩形範囲選択開始 */
	case F_UP_BOX:			Sub_BoxSelectLock(lparam1); this->Command_UP( true, bRepeat ); break;		//(矩形選択)カーソル上移動
	case F_DOWN_BOX:		Sub_BoxSelectLock(lparam1); this->Command_DOWN( true, bRepeat ); break;		//(矩形選択)カーソル下移動
	case F_LEFT_BOX:		Sub_BoxSelectLock(lparam1); this->Command_LEFT( true, bRepeat ); break;		//(矩形選択)カーソル左移動
	case F_RIGHT_BOX:		Sub_BoxSelectLock(lparam1); this->Command_RIGHT( true, false, bRepeat ); break;	//(矩形選択)カーソル右移動
	case F_UP2_BOX:			Sub_BoxSelectLock(lparam1); this->Command_UP2( true ); break;				//(矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX:		Sub_BoxSelectLock(lparam1); this->Command_DOWN2( true );break;				//(矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX:	Sub_BoxSelectLock(lparam1); this->Command_WORDLEFT( true );break;			//(矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX:	Sub_BoxSelectLock(lparam1); this->Command_WORDRIGHT( true );break;			//(矩形選択)単語の右端に移動
	case F_GOLOGICALLINETOP_BOX:Sub_BoxSelectLock(lparam2); this->Command_GOLINETOP( true, 8 | lparam1 );break;	//(矩形選択)行頭に移動(改行単位)
//	case F_GOLOGICALLINEEND_BOX:Sub_BoxSelectLock(lparam2); this->Command_GOLINEEND( true, 0, 8 | lparam1 );break;	//(矩形選択)行末に移動(改行単位)
	case F_GOLINETOP_BOX:	Sub_BoxSelectLock(lparam2); this->Command_GOLINETOP( true, lparam1 );break;	//(矩形選択)行頭に移動(折り返し単位/改行単位)
	case F_GOLINEEND_BOX:	Sub_BoxSelectLock(lparam2); this->Command_GOLINEEND( true, 0, lparam1 );break;	//(矩形選択)行末に移動(折り返し単位/改行単位)
	case F_HalfPageUp_BOX:	Sub_BoxSelectLock(lparam2); this->Command_HalfPageUp( true, CLayoutYInt(lparam1) ); break;		//(矩形選択)半ページアップ
	case F_HalfPageDown_BOX:Sub_BoxSelectLock(lparam2); this->Command_HalfPageDown( true, CLayoutYInt(lparam1) ); break;		//(矩形選択)半ページダウン
	case F_1PageUp_BOX:		Sub_BoxSelectLock(lparam2); this->Command_1PageUp( true, CLayoutYInt(lparam1) ); break;			//(矩形選択)１ページアップ
	case F_1PageDown_BOX:	Sub_BoxSelectLock(lparam2); this->Command_1PageDown( true, CLayoutYInt(lparam1) ); break;			//(矩形選択)１ページダウン
	case F_GOFILETOP_BOX:	Sub_BoxSelectLock(lparam1); this->Command_GOFILETOP( true );break;			//(矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX:	Sub_BoxSelectLock(lparam1); this->Command_GOFILEEND( true );break;			//(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	case F_CUT:						Command_CUT();break;					//切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:					Command_COPY( false, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy );break;			//コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF:			Command_COPY( false, true );break;		//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:				Command_COPY( false, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy, EOL_CRLF );break;	//CRLF改行でコピー(選択範囲をクリップボードにコピー)
	case F_PASTE:					Command_PASTE( (int)lparam1 );break;				//貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:				Command_PASTEBOX( (int)lparam1 );break;				//矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSBOXTEXT:				Command_INSBOXTEXT((const wchar_t*)lparam1, (int)lparam2 );break;				//矩形テキスト挿入
	case F_INSTEXT_W:				Command_INSTEXT( bRedraw, (const wchar_t*)lparam1, (CLogicInt)lparam2, lparam3!=FALSE );break;/* テキストを貼り付け */ // 2004.05.14 Moca 長さを示す引数追加
	case F_ADDTAIL_W:				Command_ADDTAIL( (const wchar_t*)lparam1, (int)lparam2 );break;	/* 最後にテキストを追加 */
	case F_COPYFNAME:				Command_COPYFILENAME();break;			//このファイル名をクリップボードにコピー / /2002/2/3 aroka
	case F_COPYPATH:				Command_COPYPATH();break;				//このファイルのパス名をクリップボードにコピー
	case F_COPYTAG:					Command_COPYTAG();break;				//このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	case F_COPYLINES:				Command_COPYLINES();break;				//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		Command_COPYLINESASPASSAGE();break;		//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	Command_COPYLINESWITHLINENUMBER();break;//選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML:				Command_COPY_COLOR_HTML();break;	//選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER:	Command_COPY_COLOR_HTML_LINENUMBER();break;	//選択範囲内行番号色付きHTMLコピー

	case F_CREATEKEYBINDLIST:		Command_CREATEKEYBINDLIST();break;		//キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE:				Command_INS_DATE();break;	//日付挿入
	case F_INS_TIME:				Command_INS_TIME();break;	//時刻挿入
    case F_CTRL_CODE_DIALOG:		Command_CtrlCode_Dialog();break;	/* コントロールコードの入力(ダイアログ) */	//@@@ 2002.06.02 MIK
    case F_CTRL_CODE:				Command_WCHAR( (wchar_t)lparam1, false );break;

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
	case F_SEARCH_NEXT:			Command_SEARCH_NEXT( true, bRedraw, false, (HWND)lparam1, (const WCHAR*)lparam2 );break;	//次を検索
	case F_SEARCH_PREV:			Command_SEARCH_PREV( bRedraw, (HWND)lparam1 );break;						//前を検索
	case F_REPLACE_DIALOG:	//置換(置換ダイアログ)
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI ダイアログ呼び出しと、実行を分離
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//置換実行 @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//すべて置換実行(通常) 2002.2.8 hor 2006.04.02 かろと
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//検索マークのクリア
	case F_GREP_DIALOG:	//Grepダイアログの表示
		/* 再帰処理対策 */
		m_pCommanderView->SetUndoBuffer( true );
		Command_GREP_DIALOG();
		return bRet;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_GREP_REPLACE_DLG:	//Grep置換ダイアログの表示
		/* 再帰処理対策 */
		m_pCommanderView->SetUndoBuffer( true );
		Command_GREP_REPLACE_DLG();
		return bRet;
	case F_GREP_REPLACE:	Command_GREP_REPLACE();break;							//Grep置換
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//指定行ヘジャンプダイアログの表示
	case F_JUMP:			Command_JUMP();break;							//指定行ヘジャンプ
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1, OUTLINE_DEFAULT );break;	//アウトライン解析
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE, OUTLINE_DEFAULT );break;	//アウトライン解析(toggle) // 20060201 aroka
	case F_FILETREE:		bRet = Command_FUNCLIST( (BOOL)lparam1 ,OUTLINE_FILETREE );break;	//ファイルツリー
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
	case F_FUNCLIST_NEXT:	Command_FUNCLIST_NEXT();break;					// 次の関数リストマーク	2014.01.05
	case F_FUNCLIST_PREV:	Command_FUNCLIST_PREV();break;					// 前の関数リストマーク	2014.01.05


	/* モード切り替え系 */
	case F_CHGMOD_INS:		Command_CHGMOD_INS();break;		//挿入／上書きモード切り替え
	case F_CHG_CHARSET:		Command_CHG_CHARSET( (ECodeType)lparam1, lparam2 != 0 );break;	//文字コードセット指定	2010/6/14 Uchi
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
	case F_SHOWMINIMAP:		Command_SHOWMINIMAP();break;	// ミニマップの表示/非表示
	case F_TYPE_LIST:		Command_TYPE_LIST();break;		/* タイプ別設定一覧 */
	case F_CHANGETYPE:		Command_CHANGETYPE((int)lparam1);break;		// タイプ別設定一時適用
	case F_OPTION_TYPE:		Command_OPTION_TYPE();break;	/* タイプ別設定 */
	case F_OPTION:			Command_OPTION();break;			/* 共通設定 */
	case F_FONT:			Command_FONT();break;			/* フォント設定 */
	case F_SETFONTSIZE:		Command_SETFONTSIZE((int)lparam1, (int)lparam2, (int)lparam3);break;	/* フォントサイズ設定 */
	case F_SETFONTSIZEUP:	HandleCommand( F_SETFONTSIZE, bRedraw, 0, 1, 2, 0);break;	/* フォントサイズ拡大 */
	case F_SETFONTSIZEDOWN:	HandleCommand( F_SETFONTSIZE, bRedraw, 0, -1, 2, 0);break;	/* フォントサイズ縮小 */
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
		m_pCommanderView->SetUndoBuffer( true );
		Command_EXECKEYMACRO(); return bRet;
	case F_EXECEXTMACRO:
		/* 再帰処理対策 */
		m_pCommanderView->SetUndoBuffer( true );
		/* 名前を指定してマクロ実行 */
		Command_EXECEXTMACRO( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		return bRet;
	//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* 外部コマンド実行 */
	case F_EXECMD_DIALOG:
		//Command_EXECCOMMAND_DIALOG((const char*)lparam1);	/* 外部コマンド実行 */
		Command_EXECCOMMAND_DIALOG();	/* 外部コマンド実行 */	//	引数つかってないみたいなので
		break;
	//	To Here Sept. 20, 2000
	case F_EXECMD:
		//Command_EXECCOMMAND((const char*)lparam1);
		Command_EXECCOMMAND((LPCWSTR)lparam1, (int)lparam2, (LPCWSTR)lparam3);	//	2006.12.03 maru 引数の拡張のため
		break;

	/* カスタムメニュー */
	case F_MENU_RBUTTON:	/* 右クリックメニュー */
		/* 再帰処理対策 */
		m_pCommanderView->SetUndoBuffer( true );
		Command_MENU_RBUTTON();
		return bRet;
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
		m_pCommanderView->SetUndoBuffer( true );
		nFuncID = Command_CUSTMENU( nCommand - F_CUSTMENU_1 + 1 );
		if( 0 != nFuncID ){
			/* コマンドコードによる処理振り分け */
//			HandleCommand( nFuncID, true, 0, 0, 0, 0 );
			::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nFuncID, 0 ), (LPARAM)NULL );
		}
		return bRet;

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
	case F_TRACEOUT:		Command_TRACEOUT((const wchar_t*)lparam1, (int)lparam2, (int)lparam3);break;		//マクロ用アウトプットウィンドウに表示 maru 2006.04.26
	case F_TOPMOST:			Command_WINTOPMOST( lparam1 );break;	//常に手前に表示 Moca
	case F_WINLIST:			Command_WINLIST( nCommandFrom );break;		/* ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon // 2006.05.19 genta 引数追加
	case F_DLGWINLIST:		Command_DLGWINLIST();break;		// ウィンドウ一覧表示
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
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch((int)lparam1);break;	/* キャレット位置の単語を辞書検索する機能ON-OFF */	// 2006.03.24 fon
	case F_MENU_ALLFUNC:									/* コマンド一覧 */
		/* 再帰処理対策 */
		m_pCommanderView->SetUndoBuffer( true );
		Command_MENU_ALLFUNC();return bRet;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* 外部ヘルプ１ */
	case F_EXTHTMLHELP:	/* 外部HTMLヘルプ */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	case F_ABOUT:	Command_ABOUT();break;				/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */

	case F_0: break; // F_0でプラグインが実行されるバグ対策	// ← rev1886 の問題は呼び元で対策したが安全弁として残す

	default:
		//プラグインコマンドを実行する
		{
			m_pCommanderView->SetUndoBuffer( true ); // 2013.05.01 追加。再帰対応

			CPlug::Array plugs;
			CJackManager::getInstance()->GetUsablePlug( PP_COMMAND, nCommand, &plugs );

			if( plugs.size() > 0 ){
				assert_warning( 1 == plugs.size() );
				//インタフェースオブジェクト準備
				CWSHIfObj::List params;
				//プラグイン呼び出し
				( *plugs.begin() )->Invoke( m_pCommanderView, params );

				return bRet;
			}
		}

	}

	/* アンドゥバッファの処理 */
	m_pCommanderView->SetUndoBuffer( true );

	return bRet;
}



/*!
	@date 2014.07.11 新規追加
*/
void CViewCommander::Sub_BoxSelectLock( int flags )
{
	bool bSelLock;
	if( flags == 0x00 ){
		bSelLock = GetDllShareData().m_Common.m_sEdit.m_bBoxSelectLock;
	}else if( flags == 0x01 ){
		bSelLock = true;
	}else if( flags == 0x02 ){
		bSelLock = false;
	}
	if( ! this->m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) {
		this->Command_BEGIN_BOXSELECT( bSelLock );
	}
}


CLogicInt CViewCommander::ConvertEol(const wchar_t* pszText, CLogicInt nTextLen, wchar_t* pszConvertedText)
{
	// original by 2009.02.28 salarm
	CLogicInt nConvertedTextLen;
	CEol eol = GetDocument()->m_cDocEditor.GetNewLineCode();

	nConvertedTextLen = 0;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	if( pszConvertedText == NULL ){
		for( int i = 0; i < nTextLen; i++ ){
			if( WCODE::IsLineDelimiter(pszText[i], bExtEol) ){
				if( pszText[i] == WCODE::CR ){
					if( i + 1 < nTextLen && pszText[i + 1] == WCODE::LF ){
						i++;
					}
				}
				nConvertedTextLen += eol.GetLen();
			} else {
				nConvertedTextLen++;
			}
		}
	}else{
		for( int i = 0; i < nTextLen; i++ ){
			if( WCODE::IsLineDelimiter(pszText[i], bExtEol) ){
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
	}
	return nConvertedTextLen;
}



/*!
	@brief 検索で見つからないときの警告（メッセージボックス／サウンド）

	@date 2010.04.21 ryoji	新規作成（数カ所で用いられていた類似コードの共通化）
*/
void CViewCommander::AlertNotFound(HWND hwnd, bool bReplaceAll, LPCTSTR format, ...)
{
	if( GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND
		&& !bReplaceAll
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
