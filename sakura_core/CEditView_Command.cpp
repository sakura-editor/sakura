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
	Copyright (C) 2006, genta, aroka, ryoji, かろと, fon, yukihane, Moca, maru
	Copyright (C) 2007, ryoji, maru, genta, nasukoji
	Copyright (C) 2008, ryoji, nasukoji, novice, syat
	Copyright (C) 2009, ryoji, syat, genta, salarm
	Copyright (C) 2010, ryoji, Moca
	Copyright (C) 2012, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <HtmlHelp.h>
#include <stdlib.h>
#include <io.h>
#include <mbstring.h>
#include "CEditView.h"
#include "Debug.h"
#include "Funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "CControlTray.h"
#include "CWaitCursor.h"
#include "CSplitterWnd.h"
//@@@ 2002.2.2 YAZAKI マクロはCSMacroMgrに統一
#include "CSMacroMgr.h"
#include "etc_uty.h"
#include "os.h"
#include "CDlgTypeList.h"
#include "CDlgProperty.h"
#include "CDlgCompare.h"
#include "global.h"
#include "CRunningTimer.h"
#include "CDlgExec.h"
#include "CDlgAbout.h"	//Dec. 24, 2000 JEPRO 追加
#include "COpe.h"/// 2002/2/3 aroka 追加 from here
#include "COpeBlk.h"///
#include "CLayout.h"///
#include "CEditWnd.h"///
#include "CFuncInfoArr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h"///
#include "CSMacroMgr.h"///
#include "mymessage.h"/// 2002/2/3 aroka 追加 to here
#include "CDlgCancel.h"// 2002/2/8 hor
#include "CPrintPreview.h"
#include "CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "CDlgCancel.h"
#include "CDlgTagJumpList.h"
#include "CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "COsVersionInfo.h"
#include "my_icmp.h"
#include "sakura_rc.h"

/*!
	コマンドコードによる処理振り分け

	@param nCommand コマンドコード
	@param lparam1 parameter1(内容はコマンドコードによって変わります)
	@param lparam2 parameter2(内容はコマンドコードによって変わります)
	@param lparam3 parameter3(内容はコマンドコードによって変わります)
	@param lparam4 parameter4(内容はコマンドコードによって変わります)
*/
BOOL CEditView::HandleCommand(
	int				nCommand,
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
	nCommand = LOWORD( nCommand );

	//	Oct. 30, 2004 genta
	//	漢字の上下バイトが１つのWM_IME_CHARではなく別々のWM_CHARにて来る
	//	ケースの取り扱いを処理先頭に移動．
	//	* マクロで分離して記録されないように
	//	* トラップした場合に2バイト目に反応してしまうのを防ぐ
	//	From Here Oct. 5, 2002 genta
	//	WM_CHARでくる漢字コードを受け入れる
	if( nCommand == F_CHAR ){
		// 厳密にはCEditViewのメンバーにすべきだが分離メッセージの救済はこれでも十分
		static unsigned int ucSjis1 = 0;
		if( ucSjis1 == 0 ){
			if( _IS_SJIS_1( (unsigned char)lparam1 )){
				ucSjis1 = lparam1;
				return TRUE;
			}
		}
		else {
			//	一文字前にSJISの1バイト目が来ている
			if( _IS_SJIS_2( (unsigned char)lparam1 )){
				lparam1 = (ucSjis1 << 8 | lparam1 );
				nCommand = F_IME_CHAR;
			}
			ucSjis1 = 0;
		}
	}
	//	To Here Oct. 5, 2002 genta

	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call message translators
	// -------------------------------------
	TranslateCommand_grep( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );
	TranslateCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );

	//	Aug, 14. 2000 genta
	if( m_pcEditDoc->IsModificationForbidden( nCommand ) ){
		return TRUE;
	}

	++m_pcEditDoc->m_nCommandExecNum;		/* コマンド実行回数 */
//	if( nCommand != F_COPY ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
//	}
	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( pCEditWnd->m_pPrintPreview && F_PRINT_PREVIEW != nCommand ){
		ErrorBeep();
		return -1;
	}
	/* キーリピート状態 */
	if( m_bPrevCommand == nCommand ){
		bRepeat = true;
	}
	m_bPrevCommand = nCommand;
	if( m_pShareData->m_sFlags.m_bRecordingKeyMacro &&									/* キーボードマクロの記録中 */
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == ::GetParent( m_hwndParent ) &&	/* キーボードマクロを記録中のウィンドウ */
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
			m_pcEditDoc->m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparam1, this );
		}
	}

	//	2007.07.07 genta マクロ実行中フラグの設定
	//	マクロからのコマンドかどうかはnCommandFromでわかるが
	//	nCommandFromを引数で浸透させるのが大変なので，従来のフラグにも値をコピーする
	m_bExecutingKeyMacro = ( nCommandFrom & FA_FROMMACRO ) ? true : false;

	/* キーボードマクロの実行中 */
	if( m_bExecutingKeyMacro ){
		/* キーリピート状態をなくする */
		bRepeat = false;
	}

	//	From Here Sep. 29, 2001 genta マクロの実行機能追加
	if( F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO ){
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一（インターフェースの変更）
		if( !m_pcEditDoc->m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, m_hInstance, this,
			nCommandFrom & FA_NONRECORD )){
			InfoMessage(
				m_hwndParent,
				_T("マクロ %d (%s) の実行に失敗しました。"),
				nCommand - F_USERMACRO_0,
				m_pcEditDoc->m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
			);
		}

		/* フォーカス移動時の再描画 */
		RedrawAll();

		return TRUE;
	}
	//	To Here Sep. 29, 2001 genta マクロの実行機能追加

	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call mode basis message handler
	// -------------------------------------
	PreprocessCommand_hokan(nCommand);
	if( ProcessCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 ))
		return TRUE;

	// -------------------------------------
	//	Jan. 10, 2005 genta コメント
	//	ここより前ではUndoバッファの準備ができていないので
	//	文書の操作を行ってはいけない
	//@@@ 2002.2.2 YAZAKI HandleCommand内でHandleCommandを呼び出せない問題に対処（何か副作用がある？）
	if( NULL == m_pcOpeBlk ){	/* 操作ブロック */
		m_pcOpeBlk = new COpeBlk;
	}
	m_pcOpeBlk->AddRef();	//参照カウンタ増加
	
	//	Jan. 10, 2005 genta コメント
	//	ここより後ではswitchの後ろでUndoを正しく登録するため，
	//	途中で処理の打ち切りを行ってはいけない
	// -------------------------------------

	switch( nCommand ){
	case F_CHAR:	/* 文字入力 */
		{
			/* コントロールコード入力禁止 */
			if(
				( ( (unsigned char)0x0 <= (unsigned char)lparam1 && (unsigned char)lparam1 <= (unsigned char)0x1F ) ||
				  ( (unsigned char)'~' <  (unsigned char)lparam1 && (unsigned char)lparam1 <  (unsigned char)'｡'  ) ||
				  ( (unsigned char)'ﾟ' <  (unsigned char)lparam1 && (unsigned char)lparam1 <= (unsigned char)0xff )
				) &&
				(unsigned char)lparam1 != TAB && (unsigned char)lparam1 != CR && (unsigned char)lparam1 != LF
			){
				ErrorBeep();
			}else{
				Command_CHAR( (char)lparam1 );
			}
		}
		break;

	/* ファイル操作系 */
	case F_FILENEW:				Command_FILENEW();break;			/* 新規作成 */
	case F_FILENEW_NEWWINDOW:	Command_FILENEW_NEWWINDOW();break;
	//	Oct. 2, 2001 genta マクロ用機能拡張
	case F_FILEOPEN:			Command_FILEOPEN((const char*)lparam1);break;			/* ファイルを開く */
	case F_FILEOPEN_DROPDOWN:	Command_FILEOPEN((const char*)lparam1);break;			/* ファイルを開く(ドロップダウン) */	//@@@ 2002.06.15 MIK
	case F_FILESAVE:			bRet = Command_FILESAVE();break;	/* 上書き保存 */
	case F_FILESAVEAS_DIALOG:	bRet = Command_FILESAVEAS_DIALOG();break;	/* 名前を付けて保存 */
	case F_FILESAVEAS:			bRet = Command_FILESAVEAS((const char*)lparam1);break;	/* 名前を付けて保存 */
	case F_FILESAVEALL:			bRet = Command_FILESAVEALL();break;	/* 全ての編集ウィンドウで上書き保存 */ // Jan. 23, 2005 genta
	case F_FILESAVE_QUIET:		bRet = Command_FILESAVE(false,false); break;	/* 静かに上書き保存 */ // Jan. 24, 2005 genta
	case F_FILESAVECLOSE:
		//	Feb. 28, 2004 genta 保存＆閉じる
		//	保存が不要なら単に閉じる
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
	case F_FILE_REOPEN:				Command_FILE_REOPEN( m_pcEditDoc->m_nCharCode, lparam1!=0 );break;//	Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS:		Command_FILE_REOPEN( CODE_SJIS, lparam1!=0 );break;		//SJISで開き直す
	case F_FILE_REOPEN_JIS:			Command_FILE_REOPEN( CODE_JIS, lparam1!=0 );break;		//JISで開き直す
	case F_FILE_REOPEN_EUC:			Command_FILE_REOPEN( CODE_EUC, lparam1!=0 );break;		//EUCで開き直す
	case F_FILE_REOPEN_UNICODE:		Command_FILE_REOPEN( CODE_UNICODE, lparam1!=0 );break;	//Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE: 	Command_FILE_REOPEN( CODE_UNICODEBE, lparam1!=0 );break;	//UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8:		Command_FILE_REOPEN( CODE_UTF8, lparam1!=0 );break;		//UTF-8で開き直す
	case F_FILE_REOPEN_UTF7:		Command_FILE_REOPEN( CODE_UTF7, lparam1!=0 );break;		//UTF-7で開き直す
	case F_PRINT:				Command_PRINT();break;					/* 印刷 */
	case F_PRINT_PREVIEW:		Command_PRINT_PREVIEW();break;			/* 印刷プレビュー */
	case F_PRINT_PAGESETUP:		Command_PRINT_PAGESETUP();break;		/* 印刷ページ設定 */	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	case F_OPEN_HfromtoC:		bRet = Command_OPEN_HfromtoC( (BOOL)lparam1 );break;	/* 同名のC/C++ヘッダ(ソース)を開く */	//Feb. 7, 2001 JEPRO 追加
	case F_OPEN_HHPP:			bRet = Command_OPEN_HHPP( (BOOL)lparam1, TRUE );break;		/* 同名のC/C++ヘッダファイルを開く */	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	case F_OPEN_CCPP:			bRet = Command_OPEN_CCPP( (BOOL)lparam1, TRUE );break;		/* 同名のC/C++ソースファイルを開く */	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	case F_ACTIVATE_SQLPLUS:	Command_ACTIVATE_SQLPLUS();break;		/* Oracle SQL*Plusをアクティブ表示 */
	case F_PLSQL_COMPILE_ON_SQLPLUS:									/* Oracle SQL*Plusで実行 */
		Command_PLSQL_COMPILE_ON_SQLPLUS();
		break;
	case F_BROWSE:				Command_BROWSE();break;				/* ブラウズ */
	case F_READONLY:			Command_READONLY();break;			/* 読み取り専用 */
	case F_PROPERTY_FILE:		Command_PROPERTY_FILE();break;		/* ファイルのプロパティ */
	case F_EXITALLEDITORS:		Command_EXITALLEDITORS();break;		/* 編集の全終了 */	// 2007.02.13 ryoji 追加
	case F_EXITALL:				Command_EXITALL();break;			/* サクラエディタの全終了 */	//Dec. 26, 2000 JEPRO 追加
	case F_PUTFILE:				Command_PUTFILE((const char*)lparam1, (ECodeType)lparam2, (int)lparam3);break;	/* 作業中ファイルの一時出力 */ //maru 2006.12.10
	case F_INSFILE:				Command_INSFILE((const char*)lparam1, (ECodeType)lparam2, (int)lparam3);break;	/* キャレット位置にファイル挿入 */ //maru 2006.12.10

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
	case F_INDENT_TAB:			Command_INDENT_TAB();break;			//TABインデント
	case F_UNINDENT_TAB:		Command_UNINDENT( TAB );break;		//逆TABインデント
	case F_INDENT_SPACE:											//SPACEインデント
		/* テキストが２行以上にまたがって選択されているか */
		if( IsTextSelected() &&
			0 != ( m_nSelectLineFrom - m_nSelectLineTo )
		){
			Command_INDENT( SPACE );
		}else{
			/* １バイト文字入力 */
			Command_CHAR( (char)' ' );
		}
		break;
	case F_UNINDENT_SPACE:			Command_UNINDENT( SPACE );break;	//逆SPACEインデント
//	case F_WORDSREFERENCE:			Command_WORDSREFERENCE();break;		/* 単語リファレンス */
	case F_LTRIM:					Command_TRIM(TRUE);break;			// 2001.12.03 hor
	case F_RTRIM:					Command_TRIM(FALSE);break;			// 2001.12.03 hor
	case F_SORT_ASC:				Command_SORT(TRUE);break;			// 2001.12.06 hor
	case F_SORT_DESC:				Command_SORT(FALSE);break;			// 2001.12.06 hor
	case F_MERGE:					Command_MERGE();break;				// 2001.12.06 hor
	case F_RECONVERT:				Command_Reconvert();break;			/* メニューからの再変換対応 minfu 2002.04.09 */ 

	/* カーソル移動系 */
	case F_IME_CHAR:		Command_IME_CHAR( (WORD)lparam1 ); break;					//全角文字入力
	case F_UP:				Command_UP( m_bSelectingLock, bRepeat ); break;				//カーソル上移動
	case F_DOWN:			Command_DOWN( m_bSelectingLock, bRepeat ); break;			//カーソル下移動
	case F_LEFT:			Command_LEFT( m_bSelectingLock, bRepeat ); break;			//カーソル左移動
	case F_RIGHT:			Command_RIGHT( m_bSelectingLock, false, bRepeat ); break;	//カーソル右移動
	case F_UP2:				Command_UP2( m_bSelectingLock ); break;						//カーソル上移動(２行づつ)
	case F_DOWN2:			Command_DOWN2( m_bSelectingLock ); break;					//カーソル下移動(２行づつ)
	case F_WORDLEFT:		Command_WORDLEFT( m_bSelectingLock ); break;				/* 単語の左端に移動 */
	case F_WORDRIGHT:		Command_WORDRIGHT( m_bSelectingLock ); break;				/* 単語の右端に移動 */
	//	0ct. 29, 2001 genta マクロ向け機能拡張
	case F_GOLINETOP:		Command_GOLINETOP( m_bSelectingLock, lparam1  ); break;		//行頭に移動(折り返し単位)
	case F_GOLINEEND:		Command_GOLINEEND( m_bSelectingLock, 0 ); break;			//行末に移動(折り返し単位)
//	case F_ROLLDOWN:		Command_ROLLDOWN( m_bSelectingLock ); break;				//スクロールダウン
//	case F_ROLLUP:			Command_ROLLUP( m_bSelectingLock ); break;					//スクロールアップ
	case F_HalfPageUp:		Command_HalfPageUp( m_bSelectingLock ); break;				//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown:	Command_HalfPageDown( m_bSelectingLock ); break;			//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp:			Command_1PageUp( m_bSelectingLock ); break;					//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown:		Command_1PageDown( m_bSelectingLock ); break;				//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_GOFILETOP:		Command_GOFILETOP( m_bSelectingLock ); break;				//ファイルの先頭に移動
	case F_GOFILEEND:		Command_GOFILEEND( m_bSelectingLock ); break;				//ファイルの最後に移動
	case F_CURLINECENTER:	Command_CURLINECENTER(); break;								/* カーソル行をウィンドウ中央へ */
	case F_JUMPHIST_PREV:	Command_JUMPHIST_PREV(); break;								//移動履歴: 前へ
	case F_JUMPHIST_NEXT:	Command_JUMPHIST_NEXT(); break;								//移動履歴: 次へ
	case F_JUMPHIST_SET:	Command_JUMPHIST_SET(); break;								//現在位置を移動履歴に登録
	case F_WndScrollDown:	Command_WndScrollDown(); break;								//テキストを１行下へスクロール	// 2001/06/20 asa-o
	case F_WndScrollUp:		Command_WndScrollUp(); break;								//テキストを１行上へスクロール	// 2001/06/20 asa-o
	case F_GONEXTPARAGRAPH:	Command_GONEXTPARAGRAPH( m_bSelectingLock ); break;			//次の段落へ進む
	case F_GOPREVPARAGRAPH:	Command_GOPREVPARAGRAPH( m_bSelectingLock ); break;			//前の段落へ戻る

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
	case F_GOLINETOP_SEL:	Command_GOLINETOP( true, 0 );break;				//(範囲選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_SEL:	Command_GOLINEEND( true, 0 );break;				//(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL:	Command_ROLLDOWN( TRUE ); break;				//(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL:		Command_ROLLUP( TRUE ); break;					//(範囲選択)スクロールアップ
	case F_HalfPageUp_Sel:	Command_HalfPageUp( true ); break;				//(範囲選択)半ページアップ
	case F_HalfPageDown_Sel:Command_HalfPageDown( true ); break;			//(範囲選択)半ページダウン
	case F_1PageUp_Sel:		Command_1PageUp( true ); break;					//(範囲選択)１ページアップ
	case F_1PageDown_Sel:	Command_1PageDown( true ); break;				//(範囲選択)１ページダウン
	case F_GOFILETOP_SEL:	Command_GOFILETOP( true );break;				//(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL:	Command_GOFILEEND( true );break;				//(範囲選択)ファイルの最後に移動
	case F_GONEXTPARAGRAPH_SEL:	Command_GONEXTPARAGRAPH( true ); break;			//次の段落へ進む
	case F_GOPREVPARAGRAPH_SEL:	Command_GOPREVPARAGRAPH( true ); break;			//前の段落へ戻る

	/* 矩形選択系 */
//	case F_BOXSELALL:		Command_BOXSELECTALL();break;		//矩形ですべて選択
	case F_BEGIN_BOX:		Command_BEGIN_BOXSELECT();break;	/* 矩形範囲選択開始 */
//	case F_UP_BOX:			Command_UP_BOX( bRepeat ); break;			//(矩形選択)カーソル上移動
//	case F_DOWN_BOX:		Command_DOWN( true, bRepeat ); break;		//(矩形選択)カーソル下移動
//	case F_LEFT_BOX:		Command_LEFT( true, bRepeat ); break;		//(矩形選択)カーソル左移動
//	case F_RIGHT_BOX:		Command_RIGHT( true, false, bRepeat ); break;//(矩形選択)カーソル右移動
//	case F_UP2_BOX:			Command_UP2( true ); break;					//(矩形選択)カーソル上移動(２行ごと)
//	case F_DOWN2_BOX:		Command_DOWN2( true );break;				//(矩形選択)カーソル下移動(２行ごと)
//	case F_WORDLEFT_BOX:	Command_WORDLEFT( true );break;				//(矩形選択)単語の左端に移動
//	case F_WORDRIGHT_BOX:	Command_WORDRIGHT( true );break;			//(矩形選択)単語の右端に移動
//	case F_GOLINETOP_BOX:	Command_GOLINETOP( true, 0 );break;			//(矩形選択)行頭に移動(折り返し単位)
//	case F_GOLINEEND_BOX:	Command_GOLINEEND( true, 0 );break;			//(矩形選択)行末に移動(折り返し単位)
//	case F_HalfPageUp_BOX:	Command_HalfPageUp( true ); break;			//(矩形選択)半ページアップ
//	case F_HalfPageDown_BOX:Command_HalfPageDown( true ); break;		//(矩形選択)半ページダウン
//	case F_1PageUp_BOX:		Command_1PageUp( true ); break;				//(矩形選択)１ページアップ
//	case F_1PageDown_BOX:	Command_1PageDown( true ); break;			//(矩形選択)１ページダウン
//	case F_GOFILETOP_BOX:	Command_GOFILETOP( true );break;			//(矩形選択)ファイルの先頭に移動
//	case F_GOFILEEND_BOX:	Command_GOFILEEND( true );break;			//(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	case F_CUT:						Command_CUT();break;					//切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:					Command_COPY( false, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy );break;			//コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF:			Command_COPY( false, true );break;		//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:				Command_COPY( false, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy, EOL_CRLF );break;	//CRLF改行でコピー(選択範囲をクリップボードにコピー)
	case F_PASTE:					Command_PASTE( (int)lparam1 );break;	//貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:				Command_PASTEBOX( (int)lparam1 );break;	//矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSTEXT:					Command_INSTEXT( bRedraw, (const char*)lparam1, -1, (BOOL)lparam2 );break;/* テキストを貼り付け */ // 2004.05.14 Moca 長さを示す引数追加(-1は\0終端まで)
	case F_ADDTAIL:					Command_ADDTAIL( (const char*)lparam1, (int)lparam2 );break;	/* 最後にテキストを追加 */
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
	case F_HANKATATOZENKATA:		Command_HANKATATOZENKAKUKATA();break;	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENHIRA:		Command_HANKATATOZENKAKUHIRA();break;	/* 半角カタカナ→全角ひらがな */
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
	case F_SEARCH_NEXT:			Command_SEARCH_NEXT( true, bRedraw, (HWND)lparam1, (const char*)lparam2 );break;	//次を検索
	case F_SEARCH_PREV:			Command_SEARCH_PREV( bRedraw, (HWND)lparam1 );break;						//前を検索
	case F_REPLACE_DIALOG:	//置換(置換ダイアログ)
		/* 再帰処理対策 */
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI ダイアログ呼び出しと、実行を分離
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//置換実行 @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//すべて置換実行(通常) 2002.2.8 hor 2006.04.02 かろと
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//検索マークのクリア
	case F_GREP_DIALOG:	//Grepダイアログの表示
		/* 再帰処理対策 */
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		Command_GREP_DIALOG();
		break;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//指定行ヘジャンプダイアログの表示
	case F_JUMP:			Command_JUMP();break;							//指定行ヘジャンプ
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1 );break;	//アウトライン解析
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE );break;	//アウトライン解析(toggle) // 20060201 aroka
	case F_TAGJUMP:			Command_TAGJUMP(lparam1 != 0);break;			/* タグジャンプ機能 */ //	Apr. 03, 2003 genta 引数追加
	case F_TAGJUMP_CLOSE:	Command_TAGJUMP(true);break;					/* タグジャンプ(元ウィンドウclose) *///	Apr. 03, 2003 genta
	case F_TAGJUMPBACK:		Command_TAGJUMPBACK();break;					/* タグジャンプバック機能 */
	case F_TAGS_MAKE:		Command_TagsMake();break;						//タグファイルの作成	//@@@ 2003.04.13 MIK
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFile();break;				/* ダイレクトタグジャンプ機能 */	//@@@ 2003.04.15 MIK
	case F_TAGJUMP_KEYWORD:	Command_TagJumpByTagsFileKeyword( (const char*)lparam1 );break;	/* @@ 2005.03.31 MIK キーワードを指定してダイレクトタグジャンプ機能 */
	case F_COMPARE:			Command_COMPARE();break;						/* ファイル内容比較 */
	case F_DIFF_DIALOG:		Command_Diff_Dialog();break;					/* DIFF差分表示(ダイアログ) */	//@@@ 2002.05.25 MIK
	case F_DIFF:			Command_Diff( (const char*)lparam1, (int)lparam2 );break;		/* DIFF差分表示 */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
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
	case F_SETFONTSIZE:		Command_SETFONTSIZE((int)lparam1, (int)lparam2);break;	/* フォントサイズ設定 */
	case F_WRAPWINDOWWIDTH:	Command_WRAPWINDOWWIDTH();break;/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	case F_FAVORITE:		Command_Favorite();break;		//履歴の管理	//@@@ 2003.04.08 MIK
	//	Jan. 29, 2005 genta 引用符の設定
	case F_SET_QUOTESTRING:	Command_SET_QUOTESTRING((const char*)lparam1);	break;
	case F_TMPWRAPNOWRAP:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_NO_TEXT_WRAP, 0, 0, 0 );break;	// 折り返さない（一時設定）			// 2008.05.30 nasukoji
	case F_TMPWRAPSETTING:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_SETTING_WIDTH, 0, 0, 0 );break;	// 指定桁で折り返す（一時設定）		// 2008.05.30 nasukoji
	case F_TMPWRAPWINDOW:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_WINDOW_WIDTH, 0, 0, 0 );break;	// 右端で折り返す（一時設定）		// 2008.05.30 nasukoji
	case F_TEXTWRAPMETHOD:	Command_TEXTWRAPMETHOD( (int)lparam1 );break;		// テキストの折り返し方法		// 2008.05.30 nasukoji

	/* マクロ系 */
	case F_RECKEYMACRO:		Command_RECKEYMACRO();break;	/* キーマクロの記録開始／終了 */
	case F_SAVEKEYMACRO:	Command_SAVEKEYMACRO();break;	/* キーマクロの保存 */
	case F_LOADKEYMACRO:	Command_LOADKEYMACRO();break;	/* キーマクロの読み込み */
	case F_EXECKEYMACRO:									/* キーマクロの実行 */
		/* 再帰処理対策 */
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		Command_EXECKEYMACRO();break;
	case F_EXECEXTMACRO:
		/* 再帰処理対策 */
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		Command_EXECEXTMACRO( (const char*)lparam1, (const char*)lparam2 );		/* 名前を指定してマクロ実行 */
		break;
	//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* 外部コマンド実行 */
	case F_EXECMD_DIALOG:
		/* 再帰処理対策 */// 2001/06/23 N.Nakatani
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		//Command_EXECCOMMAND_DIALOG((const char*)lparam1);	/* 外部コマンド実行 */
		Command_EXECCOMMAND_DIALOG();	/* 外部コマンド実行 */	//	引数つかってないみたいなので
		break;
	//	To Here Sept. 20, 2000
	case F_EXECMD:
		//Command_EXECCOMMAND((const char*)lparam1);
		Command_EXECCOMMAND((const char*)lparam1, (int)lparam2);	//	2006.12.03 maru 引数の拡張のため
		break;

	/* カスタムメニュー */
	case F_MENU_RBUTTON:	/* 右クリックメニュー */
		/* 再帰処理対策 */
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
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
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		nFuncID = Command_CUSTMENU( nCommand - F_CUSTMENU_1 + 1 );
		if( 0 != nFuncID ){
			/* コマンドコードによる処理振り分け */
//			HandleCommand( nFuncID, true, 0, 0, 0, 0 );
			::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ), (LPARAM)NULL );
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
	case F_TRACEOUT:		Command_TRACEOUT((const char*)lparam1, (int)lparam2);break;		//マクロ用アウトプットウィンドウに表示 maru 2006.04.26
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
	case F_TAB_CLOSEOTHER:	Command_TAB_CLOSEOTHER();break;	/* このタブ以外を閉じる */	// 2009.07.20 syat 追加
	case F_TAB_CLOSELEFT:	Command_TAB_CLOSELEFT();break;	/* 左をすべて閉じる */		// 2009.07.20 syat 追加
	case F_TAB_CLOSERIGHT:	Command_TAB_CLOSERIGHT();break;	/* 右をすべて閉じる */		// 2009.07.20 syat 追加

	/* 支援 */
	case F_HOKAN:			Command_HOKAN();break;			//入力補完
	case F_HELP_CONTENTS:	Command_HELP_CONTENTS();break;	/* ヘルプ目次 */				//Nov. 25, 2000 JEPRO 追加
	case F_HELP_SEARCH:		Command_HELP_SEARCH();break;	/* ヘルプトキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch();break;	/* キャレット位置の単語を辞書検索する機能ON-OFF */	// 2006.03.24 fon
	case F_MENU_ALLFUNC:									/* コマンド一覧 */
		/* 再帰処理対策 */
		delete m_pcOpeBlk;
		m_pcOpeBlk = NULL;
		Command_MENU_ALLFUNC();break;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* 外部ヘルプ１ */
	case F_EXTHTMLHELP:	/* 外部HTMLヘルプ */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const char*)lparam1, (const char*)lparam2 );
		break;
	case F_ABOUT:	Command_ABOUT();break;				/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */
//	case F_SENDMAIL:	Command_SENDMAIL();break;		/* メール送信 */

	}

	// アンドゥバッファの処理
	SetUndoBuffer(true);

	return bRet;
}

/////////////////////////////////// 以下はコマンド群 (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

/*! カーソル上移動 */
int CEditView::Command_UP( bool bSelect, bool bRepeat, int lines )
{
	//	From Here Oct. 24, 2001 genta
	if( lines != 0 ){
		Cursor_UPDOWN( lines, FALSE );
		return 1;
	}
	//	To Here Oct. 24, 2001 genta

	int		i;
	int		nRepeat = 0;

	/* キーリピート時のスクロールを滑らかにするか */
	if( !m_pShareData->m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		if( !bRepeat ){
			i = -1;
		}else{
			i = -1 * m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* キーリピート時のスクロール行数 */
		}
		Cursor_UPDOWN( i, bSelect );
		nRepeat = -1 * i;
	}
	else{
		++nRepeat;
		if( Cursor_UPDOWN( -1, bSelect ) && bRepeat ){
			for( i = 0; i < m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){		/* キーリピート時のスクロール行数 */
				::UpdateWindow( m_hWnd );	//	YAZAKI
				Cursor_UPDOWN( -1, bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}




/* カーソル下移動 */
int CEditView::Command_DOWN( bool bSelect, bool bRepeat )
{
	int		i;
	int		nRepeat;
	nRepeat = 0;
	/* キーリピート時のスクロールを滑らかにするか */
	if( !m_pShareData->m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		if( !bRepeat ){
			i = 1;
		}else{
			i = m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* キーリピート時のスクロール行数 */
		}
		Cursor_UPDOWN( i, bSelect );
		nRepeat = i;
	}else{
		++nRepeat;
		if( Cursor_UPDOWN( 1, bSelect ) && bRepeat ){
			for( i = 0; i < m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){	/* キーリピート時のスクロール行数 */
				//	ここで再描画。
				::UpdateWindow( m_hWnd );	//	YAZAKI
				Cursor_UPDOWN( 1, bSelect );
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
int CEditView::Command_LEFT( bool bSelect, bool bRepeat )
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
		int				nPosX = 0;
		int				nPosY = m_nCaretPosY;
		RECT			rcSel;
		const CLayout*	pcLayout;
		if( bSelect ){
			if( !IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在のカーソル位置から選択を開始する */
				BeginSelectArea();
			}
		}else{
			if( IsTextSelected() ){	/* テキストが選択されているか */
				/* 矩形範囲選択中か */
				if( m_bBeginBoxSelect ){
					/* 2点を対角とする矩形を求める */
					TwoPointToRect(
						&rcSel,
						m_nSelectLineFrom,		/* 範囲選択開始行 */
						m_nSelectColmFrom,		/* 範囲選択開始桁 */
						m_nSelectLineTo,		/* 範囲選択終了行 */
						m_nSelectColmTo			/* 範囲選択終了桁 */
					);
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( true );
					/* カーソルを選択開始位置に移動 */
					MoveCursor( rcSel.left, rcSel.top, true );
					m_nCaretPosX_Prev = m_nCaretPosX;
				}else{
					nPosX = m_nSelectColmFrom;
					nPosY = m_nSelectLineFrom;
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( true );
					/* カーソルを選択開始位置に移動 */
					MoveCursor( nPosX, nPosY, true );
					m_nCaretPosX_Prev = m_nCaretPosX;
				}
				nRes = 1;
				goto end_of_func;
			}
		}
		/* 現在行のデータを取得 */
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
		/* カーソルが左端にある */
		if( m_nCaretPosX == (pcLayout ? pcLayout->GetIndent() : 0)){
			if( m_nCaretPosY > 0 ){
				pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY - 1 );
				CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
				while( !it.end() ){
					it.scanNext();
					if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
						nPosX += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				nPosX += it.getColumn() - it.getColumnDelta();
				nPosY --;
			} else {
				nRes = 0;
				goto end_of_func;
			}
		}
		//  2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正
		else if( pcLayout ) {
			CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > m_nCaretPosX - 1 ){
					nPosX += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			nPosX += it.getColumn() - it.getColumnDelta();
			//	Oct. 18, 2002 YAZAKI
			if( it.getIndex() >= pcLayout->GetLengthWithEOL() ){
				nPosX = m_nCaretPosX - 1;
			}
		}

		MoveCursor( nPosX, nPosY, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/*	現在のカーソル位置によって選択範囲を変更．
			
				2004.04.02 Moca 
				キャレット位置が不正だった場合にMoveCursorの移動結果が
				引数で与えた座標とは異なることがあるため，
				nPosX, nPosYの代わりに実際の移動結果を使うように．
			*/
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}
		nRes = 1;
		goto end_of_func;
end_of_func:;
	}
	return nRes;
}




/* カーソル右移動 */
void CEditView::Command_RIGHT( bool bSelect,  bool bIgnoreCurrentSelection, bool bRepeat )
{
	int		nRepCount;
	int		nRepeat;
	if( bRepeat ){
		nRepeat = 2;
	}else{
		nRepeat = 1;
	}
	for( nRepCount = 0; nRepCount < nRepeat; ++nRepCount ){
		int			nPosX;
		int			nPosY = m_nCaretPosY;
		RECT		rcSel;
		const CLayout*	pcLayout;
		// 2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に
		// 対応するため、現在行のデータを取得を移動
		if( !bIgnoreCurrentSelection ) {
			if( bSelect ) {
				if( !IsTextSelected() ){	/* テキストが選択されているか */
					/* 現在のカーソル位置から選択を開始する */
					BeginSelectArea();
				}
			}else{
				if( IsTextSelected() ){	/* テキストが選択されているか */
					/* 矩形範囲選択中か */
					if( m_bBeginBoxSelect ){
						/* 2点を対角とする矩形を求める */
						TwoPointToRect(
							&rcSel,
							m_nSelectLineFrom,		/* 範囲選択開始行 */
							m_nSelectColmFrom,		/* 範囲選択開始桁 */
							m_nSelectLineTo,		/* 範囲選択終了行 */
							m_nSelectColmTo			/* 範囲選択終了桁 */
						);
						/* 現在の選択範囲を非選択状態に戻す */
						DisableSelectArea( true );
						/* カーソルを選択終了位置に移動 */
						MoveCursor( rcSel.right, rcSel.bottom, true );
						m_nCaretPosX_Prev = m_nCaretPosX;
					}else{
						nPosX = m_nSelectColmTo;
						nPosY = m_nSelectLineTo;

						/* 現在の選択範囲を非選択状態に戻す */
						DisableSelectArea( true );
						if( nPosY >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
							/* ファイルの最後に移動 */
							Command_GOFILEEND(false);
						}else{
							/* カーソルを選択終了位置に移動 */
							MoveCursor( nPosX, nPosY, true );
							m_nCaretPosX_Prev = m_nCaretPosX;
						}
					}
					goto end_of_func;
				}
			}
		}
//		2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に対応
		/* 現在行のデータを取得 */
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
		//	2004.04.02 EOF以降にカーソルがあったときに右を押しても何も起きなかったのを、EOFに移動するように
		if( pcLayout )
		{
			int nIndex = 0;
			CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > m_nCaretPosX + it.getColumnDelta() ){
					break;
				}
				if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
					nIndex += it.getIndexDelta();
					break;
				}
				it.addDelta();
			}
			nPosX = it.getColumn();
			nIndex += it.getIndex();
			if( nIndex >= pcLayout->GetLengthWithEOL() ){
				/* フリーカーソルモードか */
				if( (
					m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode
				 || IsTextSelected() && m_bBeginBoxSelect	/* 矩形範囲選択中 */
					)
				 &&
					/* 改行で終わっているか */
					( EOL_NONE != pcLayout->m_cEol )
				){
					/*-- フリーカーソルモードの場合 --*/
					if( nPosX <= m_nCaretPosX ){
						/* 最終行か */
						if( m_nCaretPosY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
							/* 改行で終わっているか */
							if( EOL_NONE != pcLayout->m_cEol.GetType() ){
								nPosX = m_nCaretPosX + 1;
							}else{
								nPosX = m_nCaretPosX;
							}
						}else{
							nPosX = m_nCaretPosX + 1;
						}
					}else{
						nPosX = nPosX;
					}
				}else{
					/*-- フリーカーソルモードではない場合 --*/
					/* 最終行か */
					if( m_nCaretPosY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
						/* 改行で終わっているか */
						if( EOL_NONE != pcLayout->m_cEol.GetType() ){
							nPosX = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
							++nPosY;
						}else{
						}
					}else{
						nPosX = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
						++nPosY;
					}
				}
				//	キャレット位置が折り返し位置より右側だった場合の処理
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( nPosX >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
					nPosX = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
					++nPosY;
				}
			}
		} else {
			// pcLayoutがNULLの場合はnPosX=0に調整
			nPosX = 0;
		}
		MoveCursor( nPosX, nPosY, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}

end_of_func:;
	}
	return;
}



/* 半ページアップ */	//Oct. 6, 2000 JEPRO added (実は従来のスクロールダウンそのもの)
void CEditView::Command_HalfPageUp( bool bSelect )
{
	Cursor_UPDOWN( - ( m_nViewRowNum / 2 ), bSelect );
	return;
}




/* 半ページダウン */	//Oct. 6, 2000 JEPRO added (実は従来のスクロールアップそのもの)
void CEditView::Command_HalfPageDown( bool bSelect )
{
	Cursor_UPDOWN( ( m_nViewRowNum / 2 ), bSelect );
	return;
}




/*! １ページアップ

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページアップに動作変更
*/	//Oct. 10, 2000 JEPRO added
void CEditView::Command_1PageUp( bool bSelect )
{
//	Cursor_UPDOWN( - m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページアップ
	if(m_nViewTopLine>=m_nViewRowNum-1){
		m_bDrawSWITCH = false;
		int nViewTopLine=m_nCaretPosY-m_nViewTopLine;
		Cursor_UPDOWN( -m_nViewRowNum+1, bSelect );
		//	Sep. 11, 2004 genta 同期スクロール処理のため
		//	RedrawAllではなくScrollAtを使うように
		SyncScrollV( ScrollAtV( m_nCaretPosY-nViewTopLine ));
		m_bDrawSWITCH = true;
		RedrawAll();
		
	}else{
		Cursor_UPDOWN( -m_nViewRowNum+1, bSelect );
	}
	return;
}




/*!	１ページダウン

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページダウンに動作変更
*/
void CEditView::Command_1PageDown( bool bSelect )
{
//	Cursor_UPDOWN( m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページダウン
	if(m_nViewTopLine+m_nViewRowNum <= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){ //- m_nViewRowNum){
		m_bDrawSWITCH = false;
		int nViewTopLine=m_nCaretPosY-m_nViewTopLine;
		Cursor_UPDOWN( m_nViewRowNum-1, bSelect );
		//	Sep. 11, 2004 genta 同期スクロール処理のため
		//	RedrawAllではなくScrollAtを使うように
		SyncScrollV( ScrollAtV( m_nCaretPosY-nViewTopLine ));
		m_bDrawSWITCH = true;
		RedrawAll();
	}else{
		Cursor_UPDOWN( m_nViewRowNum , bSelect );
		Command_DOWN( bSelect, true );
	}

	return;
}




/* カーソル上移動(２行づつ) */
void CEditView::Command_UP2( bool bSelect )
{
	Cursor_UPDOWN( -2, bSelect );
	return;
}




/* カーソル下移動(２行づつ) */
void CEditView::Command_DOWN2( bool bSelect )
{
	Cursor_UPDOWN( 2, bSelect );
	return;
}




/*! @brief 行頭に移動

	@date Oct. 29, 2001 genta マクロ用機能拡張(パラメータ追加) + goto排除
	@date May. 15, 2002 oak   改行単位移動
	@date Oct.  7, 2002 YAZAKI 冗長な引数 bLineTopOnly を削除
	@date Jun. 18, 2007 maru 行頭判定に全角空白のインデント設定も考慮する
*/
void CEditView::Command_GOLINETOP(
	bool	bSelect,	//!< [in] 選択の有無。true: 選択しながら移動。false: 選択しないで移動。
	int		lparam		/*!< [in] マクロから使用する拡張フラグ
								  @li 0: キー操作と同一(default)
								  @li 1: カーソル位置に関係なく行頭に移動。
								  @li 4: 選択して移動(合成可)
								  @li 8: 改行単位で先頭に移動(合成可)
						*/
)
{
	bool			bLineTopOnly = false;
	int				nCaretPosX;
	int				nCaretPosY;
	int				nPos;
	int				nPosY;
	const CLayout*	pcLayout;

	// lparamの解釈
	if( lparam & 1 ){
		bLineTopOnly = true;
	}
	
	if( lparam & 4 ){
		bSelect = true;
	}

	if ( lparam & 8 ){
		/* 改行単位指定の場合は、物理行頭位置から目的論理位置を求める */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			0,
			m_ptCaretPos_PHY.y,
			&nCaretPosX,
			&nCaretPosY
		);
	}
	else{
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
		nCaretPosX = pcLayout ? pcLayout->GetIndent() : 0;
		nCaretPosY = m_nCaretPosY;
	}
	if( !bLineTopOnly ){
		/* 目的行のデータを取得 */
		/* 改行単位指定で、先頭から空白が1折り返し行以上続いている場合は次の行データを取得 */
		nPosY = nCaretPosY - 1;
		const char*		pLine;
		const CLayout*	pcLayout;
		BOOL			bZenSpace = m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
		
		int				nLineLen;
		do {
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ++nPosY, &nLineLen, &pcLayout );
			if( !pLine ){
				return;
			}
			for( nPos = 0; nPos < nLineLen; ++nPos ){
				if(' ' == pLine[nPos]) continue;
				if('\t' == pLine[nPos]) continue;
				
				/* 2007.06.18 maru 全角空白もインデントの設定になっていれば行頭判定に考慮する */
				if( TRUE == bZenSpace && nPos+1 < nLineLen ){
					if( (char)0x81 == pLine[nPos] && (char)0x40 == pLine[nPos+1] ){
						nPos++;
						continue;
					}
				}
				if( CR == pLine[nPos] || LF == pLine[nPos] ){
					nPos = 0;	// 空白またはタブおよび改行だけの行だった
				}
				break;
			}
		}
		while (( lparam & 8 ) && (nPos >= nLineLen) && (m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 > nPosY) );

		if( nPos >= nLineLen ){
			/* 折り返し単位の行頭を探して物理行末まで到達した
			または、最終行のため改行コードに遭遇せずに行末に到達した */
			nPos = 0;
		}
		
		if(0 == nPos) nPosY = nCaretPosY;	/* 物理行の移動なし */
		
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		nPos = LineIndexToColmn( pcLayout, nPos );
		if( (m_nCaretPosX != nPos) || (m_nCaretPosY != nPosY) ){
			nCaretPosX = nPos;
			nCaretPosY = nPosY;
		}
	}

	//	2006.07.09 genta 新規関数にまとめた
	MoveCursorSelecting( nCaretPosX, nCaretPosY, bSelect );
}




// 行末に移動(折り返し単位)
void CEditView::Command_GOLINEEND( bool bSelect, int bIgnoreCurrentSelection )
{
	int				nPosX = 0;
	int				nPosY = m_nCaretPosY;
	const CLayout*	pcLayout;
	if( !bIgnoreCurrentSelection ){
		if( bSelect ){
			if( !IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在のカーソル位置から選択を開始する */
				BeginSelectArea();
			}
		}else{
			if( IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( true );
			}
		}
	}

	/* 現在行のデータを取得 */
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			nPosX += it.getColumnDelta();
			break;
		}
		it.addDelta();
	}
	nPosX += it.getColumn() - it.getColumnDelta();
	if( it.getIndex() >= (pcLayout ? pcLayout->GetLengthWithEOL() : 0) ){
		if( m_nCaretPosY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
			/* 改行で終わっているか */
			if( EOL_NONE != pcLayout->m_cEol ){
			}else{
				nPosX += it.getColumnDelta();
			}
		}
	}
	
	MoveCursor( nPosX, m_nCaretPosY, true );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( bSelect ){
		// 現在のカーソル位置によって選択範囲を変更
		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY );
	}
}




/* ファイルの先頭に移動 */
void CEditView::Command_GOFILETOP( bool bSelect )
{
	/* 先頭へカーソルを移動 */
	//	Sep. 8, 2000 genta
	AddCurrentLineToHistory();

	//	2006.07.09 genta 新規関数にまとめた
	MoveCursorSelecting( (!m_bBeginBoxSelect)? 0: m_nCaretPosX, 0, bSelect );	//	通常は、(0, 0)へ移動。ボックス選択中は、(m_nCaretPosX, 0)へ移動
}




/* ファイルの最後に移動 */
void CEditView::Command_GOFILEEND( bool bSelect )
{
// 2001.12.13 hor BOX選択中にファイルの最後にジャンプすると[EOF]の行が反転したままになるの修正
	if( !bSelect && IsTextSelected() ) DisableSelectArea( true );	// 2001.12.21 hor Add
	AddCurrentLineToHistory();
	Cursor_UPDOWN( m_pcEditDoc->m_cLayoutMgr.GetLineCount() , bSelect );
	Command_DOWN( bSelect, true );
	if ( !m_bBeginBoxSelect ){							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			改行のない最終行で選択肢ながら文書末へ移動した場合に
			選択範囲が正しくない場合がある問題に対応
		*/
		Command_GOLINEEND( bSelect, 0 );				// 2001.12.21 hor Add
	}
	MoveCursor( m_nCaretPosX, m_nCaretPosY, true );	// 2001.12.21 hor Add
	// 2002.02.16 hor 矩形選択中を除き直前のカーソル位置をリセット
	if( !(IsTextSelected() && m_bBeginBoxSelect) ) m_nCaretPosX_Prev = m_nCaretPosX;

	// 選択範囲情報メッセージを表示する	// 2009.05.06 ryoji 追加
	if( bSelect ){
		PrintSelectionInfoMsg();
	}
}




/* 単語の左端に移動 */
void CEditView::Command_WORDLEFT( bool bSelect )
{
	int				nIdx;
	int				nLineNew;
	int				nColmNew;
	bool			bIsFreeCursorModeOld;
	if( bSelect ){
		if( !IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){		/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( true );
		}
	}

	const CLayout* pcLayout;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL == pcLayout ){
		bIsFreeCursorModeOld = m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* フリーカーソルモードか */
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* カーソル左移動 */
		Command_LEFT( bSelect, false );
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* フリーカーソルモードか */
		return;
	}

	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

	/* 現在位置の左の単語の先頭位置を調べる */
	int nResult = m_pcEditDoc->m_cLayoutMgr.PrevWord(
		m_nCaretPosY,
		nIdx,
		&nLineNew,
		&nColmNew,
		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* 行が変わった */
		if( nLineNew != m_nCaretPosY ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNew );
			if( NULL == pcLayout ){
				return;
			}
		}

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		nColmNew = LineIndexToColmn( pcLayout, nColmNew );
		/* カーソル移動 */
		MoveCursor( nColmNew, nLineNew, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( nColmNew, nLineNew );
		}
	}else{
		bIsFreeCursorModeOld = m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* フリーカーソルモードか */
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* カーソル左移動 */
		Command_LEFT( bSelect, false );
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* フリーカーソルモードか */
	}
	return;
}




/* 単語の右端に移動 */
void CEditView::Command_WORDRIGHT( bool bSelect )
{
	int			nIdx;
	int			nCurLine;
	int			nLineNew;
	int			nColmNew;
	if( bSelect ){
		if( !IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( true );
		}
	}
	int		bTryAgain = FALSE;
try_again:;
	nCurLine = m_nCaretPosY;
	const CLayout* pcLayout;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nCurLine );
	if( NULL == pcLayout ){
		return;
	}
	if( bTryAgain ){
		const char*	pLine = pcLayout->GetPtr();
		if( pLine[0] != ' ' && pLine[0] != TAB ){
			return;
		}
	}
	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

	/* 現在位置の右の単語の先頭位置を調べる */
	int nResult = m_pcEditDoc->m_cLayoutMgr.NextWord(
		nCurLine,
		nIdx,
		&nLineNew,
		&nColmNew,
		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* 行が変わった */
		if( nLineNew != nCurLine ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNew );
			if( NULL == pcLayout ){
				return;
			}
		}
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		nColmNew = LineIndexToColmn( pcLayout, nColmNew );
		// カーソル移動
		MoveCursor( nColmNew, nLineNew, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( nColmNew, nLineNew );
		}
	}
	else{
		bool	bIsFreeCursorModeOld = m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* フリーカーソルモードか */
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* カーソル右移動 */
		Command_RIGHT( bSelect, false, false );
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* フリーカーソルモードか */
		if( !bTryAgain ){
			bTryAgain = TRUE;
			goto try_again;
		}
	}
	return;
}



/** カーソル行をクリップボードにコピーする
	@param bAddCRLFWhenCopy [in] 折り返し位置に改行コードを挿入するか？
	@param neweol [in] コピーするときのEOL。
	@param bEnableLineModePaste [in] ラインモード貼り付けを可能にする

	@date 2007.10.08 ryoji 新規（Command_COPY()から処理抜き出し）
*/
void CEditView::CopyCurLine(
	bool bAddCRLFWhenCopy,
	EEolType neweol,
	bool bEnableLineModePaste
)
{
	if( IsTextSelected() ){
		return;
	}

	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL == pcLayout ){
		return;
	}

	/* クリップボードに入れるべきテキストデータを、cmemBufに格納する */
	CMemory cmemBuf;
	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() );
	if( pcLayout->m_cEol.GetLen() != 0 ){
		cmemBuf.AppendString(
			( neweol == EOL_UNKNOWN ) ?
				pcLayout->m_cEol.GetValue() : CEol(neweol).GetValue()
		);
	}else if( bAddCRLFWhenCopy ){	// 2007.10.08 ryoji bAddCRLFWhenCopy対応処理追加
		cmemBuf.AppendString(
			( neweol == EOL_UNKNOWN ) ?
				CRLF : CEol(neweol).GetValue()
		);
	}

	/* クリップボードにデータcmemBufの内容を設定 */
	if( false == MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), false, bEnableLineModePaste ) ){
		ErrorBeep();
		return;
	}

	return;
}



/**	選択範囲をクリップボードにコピー

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CEditView::Command_COPY(
	bool		bIgnoreLockAndDisable,	//!< [in] 選択範囲を解除するか？
	bool		bAddCRLFWhenCopy,		//!< [in] 折り返し位置に改行コードを挿入するか？
	EEolType	neweol					//!< [in] コピーするときのEOL。
)
{
	CMemory		cmemBuf;
	bool		bBeginBoxSelect = false;

	/* クリップボードに入れるべきテキストデータを、cmemBufに格納する */
	if( !IsTextSelected() ){
		/* 非選択時は、カーソル行をコピーする */
		if( !m_pShareData->m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste
		);
	}
	else{
		/* テキストが選択されているときは、選択範囲のデータを取得 */

		if( m_bBeginBoxSelect ){
			bBeginBoxSelect = true;
		}
		/* 選択範囲のデータを取得 */
		/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
		if( !GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, bAddCRLFWhenCopy, neweol ) ){
			ErrorBeep();
			return;
		}

		/* クリップボードにデータcmemBufの内容を設定 */
		if( !MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect, false ) ){
			ErrorBeep();
			return;
		}
	}

	/* 選択範囲の後片付け */
	if( !bIgnoreLockAndDisable ){
		/* 選択状態のロック */
		if( m_bSelectingLock ){
			m_bSelectingLock = false;
		}
	}
	if( m_pShareData->m_Common.m_sEdit.m_bCopyAndDisablSelection ){	/* コピーしたら選択解除 */
		/* テキストが選択されているか */
		if( IsTextSelected() ){
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( true );
		}
	}
	return;
}




/** 切り取り(選択範囲をクリップボードにコピーして削除)

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void CEditView::Command_CUT( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CMemory	cmemBuf;
	bool	bBeginBoxSelect;
	/* 範囲選択がされていない */
	if( !IsTextSelected() ){
		/* 非選択時は、カーソル行を切り取り */
		if( !m_pShareData->m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		//行切り取り(折り返し単位)
		Command_CUT_LINE();
		return;
	}
	if( m_bBeginBoxSelect ){
		bBeginBoxSelect = true;
	}else{
		bBeginBoxSelect = false;
	}

	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
	if( !GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}
	/* クリップボードにデータを設定 */
	if( !MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect ) ){
		ErrorBeep();
		return;
	}

	/* カーソル位置または選択エリアを削除 */
	DeleteData( true );
	return;
}




//カーソル位置または選択エリアを削除
void CEditView::Command_DELETE( void )
{
	if( m_bBeginSelect ){		/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( !IsTextSelected() ){	/* テキストが選択されているか */
		// 2008.08.03 nasukoji	選択範囲なしでDELETEを実行した場合、カーソル位置まで半角スペースを挿入した後改行を削除して次行と連結する
		if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > m_nCaretPosY ){
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
			if( pcLayout ){
				int nLineLen;
				LineColmnToIndex2( pcLayout, m_nCaretPosX, nLineLen );
				if( nLineLen ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					if( EOL_NONE != pcLayout->m_cEol ){	// 行終端は改行コードか?
						Command_INSTEXT( true, "", 0, FALSE );	// カーソル位置まで半角スペース挿入
					}
				}
			}
		}
	}
	DeleteData( true );
	return;
}




//カーソル前を削除
void CEditView::Command_DELETE_BACK( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	COpe*		pcOpe = NULL;
	BOOL		bBool;
	int			nPosX;
	int			nPosY;
	//	May 29, 2004 genta 実際に削除された文字がないときはフラグをたてないように
	//m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
	if( IsTextSelected() ){				/* テキストが選択されているか */
		DeleteData( true );
	}
	else{
		nPosX = m_nCaretPosX;
		nPosY = m_nCaretPosY;
		bBool = Command_LEFT( false, false );
		// 2008.08.03 nasukoji	改行より右側でのBACKSPACEでもUndoデータを作成しない
		if( bBool ){
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
			if( pcLayout ){
				int nLineLen;
				int nIdx = LineColmnToIndex2( pcLayout, m_nCaretPosX, nLineLen );
				if( nLineLen == 0 ){	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					// 右からの移動では折り返し末尾文字は削除するが改行は削除しない
					// 下から（下の行の行頭から）の移動では改行も削除する
					if( nIdx < pcLayout->GetLengthWithoutEOL() || m_nCaretPosY < nPosY ){
						if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
							pcOpe = new COpe;
							pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
							m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
								nPosX,
								nPosY,
								&pcOpe->m_ptCaretPos_PHY_Before.x,
								&pcOpe->m_ptCaretPos_PHY_Before.y
							);
							pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
							pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
							/* 操作の追加 */
							m_pcOpeBlk->AppendOpe( pcOpe );
						}
						DeleteData( true );
					}
				}
			}
		}
	}
	PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}




//単語の右端まで削除
void CEditView::Command_WordDeleteToEnd( void )
{
	COpe*	pcOpe = NULL;
	CMemory	cmemData;

	/* 矩形選択状態では実行不能((★★もろ手抜き★★)) */
	if( IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_bBeginBoxSelect ){
			ErrorBeep();
			return;
		}
	}
	/* 単語の右端に移動 */
	CEditView::Command_WORDRIGHT( true );
	if( !IsTextSelected() ){
		ErrorBeep();
		return;
	}
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;							/* 操作種別 */
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			m_nSelectColmFrom,
			m_nSelectLineFrom,
			&pcOpe->m_ptCaretPos_PHY_Before.x,
			&pcOpe->m_ptCaretPos_PHY_Before.y
		);
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	// 操作後のキャレット位置Ｘ
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	// 操作後のキャレット位置Ｙ
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	/* 削除 */
	DeleteData( true );
}




//単語の左端まで削除
void CEditView::Command_WordDeleteToStart( void )
{
	COpe*	pcOpe = NULL;
	CMemory	cmemData;
	/* 矩形選択状態では実行不能(★★もろ手抜き★★) */
	if( IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_bBeginBoxSelect ){
			ErrorBeep();
			return;
		}
	}

	// 単語の左端に移動
	CEditView::Command_WORDLEFT( true );
	if( !IsTextSelected() ){
		ErrorBeep();
		return;
	}

	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			m_nSelectColmTo,
			m_nSelectLineTo,
			&pcOpe->m_ptCaretPos_PHY_Before.x,
			&pcOpe->m_ptCaretPos_PHY_Before.y
		);
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	// 操作後のキャレット位置Ｘ
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	// 操作後のキャレット位置Ｙ

		// 操作の追加
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	// 削除
	DeleteData( true );
}




//単語切り取り
void CEditView::Command_WordCut( void )
{
	if( IsTextSelected() ){
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	//現在位置の単語選択
	Command_SELECTWORD();
	/* 切り取り(選択範囲をクリップボードにコピーして削除) */
	if ( !IsTextSelected() ){
		//	単語選択で選択できなかったら、次の文字を選ぶことに挑戦。
		Command_RIGHT( true, false, false );
	}
	Command_CUT();
	return;
}




//単語削除
void CEditView::Command_WordDelete( void )
{
	if( IsTextSelected() ){
		/* 削除 */
		DeleteData( true );
		return;
	}
	//現在位置の単語選択
	Command_SELECTWORD();
	/* 削除 */
	DeleteData( true );
	return;
}




//行頭まで切り取り(改行単位)
void CEditView::Command_LineCutToStart( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( 0, pCLayout->m_nLinePhysical, &nX, &nY );
	if( m_nCaretPosX == nX && m_nCaretPosY == nY ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	SetSelectArea( nY, nX, m_nCaretPosY, m_nCaretPosX );

	/*切り取り(選択範囲をクリップボードにコピーして削除) */
	Command_CUT();
}




//行末まで切り取り(改行単位)
void CEditView::Command_LineCutToEnd( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 切り取り(選択範囲をクリップボードにコピーして削除) */
		Command_CUT();
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	if( EOL_NONE == pCLayout->m_pCDocLine->m_cEol ){	/* 改行コードの種類 */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( pCLayout->m_pCDocLine->m_cLine.GetStringLength() , pCLayout->m_nLinePhysical, &nX, &nY );
	}
	else{
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( pCLayout->m_pCDocLine->m_cLine.GetStringLength() - pCLayout->m_pCDocLine->m_cEol.GetLen(), pCLayout->m_nLinePhysical, &nX, &nY );
	}
	if( ( m_nCaretPosX == nX && m_nCaretPosY == nY )
	 || ( m_nCaretPosX >  nX && m_nCaretPosY == nY )
	){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	SetSelectArea( m_nCaretPosY, m_nCaretPosX, nY, nX );

	/*切り取り(選択範囲をクリップボードにコピーして削除) */
	Command_CUT();
}




//行頭まで削除(改行単位)
void CEditView::Command_LineDeleteToStart( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		DeleteData( true );
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( 0, pCLayout->m_nLinePhysical, &nX, &nY );
	if( m_nCaretPosX == nX && m_nCaretPosY == nY ){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	SetSelectArea( nY, nX, m_nCaretPosY, m_nCaretPosX );

	/* 選択領域削除 */
	DeleteData( true );
}




//行末まで削除(改行単位)
void CEditView::Command_LineDeleteToEnd( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		DeleteData( true );
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	if( EOL_NONE == pCLayout->m_pCDocLine->m_cEol ){	/* 改行コードの種類 */
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( pCLayout->m_pCDocLine->m_cLine.GetStringLength() , pCLayout->m_nLinePhysical, &nX, &nY );
	}else{
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( pCLayout->m_pCDocLine->m_cLine.GetStringLength() - pCLayout->m_pCDocLine->m_cEol.GetLen(), pCLayout->m_nLinePhysical, &nX, &nY );
	}
	if( ( m_nCaretPosX == nX && m_nCaretPosY == nY )
	 || ( m_nCaretPosX >  nX && m_nCaretPosY == nY )
	){
		ErrorBeep();
		return;
	}

	/* 選択範囲の変更 */
	//	2005.06.24 Moca
	SetSelectArea( m_nCaretPosY, m_nCaretPosX, nY, nX );

	/* 選択領域削除 */
	DeleteData( true );
}




//行切り取り(折り返し単位)
void CEditView::Command_CUT_LINE( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	if( IsTextSelected() ){	/* テキストが選択されているか */
		ErrorBeep();
		return;
	}

	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji 処理簡素化
	CopyCurLine(
		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy,
		EOL_UNKNOWN,
		m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}




/* 行削除(折り返し単位) */
void CEditView::Command_DELETE_LINE( void )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	int				nCaretPosX_OLD;
	int				nCaretPosY_OLD;
	COpe*			pcOpe = NULL;
	const CLayout*	pcLayout;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		ErrorBeep();
		return;
	}
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}
	m_nSelectLineFrom = m_nCaretPosY;		/* 範囲選択開始行 */
	m_nSelectColmFrom = 0; 					/* 範囲選択開始桁 */
	m_nSelectLineTo = m_nCaretPosY + 1;		/* 範囲選択終了行 */
	m_nSelectColmTo = 0;					/* 範囲選択終了桁 */

	nCaretPosX_OLD = m_nCaretPosX;
	nCaretPosY_OLD = m_nCaretPosY;

	Command_DELETE();
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL != pcLayout ){
		// 2003-04-30 かろと
		// 行削除した後、フリーカーソルでないのにカーソル位置が行端より右になる不具合対応
		// フリーカーソルモードでない場合は、カーソル位置を調整する
		if( !m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode ) {
			int nIndex;
			nIndex = LineColmnToIndex2( pcLayout, nCaretPosX_OLD, nCaretPosX_OLD );

			if (nCaretPosX_OLD > 0) {
				nCaretPosX_OLD--;
			} else {
				nCaretPosX_OLD = LineIndexToColmn( pcLayout, nIndex );
			}
		}
		/* 操作前の位置へカーソルを移動 */
		MoveCursor( nCaretPosX_OLD, nCaretPosY_OLD, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;					/* 操作種別 */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */

			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	return;
}




/* すべて選択 */
void CEditView::Command_SELECTALL( void )
{
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( true );
	}

	/* 先頭へカーソルを移動 */
	//	Sep. 8, 2000 genta
	AddCurrentLineToHistory();
	m_nCaretPosX_Prev = m_nCaretPosX;

	//	Jul. 29, 2006 genta 選択位置の末尾を正確に取得する
	//	マクロから取得した場合に正しい範囲が取得できないため
	int nX, nY;
	m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos(nX, nY);
	SetSelectArea( 0, 0, nY, nX );

	/* 選択領域描画 */
	DrawSelectArea();
}




/*!	1行選択
	@brief カーソル位置を1行選択する
	@param lparam [in] マクロから使用する拡張フラグ（拡張用に予約）
	
	@date 2007.10.06 nasukoji	新規作成
	@date 2007.10.11 nasukoji	1行選択を改行単位で行うように変更
	@date 2007.10.13 nasukoji	引数を追加
	@date 2007.11.05 nasukoji	EOFのみの行を選択した時、アンダーラインが表示されなくなる不具合を修正
								選択後選択行の行頭へ戻すように変更
*/
void CEditView::Command_SELECTLINE( int lparam )
{
	// 改行単位で1行選択する
	Command_GOLINETOP( false, 0x9 );	// 物理行頭に移動

	m_bBeginLineSelect = TRUE;		// 行単位選択中

	int nCaretPosX;
	int nCaretPosY;

	// 最下行（物理行）でない
	if( m_ptCaretPos_PHY.y < m_pcEditDoc->m_cDocLineMgr.GetLineCount() ){
		// 1行先の物理行からレイアウト行を求める
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( 0, m_ptCaretPos_PHY.y + 1, &nCaretPosX, &nCaretPosY );

		// カーソルを次の物理行頭へ移動する
		MoveCursorSelecting( nCaretPosX, nCaretPosY, true );
	}else{
		// カーソルを最下行（レイアウト行）へ移動する
		MoveCursorSelecting( 0, m_pcEditDoc->m_cLayoutMgr.GetLineCount(), true );
		Command_GOLINEEND( true, 0 );	// 行末に移動

		// 選択するものが無い（[EOF]のみの行）時は選択状態としない
		if(( ! IsTextSelected() )&&( m_ptCaretPos_PHY.y >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() ))
		{
			// 現在の選択範囲を非選択状態に戻す
			DisableSelectArea( true );
		}
	}

	if( m_bBeginLineSelect ){
		// クリック行より上へ選択移動した時にクリック行が非選択となってしまうことへの対処
		m_nSelectLineBgnTo = m_nSelectLineTo = m_nCaretPosY;	// 範囲選択開始行(原点)
		m_nSelectColmBgnTo = m_nSelectColmTo = m_nCaretPosX;	// 範囲選択開始桁(原点)
	}

	return;
}




/* 現在位置の単語選択 */
bool CEditView::Command_SELECTWORD( void )
{
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
	int				nIdx;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( true );
	}
	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL == pcLayout ){
		return false;	//	単語選択に失敗
	}
	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

	/* 現在位置の単語の範囲を調べる */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord( m_nCaretPosY, nIdx, &nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL ) ){

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineFrom );
		nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineTo );
		nColmTo = LineIndexToColmn( pcLayout, nColmTo );

		/* 選択範囲の変更 */
		//	2005.06.24 Moca
		SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

		/* 単語の先頭にカーソルを移動 */
		MoveCursor( nColmTo, nLineTo, true );
		m_nCaretPosX_Prev = m_nCaretPosX;

		/* 選択領域描画 */
		DrawSelectArea();
		return true;	//	単語選択に成功。
	}
	else {
		return false;	//	単語選択に失敗
	}
}




/** 貼り付け(クリップボードから貼り付け)
	@param [in] option 貼り付け時のオプション
	@li 0x01 改行コード変換有効
	@li 0x02 改行コード変換無効
	@li 0x04 ラインモード貼り付け有効
	@li 0x08 ラインモード貼り付け無効
	@li 0x10 矩形コピーは常に矩形貼り付け
	@li 0x20 矩形コピーは常に通常貼り付け

	@date 2007.10.04 ryoji MSDEVLineSelect形式の行コピー対応処理を追加（VS2003/2005のエディタと類似の挙動に）
*/
void CEditView::Command_PASTE( int option )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	//砂時計
	CWaitCursor cWaitCursor( m_hWnd );

	// クリップボードからデータを取得
	CMemory		cmemClip;
	bool		bColmnSelect;
	bool		bLineSelect = false;
	bool		bLineSelectOption = 
		((option & 0x04) == 0x04) ? TRUE :
		((option & 0x08) == 0x08) ? FALSE :
		m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste;

	if( !MyGetClipboardData( cmemClip, &bColmnSelect, bLineSelectOption ? &bLineSelect: NULL ) ){
		ErrorBeep();
		return;
	}

	// クリップボードデータ取得 -> pszText, nTextLen
	int			nTextLen;
	char*		pszText = cmemClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		m_pShareData->m_Common.m_sEdit.m_bConvertEOLPaste;

	bool bAutoColmnPaste = 
		((option & 0x10) == 0x10) ? false :
		((option & 0x20) == 0x20) ? false :
		m_pShareData->m_Common.m_sEdit.m_bAutoColmnPaste != FALSE;

	// 矩形コピーのテキストは常に矩形貼り付け
	if( bAutoColmnPaste ){
		// 矩形コピーのデータなら矩形貼り付け
		if( bColmnSelect ){
			if( m_bBeginBoxSelect ){
				ErrorBeep();
				return;
			}
			if( !m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){
				return;
			}
			if( bConvertEol ){
				char	*pszConvertedText = new char[nTextLen * 2]; // 全文字\n→\r\n変換で最大の２倍になる
				int nConvertedTextLen = ConvertEol(pszText, nTextLen, pszConvertedText);
				Command_PASTEBOX(pszConvertedText, nConvertedTextLen);
				delete [] pszConvertedText;
			}else{
				Command_PASTEBOX(pszText, nTextLen);
			}
			AdjustScrollBars();
			Redraw();
			return;
		}
	}

	// 2007.10.04 ryoji
	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if( bLineSelect ){
		// ※CRやLFは2バイト文字の2バイト目として扱われることはないので末尾だけで判定（CMemory::GetSizeOfChar()参照）
		if( pszText[nTextLen - 1] != CR && pszText[nTextLen - 1] != LF ){
			cmemClip.AppendString(m_pcEditDoc->GetNewLineCode().GetValue());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	if( bConvertEol ){
		char	*pszConvertedText = new char[nTextLen * 2]; // 全文字\n→\r\n変換で最大の２倍になる
		int nConvertedTextLen = ConvertEol( pszText, nTextLen, pszConvertedText );
		// テキストを貼り付け
		Command_INSTEXT( true, pszConvertedText, nConvertedTextLen, TRUE, bLineSelect );	// 2010.09.17 ryoji
		delete [] pszConvertedText;
	}else{
		// テキストを貼り付け
		Command_INSTEXT( true, pszText, nTextLen, TRUE, bLineSelect );	// 2010.09.17 ryoji
	}

	return;
}

int CEditView::ConvertEol(const char* pszText, int nTextLen, char* pszConvertedText)
{
	// original by 2009.02.28 salarm
	int nConvertedTextLen;
	CEol eol = m_pcEditDoc->GetNewLineCode();

	nConvertedTextLen = 0;
	for( int i = 0; i < nTextLen; i++ ){
		if( pszText[i] == CR || pszText[i] == LF ){
			if( pszText[i] == CR ){
				if( i + 1 < nTextLen && pszText[i + 1] == LF ){
					i++;
				}
			}else{
				// LF+CRの場合 ※UNICODE版との差分
				if( i + 1 < nTextLen && pszText[i + 1] == CR ){
					i++;
				}
			}
			memcpy( &pszConvertedText[nConvertedTextLen], eol.GetValue(), eol.GetLen() );
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
void CEditView::Command_INSTEXT(
	bool			bRedraw,		//!<
	const char*		pszText,		//!< [in] 貼り付ける文字列。
	int				nTextLen,		//!< [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	BOOL			bNoWaitCursor,	//!<
	BOOL			bLinePaste		//!< [in] ラインモード貼り付け
)
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	int				nNewLine;			/* 挿入された部分の次の位置の行 */
	int				nNewPos;			/* 挿入された部分の次の位置のデータ位置 */
	COpe*			pcOpe = NULL;
	CWaitCursor*	pcWaitCursor;
	int				i;

	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_hWnd );
	}

	if( nTextLen < 0 ){
		nTextLen = lstrlen( pszText );
	}

	m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	// テキストが選択されているか
	if( IsTextSelected() ){
		// 矩形範囲選択中か
		if( m_bBeginBoxSelect ){
			for( i = 0; i < nTextLen; i++ ){
				if( pszText[i] == CR || pszText[i] == LF ){
					break;
				}
			}
			Command_INDENT( pszText, i );
			goto end_of_func;
		}
		else{
			//	Jun. 23, 2000 genta
			//	同一行の行末以降のみが選択されている場合には選択無しと見なす
			int			len;
			int pos;
			const char	*line;
			const CLayout* pcLayout;
			line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &len, &pcLayout );

			pos = ( line == NULL ) ? 0 : LineColmnToIndex( pcLayout, m_nSelectColmFrom );
			if( pos >= len &&	//	開始位置が行末より後ろで
				m_nSelectLineFrom == m_nSelectLineTo	//	終了位置が同一行
				){
				m_nCaretPosX = m_nSelectColmFrom;
				DisableSelectArea(false);
			}
			else{
				// データ置換 削除&挿入にも使える
				// 行コピーの貼り付けでは選択範囲は削除（後で行頭に貼り付ける）	// 2007.10.04 ryoji
				ReplaceData_CEditView(
					m_nSelectLineFrom,		/* 範囲選択開始行 */
					m_nSelectColmFrom,		/* 範囲選択開始桁 */
					m_nSelectLineTo,		/* 範囲選択終了行 */
					m_nSelectColmTo,		/* 範囲選択終了桁 */
					NULL,					// 削除されたデータのコピー(NULL可能)
					bLinePaste? "": pszText,	// 挿入するデータ
					bLinePaste? 0: nTextLen,	// 挿入するデータの長さ
					true
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
		int nPosX_PHY_Delta;
		if( bLinePaste ){	// 2007.10.04 ryoji
			/* 挿入ポイント（折り返し単位行頭）にカーソルを移動 */
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;						/* 操作種別 */
				pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
			}
			Command_GOLINETOP( false, 1 );	// 行頭に移動(折り返し単位)
			nPosX_PHY_Delta = pcOpe->m_ptCaretPos_PHY_Before.x - m_ptCaretPos_PHY.x;	// 挿入ポイントと元の位置との差分桁数
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}
		if( !m_bDoing_UndoRedo ){						/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
		}

		// 現在位置にデータを挿入
		InsertData_CEditView(
			m_nCaretPosX,
			m_nCaretPosY,
			pszText,
			nTextLen,
			&nNewLine,
			&nNewPos,
			pcOpe,
			true
		);

		// 挿入データの最後へカーソルを移動
		MoveCursor( nNewPos, nNewLine, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){								/* アンドゥ・リドゥの実行中か */
			pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		if( bLinePaste ){	// 2007.10.04 ryoji
			/* 元の位置へカーソルを移動 */
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;						/* 操作種別 */
				pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
			}
			int nPosX;
			int nPosY;
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				m_ptCaretPos_PHY.x + nPosX_PHY_Delta,
				m_ptCaretPos_PHY.y,
				&nPosX,
				&nPosY
			);
			MoveCursor( nPosX, nPosY, true );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
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
// 2769 : m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
// から、
// 3057 : m_bDrawSWITCH = true;	// 2002.01.25 hor
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
void CEditView::Command_PASTEBOX( const char *szPaste, int nPasteSize )
{
	/* これらの動作は残しておきたいのだが、呼び出し側で責任を持ってやってもらうことに変更。
	if( m_bBeginSelect )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}
	if( !m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}
	*/

	int				nBgn;
	int				nPos;
	int				nCount;
	int				nNewLine;		// 挿入された部分の次の位置の行
	int				nNewPos;		// 挿入された部分の次の位置のデータ位置
	int				nCurXOld;
	int				nCurYOld;
	COpe*			pcOpe = NULL;
	BOOL			bAddLastCR;
	int				nInsPosX;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	m_bDrawSWITCH = false;	// 2002.01.25 hor

	// とりあえず選択範囲を削除
	// 2004.06.30 Moca IsTextSelected()がないと未選択時、一文字消えてしまう
	if( IsTextSelected() ){
		DeleteData( false/*true 2002.01.25 hor*/ );
	}

	nCurXOld = m_nCaretPosX;
	nCurYOld = m_nCaretPosY;

	nCount = CLayoutInt(0);

	// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無い場合の対策
	//	データの最後まで処理 i.e. nBgnがnPasteSizeを超えたら終了
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無いと
		//	最終行のPaste処理が動かないので，
		//	データの末尾に来た場合は強制的に処理するようにする
		if( szPaste[nPos] == CR || szPaste[nPos] == LF || nPos == nPasteSize )
		{
			if( !m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
			{
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
												nCurXOld,
												nCurYOld + nCount,
												&pcOpe->m_ptCaretPos_PHY_Before.x,
												&pcOpe->m_ptCaretPos_PHY_Before.y
												);
			}

			/* 現在位置にデータを挿入 */
			if( nPos - nBgn > 0 ){
				InsertData_CEditView(
					nCurXOld,
					nCurYOld + nCount,
					&szPaste[nBgn],
					nPos - nBgn,
					&nNewLine,
					&nNewPos,
					pcOpe,
					false
				);
			}

			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
												nNewPos,
												nNewLine,
												&pcOpe->m_ptCaretPos_PHY_After.x,
												&pcOpe->m_ptCaretPos_PHY_After.y
												);

				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}

			/* この行の挿入位置へカーソルを移動 */
			MoveCursor( nCurXOld, nCurYOld + nCount, false/*true 2002.01.25 hor*/ );
			m_nCaretPosX_Prev = m_nCaretPosX;
			/* カーソル行が最後の行かつ行末に改行が無く、挿入すべきデータがまだある場合 */
			bAddLastCR = FALSE;
			const CLayout*	pcLayout;
			int				nLineLen = 0;
			const char*		pLine;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );

			if( NULL != pLine && 1 <= nLineLen )
			{
				if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )
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
//				MYTRACE( _T(" カーソル行が最後の行かつ行末に改行が無く、\n挿入すべきデータがまだある場合は行末に改行を挿入。\n") );
				nInsPosX = LineIndexToColmn( pcLayout, nLineLen );
				if( !m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
				{
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
														nInsPosX,
														m_nCaretPosY,
														&pcOpe->m_ptCaretPos_PHY_Before.x,
														&pcOpe->m_ptCaretPos_PHY_Before.y
														);
				}

				InsertData_CEditView(
					nInsPosX,
					m_nCaretPosY,
					m_pcEditDoc->GetNewLineCode().GetValue(),
					m_pcEditDoc->GetNewLineCode().GetLen(),
					&nNewLine,
					&nNewPos,
					pcOpe,
					false
				);

				if( !m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
				{
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
													nNewPos,
													nNewLine,
													&pcOpe->m_ptCaretPos_PHY_After.x,
													&pcOpe->m_ptCaretPos_PHY_After.y
													);

					/* 操作の追加 */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}
			}

			if(
				(nPos + 1 < nPasteSize ) &&
				(
// 2004.06.30 Moca WORD*では非x86で境界不整列の可能性あり
				 ( szPaste[nPos] == '\n' && szPaste[nPos + 1] == '\r') ||
				 ( szPaste[nPos] == '\r' && szPaste[nPos + 1] == '\n')
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
	MoveCursor( nCurXOld, nCurYOld, true );
	m_nCaretPosX_Prev = m_nCaretPosX;

	if( !m_bDoing_UndoRedo )	/* アンドゥ・リドゥの実行中か */
	{
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;						/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */

		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	m_bDrawSWITCH = true;	// 2002.01.25 hor
	return;
}

/** 矩形貼り付け(クリップボードから矩形貼り付け)
	@param [in] option 未使用

	@date 2004.06.30 2004.06.29 Moca 未使用だったものを有効にする
	オリジナルのCommand_PASTEBOX(void)はばっさり削除 (genta)
*/
void CEditView::Command_PASTEBOX( int option )
{
	if( m_bBeginSelect )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}

	if( !m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// 現在のフォントは固定幅フォントである
	{
		return;
	}

	// クリップボードからデータを取得
	CMemory	cmemClip;
	if( !MyGetClipboardData( cmemClip, NULL ) ){
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0コピー対策
	int nstrlen;
	const char *lptstr = cmemClip.GetStringPtr( &nstrlen );

	Command_PASTEBOX(lptstr, nstrlen);
	AdjustScrollBars(); // 2007.07.22 ryoji
	Redraw();			// 2002.01.25 hor
}

//>> 2002/03/29 Azumaiya

/* １バイト文字入力 */
void CEditView::Command_CHAR( char cChar )
{
	if( m_bBeginSelect )	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}


	CMemory			cmemData;
	CMemory			cmemIndent;
	int				nPos;
	int				nCharChars;
	int				nIdxTo;
	int				nPosX;
	int				nNewLine;	/* 挿入された部分の次の位置の行 */
	int				nNewPos;	/* 挿入された部分の次の位置のデータ位置 */
	COpe*			pcOpe = NULL;
	char			szCurrent[10];

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	/* 現在位置にデータを挿入 */
	nPosX = CLayoutInt(-1);
	cmemData = cChar;
	if( cChar == CR || cChar == LF ){
		/* 現在、Enterなどで挿入する改行コードの種類を取得 */
		CEol cWork = m_pcEditDoc->GetNewLineCode();
		cmemData.SetString( cWork.GetValue(), cWork.GetLen() );

		/* テキストが選択されているか */
		if( IsTextSelected() ){
			DeleteData( true );
		}
		if( m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent ){	/* オートインデント */
			const CLayout* pCLayout;
			const char*		pLine;
			int				nLineLen;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pCLayout );
			if( NULL != pCLayout ){
				const CDocLine* pcDocLine;
				pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( pCLayout->m_nLinePhysical );
				pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( pCLayout->m_nLinePhysical, &nLineLen );
				if( NULL != pLine ){
					/*
					  カーソル位置変換
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					  →
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					*/
					int		nX;
					int		nY;
					m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
						m_nCaretPosX,
						m_nCaretPosY,
						&nX,
						&nY
					);

					/* 指定された桁に対応する行のデータ内の位置を調べる */
					nIdxTo = LineColmnToIndex( pcDocLine, m_nCaretPosX );
					for( nPos = 0; nPos < /*nIdxTo*/nLineLen && nPos < nX; ){
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );

						/* その他のインデント文字 */
						if( 0 < nCharChars
						 && pLine[nPos] != '\0'	// その他のインデント文字に '\0' は含まれない	// 2009.02.04 ryoji L'\0'がインデントされてしまう問題修正
						 && m_pcEditDoc->GetDocumentAttribute().m_szIndentChars[0] != '\0'
						){
							memcpy( szCurrent, &pLine[nPos], nCharChars );
							szCurrent[nCharChars] = '\0';
							/* その他のインデント対象文字 */
							if( NULL != _mbsstr(
								(const unsigned char*)m_pcEditDoc->GetDocumentAttribute().m_szIndentChars,
								(const unsigned char*)szCurrent
							) ){
								goto end_of_for;
							}
						}
						if( nCharChars == 1 ){
							if( pLine[nPos] == SPACE ||
								pLine[nPos] == TAB ){
							}else{
								break;
							}
						}else
						if( nCharChars == 2 ){
							if( m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent_ZENSPACE ){	/* 日本語空白もインデント */
								if( pLine[nPos    ] == (char)0x81 &&
									pLine[nPos + 1] == (char)0x40 ){
								}else{
									break;
								}
							}else{
								break;
							}
						}else
						if( nCharChars == 0 ){
							break;
						}else{
							break;
						}
end_of_for:;
						nPos += nCharChars;
					}
					if( nPos > 0 ){
						nPosX = LineIndexToColmn( pcDocLine, nPos );
					}
					cmemIndent.SetString( pLine, nPos );
					cmemData += cmemIndent;
				}
			}
		}
	}
	else{
		/* テキストが選択されているか */
		if( IsTextSelected() ){
			/* 矩形範囲選択中か */
			if( m_bBeginBoxSelect ){
				Command_INDENT( cChar );
				return;
			}else{
				DeleteData( true );
			}
		}
		else{
			if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){
				DelCharForOverwrite();	// 上書き用の一文字削除	// 2009.04.11 ryoji
			}
		}
	}
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
	}


	InsertData_CEditView(
		m_nCaretPosX,
		m_nCaretPosY,
		cmemData.GetStringPtr(),
		cmemData.GetStringLength(),
		&nNewLine,
		&nNewPos,
		pcOpe,
		true
	);
	/* 挿入データの最後へカーソルを移動 */
	MoveCursor( nNewPos, nNewLine, true );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}


	/* スマートインデント */
	if( SMARTINDENT_CPP == m_pcEditDoc->GetDocumentAttribute().m_nSmartIndent ){	/* スマートインデント種別 */
		/* C/C++スマートインデント処理 */
		SmartIndent_CPP( cChar );
	}

	/* 2005.10.11 ryoji 改行時に末尾の空白を削除 */
	if( CR == cChar && TRUE == m_pcEditDoc->GetDocumentAttribute().m_bRTrimPrevLine ){	/* 改行時に末尾の空白を削除 */
		/* 前の行にある末尾の空白を削除する */
		RTrimPrevLine();
	}

	PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
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
void CEditView::Command_IME_CHAR( WORD wChar )
{
	if( m_bBeginSelect ){	/* マウスによる範囲選択中 */
		ErrorBeep();
		return;
	}

	CMemory			cmemData;
	int				nNewLine;		/* 挿入された部分の次の位置の行 */
	int				nNewPos;		/* 挿入された部分の次の位置のデータ位置 */
	COpe*			pcOpe = NULL;
	char	sWord[2];
	//	Oct. 6 ,2002 genta 上下逆転
	if( 0 == (wChar & 0xff00) ){
		Command_CHAR( wChar & 0xff );
		return;
	}
	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	// Oct. 6 ,2002 genta バッファに格納する
	sWord[0] = (wChar >> 8) & 0xff;
	sWord[1] = wChar & 0xff;

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		/* 矩形範囲選択中か */
		if( m_bBeginBoxSelect ){
			Command_INDENT( &sWord[0], 2 );	//	Oct. 6 ,2002 genta 
			return;
		}else{
			DeleteData( true );
		}
	}
	else{
		if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){
			DelCharForOverwrite();	// 上書き用の一文字削除	// 2009.04.11 ryoji
		}
	}
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
	}
	//	Oct. 6 ,2002 genta 
	InsertData_CEditView( m_nCaretPosX, m_nCaretPosY, &sWord[0], 2, &nNewLine, &nNewPos, pcOpe, true );

	/* 挿入データの最後へカーソルを移動 */
	MoveCursor( nNewPos, nNewLine, true );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	PostprocessCommand_hokan();	//	Jan. 10, 2005 genta 関数化
}




/*! 挿入／上書きモード切り替え

	@date 2005.10.02 genta InsMode関数化
*/
void CEditView::Command_CHGMOD_INS( void )
{
	/* 挿入モードか？ */
	if( IsInsMode() ){
		SetInsMode( false );
	}else{
		SetInsMode( true );
	}
	/* キャレットの表示・更新 */
	ShowEditCaret();
	/* キャレットの行桁位置を表示する */
	DrawCaretPosInfo();
}


/*!
検索(ボックス)コマンド実行.
ツールバーの検索ボックスにフォーカスを移動する.
	@date 2006.06.04 yukihane 新規作成
*/
void CEditView::Command_SEARCH_BOX( void )
{
	const CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;
	pCEditWnd->SetFocusSearchBox();
}

/* 検索(単語検索ダイアログ) */
void CEditView::Command_SEARCH_DIALOG( void )
{
//	int			nRet;
	CMemory		cmemCurText;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* 検索文字列を初期化 */
	strcpy( m_pcEditDoc->m_cDlgFind.m_szText, cmemCurText.GetStringPtr() );

	/* 検索ダイアログの表示 */
	if( NULL == m_pcEditDoc->m_cDlgFind.m_hWnd ){
		m_pcEditDoc->m_cDlgFind.DoModeless( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc->m_pcEditViewArr[m_pcEditDoc->m_nActivePaneIndex] );
	}
	else{
		/* アクティブにする */
		ActivateFrameWindow( m_pcEditDoc->m_cDlgFind.m_hWnd );
		::SetDlgItemText( m_pcEditDoc->m_cDlgFind.m_hWnd, IDC_COMBO_TEXT, cmemCurText.GetStringPtr() );
	}
	return;
}


/* 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときはFALSEを返す) */
/* 2002.01.16 hor 共通ロジックを関数にしただけ・・・ */
BOOL CEditView::ChangeCurRegexp( bool bRedrawIfChanged )
{
	BOOL	bChangeState;
	if( !m_bCurSrchKeyMark
	 || 0 != strcmp( m_szCurSrchKey, m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0] )
	 || m_sCurSearchOption != m_pShareData->m_Common.m_sSearch.m_sSearchOption
	){
		bChangeState = TRUE;
	}else{
		bChangeState = FALSE;
	}

	m_bCurSrchKeyMark = true;									/* 検索文字列のマーク */
	strcpy( m_szCurSrchKey, m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0] );/* 検索文字列 */
	m_sCurSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;// 検索／置換  オプション
	/* 正規表現 */
	if( m_sCurSearchOption.bRegularExp
	 && bChangeState
	){
		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
			return FALSE;
		}
		int nFlag = 0x00;
		nFlag |= m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;
		/* 検索パターンのコンパイル */
		m_CurRegexp.Compile( m_szCurSrchKey, nFlag );
	}

	if( bChangeState && bRedrawIfChanged ){
		/* フォーカス移動時の再描画 */
		RedrawAll();
	}

	return TRUE;
}


/* 前を検索 */
void CEditView::Command_SEARCH_PREV( bool bReDraw, HWND hwndParent )
{
	int			nLineNum;
	int			nIdx;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	BOOL		bSelecting;
	int			nSelectLineBgnFrom_Old;
	int			nSelectColBgnFrom_Old;
	int			nSelectLineBgnTo_Old;
	int			nSelectColBgnTo_Old;
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
	bool		bSelectingLock_Old;
	BOOL		bFound = FALSE;
	BOOL		bRedo = FALSE;			//	hor
	int			nLineNumOld,nIdxOld;	//	hor
	const CLayout* pcLayout = NULL;

	nLineFrom = m_nCaretPosY;
	nColmFrom = m_nCaretPosX;
	nLineTo = m_nCaretPosY;
	nColmTo = m_nCaretPosX;
//	bFlag1 = FALSE;
	bSelecting = FALSE;
	if( '\0' == m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0][0] ){
		goto end_of_func;
	}
	if( IsTextSelected() ){	/* テキストが選択されているか */
		nSelectLineBgnFrom_Old = m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
		nSelectColBgnFrom_Old = m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
		nSelectLineBgnTo_Old = m_nSelectLineBgnTo;		/* 範囲選択開始行(原点) */
		nSelectColBgnTo_Old = m_nSelectColmBgnTo;		/* 範囲選択開始桁(原点) */
		nSelectLineFrom_Old = m_nSelectLineFrom;
		nSelectColFrom_Old = m_nSelectColmFrom;
		nSelectLineTo_Old = m_nSelectLineTo;
		nSelectColTo_Old = m_nSelectColmTo;
		bSelectingLock_Old = m_bSelectingLock;

		/* 矩形範囲選択中か */
		if( !m_bBeginBoxSelect && m_bSelectingLock ){	/* 選択状態のロック */
			bSelecting = TRUE;
		}
		else{
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( bReDraw );
		}
	}

	nLineNum = m_nCaretPosY;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );


	if( NULL == pcLayout ){
		// pcLayoutはNULLとなるのは、[EOF]から前検索した場合
		// １行前に移動する処理
		nLineNum--;
		if( nLineNum < 0 ){
			goto end_of_func;
		}
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		if( NULL == pcLayout ){
			goto end_of_func;
		}
		// カーソル左移動はやめて nIdxは行の長さとしないと[EOF]から改行を前検索した時に最後の改行を検索できない 2003.05.04 かろと
		CLayout* pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		nIdx = pCLayout->m_pCDocLine->m_cLine.GetStringLength() + 1;		// 行末のヌル文字(\0)にマッチさせるために+1 2003.05.16 かろと
	} else {
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
	}
	// 2002.01.16 hor
	// 共通部分のくくりだし
	if(!ChangeCurRegexp()){
		return;
	}

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;							//	hor
	/* 現在位置より前の位置を検索する */
	if( m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLineNum,								// 検索開始行
		nIdx,									// 検索開始位置
		m_szCurSrchKey,							// 検索条件
		SEARCH_BACKWARD,						// 0==前方検索 1==後方検索
		m_sCurSearchOption,						// 検索オプション
		&nLineFrom,								// マッチレイアウト行from
		&nColmFrom, 							// マッチレイアウト位置from
		&nLineTo, 								// マッチレイアウト行to
		&nColmTo, 								// マッチレイアウト位置to
		&m_CurRegexp							// 正規表現コンパイルデータ
	) ){
		if( bSelecting ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
			m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
		}else{
			/* 選択範囲の変更 */
			//	2005.06.24 Moca
			SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

			if( bReDraw ){
				/* 選択領域描画 */
				DrawSelectArea();
			}
		}
		/* カーソル移動 */
		//	Sep. 8, 2000 genta
		AddCurrentLineToHistory();
		MoveCursor( nColmFrom, nLineFrom, bReDraw );
		m_nCaretPosX_Prev = m_nCaretPosX;
		bFound = TRUE;
	}else{
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( bReDraw );
		}
		if( bSelecting ){
			m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
			/* 選択範囲の変更 */
			m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* 範囲選択開始行(原点) */
			m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* 範囲選択開始桁(原点) */
			m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
			m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* 範囲選択開始桁(原点) */

			m_nSelectLineFrom =	nSelectLineFrom_Old;
			m_nSelectColmFrom = nSelectColFrom_Old;
			m_nSelectLineTo = nSelectLineTo_Old;
			m_nSelectColmTo = nSelectColTo_Old;
			/* カーソル移動 */
			MoveCursor( nColmFrom, nLineFrom, bReDraw );
			m_nCaretPosX_Prev = m_nCaretPosX;
			/* 選択領域描画 */
			DrawSelectArea();
		}
	}
end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if(m_pShareData->m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		){
			nLineNum=m_pcEditDoc->m_cLayoutMgr.GetLineCount()-1;
			nIdx=MAXLINEKETAS;
			bRedo=FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if(bFound){
		if((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			SendStatusMessage(_T("▲末尾から再検索しました"));
	}else{
		SendStatusMessage(_T("△見つかりませんでした"));
// To Here 2002.01.26 hor

		/* 検索／置換  見つからないときメッセージを表示 */
		AlertNotFound(
			hwndParent,
			_T("後方(↑) に文字列 '%s' が１つも見つかりません。"),	//Jan. 25, 2001 jepro メッセージを若干変更
			m_szCurSrchKey
		);
	}
	return;
}




/*! 次を検索
	@param bChangeCurRegexp 共有データの検索文字列を使う
	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理見直し．
	@date 2004.05.30 Moca bChangeCurRegexp=trueで従来通り。falseで、CEditViewの現在設定されている検索パターンを使う
*/
void CEditView::Command_SEARCH_NEXT(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	HWND			hwndParent,
	const char*		pszNotFoundMessage
)
{
	int			nLineNum;
	int			nIdx;
	BOOL		bSelecting;
	int			nSelectLineBgnFrom_Old;
	int			nSelectColBgnFrom_Old;
	int			nSelectLineBgnTo_Old;
	int			nSelectColBgnTo_Old;
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
	BOOL		bFlag1;
	bool		bSelectingLock_Old;
	BOOL		bFound = FALSE;
	BOOL		bRedo = FALSE;			//	hor
	int			nLineNumOld,nIdxOld;	//	hor
	const CLayout* pcLayout;
	bool b0Match = false;		//!< 長さ０でマッチしているか？フラグ by かろと
	int nLineLen;
	const char *pLine;

	int			nLineFrom = m_nCaretPosY;
	int			nColmFrom = m_nCaretPosX;
	int			nLineTo = m_nCaretPosY;
	int			nColmTo = m_nCaretPosX;

	bSelecting = FALSE;
	//	2004.05.30 Moca bChangeCurRegexpに応じて対象文字列を変える
	if( bChangeCurRegexp  && '\0' == m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0][0] 
	 || !bChangeCurRegexp && '\0' == m_szCurSrchKey[0] ){
		goto end_of_func;
	}

	// 検索開始位置を調整
	bFlag1 = FALSE;
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 矩形範囲選択中でない & 選択状態のロック */
		if( !m_bBeginBoxSelect && m_bSelectingLock ){
			bSelecting = TRUE;
			bSelectingLock_Old = m_bSelectingLock;
			nSelectLineBgnFrom_Old = m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
			nSelectColBgnFrom_Old = m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
			nSelectLineBgnTo_Old = m_nSelectLineBgnTo;		/* 範囲選択開始行(原点) */
			nSelectColBgnTo_Old = m_nSelectColmBgnTo;		/* 範囲選択開始桁(原点) */
			nSelectLineFrom_Old = m_nSelectLineFrom;
			nSelectColFrom_Old = m_nSelectColmFrom;
			nSelectLineTo_Old = m_nSelectLineTo;
			nSelectColTo_Old = m_nSelectColmTo;

			if( ( m_nSelectLineBgnFrom >  m_nCaretPosY ) ||
				( m_nSelectLineBgnFrom == m_nCaretPosY && m_nSelectColmBgnFrom >= m_nCaretPosX )
			){
				// カーソル移動
				m_nCaretPosX=m_nSelectColmFrom;
				m_nCaretPosY=m_nSelectLineFrom;
				if (m_nSelectColmTo == m_nSelectColmFrom && m_nSelectLineTo == m_nSelectLineFrom) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
				bFlag1 = TRUE;
			}
			else{
				// カーソル移動
				m_nCaretPosX=m_nSelectColmTo;
				m_nCaretPosY=m_nSelectLineTo;
				if (m_nSelectColmTo == m_nSelectColmFrom && m_nSelectLineTo == m_nSelectLineFrom) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
			}
		}
		else{
			/* カーソル移動 */
			m_nCaretPosX=m_nSelectColmTo;
			m_nCaretPosY=m_nSelectLineTo;
			if (m_nSelectColmTo == m_nSelectColmFrom && m_nSelectLineTo == m_nSelectLineFrom) {
				// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
				b0Match = true;
			}

			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( bRedraw );
		}
	}
	nLineNum = m_nCaretPosY;
	nLineLen = 0; // 2004.03.17 Moca NULL == pLineのとき、nLineLenが未設定になり落ちるバグ対策
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);
	/* 指定された桁に対応する行のデータ内の位置を調べる */
// 2002.02.08 hor EOFのみの行からも次検索しても再検索可能に (2/2)
	nIdx = pcLayout ? LineColmnToIndex( pcLayout, m_nCaretPosX ) : 0;
	if( b0Match ) {
		// 現在、長さ０でマッチしている場合は物理行で１文字進める(無限マッチ対策)
		if( nIdx < nLineLen ) {
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nIdx += (CMemory::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
		} else {
			// 念のため行末は別処理
			++nIdx;
		}
	}

	// 2002.01.16 hor
	// 共通部分のくくりだし
	// 2004.05.30 Moca CEditViewの現在設定されている検索パターンを使えるように
	if(bChangeCurRegexp && !ChangeCurRegexp())return;

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;
	 /* 現在位置より後ろの位置を検索する */
	// 2004.05.30 Moca 引数をm_pShareDataからメンバ変数に変更。他のプロセス/スレッドに書き換えられてしまわないように。
	int nSearchResult;
	nSearchResult = m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLineNum, 								// 検索開始行
		nIdx, 									// 検索開始位置
		m_szCurSrchKey,							// 検索条件
		SEARCH_FORWARD,							// 0==前方検索 1==後方検索
		m_sCurSearchOption,						// 検索オプション
		&nLineFrom,								// マッチレイアウト行from
		&nColmFrom, 							// マッチレイアウト位置from
		&nLineTo, 								// マッチレイアウト行to
		&nColmTo, 								// マッチレイアウト位置to
		&m_CurRegexp							// 正規表現コンパイルデータ
	);
	if( nSearchResult ){
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		if( bFlag1 && m_nCaretPosX == nColmFrom && m_nCaretPosY == nLineFrom ){
			nLineNum = nLineTo;
			nIdx = nColmTo;
			goto re_do;
		}

		if( bSelecting ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
			m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */
		}
		else{
			/* 選択範囲の変更 */
			//	2005.06.24 Moca
			SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

			if( bRedraw ){
				/* 選択領域描画 */
				DrawSelectArea();
			}
		}

		/* カーソル移動 */
		//	Sep. 8, 2000 genta
		if ( m_bDrawSWITCH ) AddCurrentLineToHistory();	// 2002.02.16 hor すべて置換のときは不要
		MoveCursor( nColmFrom, nLineFrom, bRedraw );
		m_nCaretPosX_Prev = m_nCaretPosX;
		bFound = TRUE;
	}
	else{
		if( bSelecting ){
			m_bSelectingLock = bSelectingLock_Old;	/* 選択状態のロック */

			/* 選択範囲の変更 */
			m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* 範囲選択開始行(原点) */
			m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* 範囲選択開始桁(原点) */
			m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
			m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* 範囲選択開始桁(原点) */
			m_nSelectLineFrom =	nSelectLineFrom_Old;
			m_nSelectColmFrom = nSelectColFrom_Old;
			m_nSelectLineTo = nLineFrom;
			m_nSelectColmTo = nColmFrom;

			/* カーソル移動 */
			MoveCursor( nColmFrom, nLineFrom, bRedraw );
			m_nCaretPosX_Prev = m_nCaretPosX;

			if( bRedraw ){
				/* 選択領域描画 */
				DrawSelectArea();
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if(m_pShareData->m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// 見つからなかった
			bRedo	&&		// 最初の検索
			m_bDrawSWITCH	// 全て置換の実行中じゃない
		){
			nLineNum=0;
			nIdx=0;
			bRedo=FALSE;
			goto re_do;		// 先頭から再検索
		}
	}

	if(bFound){
		if((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx))
			SendStatusMessage(_T("▼先頭から再検索しました"));
	}
	else{
		ShowEditCaret();	// 2002/04/18 YAZAKI
		DrawCaretPosInfo();	// 2002/04/18 YAZAKI
		SendStatusMessage(_T("▽見つかりませんでした"));
// To Here 2002.01.26 hor

		/* 検索／置換  見つからないときメッセージを表示 */
		if( NULL == pszNotFoundMessage ){
			AlertNotFound(
				hwndParent,
				_T("前方(↓) に文字列 '%s' が１つも見つかりません。"),
				m_szCurSrchKey
			);
		}else{
			AlertNotFound(hwndParent, _T("%s"),pszNotFoundMessage);
		}
	}
}




/** 各種モードの取り消し
*/
void CEditView::Command_CANCEL_MODE( void )
{
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( true );
	}
	m_bSelectingLock = false;	/* 選択状態のロック */
}




/* 範囲選択開始 */
void CEditView::Command_BEGIN_SELECT( void )
{
	if( !IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea();
	}
	
	//	ロックの解除切り替え
	if ( m_bSelectingLock ) {
		m_bSelectingLock = false;	/* 選択状態のロック解除 */
	}
	else {
		m_bSelectingLock = true;	/* 選択状態のロック */
	}
	return;
}




/* 矩形範囲選択開始 */
void CEditView::Command_BEGIN_BOXSELECT( void )
{
	if( !m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		return;
	}

//@@@ 2002.01.03 YAZAKI 範囲選択中にShift+F6を実行すると選択範囲がクリアされない問題に対処
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( true );
	}

	/* 現在のカーソル位置から選択を開始する */
	BeginSelectArea();

	m_bSelectingLock = true;	/* 選択状態のロック */
	m_bBeginBoxSelect = TRUE;	/* 矩形範囲選択中 */
	return;
}




/* 新規作成 */
void CEditView::Command_FILENEW( void )
{
	/* 新たな編集ウィンドウを起動 */
	CControlTray::OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, CODE_NONE, FALSE, false, NULL, false );
	return;
}

/* 新規作成（新しいウインドウで開く） */
void CEditView::Command_FILENEW_NEWWINDOW( void )
{
	/* 新たな編集ウィンドウを起動 */
	CControlTray::OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, false,
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
void CEditView::Command_FILEOPEN( const char* filename, ECodeType nCharCode, bool bReadOnly )
{
	m_pcEditDoc->OpenFile( filename, nCharCode, bReadOnly );
}




/* 閉じて(無題) */	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
void CEditView::Command_FILECLOSE( void )
{
	m_pcEditDoc->FileClose();
}




/*! @brief 閉じて開く

	@date 2003.03.30 genta 開くダイアログでキャンセルしたとき元のファイルが残るように。
				ついでにFILEOPENと同じように引数を追加しておく
*/
void CEditView::Command_FILECLOSE_OPEN( const char *filename, ECodeType nCharCode, bool bReadOnly )
{
	m_pcEditDoc->FileCloseOpen( filename, nCharCode, bReadOnly );
}




/*! 上書き保存

	F_FILESAVEALLとの組み合わせのみで使われるコマンド．
	@param warnbeep [in] true: 保存不要 or 保存禁止のときに警告を出す
	@param askname	[in] true: ファイル名未設定の時に入力を促す

	@date 2004.02.28 genta 引数warnbeep追加
	@date 2005.01.24 genta 引数askname追加

*/
BOOL CEditView::Command_FILESAVE( bool warnbeep, bool askname )
{
	return 	m_pcEditDoc->FileSave( warnbeep, askname );
}

/* 名前を付けて保存ダイアログ */
BOOL CEditView::Command_FILESAVEAS_DIALOG()
{
	return 	m_pcEditDoc->FileSaveAs_Dialog();
}


/* 名前を付けて保存
	filenameで保存。NULLは厳禁。
*/
BOOL CEditView::Command_FILESAVEAS( const char *filename )
{
	return 	m_pcEditDoc->FileSaveAs( filename );
}

/*!	全て上書き保存

	編集中の全てのウィンドウで上書き保存を行う．
	ただし，上書き保存の指示を出すのみで実行結果の確認は行わない．

	上書き禁止及びファイル名未設定のウィンドウでは何も行わない．

	@date 2005.01.24 genta 新規作成
*/
BOOL CEditView::Command_FILESAVEALL( void )
{
	CShareData::getInstance()->SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG( F_FILESAVE_QUIET, 0 ),
		(LPARAM)0,
		NULL
	);
	return TRUE;
}


/*!	現在編集中のファイル名をクリップボードにコピー
	2002/2/3 aroka
*/
void CEditView::Command_COPYFILENAME( void )
{
	if( m_pcEditDoc->IsValidPath() ){
		/* クリップボードにデータを設定 */
		const char* pszFile = m_pcEditDoc->GetFileName();
		MySetClipboardData( pszFile , lstrlen( pszFile ), false );
	}
	else{
		ErrorBeep();
	}
}




/* 現在編集中のファイルのパス名をクリップボードにコピー */
void CEditView::Command_COPYPATH( void )
{
	if( m_pcEditDoc->IsValidPath() ){
		/* クリップボードにデータを設定 */
		MySetClipboardData( m_pcEditDoc->GetFilePath(), lstrlen( m_pcEditDoc->GetFilePath() ), false );
	}
	else{
		ErrorBeep();
	}
}




//	May 9, 2000 genta
/* 現在編集中のファイルのパス名とカーソル位置をクリップボードにコピー */
void CEditView::Command_COPYTAG( void )
{
	if( m_pcEditDoc->IsValidPath() ){
		char	buf[ MAX_PATH + 20 ];
		int		line, col;

		//	論理行番号を得る
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( m_nCaretPosX, m_nCaretPosY, &col, &line );

		/* クリップボードにデータを設定 */
		wsprintf( buf, "%s (%d,%d): ", m_pcEditDoc->GetFilePath(), line+1, col+1 );
		MySetClipboardData( buf, lstrlen( buf ), false );
	}
	else{
		ErrorBeep();
	}
}

/*! 指定行へジャンプダイアログの表示
	2002.2.2 YAZAKI
*/
void CEditView::Command_JUMP_DIALOG( void )
{
	if( !m_pcEditDoc->m_cDlgJump.DoModal(
		m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc
	) ){
		return;
	}
}



/* 指定行ヘジャンプ */
void CEditView::Command_JUMP( void )
{
	const char*	pLine;
	int			nLineLen;
	int			i;
	int			nMode;
	int			bValidLine;
	int			nCurrentLine;
	int			nCommentBegin;
	int			nBgn;

	if( 0 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		ErrorBeep();
		return;
	}

	/* 行番号 */
	int	nLineNum;
	nLineNum = m_pcEditDoc->m_cDlgJump.m_nLineNum;

	if( !m_pcEditDoc->m_cDlgJump.m_bPLSQL ){	/* PL/SQLソースの有効行か */
		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if( m_pShareData->m_bLineNumIsCRLF ){
			if( 0 >= nLineNum ){
				nLineNum = 1;
			}
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			int		nPosX;
			int		nPosY;
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				0,
				nLineNum - 1,
				&nPosX,
				&nPosY
			);
			nLineNum = nPosY + 1;
		}
		else{
			if( 0 >= nLineNum ){
				nLineNum = 1;
			}
			if( nLineNum > m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
				nLineNum = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			}
		}
		//	Sep. 8, 2000 genta
		AddCurrentLineToHistory();
		//	2006.07.09 genta 選択状態を解除しないように
		MoveCursorSelecting( 0, nLineNum - 1, m_bSelectingLock, _CARETMARGINRATE / 3 );
		return;
	}
	if( 0 >= nLineNum ){
		nLineNum = 1;
	}
	nMode = 0;
	nCurrentLine = m_pcEditDoc->m_cDlgJump.m_nPLSQL_E2 - 1;

	int	nLineCount;
	nLineCount = m_pcEditDoc->m_cDlgJump.m_nPLSQL_E1 - 1;

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( !m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){
		/*
		  カーソル位置変換
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		  →
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		int nPosX,nPosY;
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			0,
			nLineCount,
			(int*)&nPosX,
			(int*)&nPosY
		);
		nLineCount = nPosY;
	}

	for( ; nLineCount <  m_pcEditDoc->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		bValidLine = FALSE;
		nBgn = 0;
		for( i = 0; i < nLineLen; ++i ){
			if( ' ' != pLine[i] &&
				TAB != pLine[i]
			){
				break;
			}
		}
		nBgn = i;
		for( i = nBgn; i < nLineLen; ++i ){
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				bValidLine = TRUE;
				if( '\'' == pLine[i] ){
					if( i > 0 && '\\' == pLine[i - 1] ){
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
				if( '"' == pLine[i] ){
					if( i > 0 && '\\' == pLine[i - 1] ){
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
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
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '-' == pLine[i] &&  '-' == pLine[i + 1] ){
					bValidLine = TRUE;
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					nCommentBegin = nLineCount;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
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
			if( CR == pLine[nBgn] ||
				LF == pLine[nBgn] ){
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
	int		nPosX;
	int		nPosY;
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		0,
		nLineCount,
		&nPosX,
		&nPosY
	);
	//	Sep. 8, 2000 genta
	AddCurrentLineToHistory();
	//	2006.07.09 genta 選択状態を解除しないように
	MoveCursorSelecting( nPosX, nPosY, m_bSelectingLock, _CARETMARGINRATE / 3 );
}




/* フォント設定 */
void CEditView::Command_FONT( void )
{
	HWND	hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	/* フォント設定ダイアログ */
	LOGFONT cLogfont = m_pShareData->m_Common.m_sView.m_lf;
	INT nPointSize;
#ifdef USE_UNFIXED_FONT
	bool bFixedFont = false;
#else
	bool bFixedFont = true;
#endif
	if( MySelectFont( &cLogfont, &nPointSize, m_pcEditDoc->m_hWnd, bFixedFont ) ){
		m_pShareData->m_Common.m_sView.m_lf = cLogfont;
		m_pShareData->m_Common.m_sView.m_nPointSize = nPointSize;

//		/* 変更フラグ フォント */
//		m_pShareData->m_bFontModify = TRUE;

		if( m_pShareData->m_Common.m_sView.m_lf.lfPitchAndFamily & FIXED_PITCH  ){
			m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;	/* 現在のフォントは固定幅フォントである */
		}else{
			m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH = FALSE;	/* 現在のフォントは固定幅フォントである */
		}
		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONT, hwndFrame
		);

		/* キャレットの表示 */
//		::HideCaret( m_hWnd );
//		::ShowCaret( m_hWnd );

//		/* アクティブにする */
//		/* アクティブにする */
//		ActivateFrameWindow( hwndFrame );
	}
	return;
}

/*! フォントサイズ変更
	@param fontSize フォントサイズ（1/10ポイント単位）
	@param shift フォントサイズを拡大or縮小するための変更量(fontSize=0のとき有効)

	@note TrueTypeのみサポート

	@date 2013.04.10 novice 新規作成
*/
void CEditView::Command_SETFONTSIZE( int fontSize, int shift )
{
	// The point sizes recommended by "The Windows Interface: An Application Design Guide", 1/10ポイント単位
	static const INT sizeTable[] = { 8*10, 9*10, 10*10, (INT)(10.5*10), 11*10, 12*10, 14*10, 16*10, 18*10, 20*10, 22*10, 24*10, 26*10, 28*10, 36*10, 48*10, 72*10 };
	LOGFONT& lf = m_pShareData->m_Common.m_sView.m_lf;
	INT nPointSize;

	// TrueTypeのみ対応
	if( OUT_STROKE_PRECIS != lf.lfOutPrecision) {
		return;
	}

	if( 0 != fontSize ){
		// フォントサイズを直接選択する場合
		nPointSize = t_max(sizeTable[0], t_min(sizeTable[_countof(sizeTable)-1], fontSize));

		// 新しいフォントサイズ設定
		lf.lfHeight = DpiPointsToPixels(-nPointSize, 10);
		m_pShareData->m_Common.m_sView.m_nPointSize = nPointSize;
	} else if( 0 != shift ) {
		// 現在のフォントに対して、縮小or拡大したフォント選択する場合
		nPointSize = m_pShareData->m_Common.m_sView.m_nPointSize;

		// フォントの拡大or縮小するためのサイズ検索
		int i;
		for( i = 0; i < _countof(sizeTable); i++) {
			if( nPointSize <= sizeTable[i] ){
				int index = t_max(0, t_min((int)_countof(sizeTable) - 1, (int)(i + shift)));
				nPointSize = sizeTable[index];
				break;
			}
		}

		// 新しいフォントサイズ設定
		lf.lfHeight = DpiPointsToPixels(-nPointSize, 10);
		m_pShareData->m_Common.m_sView.m_nPointSize = nPointSize;
	} else {
		// フォントサイズが変わらないので終了
		return;
	}

	HWND	hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	/* 設定変更を反映させる */
	/* 全編集ウィンドウへメッセージをポストする */
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_CHANGESETTING,
		(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONTSIZE, hwndFrame
	);
}

/* 共通設定 */
void CEditView::Command_OPTION( void )
{
	/* 設定プロパティシート テスト用 */
	m_pcEditDoc->OpenPropertySheet( -1/*, -1*/ );
}




/* タイプ別設定 */
void CEditView::Command_OPTION_TYPE( void )
{
	m_pcEditDoc->OpenPropertySheetTypes( -1, m_pcEditDoc->GetDocumentType() );
}




/* タイプ別設定一覧 */
void CEditView::Command_TYPE_LIST( void )
{
	CDlgTypeList	cDlgTypeList;
	int				nSettingType;
	nSettingType = m_pcEditDoc->GetDocumentType();
	if( cDlgTypeList.DoModal( m_hInstance, m_hWnd, &nSettingType ) ){
		//	Nov. 29, 2000 genta
		//	一時的な設定適用機能を無理矢理追加
		if( nSettingType & PROP_TEMPCHANGE_FLAG ){
			m_pcEditDoc->SetDocumentType( nSettingType & ~PROP_TEMPCHANGE_FLAG, true );
			m_pcEditDoc->LockDocumentType();
			m_pcEditDoc->OnChangeType();
		}
		else{
			/* タイプ別設定 */
			m_pcEditDoc->OpenPropertySheetTypes( -1, nSettingType );
		}
	}
	return;
}




/* 行の二重化(折り返し単位) */
void CEditView::Command_DUPLICATELINE( void )
{
	COpe*			pcOpe = NULL;
	int				nNewLine;
	int				nNewPos;
	int				bCRLF;
	int				bAddCRLF;
	CMemory			cmemBuf;
	const CLayout*	pcLayout;

	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( true );
	}

	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;									/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	int nCaretPosXOld = m_nCaretPosX;
	int	nCaretPosYOld = m_nCaretPosY + 1;

	//行頭に移動(折り返し単位)
	Command_GOLINETOP( m_bSelectingLock, 0x1 /* カーソル位置に関係なく行頭に移動 */ );

	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;									/* 操作種別 */
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */
		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}



	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作前のキャレット位置Ｙ */
	}

	/* 二重化したい行を調べる
	||	・改行で終わっている
	||	・改行で終わっていない
	||	・最終行である
	||	→折り返しでない
	||	・最終行でない
	||	→折り返しである
	*/
	bCRLF = ( EOL_NONE == pcLayout->m_cEol ) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if( !bCRLF ){
		if( m_nCaretPosY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen() );	//	※pcLayout->GetLengthWithEOL()は、EOLの長さを必ず1にするので使えない。
	if( bAddCRLF ){
		/* 現在、Enterなどで挿入する改行コードの種類を取得 */
		CEol cWork = m_pcEditDoc->GetNewLineCode();
		cmemBuf.AppendString( cWork.GetValue(), cWork.GetLen() );
	}

	/* 現在位置にデータを挿入 */
	InsertData_CEditView(
		m_nCaretPosX,
		m_nCaretPosY,
		(char*)cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		&nNewLine,
		&nNewPos,
		pcOpe,
		true
	);

	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */

		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			nNewPos,
			nNewLine,
			&pcOpe->m_ptCaretPos_PHY_After.x,
			&pcOpe->m_ptCaretPos_PHY_After.y
		);

		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	/* カーソルを移動 */
	MoveCursor( nCaretPosXOld, nCaretPosYOld, true );
	m_nCaretPosX_Prev = m_nCaretPosX;


	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */

		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */

		pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	return;
}




/* 小文字 */
void CEditView::Command_TOLOWER( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOLOWER );
	return;
}




/* 大文字 */
void CEditView::Command_TOUPPER( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOUPPER );
	return;
}




/* 全角→半角 */
void CEditView::Command_TOHANKAKU( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOHANKAKU );
	return;
}


/* 全角カタカナ→半角カタカナ */		//Aug. 29, 2002 ai
void CEditView::Command_TOHANKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOHANKATA );
	return;
}


/*! 半角英数→全角英数 */			//July. 30, 2001 Misaka
void CEditView::Command_TOZENEI( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOZENEI );
	return;
}

/*! 全角英数→半角英数 */
void CEditView::Command_TOHANEI( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOHANEI );
	return;
}


/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
void CEditView::Command_TOZENKAKUKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOZENKAKUKATA );
	return;
}




/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
void CEditView::Command_TOZENKAKUHIRA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TOZENKAKUHIRA );
	return;
}




/* 半角カタカナ→全角カタカナ */
void CEditView::Command_HANKATATOZENKAKUKATA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_HANKATATOZENKATA );
	return;
}




/* 半角カタカナ→全角ひらがな */
void CEditView::Command_HANKATATOZENKAKUHIRA( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_HANKATATOZENHIRA );
	return;
}




/* TAB→空白 */
void CEditView::Command_TABTOSPACE( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_TABTOSPACE );
	return;
}

/* 空白→TAB */ //#### Stonee, 2001/05/27
void CEditView::Command_SPACETOTAB( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_SPACETOTAB );
	return;
}



//#define F_HANKATATOZENKATA	30557	/* 半角カタカナ→全角カタカナ */
//#define F_HANKATATOZENHIRA	30558	/* 半角カタカナ→全角ひらがな */




/* E-Mail(JIS→SJIS)コード変換 */
void CEditView::Command_CODECNV_EMAIL( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_EMAIL );
	return;
}




/* EUC→SJISコード変換 */
void CEditView::Command_CODECNV_EUC2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_EUC2SJIS );
	return;
}




/* Unicode→SJISコード変換 */
void CEditView::Command_CODECNV_UNICODE2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_UNICODE2SJIS );
	return;
}




/* UnicodeBE→SJISコード変換 */
void CEditView::Command_CODECNV_UNICODEBE2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_UNICODEBE2SJIS );
	return;
}




/* SJIS→JISコード変換 */
void CEditView::Command_CODECNV_SJIS2JIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_SJIS2JIS );
	return;
}




/* SJIS→EUCコード変換 */
void CEditView::Command_CODECNV_SJIS2EUC( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_SJIS2EUC );
	return;
}




/* UTF-8→SJISコード変換 */
void CEditView::Command_CODECNV_UTF82SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_UTF82SJIS );
	return;
}




/* UTF-7→SJISコード変換 */
void CEditView::Command_CODECNV_UTF72SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_UTF72SJIS );
	return;
}




/* SJIS→UTF-7コード変換 */
void CEditView::Command_CODECNV_SJIS2UTF7( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_SJIS2UTF7 );
	return;
}




/* SJIS→UTF-8コード変換 */
void CEditView::Command_CODECNV_SJIS2UTF8( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_SJIS2UTF8 );
	return;
}




/* 自動判別→SJISコード変換 */
void CEditView::Command_CODECNV_AUTO2SJIS( void )
{
	/* 選択エリアのテキストを指定方法で変換 */
	ConvSelectedArea( F_CODECNV_AUTO2SJIS );
	return;
}




/*!	アウトライン解析
	
	2002/3/13 YAZAKI nOutlineTypeとnListTypeを統合。
*/
// トグル用のフラグに変更 20060201 aroka
BOOL CEditView::Command_FUNCLIST(
	int nAction,
	int nOutlineType
)
{
//	if( bCheckOnly ){
//		return TRUE;
//	}

	static CFuncInfoArr	cFuncInfoArr;
//	int		nLine;
//	int		nListType;

	//	2001.12.03 hor & 2002.3.13 YAZAKI
	if( nOutlineType == OUTLINE_DEFAULT ){
		/* タイプ別に設定されたアウトライン解析方法 */
		nOutlineType = m_pcEditDoc->GetDocumentAttribute().m_nDefaultOutline;
		if( nOutlineType == OUTLINE_CPP ){
			if( CheckEXT( m_pcEditDoc->GetFilePath(), _T("c") ) ){
				nOutlineType = OUTLINE_C;	/* これでC関数一覧リストビューになる */
			}
		}
	}

	if( NULL != m_pcEditDoc->m_cDlgFuncList.m_hWnd && nAction != SHOW_RELOAD ){
		switch(nAction ){
		case SHOW_NORMAL: // アクティブにする
			//	開いているものと種別が同じならActiveにするだけ．異なれば再解析
			if( m_pcEditDoc->m_cDlgFuncList.CheckListType( nOutlineType )){
				ActivateFrameWindow( m_pcEditDoc->m_cDlgFuncList.m_hWnd );
				return TRUE;
			}
			break;
		case SHOW_TOGGLE: // 閉じる
			//	開いているものと種別が同じなら閉じる．異なれば再解析
			if( m_pcEditDoc->m_cDlgFuncList.CheckListType( nOutlineType )){
				::SendMessage( m_pcEditDoc->m_cDlgFuncList.m_hWnd, WM_CLOSE, 0, 0 );
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
	case OUTLINE_C:			// C/C++ は MakeFuncList_C
	case OUTLINE_CPP:		m_pcEditDoc->MakeFuncList_C( &cFuncInfoArr );break;
	case OUTLINE_PLSQL:		m_pcEditDoc->MakeFuncList_PLSQL( &cFuncInfoArr );break;
	case OUTLINE_JAVA:		m_pcEditDoc->MakeFuncList_Java( &cFuncInfoArr );break;
	case OUTLINE_COBOL:		m_pcEditDoc->MakeTopicList_cobol( &cFuncInfoArr );break;
	case OUTLINE_ASM:		m_pcEditDoc->MakeTopicList_asm( &cFuncInfoArr );break;
	case OUTLINE_PERL:		m_pcEditDoc->MakeFuncList_Perl( &cFuncInfoArr );break;	//	Sep. 8, 2000 genta
	case OUTLINE_VB:		m_pcEditDoc->MakeFuncList_VisualBasic( &cFuncInfoArr );break;	//	June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		m_pcEditDoc->MakeTopicList_wztxt(&cFuncInfoArr);break;		// 2003.05.20 zenryaku 階層付テキスト アウトライン解析
	case OUTLINE_HTML:		m_pcEditDoc->MakeTopicList_html(&cFuncInfoArr);break;		// 2003.05.20 zenryaku HTML アウトライン解析
	case OUTLINE_TEX:		m_pcEditDoc->MakeTopicList_tex(&cFuncInfoArr);break;		// 2003.07.20 naoh TeX アウトライン解析
	case OUTLINE_BOOKMARK:	m_pcEditDoc->MakeFuncList_BookMark( &cFuncInfoArr );break;	//	2001.12.03 hor
	case OUTLINE_FILE:		m_pcEditDoc->MakeFuncList_RuleFile( &cFuncInfoArr );break;	//	2002.04.01 YAZAKI アウトライン解析にルールファイルを導入
//	case OUTLINE_UNKNOWN:	//Jul. 08, 2001 JEPRO 使わないように変更
	case OUTLINE_PYTHON:	m_pcEditDoc->MakeFuncList_python(&cFuncInfoArr);break;		// 2007.02.08 genta
	case OUTLINE_ERLANG:	m_pcEditDoc->MakeFuncList_Erlang(&cFuncInfoArr);break;		// 2009.08.11 genta
	case OUTLINE_TEXT:
		//	fall though
		//	ここには何も入れてはいけない 2007.02.28 genta 注意書き
	default:
		m_pcEditDoc->MakeTopicList_txt( &cFuncInfoArr );
		break;
	}

	/* 解析対象ファイル名 */
	_tcscpy( cFuncInfoArr.m_szFilePath, m_pcEditDoc->GetFilePath() );

	/* アウトライン ダイアログの表示 */
	if( NULL == m_pcEditDoc->m_cDlgFuncList.m_hWnd ){
		m_pcEditDoc->m_cDlgFuncList.DoModeless(
			m_hInstance,
			/*m_pcEditDoc->*/m_hWnd,
			(LPARAM)this,
			&cFuncInfoArr,
			m_nCaretPosY + 1,
			m_nCaretPosX + 1,
			nOutlineType,
			m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		);
	}else{
		/* アクティブにする */
		m_pcEditDoc->m_cDlgFuncList.Redraw( nOutlineType, &cFuncInfoArr, m_nCaretPosY + 1, m_nCaretPosX + 1 );
		ActivateFrameWindow( m_pcEditDoc->m_cDlgFuncList.m_hWnd );
	}

	return TRUE;
}




/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
void CEditView::Command_SPLIT_V( void )
{
	m_pcEditDoc->m_cSplitterWnd.VSplitOnOff();
	return;
}




/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
void CEditView::Command_SPLIT_H( void )
{
	m_pcEditDoc->m_cSplitterWnd.HSplitOnOff();
	return;
}




/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
void CEditView::Command_SPLIT_VH( void )
{
	m_pcEditDoc->m_cSplitterWnd.VHSplitOnOff();
	return;
}




//From Here Nov. 25, 2000 JEPRO
/* ヘルプ目次 */
void CEditView::Command_HELP_CONTENTS( void )
{
	ShowWinHelpContents( m_hWnd );	//	目次を表示する
	return;
}




/* ヘルプキーワード検索 */
void CEditView::Command_HELP_SEARCH( void )
{
	MyWinHelp( m_hWnd, HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return;
}
//To Here Nov. 25, 2000




/*! キャレット位置の単語を辞書検索ON-OFF

	@date 2006.03.24 fon 新規作成
*/
void CEditView::Command_ToggleKeySearch( void )
{	/* 共通設定ダイアログの設定をキー割り当てでも切り替えられるように */
	if(TRUE == m_pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord ){
		m_pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord = FALSE;
	}else{
		m_pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord = TRUE;
	}
}




/* コマンド一覧 */
void CEditView::Command_MENU_ALLFUNC( void )
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
	::ClientToScreen( m_hWnd, &po );

	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更したことで1番(カーソル移動系)が前にシフトされた(この変更によって i=1→i=0 と変更)
	//	Oct. 3, 2001 genta
	for( i = 0; i < FuncLookup.GetCategoryCount(); i++ ){
		hMenuPopUp = ::CreatePopupMenu();
		for( j = 0; j < FuncLookup.GetItemCount(i); j++ ){
			//	Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode( i, j, false );	// 2007.11.02 ryoji 未登録マクロ非表示を明示指定
			if( code != 0 ){
				char	szLabel[300];
				FuncLookup.Pos2FuncName( i, j, szLabel, 256 );
				uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				//	Oct. 3, 2001 genta
				pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenuPopUp, uFlags, code, szLabel, _T("") );
			}
		}
		//	Oct. 3, 2001 genta
		pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , FuncLookup.Category2Name(i), _T("") );
//		pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i] );
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
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
	::DestroyMenu( hMenu );
	if( 0 != nId ){
		/* コマンドコードによる処理振り分け */
//		HandleCommand( nFuncID, true, 0, 0, 0, 0 );
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nId, 0 ), (LPARAM)NULL );
	}
	return;
}




/* 外部ヘルプ１
	@date 2012.09.26 Moca HTMLHELP対応
*/
void CEditView::Command_EXTHELP1( void )
{
retry:;
	if( CShareData::getInstance()->ExtWinHelpIsSet( m_pcEditDoc->GetDocumentType() ) == false){
//	if( 0 == strlen( m_pShareData->m_Common.m_szExtHelp1 ) ){
		ErrorBeep();
//From Here Sept. 15, 2000 JEPRO
//		[Esc]キーと[x]ボタンでも中止できるように変更
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
//To Here Sept. 15, 2000
			_T("外部ヘルプ１が設定されていません。\n今すぐ設定しますか?")
		) ){
			/* 共通設定 プロパティシート */
			if( !m_pcEditDoc->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHELP1*/ ) ){
				return;
			}
			goto retry;
		}
		//	Jun. 15, 2000 genta
		else{
			return;
		}
	}

	CMemory		cmemCurText;
	const TCHAR*	helpfile = CShareData::getInstance()->GetExtWinHelp( m_pcEditDoc->GetDocumentType() );

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	GetCurrentTextForSearch( cmemCurText );
	TCHAR path[_MAX_PATH];
	if( _IS_REL_PATH( helpfile ) ){
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		GetInidirOrExedir( path, helpfile );
	}else{
		_tcscpy( path, helpfile );
	}
	// 2012.09.26 Moca HTMLHELP対応
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath( path, NULL, NULL, NULL, szExt );
	if( 0 == _tcsicmp(szExt, _T(".chi")) || 0 == _tcsicmp(szExt, _T(".chm")) || 0 == _tcsicmp(szExt, _T(".col")) ){
		std::tstring patht = path;
		Command_EXTHTMLHELP( patht.c_str(), cmemCurText.GetStringPtr() );
	}else{
		::WinHelp( m_hwndParent, path, HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr() );
	}
	return;
}



/*!
	外部HTMLヘルプ
	
	@param helpfile [in] HTMLヘルプファイル名．NULLのときはタイプ別に設定されたファイル．
	@param kwd [in] 検索キーワード．NULLのときはカーソル位置or選択されたワード
	@date 2002.07.05 genta 任意のファイル・キーワードの指定ができるよう引数追加
*/
void CEditView::Command_EXTHTMLHELP( const char* helpfile, const char* kwd )
{
	HWND		hwndHtmlHelp;
	CMemory		cmemCurText;

	int			nLen;

	//	From Here Jul. 5, 2002 genta
	const TCHAR *filename = NULL;
	if ( helpfile == NULL || helpfile[0] == _T('\0') ){
		while( !CShareData::getInstance()->ExtHTMLHelpIsSet( m_pcEditDoc->GetDocumentType()) ){
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
			if( !m_pcEditDoc->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHTMLHELP*/ ) ){
				return;
			}
		}
		filename = CShareData::getInstance()->GetExtHTMLHelp( m_pcEditDoc->GetDocumentType() );
	}
	else {
		filename = helpfile;
	}
	//	To Here Jul. 5, 2002 genta

	//	Jul. 5, 2002 genta
	//	キーワードの外部指定を可能に
	if( kwd != NULL && kwd[0] != _T('\0') ){
		cmemCurText.SetString( kwd );
	}
	else {
		/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
		GetCurrentTextForSearch( cmemCurText );
	}

	/* HtmlHelpビューアはひとつ */
	if( CShareData::getInstance()->HTMLHelpIsSingle( m_pcEditDoc->GetDocumentType() ) ){
		// タスクトレイのプロセスにHtmlHelpを起動させる
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( filename ) ){
			GetInidirOrExedir( m_pShareData->m_sWorkBuffer.m_szWork, filename );
		}else{
			_tcscpy( m_pShareData->m_sWorkBuffer.m_szWork, filename ); //	Jul. 5, 2002 genta
		}
		nLen = lstrlen( m_pShareData->m_sWorkBuffer.m_szWork );
		_tcscpy( &m_pShareData->m_sWorkBuffer.m_szWork[nLen + 1], cmemCurText.GetStringPtr() );
		hwndHtmlHelp = (HWND)::SendMessage(
			m_pShareData->m_sHandles.m_hwndTray,
			MYWM_HTMLHELP,
			(WPARAM)::GetParent( m_hwndParent ),
			0
		);
	}
	else{
		/* 自分でHtmlHelpを起動させる */
		HH_AKLINK	link;
		link.cbStruct = sizeof( link ) ;
		link.fReserved = FALSE ;
		link.pszKeywords = (char*)cmemCurText.GetStringPtr();
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
				NULL/*m_pShareData->m_hwndTray*/,
				path, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else{
			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*m_pShareData->m_hwndTray*/,
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
void CEditView::Command_ABOUT( void )
{
	CDlgAbout cDlgAbout;
	cDlgAbout.DoModal( m_hInstance, m_hWnd );
	return;
}
//To Here Dec. 25, 2000




/* 右クリックメニュー */
void CEditView::Command_MENU_RBUTTON( void )
{
	int			nId;
	int			nLength;
//	HGLOBAL		hgClip;
//	char*		pszClip;
	int			i;
	/* ポップアップメニュー(右クリック) */
	nId = CreatePopUpMenu_R();
	if( 0 == nId ){
		return;
	}
	switch( nId ){
	case IDM_COPYDICINFO:
		const TCHAR*	pszStr;
		pszStr = m_cTipWnd.m_cInfo.GetStringPtr( &nLength );

		TCHAR*		pszWork;
		pszWork = (TCHAR*)malloc( nLength + 1);
		memcpy( pszWork, pszStr, nLength );
		pszWork[nLength] = _T('\0');

		// 見た目と同じように、\n を CR+LFへ変換する
		for( i = 0; i < nLength ; ++i){
			if( pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')){
				pszWork[i] = '\x0d' ;
				pszWork[i + 1] = '\x0a' ;
			}
		}
		/* クリップボードにデータを設定 */
		MySetClipboardData( pszWork, nLength, false );
		free( pszWork );

		break;

	case IDM_JUMPDICT:
		/* キーワード辞書ファイルを開く */
		if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp){		/* キーワード辞書セレクトを使用する */	// 2006.04.10 fon
			//	Feb. 17, 2007 genta 相対パスを実行ファイル基準で開くように
			TagJumpSub(
				m_pShareData->m_Types[m_pcEditDoc->GetDocumentType()].m_KeyHelpArr[m_cTipWnd.m_nSearchDict].m_szPath,
				m_cTipWnd.m_nSearchLine,
				1,
				0,
				true
			);
		}
		break;

	default:
		/* コマンドコードによる処理振り分け */
//		HandleCommand( nId, true, 0, 0, 0, 0 );
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nId, 0 ),  (LPARAM)NULL );
		break;
	}
	return;
}



// From Here 2001.12.03 hor
/* インデント ver2 */
void CEditView::Command_INDENT_TAB( void )
{
	if(!m_pcEditDoc->GetDocumentAttribute().m_bInsSpace){
		if(IsTextSelected() && m_nSelectLineFrom!=m_nSelectLineTo){
			Command_INDENT( TAB );
		}else{
			Command_CHAR( (char)TAB );
		}
		return;
	}
	if(IsTextSelected() && m_bBeginBoxSelect && m_nSelectColmFrom==m_nSelectColmTo){
		Command_INDENT( TAB );
		return;
	}
	int		nCol	=	0;
	//	Sep. 23, 2002 genta LayoutMgrの値を使う
	int		nTab	=	m_pcEditDoc->m_cLayoutMgr.GetTabSpace();
	int		nSpace	=	0;

	//インデント開始位置の取得
	if ( IsTextSelected() ) {
		nCol = (m_nSelectColmFrom<m_nSelectColmTo)?m_nSelectColmFrom:m_nSelectColmTo;
	}else{
		nCol = m_nCaretPosX;
	}

	//インデント文字数取得
	nSpace = nTab-(nCol%nTab);
	if (nSpace==0) nSpace = nTab;

	//TAB幅分だけスペースインデント
	if (IsTextSelected() && m_nSelectLineFrom == m_nSelectLineTo ) {
		ReplaceData_CEditView(
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo,		/* 範囲選択終了桁 */
			NULL,					/* 削除されたデータのコピー(NULL可能) */
			// Sep. 22, 2002 genta TABの最大幅を64に拡張
			"                                                                ",				/* 挿入するデータ */
			nSpace,					/* 挿入するデータの長さ */
			true
		);
		return;
	}
	// Sep. 22, 2002 genta TABの最大幅を64に拡張
	Command_INDENT( "                                                                " , nSpace , TRUE);

}
// To Here 2001.12.03 hor

/* インデント ver1 */
void CEditView::Command_INDENT( char cChar )
{
	// From Here 2001.12.03 hor
	/* SPACEorTABインンデントで矩形選択桁がゼロの時は選択範囲を最大にする */
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if((cChar==SPACE || cChar==TAB) && m_bBeginBoxSelect && m_nSelectColmFrom==m_nSelectColmTo ){
		m_nSelectColmTo=m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
		RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
	char szWork[2];
	wsprintf( szWork, "%c", cChar );
	Command_INDENT( szWork, lstrlen( szWork ) );
	return;
}




/* インデント ver0 */
void CEditView::Command_INDENT( const char* pData, int nDataLen , BOOL bIndent )
{
	int			nSelectLineFromOld;	/* 範囲選択開始行 */
	int			nSelectColFromOld; 	/* 範囲選択開始桁 */
	int			nSelectLineToOld;	/* 範囲選択終了行 */
	int			nSelectColToOld;	/* 範囲選択終了桁 */
	CMemory		cMem;
	CWaitCursor cWaitCursor( m_hWnd );
	COpe*		pcOpe = NULL;
	int			nNewLine;			/* 挿入された部分の次の位置の行 */
	int			nNewPos;			/* 挿入された部分の次の位置のデータ位置 */
	int			i;
	CMemory		cmemBuf;
	RECT		rcSel;
	int			nPosX;
	int			nPosY;
	int			nIdxFrom;
	int			nIdxTo;
	int			nLineNum;
	int			nDelPos;
	int			nDelLen;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	if( !IsTextSelected() ){			/* テキストが選択されているか */
//		/* 1バイト文字入力 */
		char*	pszWork;
		pszWork = new char[nDataLen + 1];
		memcpy( pszWork, pData, nDataLen );
		pszWork[nDataLen] = '\0';
		// テキストを貼り付け 2004.05.14 Moca 長さを引数で与える
		Command_INSTEXT( true, pszWork, nDataLen, FALSE );
		delete [] pszWork;
		return;
	}
	m_bDrawSWITCH = false;	// 2002.01.25 hor
	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){
		// From Here 2001.12.03 hor
		/* 上書モードのときは選択範囲削除 */
		if( ! IsInsMode() /* Oct. 2, 2005 genta */){
			nSelectLineFromOld	= m_nSelectLineFrom;
			nSelectColFromOld	= m_nSelectColmFrom;
			nSelectLineToOld	= m_nSelectLineTo;
			nSelectColToOld		= m_nSelectColmTo;
			DeleteData( false );
			m_nSelectLineFrom	= nSelectLineFromOld;
			m_nSelectColmFrom	= nSelectColFromOld;
			m_nSelectLineTo		= nSelectLineToOld;
			m_nSelectColmTo		= nSelectColToOld;
			m_bBeginBoxSelect	= TRUE;
		}
		// To Here 2001.12.03 hor

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		// 範囲選択開始行
			m_nSelectColmFrom,		// 範囲選択開始桁
			m_nSelectLineTo,		// 範囲選択終了行
			m_nSelectColmTo			// 範囲選択終了桁
		);
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( false/*true 2002.01.25 hor*/ );

		// From Here 2001.12.03 hor
		/* インデント時は空白行に書込まない */
		if( 1 == nDataLen && ( SPACE == pData[0] || TAB == pData[0] ) ){
			bIndent=TRUE;
		}
		// To Here 2001.12.03 hor

		for( nLineNum = rcSel.top; nLineNum < rcSel.bottom + 1; nLineNum++ ){
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
			//	Nov. 6, 2002 genta NULLチェック追加
			//	これがないとEOF行を含む矩形選択中の文字列入力で落ちる
			const char* pLine;
			if( pcLayout != NULL && NULL != (pLine = pcLayout->GetPtr()) ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = LineColmnToIndex( pcLayout, rcSel.left );
				nIdxTo = LineColmnToIndex( pcLayout, rcSel.right );

				for( i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == CR || pLine[i] == LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom = 0;
				nIdxTo = 0;
			}
			nDelPos = nIdxFrom;
			nDelLen = nIdxTo - nIdxFrom;

			/* TABやスペースインデントの時 */
			if( bIndent && 0 == nDelLen ) {
				continue;
			}

			//	Nov. 6, 2002 genta pcLayoutがNULLの場合を考慮
			nPosX = ( pcLayout == NULL ? 0 : LineIndexToColmn( pcLayout, nDelPos ));
			nPosY = nLineNum;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					nPosX,
					nPosY,
					&pcOpe->m_ptCaretPos_PHY_Before.x,
					&pcOpe->m_ptCaretPos_PHY_Before.y
				);
			}
			/* 現在位置にデータを挿入 */
			InsertData_CEditView(
				rcSel.left/*nPosX*/,
				nPosY,
				pData,		// cmemBuf.GetPtr(),	// 2001.12.03 hor
				nDataLen,	// cmemBuf.GetStringLength(),		// 2001.12.03 hor
				&nNewLine,
				&nNewPos,
				pcOpe,
				false
			);
			MoveCursor( nNewPos, nNewLine, false );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		//	nextline:;	// 2001.12.03 hor
		}
		/* 挿入データの先頭位置へカーソルを移動 */
		MoveCursor( rcSel.left, rcSel.top, false );

		/* 挿入文字列の情報 */
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)pData, nDataLen,
			&pnKey_CharCharsArr
		);
		for( i = 0; i < nDataLen; ){
			/* カーソル右移動 */
			Command_RIGHT( false, true, false );
			i+= pnKey_CharCharsArr[i];
		}
		if( NULL != pnKey_CharCharsArr ){
			delete [] pnKey_CharCharsArr;
		}
	// From Here 2001.12.03 hor 
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		rcSel.right += m_nCaretPosX-rcSel.left;
		if( rcSel.right>m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
			rcSel.right=m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
		}
	// To Here 2001.12.03 hor
		rcSel.left = m_nCaretPosX;

		/* カーソルを移動 */
		MoveCursor( rcSel.left, rcSel.top, true );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */

			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		m_nSelectLineFrom = rcSel.top;			/* 範囲選択開始行 */
		m_nSelectColmFrom = rcSel.left; 		/* 範囲選択開始桁 */
		m_nSelectLineTo = rcSel.bottom;			/* 範囲選択終了行 */
	//	m_nSelectColmTo = nNewPos;				/* 範囲選択終了桁 */	// 2001.12.03 hor
		m_nSelectColmTo = rcSel.right;			/* 範囲選択終了桁 */	// 2001.12.03 hor
		m_bBeginBoxSelect = TRUE;
	}else{
		nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
		nSelectColFromOld = 0;					/* 範囲選択開始桁 */
		nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
		if( m_nSelectColmTo > 0 ){
			++nSelectLineToOld;					/* 範囲選択終了行 */
		}
		nSelectColToOld = 0;					/* 範囲選択終了桁 */

		// 現在の選択範囲を非選択状態に戻す
		DisableSelectArea( false );

		for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
			int nLineCountPrev = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout ||						//	テキストが無いEOLの行は無視
				pcLayout->m_nOffset > 0 ||				//	折り返し行は無視
				pcLayout->GetLengthWithoutEOL() == 0 ){	//	改行のみの行は無視する。
				continue;
			}

			/* カーソルを移動 */
			MoveCursor( 0, i, false );
			m_nCaretPosX_Prev = m_nCaretPosX;

			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					0,
					i,
					&pcOpe->m_ptCaretPos_PHY_Before.x,
					&pcOpe->m_ptCaretPos_PHY_Before.y
				);
			}
			/* 現在位置にデータを挿入 */
			InsertData_CEditView(
				0,
				i,
				pData,		//	cmemBuf.GetPtr(),	// 2001.12.03 hor
				nDataLen,	//	cmemBuf.GetStringLength(),	// 2001.12.03 hor
				&nNewLine,
				&nNewPos,
				pcOpe,
				false
			);
			/* カーソルを移動 */
			MoveCursor( nNewPos, nNewLine, false );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
				pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}

			if ( nLineCountPrev != m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
				//	行数が変化した!!
				nSelectLineToOld += m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}

		m_nSelectLineFrom = nSelectLineFromOld;	/* 範囲選択開始行 */
		m_nSelectColmFrom = nSelectColFromOld; 	/* 範囲選択開始桁 */
		m_nSelectLineTo = nSelectLineToOld;		/* 範囲選択終了行 */
		m_nSelectColmTo = nSelectColToOld;		/* 範囲選択終了桁 */
		// From Here 2001.12.03 hor
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */
			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		// To Here 2001.12.03 hor
	}
	/* 再描画 */
	m_bDrawSWITCH = true;	// 2002.01.25 hor
	RedrawAll();		// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}




/* 逆インデント */
void CEditView::Command_UNINDENT( char cChar )
{
	//	Aug. 9, 2003 genta
	//	選択されていない場合に逆インデントした場合に
	//	注意メッセージを出す
	if( !IsTextSelected() ){	/* テキストが選択されているか */
		/* １バイト文字入力 */
		Command_CHAR( cChar );	//	2003.10.09 zenryaku警告を出すが，動作は以前のままにする 
		SendStatusMessage(_T("★逆インデントは選択時のみ"));
		return;
	}

	int			nSelectLineFromOld;	// 範囲選択開始行
	int			nSelectColFromOld; 	// 範囲選択開始桁
	int			nSelectLineToOld;	// 範囲選択終了行
	int			nSelectColToOld;	// 範囲選択終了桁
	CMemory*	pcMemDeleted;
	CMemory		cMem;
	CWaitCursor cWaitCursor( m_hWnd );
	COpe*		pcOpe = NULL;
	int			i;
	CMemory		cmemBuf;

	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){
		ErrorBeep();
//**********************************************
//	 箱型逆インデントについては、保留とする (1998.10.22)
//**********************************************
	}
	else{
		m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

		nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
		nSelectColFromOld = 0;					/* 範囲選択開始桁 */
		nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
		if( m_nSelectColmTo > 0 ){
			nSelectLineToOld++;					/* 範囲選択終了行 */
		}
		nSelectColToOld = 0;					/* 範囲選択終了桁 */

		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( false );

		int				nDelLen;
		for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
			int nLineCountPrev = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			const CLayout*	pcLayout;
			int				nLineLen;
			const char*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( i, &nLineLen, &pcLayout );
			if( NULL == pcLayout || pcLayout->m_nOffset > 0 ){
				continue;
			}
			if( TAB == cChar ){
				if( pLine[0] == cChar ){
					nDelLen = 1;
				}else{

					int i;
					for( i = 0; i < nLineLen; i++ ){
						if( SPACE != pLine[i] ){
							break;
						}
						//	Sep. 23, 2002 genta LayoutMgrの値を使う
						if( i >= m_pcEditDoc->m_cLayoutMgr.GetTabSpace() ){
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
				if( pLine[0] != cChar ){
					continue;
				}
				nDelLen = 1;
			}

			/* カーソルを移動 */
			MoveCursor( 0, i, false );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					0,
					i,
					&pcOpe->m_ptCaretPos_PHY_Before.x,
					&pcOpe->m_ptCaretPos_PHY_Before.y
				);
			}else{
				pcOpe = NULL;
			}
			pcMemDeleted = new CMemory;
			/* 指定位置の指定長データ削除 */
			DeleteData2(
				0,
				i,
				nDelLen,	// 2001.12.03 hor
				pcMemDeleted,
				pcOpe				/* 編集操作要素 COpe */
			);
			if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					0,
					i,
					&pcOpe->m_ptCaretPos_PHY_After.x,
					&pcOpe->m_ptCaretPos_PHY_After.y
				);
				/* 操作の追加 */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}else{
				delete pcMemDeleted;
				pcMemDeleted = NULL;
			}
			if ( nLineCountPrev != m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
				//	行数が変化した!!
				nSelectLineToOld += m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}
		m_nSelectLineFrom = nSelectLineFromOld;	/* 範囲選択開始行 */
		m_nSelectColmFrom = nSelectColFromOld; 	/* 範囲選択開始桁 */
		m_nSelectLineTo = nSelectLineToOld;		/* 範囲選択終了行 */
		m_nSelectColmTo = nSelectColToOld;		/* 範囲選択終了桁 */
		// From Here 2001.12.03 hor
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, true );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* 操作種別 */
			pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;				/* 操作前のキャレット位置Ｙ */
			pcOpe->m_ptCaretPos_PHY_After.x = pcOpe->m_ptCaretPos_PHY_Before.x;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_ptCaretPos_PHY_After.y = pcOpe->m_ptCaretPos_PHY_Before.y;	/* 操作後のキャレット位置Ｙ */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		// To Here 2001.12.03 hor
	}

	/* 再描画 */
	RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
}

/* 最後にテキストを追加 */
void CEditView::Command_ADDTAIL(
	const char* pszData,
	int			nDataLen
)
{
	int		nNewLine;					/* 挿入された部分の次の位置の行 */
	int		nNewPos;					/* 挿入された部分の次の位置のデータ位置 */
	COpe*	pcOpe = NULL;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
	/*ファイルの最後に移動 */
	Command_GOFILEEND( false );

	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe = new COpe;
		pcOpe->m_ptCaretPos_PHY_Before.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_Before.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
	}

	/* 現在位置にデータを挿入 */
	InsertData_CEditView(
		m_nCaretPosX,
		m_nCaretPosY,
		(char*)pszData,
		nDataLen,
		&nNewLine,
		&nNewPos,
		pcOpe,
		true
	);

	/* 挿入データの最後へカーソルを移動 */
	// Sep. 2, 2002 すなふき アンダーラインの表示が残ってしまう問題を修正
	MoveCursor( nNewPos, nNewLine, true );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
		pcOpe->m_ptCaretPos_PHY_After.x = m_ptCaretPos_PHY.x;	/* 操作後のキャレット位置Ｘ */
		pcOpe->m_ptCaretPos_PHY_After.y = m_ptCaretPos_PHY.y;	/* 操作後のキャレット位置Ｙ */
		/* 操作の追加 */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
}




/*! タグジャンプ

	@param bClose [in] true:元ウィンドウを閉じる

	@date 2003.04.03 genta 元ウィンドウを閉じるかどうかの引数を追加
	@date 2004.05.13 Moca 行桁位置の指定が無い場合は、行桁を移動しない
*/
bool CEditView::Command_TAGJUMP( bool bClose )
{
	int			nJumpToLine;
	int			nJumpToColm;
	char		szJumpToFile[1024];
	int			nPathLen;
	int			nBgn;
	memset( szJumpToFile, 0, sizeof(szJumpToFile) );
	//	2004.05.13 Moca 初期値を1ではなく元の位置を継承するように
	// 0以下は未指定扱い。(1開始)
	nJumpToLine = 0;
	nJumpToColm = 0;

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	int		nX;
	int		nY;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_nCaretPosX,
		m_nCaretPosY,
		&nX,
		&nY
	);
	/* 現在行のデータを取得 */
	int			nLineLen;
	const char*	pLine;
	pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );
	if( NULL == pLine ){
		goto can_not_tagjump_end;
	}
	/* WZ風のタグリストか */
	if( 0 == memcmp( pLine, "・", 2 )
	 || 0 == memcmp( pLine, "■\"", 3 )
	){
		if( 0 == memcmp( pLine, "■\"", 3 ) ){
			if( IsFilePath( &pLine[3], &nBgn, &nPathLen ) ){
				memcpy( szJumpToFile, &pLine[3 + nBgn], nPathLen );
				GetLineColm( &pLine[3] + nPathLen, &nJumpToLine, &nJumpToColm );
			}else{
				goto can_not_tagjump;
			}
		}else{
			GetLineColm( &pLine[2], &nJumpToLine, &nJumpToColm );
			nY--;

			for( ; 0 <= nY; nY-- ){
				pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );
				if( NULL == pLine ){
					goto can_not_tagjump;
				}
				if( 0 == memcmp( pLine, "・", 2 ) ){
					continue;
				}else
				if( 0 == memcmp( pLine, "■\"", 3 ) ){
					if( IsFilePath( &pLine[3], &nBgn, &nPathLen ) ){
						memcpy( szJumpToFile, &pLine[3 + nBgn], nPathLen );
						break;
					}else{
						goto can_not_tagjump;
					}
				}
				else{
					goto can_not_tagjump;
				}
			}
		}
	}
	else{
		//@@@ 2001.12.31 YAZAKI
		const char *p = pLine;
		const char *p_end = p + nLineLen;

		//	From Here Aug. 27, 2001 genta
		//	Borland 形式のメッセージからのTAG JUMP
		while( p < p_end ){
			//	skip space
			for( ; p < p_end && ( *p == ' ' || *p == '\t' || *p == '\n' ); ++p )
				;
			if( p >= p_end )
				break;
		
			//	Check Path
			if( IsFilePath( p, &nBgn, &nPathLen ) ){
				memcpy( szJumpToFile, &p[nBgn], nPathLen );
				GetLineColm( &p[nBgn + nPathLen], &nJumpToLine, &nJumpToColm );
				break;
			}
			//	Jan. 04, 2001 genta Directoryを対象外にしたので文字列には柔軟に対応
			//	break;	//@@@ 2001.12.31 YAZAKI 「working ...」問題に対処
			//	skip non-space
			for( ; p < p_end && ( *p != ' ' && *p != '\t' ); ++p )
				;
		}
		if( szJumpToFile[0] == '\0' ){
			if( !Command_TagJumpByTagsFile() )	//@@@ 2003.04.13
				goto can_not_tagjump;
			return true;
		}
		//	From Here Aug. 27, 2001 genta
	}

	//	Apr. 21, 2003 genta bClose追加
	if( !TagJumpSub( szJumpToFile, nJumpToLine, nJumpToColm, bClose ) )	//@@@ 2003.04.13
		goto can_not_tagjump;

	return true;

can_not_tagjump:;
can_not_tagjump_end:;
	SendStatusMessage(_T("タグジャンプできません"));	//@@@ 2003.04.13
	return false;
}




/* タグジャンプバック */
void CEditView::Command_TAGJUMPBACK( void )
{
// 2004/06/21 novice タグジャンプ機能追加
	TagJump tagJump;

	/* タグジャンプ情報の参照 */
	if( !CShareData::getInstance()->PopTagJump(&tagJump) || !IsSakuraMainWindow(tagJump.hwndReferer) ){
		SendStatusMessage(_T("タグジャンプバックできません"));
		// 2004.07.10 Moca m_TagJumpNumを0にしなくてもいいと思う
		// m_pShareData->m_TagJumpNum = 0;
		return;
	}

	/* アクティブにする */
	ActivateFrameWindow( tagJump.hwndReferer );

	/* カーソルを移動させる */
	memcpy( m_pShareData->m_sWorkBuffer.m_szWork, (void*)&(tagJump.point), sizeof( tagJump.point ) );
	::SendMessage( tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0 );

	return;
}

/*
	ダイレクトタグジャンプ

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.05.12	フォルダ階層も考慮して探す
*/
bool CEditView::Command_TagJumpByTagsFile( void )
{
	CMemory	cmemKey;
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
	GetCurrentTextForSearch( cmemKey );
	if( 0 == cmemKey.GetStringLength() ) return false;	//キーがないなら終わり

	if( ! m_pcEditDoc->IsValidPath() ) return false;

	// ファイル名に応じて探索回数を決定する
	strcpy( szCurrentPath, m_pcEditDoc->GetFilePath() );
	nLoop = CalcDirectoryDepth( szCurrentPath );

	if( nLoop <  0 ) nLoop =  0;
	if( nLoop > (_MAX_PATH/2) ) nLoop = (_MAX_PATH/2);	//\A\B\C...のようなとき1フォルダで2文字消費するので...

		//パス名のみ取り出す。
		cDlgTagJumpList.SetFileName( szCurrentPath );
		szCurrentPath[ _tcslen( szCurrentPath ) - _tcslen( m_pcEditDoc->GetFileName() ) ] = _T('\0');

		for( i = 0; i <= nLoop; i++ )
		{
			//タグファイル名を作成する。
			wsprintf( szTagFile, _T("%s%s"), szCurrentPath, TAG_FILENAME );

			//タグファイルを開く。
			fp = fopen( szTagFile, _T("r") );
			if( fp )
			{
				bNoTag = false;
				nMatch = 0;
				while( fgets( szLineData, _countof( szLineData ), fp ) )
				{
					if( szLineData[0] <= _T('!') ) goto next_line;	//コメントならスキップ
					//chop( szLineData );

					s[0][0] = s[1][0] = s[2][0] = s[3][0] = s[4][0] = _T('\0');
					n2 = 0;
					//	2004.06.04 Moca ファイル名/パスにスペースが含まれているときに
					//	ダイレクトタグジャンプに失敗していた
					//	sscanf の%[^\t\r\n] でスペースを読みとるように変更
					//	@@ 2005.03.31 MIK TAG_FORMAT定数化
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT,	//tagsフォーマット
						s[0], s[1], &n2, s[3], s[4]
					);
					if( nRet < 4 ) goto next_line;
					if( n2 <= 0 ) goto next_line;	//行番号不正(-excmd=nが指定されてないかも)

					if( 0 != strcmp( s[0], cmemKey.GetStringPtr() ) ) goto next_line;

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
					if( ! cDlgTagJumpList.DoModal( m_hInstance, m_hWnd, (LPARAM)0 ) ) 
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
							strcpy( szTagFile, p );	//何も加工しない。
						}
						else
						{
							//ドライブ加工したほうがよい？
							strcpy( szTagFile, p );	//何も加工しない。
						}
					}
					else if( isalpha( p[0] ) && p[1] == ':' )	//絶対パスか？
					{
						strcpy( szTagFile, p );	//何も加工しない。
					}
					else
					{
						wsprintf( szTagFile, _T("%s%s"), szCurrentPath, p );
					}

					return TagJumpSub( szTagFile, n2, 0 );
				}
			}	//fp

			//カレントパスを1階層上へ。
			strcat( szCurrentPath, _T("..\\") );
		}

	return false;
}

/*
	指定ファイルの指定位置にタグジャンプする。

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.04.21 genta bClose追加
	@date	2004.05.29 Moca 0以下が指定されたときは、善処する
	@date	2007.02.17 genta 相対パスの基準ディレクトリ指示を追加
*/
bool CEditView::TagJumpSub(
	const TCHAR*	pszFileName,
	int				nJumpToLine,	//!< [in] 論理行番号(1開始)。0以下を指定したら行ジャンプはしない。
	int				nJumpToColm,	//!< [in] 論理行単位の行内の位置(1開始)
	bool			bClose,			//!< [in] true: 元ウィンドウを閉じる / false: 元ウィンドウを閉じない
	bool			bRelFromIni
)
{
	HWND	hwndOwner;
	POINT	poCaret;
	// 2004/06/21 novice タグジャンプ機能追加
	TagJump	tagJump;

	// 参照元ウィンドウ保存
	tagJump.hwndReferer = m_pcEditDoc->m_hwndParent;

	//	Feb. 17, 2007 genta 実行ファイルからの相対指定の場合は
	//	予め絶対パスに変換する．(キーワードヘルプジャンプで用いる)
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	TCHAR	szJumpToFile[1024];
	if( bRelFromIni && _IS_REL_PATH( pszFileName ) ){
		GetInidirOrExedir( szJumpToFile, pszFileName );
	}
	else {
		_tcscpy( szJumpToFile, pszFileName );
	}

	/* ロングファイル名を取得する */
	TCHAR	szWork[1024];
	if( TRUE == ::GetLongFileName( szJumpToFile, szWork ) )
	{
		_tcscpy( szJumpToFile, szWork );
	}

// 2004/06/21 novice タグジャンプ機能追加
// 2004/07/05 みちばな
// 同一ファイルだとSendMesssageで m_nCaretPosX,m_nCaretPosYが更新されてしまい、
// ジャンプ先の場所がジャンプ元として保存されてしまっているので、
// その前で保存するように変更。

	/* カーソル位置変換 */
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_nCaretPosX,
		m_nCaretPosY,
		(int*)&tagJump.point.x,
		(int*)&tagJump.point.y
	);

	// タグジャンプ情報の保存
	CShareData::getInstance()->PushTagJump(&tagJump);


	/* 指定ファイルが開かれているか調べる */
	/* 開かれている場合は開いているウィンドウのハンドルも返す */
	/* ファイルを開いているか */
	if( CShareData::getInstance()->IsPathOpened( (const char*)szJumpToFile, &hwndOwner ) )
	{
		// 2004.05.13 Moca マイナス値は無効
		if( 0 < nJumpToLine ){
			/* カーソルを移動させる */
			poCaret.y = nJumpToLine - 1;
			if( 0 < nJumpToColm ){
				poCaret.x = nJumpToColm - 1;
			}else{
				poCaret.x = 0;
			}
			memcpy( m_pShareData->m_sWorkBuffer.m_szWork, (void*)&poCaret, sizeof(poCaret) );
			::SendMessage( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
		}
		/* アクティブにする */
		ActivateFrameWindow( hwndOwner );
	}
	else{
		/* 新しく開く */
		EditInfo	inf;
		bool		bSuccess;

		_tcscpy( inf.m_szPath, szJumpToFile );
		inf.m_nX           = nJumpToColm - 1;
		inf.m_nY           = nJumpToLine - 1;
		inf.m_nViewLeftCol = inf.m_nViewTopLine = -1;
		inf.m_nCharCode    = CODE_AUTODETECT;

		bSuccess = CControlTray::OpenNewEditor2(
			m_hInstance,
			m_hWnd,
			&inf,
			false,	/* 読み取り専用か */
			true	//	同期モードで開く
		);

		if( ! bSuccess )	//	ファイルが開けなかった
			return false;

		//	Apr. 23, 2001 genta
		//	hwndOwnerに値が入らなくなってしまったために
		//	Tag Jump Backが動作しなくなっていたのを修正
		if( !CShareData::getInstance()->IsPathOpened( (const char*)szJumpToFile, &hwndOwner ) )
			return false;
	}

	// 2006.12.30 ryoji 閉じる処理は最後に（処理位置移動）
	//	Apr. 2003 genta 閉じるかどうかは引数による
	//	grep結果からEnterでジャンプするところにCtrl判定移動
	if( bClose )
	{
		Command_WINCLOSE();	//	挑戦するだけ。
	}

	return true;
}

/*
	タグファイルを作成する。

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.05.12	ダイアログ表示でフォルダ等を細かく指定できるようにした。
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CEditView::Command_TagsMake( void )
{
#define	CTAGS_COMMAND	_T("ctags.exe")

	TCHAR	szTargetPath[1024 /*_MAX_PATH+1*/ ];
	if( m_pcEditDoc->IsValidPath() )
	{
		_tcscpy( szTargetPath, m_pcEditDoc->GetFilePath() );
		szTargetPath[ _tcslen( szTargetPath ) - _tcslen( m_pcEditDoc->GetFileName() ) ] = _T('\0');
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
	if( !cDlgTagsMake.DoModal( m_hInstance, m_hWnd, (LPARAM)0, szTargetPath ) ) return false;

	TCHAR	cmdline[1024];
	/* exeのあるフォルダ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	GetExedir( cmdline, CTAGS_COMMAND );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//ctags.exeの存在チェック
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		WarningMessage( m_hWnd,	_T( "タグ作成コマンド実行は失敗しました。\n\nCTAGS.EXE が見つかりません。" ) );
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_hWnd );

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
	if( cDlgTagsMake.m_szTagsCmdLine[0] != _T('\0') )	//個別指定のコマンドライン
	{
		_tcscat( options, _T(" ") );
		_tcscat( options, cDlgTagsMake.m_szTagsCmdLine );
	}
	_tcscat( options, _T(" *") );	//配下のすべてのファイル

	//コマンドライン文字列作成(MAX:1024)
	if (IsWin32NT())
	{
		// 2010.08.28 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
		//	2006.08.04 genta add /D to disable autorun
		wsprintf( cmdline, _T("\"%s\\cmd.exe\" /D /C \"\"%s\\%s\" %s\""),
				szCmdDir,
				szExeFolder,	//sakura.exeパス
				CTAGS_COMMAND,	//ctags.exe
				options			//ctagsオプション
			);
	}
	else
	{
		// 2010.08.27 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
		wsprintf( cmdline, _T("\"%s\\command.com\" /C \"%s\\%s\" %s"),
				szCmdDir,
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
	if( !bProcessResult )
	{
		WarningMessage( m_hWnd,	_T("タグ作成コマンド実行は失敗しました。\n\n%s"), cmdline );
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
		hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		hwndMsg = ::GetDlgItem( hwndCancel, IDC_STATIC_CMD );
		::SendMessage( hwndMsg, WM_SETTEXT, 0, (LPARAM)"タグファイルを作成中です。" );

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
					if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
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

						work[ read_cnt ] = '\0';	// Nov. 15, 2003 genta 表示用に0終端する
						WarningMessage( m_hWnd,	_T("タグ作成コマンド実行は失敗しました。\n\n%s"), work ); // 2003.11.09 じゅうじ

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

	InfoMessage( m_hWnd, _T("タグファイルの作成が終了しました。"));

	return true;
}

/*!
	キーワードを指定してタグジャンプ

	@author MIK
	@date 2005.03.31 新規作成
*/
bool CEditView::Command_TagJumpByTagsFileKeyword( const char* keyword )
{
	CMemory	cmemKey;
	CDlgTagJumpList	cDlgTagJumpList;
	TCHAR	s[5][1024];
	int		n2;
	int depth;
	TCHAR	szTagFile[1024];		//タグファイル
	TCHAR	szCurrentPath[1024];

	if( ! m_pcEditDoc->IsValidPath() ) return false;
	_tcscpy( szCurrentPath, m_pcEditDoc->GetFilePath() );

	cDlgTagJumpList.SetFileName( szCurrentPath );
	cDlgTagJumpList.SetKeyword( keyword );

	szCurrentPath[ _tcslen( szCurrentPath ) - _tcslen( m_pcEditDoc->GetFileName() ) ] = _T('\0');

	if( ! cDlgTagJumpList.DoModal( m_hInstance, m_hWnd, (LPARAM)1 ) ) 
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
			strcpy( szTagFile, p );	//何も加工しない。
		}
		else
		{
			//ドライブ加工したほうがよい？
			strcpy( szTagFile, p );	//何も加工しない。
		}
	}
	else if( isalpha( p[0] ) && p[1] == _T(':') )	//絶対パスか？
	{
		strcpy( szTagFile, p );	//何も加工しない。
	}
	else
	{
		for( int i = 0; i < depth; i++ )
		{
			strcat( szCurrentPath, _T("..\\") );
		}
		wsprintf( szTagFile, _T("%s%s"), szCurrentPath, p );
	}

	return TagJumpSub( szTagFile, n2, 0 );
}



/* C/C++ヘッダファイル オープン機能 */		//Feb. 10, 2001 jepro	説明を「インクルードファイル」から変更
//BOOL CEditView::Command_OPENINCLUDEFILE( BOOL bCheckOnly )
BOOL CEditView::Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca ヘッダ・ソースのコードを統合＆削除
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		_T("C/C++ヘッダファイルのオープンに失敗しました。") );
}




/* C/C++ソースファイル オープン機能 */
//BOOL CEditView::Command_OPENCCPP( BOOL bCheckOnly )	//Feb. 10, 2001 JEPRO	コマンド名を若干変更
BOOL CEditView::Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca ヘッダ・ソースのコードを統合＆削除
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		_T("C/C++ソースファイルのオープンに失敗しました。"));
}


/*! 指定拡張子のファイルに対応するファイルを開く補助関数

	@date 2003.06.28 Moca ヘッダ・ソースファイルオープン機能のコードを統合
	@date 2008.04.09 ryoji 処理対象(file_ext)と開く対象(open_ext)の扱いが逆になっていたのを修正
*/
BOOL CEditView::OPEN_ExtFromtoExt(
	BOOL			bCheckOnly,		//!< [in] true: チェックのみ行ってファイルは開かない
	BOOL			bBeepWhenMiss,	//!< [in] true: ファイルを開けなかった場合に警告音を出す
	const TCHAR*	file_ext[],		//!< [in] 処理対象とする拡張子
	const TCHAR*	open_ext[],		//!< [in] 処理対象拡張子リストの要素数
	int				file_extno,		//!< [in] 開く対象とする拡張子
	int				open_extno,		//!< [in] 開く対象拡張子リストの要素数
	const TCHAR*	errmes			//!< [in] ファイルを開けなかった場合に表示するエラーメッセージ
)
{
//From Here Feb. 7, 2001 JEPRO 追加
	int		i;
	BOOL	bwantopen_c;
//To Here Feb. 7, 2001

	/* 編集中ファイルの拡張子を調べる */
	for( i = 0; i < file_extno; i++ ){
		if( CheckEXT( m_pcEditDoc->GetFilePath(), file_ext[i] ) ){
			bwantopen_c = TRUE;
			goto open_c;
		}
	}
	if( bBeepWhenMiss ){
		ErrorBeep();
	}
	return FALSE;

open_c:;

	TCHAR	szPath[_MAX_PATH];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	HWND	hwndOwner;

	_splitpath( m_pcEditDoc->GetFilePath(), szDrive, szDir, szFname, szExt );

	for( i = 0; i < open_extno; i++ ){
		_makepath( szPath, szDrive, szDir, szFname, open_ext[i] );
		if( !fexist( (const char *)szPath ) ){
			if( i < open_extno - 1 )
				continue;
			if( bBeepWhenMiss ){
				ErrorBeep();
			}
			return FALSE;
		}
		break;
	}
	if( bCheckOnly ){
		return TRUE;
	}

	/* 指定ファイルが開かれているか調べる */
	/* 開かれている場合は開いているウィンドウのハンドルも返す */
	/* ファイルを開いているか */
	if( CShareData::getInstance()->IsPathOpened( (const char*)szPath, &hwndOwner ) ){
	}else{
		/* 新しく開く */
		char	szPath2[_MAX_PATH + 3];
		if( strchr( szPath, ' ' ) ){
			wsprintf( szPath2, "\"%s\"", szPath );
		}else{
			strcpy( szPath2, szPath );
		}
		/* 文字コードはこのファイルに合わせる */
		CControlTray::OpenNewEditor(
			m_hInstance,
			m_hWnd,
			szPath2,
			m_pcEditDoc->m_nCharCode,
			false,	/* 読み取り専用か */
			true
		);
		/* ファイルを開いているか */
		if( CShareData::getInstance()->IsPathOpened( (const char*)szPath, &hwndOwner ) ){
		}else{
			ErrorMessage( m_hWnd, _T("%s\n\n%s\n\n"), errmes, szPath );
			return FALSE;
		}
	}
	/* アクティブにする */
	ActivateFrameWindow( hwndOwner );

// 2004/06/21 novice タグジャンプ機能追加
// 2004/07/09 genta/Moca タグジャンプバックの登録が取り除かれていたが、
//            こちらでも従来どおり登録する
	TagJump	tagJump;
	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_nCaretPosX,
		m_nCaretPosY,
		(int*)&tagJump.point.x,
		(int*)&tagJump.point.y
	);
	tagJump.hwndReferer = m_pcEditDoc->m_hwndParent;
	// タグジャンプ情報の保存
	CShareData::getInstance()->PushTagJump(&tagJump);
	return TRUE;
}




//From Here Feb. 10, 2001 JEPRO 追加
/* C/C++ヘッダファイルまたはソースファイル オープン機能 */
BOOL CEditView::Command_OPEN_HfromtoC( BOOL bCheckOnly )
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
	@date 2007.06.20 ryoji m_pShareData->m_TabWndWndplの廃止，グループIDリセット
*/
void CEditView::Command_BIND_WINDOW( void )
{
	//タブモードであるならば
	if (m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd)
	{
		//タブウィンドウの設定を変更
		m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin = !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin;

		// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
		if( !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			m_pcEditDoc->m_pcEditWnd->WindowTopMost(
				( (DWORD)::GetWindowLongPtr( m_pcEditDoc->m_pcEditWnd->m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
			);
		}

		//Start 2004.08.27 Kazika 変更
		//タブウィンドウの設定を変更をブロードキャストする
		CShareData::getInstance()->ResetGroupId();
		CShareData::getInstance()->PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,						//タブウィンドウイベント
			(WPARAM)((m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin) ? TWNT_MODE_DISABLE : TWNT_MODE_ENABLE),//タブモード有効/無効化イベント
			(LPARAM)m_pcEditDoc->m_pcEditWnd->m_hWnd,	//CEditWndのウィンドウハンドル
			m_hWnd);									//自分自身
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
void CEditView::Command_CASCADE( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

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
			if( ::IsIconic( pEditNodeArr[i].m_hWnd ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].m_hWnd ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	Mar. 20, 2004 genta
			//	現在のウィンドウを末尾に持っていくためここではスキップ
			if( pEditNodeArr[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				current_win_index = i;
				continue;
			}
			pWndArr[count].hWnd = pEditNodeArr[i].m_hWnd;
			count++;
		}

		//	Mar. 20, 2004 genta
		//	現在のウィンドウを末尾に挿入 inspired by crayonzen
		if( current_win_index >= 0 ){
			pWndArr[count].hWnd = pEditNodeArr[current_win_index].m_hWnd;
			count++;
		}

		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_hWnd, &rcDesktop );
		
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
void CEditView::Command_TILE_H( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_hWnd, &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].m_hWnd ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].m_hWnd ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	現在のウィンドウを先頭に持ってくる
			if( pEditNodeArr[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = m_pcEditDoc->m_hwndParent;
			}
			else {
				phwndArr[count] = pEditNodeArr[i].m_hWnd;
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
void CEditView::Command_TILE_V( void )
{
	int i;

	/* 現在開いている編集窓のリストを取得する */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	デスクトップサイズを得る
		RECT	rcDesktop;
		//	May 01, 2004 genta マルチモニタ対応
		::GetMonitorWorkRect( m_hWnd, &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].m_hWnd ) ){	//	最小化しているウィンドウは無視。
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].m_hWnd ) ){	//	不可視ウィンドウは無視。
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	現在のウィンドウを先頭に持ってくる
			if( pEditNodeArr[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = m_pcEditDoc->m_hwndParent;
			}
			else {
				phwndArr[count] = pEditNodeArr[i].m_hWnd;
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
void CEditView::Command_MAXIMIZE_V( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;
	hwndFrame = ::GetParent( m_hwndParent );
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
void CEditView::Command_MAXIMIZE_H( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;

	hwndFrame = ::GetParent( m_hwndParent );
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
void CEditView::Command_MINIMIZE_ALL( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = m_pShareData->m_sNodes.m_nEditArrNum;
	if( 0 == j ){
		return;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd;
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
void CEditView::Command_REPLACE_DIALOG( void )
{
	CMemory		cmemCurText;
	BOOL		bSelected = FALSE;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	/* 検索文字列を初期化 */
	strcpy( m_pcEditDoc->m_cDlgReplace.m_szText, cmemCurText.GetStringPtr() );
	strncpy( m_pcEditDoc->m_cDlgReplace.m_szText2, m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[0], MAX_PATH - 1 );	// 2006.08.23 ryoji 前回の置換後文字列を引き継ぐ
	m_pcEditDoc->m_cDlgReplace.m_szText2[MAX_PATH - 1] = '\0';

	if ( IsTextSelected() && m_nSelectLineFrom!=m_nSelectLineTo ) {
		bSelected = TRUE;	//選択範囲をチェックしてダイアログ表示
	}else{
		bSelected = FALSE;	//ファイル全体をチェックしてダイアログ表示
	}
	/* 置換オプションの初期化 */
	m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget=0;	/* 置換対象 */
	m_pcEditDoc->m_cDlgReplace.m_nPaste=FALSE;		/* 貼り付ける？ */
// To Here 2001.12.03 hor

	/* 置換ダイアログの表示 */
	//	From Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	if( !::IsWindow( m_pcEditDoc->m_cDlgReplace.m_hWnd ) ){
		m_pcEditDoc->m_cDlgReplace.DoModeless( m_hInstance, m_hWnd/*::GetParent( m_hwndParent )*/, (LPARAM)this, bSelected );
	}
	else {
		/* アクティブにする */
		ActivateFrameWindow( m_pcEditDoc->m_cDlgReplace.m_hWnd );
		::SetDlgItemText( m_pcEditDoc->m_cDlgReplace.m_hWnd, IDC_COMBO_TEXT, cmemCurText.GetStringPtr() );
	}
	//	To Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	return;
}

/*! 置換実行
	
	@date 2002/04/08 親ウィンドウを指定するように変更。
	@date 2003.05.17 かろと 長さ０マッチの無限置換回避など
*/
void CEditView::Command_REPLACE( HWND hwndParent )
{
	if ( hwndParent == NULL ){	//	親ウィンドウが指定されていなければ、CEditViewが親。
		hwndParent = m_hWnd;
	}
	//2002.02.10 hor
	int nPaste			=	m_pcEditDoc->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	=	m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		=	m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp;
	int nFlag			=	m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase ? 0x01 : 0x00;

	// From Here 2001.12.03 hor
	if( nPaste && !m_pcEditDoc->IsEnablePaste()){
		OkMessage( hwndParent, _T("クリップボードに有効なデータがありません！") );
		::CheckDlgButton( m_pcEditDoc->m_cDlgReplace.m_hWnd, IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_pcEditDoc->m_cDlgReplace.m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
		return;	//	失敗return;
	}

	// 2002.01.09 hor
	// 選択エリアがあれば、その先頭にカーソルを移す
	if( IsTextSelected() ){
		if( m_bBeginBoxSelect ){
			MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, true );
		} else {
//			HandleCommand( F_LEFT, true, 0, 0, 0, 0 );
			Command_LEFT( false, false );
		}
	}
	// To Here 2002.01.09 hor
	
	// 矩形選択？
//			bBeginBoxSelect = m_bBeginBoxSelect;

	/* カーソル左移動 */
	//HandleCommand( F_LEFT, true, 0, 0, 0, 0 );	//？？？
	// To Here 2001.12.03 hor

	/* テキスト選択解除 */
	/* 現在の選択範囲を非選択状態に戻す */
	DisableSelectArea( true );

	// 2004.06.01 Moca 検索中に、他のプロセスによってm_szREPLACEKEYArrが書き換えられても大丈夫なように
	const CMemory	cMemRepKey( m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[0], _tcslen(m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[0]) );

	/* 次を検索 */
	Command_SEARCH_NEXT( true, true, hwndParent, 0 );

	/* テキストが選択されているか */
	if( IsTextSelected() ){
		// From Here 2001.12.03 hor
		int colTmp = 0;
		int linTmp = 0;
		if ( nPaste || !bRegularExp ) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if(nReplaceTarget==1){	//挿入位置へ移動
				colTmp = m_nSelectColmTo - m_nSelectColmFrom;
				linTmp = m_nSelectLineTo - m_nSelectLineFrom;
				m_nSelectColmFrom=-1;
				m_nSelectLineFrom=-1;
				m_nSelectColmTo	 =-1;
				m_nSelectLineTo	 =-1;
			}
			else if(nReplaceTarget==2){	//追加位置へ移動
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				MoveCursor( m_nSelectColmTo, m_nSelectLineTo, false );
				m_nSelectColmFrom=-1;
				m_nSelectLineFrom=-1;
				m_nSelectColmTo	 =-1;
				m_nSelectLineTo	 =-1;
			}
			else{
				// 位置指定ないので、何もしない
			}
		}
		/* コマンドコードによる処理振り分け */
		/* テキストを貼り付け */
		if(nPaste){
			Command_PASTE(0);
		} else if ( bRegularExp ) { /* 検索／置換  1==正規表現 */
			// 先読みに対応するために物理行末までを使うように変更 2005/03/27 かろと
			// 2002/01/19 novice 正規表現による文字列置換
			CMemory cmemory;
			CBregexp cRegexp;

			if( !InitRegexp( m_hWnd, cRegexp, true ) ){
				return;	//	失敗return;
			}

			// 物理行、物理行長、物理行での検索マッチ位置
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(m_nSelectLineFrom);
			const char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nIdx = LineColmnToIndex( pcLayout, m_nSelectColmFrom ) + pcLayout->m_nOffset;
			int nLen = pcLayout->m_pCDocLine->GetLength();
			// 正規表現で選択始点・終点への挿入を記述
			//	Jun. 6, 2005 かろと
			// →これでは「検索の後ろの文字が改行だったら次の行頭へ移動」が処理できない
			// → Oct. 30, 「検索の後ろの文字が改行だったら・・」の処理をやめる（誰もしらないみたいなので）
			// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
			CMemory cMemMatchStr = CMemory(_T("$&"), _tcslen(_T("$&")));
			CMemory cMemRepKey2;
			if (nReplaceTarget == 1) {	//選択始点へ挿入
				cMemRepKey2 = cMemRepKey;
				cMemRepKey2 += cMemMatchStr;
			} else if (nReplaceTarget == 2) { // 選択終点へ挿入
				cMemRepKey2 = cMemMatchStr;
				cMemRepKey2 += cMemRepKey;
			} else {
				cMemRepKey2 = cMemRepKey;
			}
			cRegexp.Compile( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0], cMemRepKey2.GetStringPtr(), nFlag);
			if( cRegexp.Replace(pLine, nLen, nIdx) ){
				// From Here Jun. 6, 2005 かろと
				// 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				// キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				// そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				CLayoutMgr& rLayoutMgr = m_pcEditDoc->m_cLayoutMgr;
				int matchLen = cRegexp.GetMatchLen();
				int nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				if (matchLen == 0) {
					// ０文字マッチの時(無限置換にならないように１文字進める)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += (CMemory::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// 無限置換しないように、１文字増やしたので１文字選択に変更
					// 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
					rLayoutMgr.LogicToLayout( nIdxTo, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji 行位置も取得する
				}
				// 行末から検索文字列末尾までの文字数
				int colDiff = nLen - nIdxTo;
				//	Oct. 22, 2005 Karoto
				//	\rを置換するとその後ろの\nが消えてしまう問題の対応
				if (colDiff < pcLayout->m_pCDocLine->m_cEol.GetLen()) {
					// 改行にかかっていたら、行全体をINSTEXTする。
					colDiff = 0;
					rLayoutMgr.LogicToLayout( nLen, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji 追加
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT( false, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 かろと
			}
		}else{
			//	HandleCommand( F_INSTEXT, false, (LPARAM)m_pShareData->m_szREPLACEKEYArr[0], FALSE, 0, 0 );
			Command_INSTEXT( false, cMemRepKey.GetStringPtr(), cMemRepKey.GetStringLength(), TRUE );
		}

		// 挿入後の検索開始位置を調整
		if(nReplaceTarget==1){
			m_nCaretPosX+=colTmp;
			m_nCaretPosY+=linTmp;
		}

		// To Here 2001.12.03 hor
		/* 最後まで置換した時にOK押すまで置換前の状態が表示されるので、
		** 置換後、次を検索する前に書き直す 2003.05.17 かろと
		*/
		Redraw();

		/* 次を検索 */
		Command_SEARCH_NEXT( true, true, hwndParent, "最後まで置換しました。" );
	}
}

/*! すべて置換実行

	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理など見直し
	@date 2006.03.31 かろと 行置換機能追加
	@date 2007.01.16 ryoji 行置換機能を全置換のオプションに変更
	@date 2009.09.20 genta 左下～右上で矩形選択された領域の置換が行われない
	@date 2010.09.17 ryoji ラインモード貼り付け処理を追加
*/
void CEditView::Command_REPLACE_ALL()
{
// From Here 2001.12.03 hor
	int			colFrom;		//選択範囲開始桁
	int			linFrom;		//選択範囲開始行
	int			colTo,colToP;	//選択範囲終了桁
	int			linTo,linToP;	//選択範囲終了行
	int			colDif = 0;		//置換後の桁調整
	int			linDif = 0;		//置換後の行調整
	int			colOld = 0;		//検索後の選択範囲次桁
	int			linOld = 0;		//検索後の行
	int			linOldLen = 0;	//検査後の行の長さ
	int			lineCnt;		//置換前の行数
	int			linPrev = 0;	//前回の検索行(矩形) @@@2001.12.31 YAZAKI warning退治
	int			linNext;		//次回の検索行(矩形)
	int			colTmp,linTmp,colLast,linLast;

	//2002.02.10 hor
	int nPaste			= m_pcEditDoc->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	= m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		= m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp;
	int bSelectedArea	= m_pShareData->m_Common.m_sSearch.m_bSelectedArea;
	int bConsecutiveAll	= m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll;	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji

	m_pcEditDoc->m_cDlgReplace.m_bCanceled=false;
	m_pcEditDoc->m_cDlgReplace.m_nReplaceCnt=0;

	// From Here 2001.12.03 hor
	if( nPaste && !m_pcEditDoc->IsEnablePaste() ){
		OkMessage( m_hWnd, _T("クリップボードに有効なデータがありません！") );
		::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor

	int			bBeginBoxSelect; // 矩形選択？
	if(IsTextSelected()){
		bBeginBoxSelect=m_bBeginBoxSelect;
	}
	else{
		bSelectedArea=FALSE;
		bBeginBoxSelect=FALSE;
	}

	/* 表示処理ON/OFF */
	bool bDisplayUpdate = false;

	m_bDrawSWITCH = bDisplayUpdate;
	CDlgCancel	cDlgCancel;
	int	nAllLineNum = m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	/* 進捗表示&中止ダイアログの作成 */
	HWND		hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hWnd, IDD_REPLACERUNNING );
	::EnableWindow( m_hWnd, FALSE );
	::EnableWindow( ::GetParent( m_hWnd ), FALSE );
	::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), FALSE );
	//<< 2002/03/26 Azumaiya
	// 割り算掛け算をせずに進歩状況を表せるように、シフト演算をする。
	int nShiftCount;
	for ( nShiftCount = 0; SHRT_MAX < nAllLineNum; nShiftCount++ )
	{
		nAllLineNum >>= 1;
	}
	//>> 2002/03/26 Azumaiya

	/* プログレスバー初期化 */
	HWND		hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
	::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, nAllLineNum ) );
	int			nNewPos = 0;
 	::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );

	/* 置換個数初期化 */
	int			nReplaceNum = 0;
	HWND		hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
	TCHAR szLabel[64];
	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	// From Here 2001.12.03 hor
	if (bSelectedArea){
		/* 選択範囲置換 */
		/* 選択範囲開始位置の取得 */
		linFrom = m_nSelectLineFrom;
		linTo   = m_nSelectLineTo;
		//	From Here 2007.09.20 genta 矩形範囲の選択置換ができない
		//	左下～右上と選択した場合，m_nSelectColmTo < m_nSelectColmFrom となるが，
		//	範囲チェックで colFrom < colTo を仮定しているので，
		//	矩形選択の場合は左上～右下指定になるよう桁を入れ換える．
		if( bBeginBoxSelect && m_nSelectColmTo < m_nSelectColmFrom ){
			colFrom = m_nSelectColmTo;
			colTo   = m_nSelectColmFrom;
		}
		else {
			colFrom = m_nSelectColmFrom;
			colTo   = m_nSelectColmTo;
		}
		//	To Here 2007.09.20 genta 矩形範囲の選択置換ができない

		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			colTo,
			linTo,
			&colToP,
			&linToP
		);
		//選択範囲開始位置へ移動
		MoveCursor( colFrom, linFrom, bDisplayUpdate );
	}
	else{
		/* ファイル全体置換 */
		/* ファイルの先頭に移動 */
	//	HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
		Command_GOFILETOP(bDisplayUpdate);
	}

	colLast=m_nCaretPosX;
	linLast=m_nCaretPosY;

	/* テキスト選択解除 */
	/* 現在の選択範囲を非選択状態に戻す */
	DisableSelectArea( bDisplayUpdate );
	/* 次を検索 */
	Command_SEARCH_NEXT( true, bDisplayUpdate, 0, 0 );
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// 速く動かすことを最優先に組んでみました。
	// ループの外で文字列の長さを特定できるので、一時変数化。
	char *szREPLACEKEY;			// 置換後文字列。
	int nREPLACEKEY;			// 置換後文字列の長さ。
	bool		bColmnSelect;	// 矩形貼り付けを行うかどうか。
	bool		bLineSelect = false;	// ラインモード貼り付けを行うかどうか
	CMemory		cmemClip;		// 置換後文字列のデータ（データを格納するだけで、ループ内ではこの形ではデータを扱いません）。

	// クリップボードからのデータ貼り付けかどうか。
	if( nPaste != 0 )
	{
		// クリップボードからデータを取得。
		if ( !MyGetClipboardData( cmemClip, &bColmnSelect, m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: NULL ) )
		{
			ErrorBeep();
			return;
		}

		// 矩形貼り付けが許可されていて、クリップボードのデータが矩形選択のとき。
		if ( m_pShareData->m_Common.m_sEdit.m_bAutoColmnPaste == TRUE && bColmnSelect == TRUE )
		{
			// マウスによる範囲選択中
			if( m_bBeginSelect )
			{
				ErrorBeep();
				return;
			}

			// 現在のフォントは固定幅フォントである
			if( !m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH )
			{
				return;
			}
		}
		else
		// クリップボードからのデータは普通に扱う。
		{
			bColmnSelect = FALSE;
		}
	}
	else
	{
		// 2004.05.14 Moca 全置換の途中で他のウィンドウで置換されるとまずいのでコピーする
		cmemClip.SetString( m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[0] );
	}

	szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);

	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if( bLineSelect ){
		if( szREPLACEKEY[nREPLACEKEY - 1] != CR && szREPLACEKEY[nREPLACEKEY - 1] != LF ){
			cmemClip.AppendString(m_pcEditDoc->GetNewLineCode().GetValue());
			szREPLACEKEY = cmemClip.GetStringPtr( &nREPLACEKEY );
		}
	}

	if( m_pShareData->m_Common.m_sEdit.m_bConvertEOLPaste ){
		char *pszConvertedText = new char[nREPLACEKEY * 2]; // 全文字\n→\r\n変換で最大の２倍になる
		int nConvertedTextLen = ConvertEol(szREPLACEKEY, nREPLACEKEY, pszConvertedText);
		cmemClip.SetString(pszConvertedText, nConvertedTextLen);
		szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);
		delete [] pszConvertedText;
	}

	// 取得にステップがかかりそうな変数などを、一時変数化する。
	// とはいえ、これらの操作をすることによって得をするクロック数は合わせても 1 ループで数十だと思います。
	// 数百クロック毎ループのオーダーから考えてもそんなに得はしないように思いますけど・・・。
	bool bAddCRLFWhenCopy = m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy;
	BOOL &bCANCEL = cDlgCancel.m_bCANCEL;
	CDocLineMgr& rDocLineMgr = m_pcEditDoc->m_cDocLineMgr;
	CLayoutMgr& rLayoutMgr = m_pcEditDoc->m_cLayoutMgr;

	//  クラス関係をループの中で宣言してしまうと、毎ループごとにコンストラクタ、デストラクタが
	// 呼ばれて遅くなるので、ここで宣言。
	CMemory cmemory;
	CBregexp cRegexp;
	// 初期化も同様に毎ループごとにやると遅いので、最初に済ましてしまう。
	if( bRegularExp )
	{
		if ( !InitRegexp( m_hWnd, cRegexp, true ) )
		{
			return;
		}

		const CMemory	cMemRepKey( szREPLACEKEY, _tcslen(szREPLACEKEY) );
		// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
		CMemory cMemRepKey2;
		CMemory cMemMatchStr = CMemory(_T("$&"), _tcslen(_T("$&")));
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
		int nFlag = (m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase ? CBregexp::optCaseSensitive : CBregexp::optNothing);
		nFlag |= (bConsecutiveAll ? CBregexp::optNothing : CBregexp::optGlobal);	// 2007.01.16 ryoji
		cRegexp.Compile(m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0], cMemRepKey2.GetStringPtr(), nFlag);
	}

	/* テキストが選択されているか */
	while( IsTextSelected() )
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
			nNewPos = m_nSelectLineFrom >> nShiftCount;
			::PostMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
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
				lineCnt = rLayoutMgr.GetLineCount();
				// 検索後の範囲終端
				colOld = m_nSelectColmTo;
				linOld = m_nSelectLineTo;
				// 前回の検索行と違う？
				if(linOld!=linPrev){
					colDif=0;
				}
				linPrev=linOld;
				// 行は範囲内？
				if ((linTo+linDif == linOld && colTo+colDif < colOld) ||
					(linTo+linDif <  linOld)) {
					break;
				}
				// 桁は範囲内？
				if(!((colFrom<=m_nSelectColmFrom)&&
					 (colOld<=colTo+colDif))){
					if(colOld<colTo+colDif){
						linNext=m_nSelectLineTo;
					}else{
						linNext=m_nSelectLineTo+1;
					}
					//次の検索開始位置へシフト
					m_nCaretPosX=colFrom;
					m_nCaretPosY=linNext;
					// 2004.05.30 Moca 現在の検索文字列を使って検索する
					Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
					colDif=0;
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
				rLayoutMgr.LayoutToLogic(
					m_nSelectColmTo,
					m_nSelectLineTo,
					&colOld,
					&linOld
				);

				// 置換前の行の長さ(改行は１文字と数える)を保存しておいて、置換前後で行位置が変わった場合に使用
				linOldLen = rDocLineMgr.GetLine(linOld)->GetLengthWithoutEOL() + 1;

				// 行は範囲内？
				// 2007.01.19 ryoji 条件追加: 選択終点が行頭(colToP == 0)になっている場合は前の行の行末までを選択範囲とみなす
				// （選択始点が行頭ならその行頭は選択範囲に含み、終点が行頭ならその行頭は選択範囲に含まない、とする）
				// 論理的に少し変と指摘されるかもしれないが、実用上はそのようにしたほうが望ましいケースが多いと思われる。
				// ※行選択で行末までを選択範囲にしたつもりでも、UI上は次の行の行頭にカーソルが行く
				// ※終点の行頭を「^」にマッチさせたかったら１文字以上選択してね、ということで．．．
				if ((linToP+linDif == linOld && (colToP+colDif < colOld || colToP == 0))
					|| linToP+linDif < linOld) {
					break;
				}
			}
		}

		colTmp = 0;
		linTmp = 0;
		if ( nPaste || !bRegularExp ) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if( nReplaceTarget == 1 )	//挿入位置セット
			{
				colTmp = m_nSelectColmTo - m_nSelectColmFrom;
				linTmp = m_nSelectLineTo - m_nSelectLineFrom;
                m_nSelectColmFrom=-1;
                m_nSelectLineFrom=-1;
                m_nSelectColmTo	 =-1;
                m_nSelectLineTo	 =-1;
			}
			else if( nReplaceTarget == 2 )	//追加位置セット
			{
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				MoveCursor( m_nSelectColmTo, m_nSelectLineTo, false );
			    m_nSelectColmFrom=-1;
			    m_nSelectLineFrom=-1;
			    m_nSelectColmTo	 =-1;
			    m_nSelectLineTo	 =-1;
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
				** →m_nSelectXXXが-1の時に ReplaceData_CEditViewを直接たたくと動作不良となるため
				**   直接たたくのやめた。2003.05.18 by かろと
				*/
				Command_INSTEXT( false, szREPLACEKEY, nREPLACEKEY, TRUE, bLineSelect );
			}
			else
			{
				Command_PASTEBOX(szREPLACEKEY, nREPLACEKEY);
				// 再描画を行わないとどんな結果が起きているのか分からずみっともないので・・・。
				AdjustScrollBars(); // 2007.07.22 ryoji
				Redraw();
			}
			++nReplaceNum;
		}
		// 2002/01/19 novice 正規表現による文字列置換
		else if( bRegularExp ) /* 検索／置換  1==正規表現 */
		{
			// 物理行、物理行長、物理行での検索マッチ位置
			const CLayout* pcLayout = rLayoutMgr.SearchLineByLayoutY(m_nSelectLineFrom);
			const char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nIdx = LineColmnToIndex( pcLayout, m_nSelectColmFrom ) + pcLayout->m_nOffset;
			int nLen = pcLayout->m_pCDocLine->GetLength();
			int colDiff = 0;
			if( !bConsecutiveAll ){	// 一括置換
				// 2007.01.16 ryoji
				// 選択範囲置換の場合は行内の選択範囲末尾まで置換範囲を縮め，
				// その位置を記憶する．
				if( bSelectedArea ){
					if( bBeginBoxSelect ){	// 矩形選択
						int wk;
						rLayoutMgr.LayoutToLogic(
							colTo,
							linOld,
							&colToP,
							&wk
						);
						if( nLen - pcLayout->m_pCDocLine->m_cEol.GetLen() > colToP + colDif )
							nLen = colToP + colDif;
					} else {	// 通常の選択
						if( linToP+linDif == linOld ){
							if( nLen - pcLayout->m_pCDocLine->m_cEol.GetLen() > colToP + colDif )
								nLen = colToP + colDif;
						}
					}
				}

				if(pcLayout->m_pCDocLine->GetLengthWithoutEOL() < nLen)
					colOld = pcLayout->m_pCDocLine->GetLengthWithoutEOL() + 1;
				else
					colOld = nLen;
			}

			if( int nReplace = cRegexp.Replace(pLine, nLen, nIdx) ){
				nReplaceNum += nReplace;
				if ( !bConsecutiveAll ) { // 2006.04.01 かろと	// 2007.01.16 ryoji
					// 行単位での置換処理
					// 選択範囲を物理行末までにのばす
					rLayoutMgr.LogicToLayout( nLen, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );
				} else {
				    // From Here Jun. 6, 2005 かろと
				    // 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				    // キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				    // そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				    int matchLen = cRegexp.GetMatchLen();
				    int nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				    if (matchLen == 0) {
					    // ０文字マッチの時(無限置換にならないように１文字進める)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += (CMemory::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // 無限置換しないように、１文字増やしたので１文字選択に変更
					    // 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
						rLayoutMgr.LogicToLayout( nIdxTo, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji 行位置も取得する
				    }
				    // 行末から検索文字列末尾までの文字数
					colDiff =  nLen - nIdxTo;
					colOld = nIdxTo;	// 2007.01.19 ryoji 追加
				    //	Oct. 22, 2005 Karoto
				    //	\rを置換するとその後ろの\nが消えてしまう問題の対応
				    if (colDiff < pcLayout->m_pCDocLine->m_cEol.GetLen()) {
					    // 改行にかかっていたら、行全体をINSTEXTする。
					    colDiff = 0;
						rLayoutMgr.LogicToLayout( nLen, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji 追加
						colOld = pcLayout->m_pCDocLine->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji 追加
				    }
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT( false, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 かろと
			}
		}
		else
		{
			/* 本当は元コードを使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
			** →m_nSelectXXXが-1の時に ReplaceData_CEditViewを直接たたくと動作不良となるため直接たたくのやめた。2003.05.18 かろと
			*/
			Command_INSTEXT( false, szREPLACEKEY, nREPLACEKEY, TRUE );
			++nReplaceNum;
		}

		// 挿入後の位置調整
		if( nReplaceTarget == 1 )
		{
			m_nCaretPosX+=colTmp;
			m_nCaretPosY+=linTmp;
			if (!bBeginBoxSelect)
			{
				rLayoutMgr.LayoutToLogic(
					m_nCaretPosX,
					m_nCaretPosY,
					&m_ptCaretPos_PHY.x,
					&m_ptCaretPos_PHY.y
				);
			}
		}

		// 最後に置換した位置を記憶
		colLast = m_nCaretPosX;
		linLast = m_nCaretPosY;

		/* 置換後の位置を確認 */
		if( bSelectedArea )
		{
			// 検索→置換の行補正値取得
			if( bBeginBoxSelect )
			{
				colDif += colLast - colOld;
				linDif += rLayoutMgr.GetLineCount() - lineCnt;
			}
			else{
				// 置換前の検索文字列の最終位置は colOld, linOld
				// 置換後のカーソル位置
				colTmp = m_ptCaretPos_PHY.x;
				linTmp = m_ptCaretPos_PHY.y;
				int linDif_thistime = rDocLineMgr.GetLineCount() - lineCnt;	// 今回置換での行数変化
				linDif += linDif_thistime;
				if( linToP + linDif == linTmp)
				{
					// 最終行で置換した時、又は、置換の結果、選択エリア最終行まで到達した時
					// 最終行なので、置換前後の文字数の増減で桁位置を調整する
					colDif += colTmp - colOld;

					// 但し、以下の場合は置換前後で行が異なってしまうので、行の長さで補正する必要がある
					// １）最終行直前で行連結が起こり、行が減っている場合（行連結なので、桁位置は置換後のカーソル桁位置分増加する）
					// 　　colTmp-colOldだと、\r\n → "" 置換で行連結した場合に、桁位置が負になり失敗する（負とは前行の後ろの方になることなので補正する）
					// 　　今回置換での行数の変化(linDif_thistime)で、最終行が行連結されたかどうかを見ることにする
					// ２）改行を置換した（linTmp!=linOld）場合、改行を置換すると置換後の桁位置が次行の桁位置になっているため
					//     colTmp-colOldだと、負の数となり、\r\n → \n や \n → "abc" などで桁位置がずれる
					//     これも前行の長さで調整する必要がある
					if (linDif_thistime < 0 || linTmp != linOld) {
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
		nNewPos = m_nSelectLineFrom >> nShiftCount;
		::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	//>> 2002/03/26 Azumaiya

	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	if( !cDlgCancel.IsCanceled() ){
		nNewPos = nAllLineNum;
		::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	cDlgCancel.CloseDialog( 0 );
	::EnableWindow( m_hWnd, TRUE );
	::EnableWindow( ::GetParent( m_hWnd ), TRUE );
	::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), TRUE );

	// From Here 2001.12.03 hor

	/* テキスト選択解除 */
	DisableSelectArea( true );

	/* カーソル・選択範囲復元 */
	if((!bSelectedArea) ||			// ファイル全体置換
	   (cDlgCancel.IsCanceled())) {		// キャンセルされた
		// 最後に置換した文字列の右へ
		MoveCursor( colLast, linLast, true );
	}
	else{
		if (bBeginBoxSelect) {
			// 矩形選択
			m_bBeginBoxSelect=bBeginBoxSelect;
			linTo+=linDif;
			if(linTo<0)linTo=0;
		}
		else{
			// 普通の選択
			colToP+=colDif;
			if(colToP<0)colToP=0;
			linToP+=linDif;
			if(linToP<0)linToP=0;
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				colToP,
				linToP,
				&colTo,
				&linTo
			);
		}
		if(linFrom<linTo || colFrom<colTo){
			SetSelectArea( linFrom, colFrom, linTo, colTo );	// 2009.07.25 ryoji
		}
		MoveCursor( colTo, linTo, true );
		m_nCaretPosX_Prev = m_nCaretPosX;	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	m_pcEditDoc->m_cDlgReplace.m_bCanceled = (cDlgCancel.IsCanceled() != FALSE);
	m_pcEditDoc->m_cDlgReplace.m_nReplaceCnt=nReplaceNum;
	m_bDrawSWITCH = true;
	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
}



/* カーソル行をウィンドウ中央へ */
void CEditView::Command_CURLINECENTER( void )
{
	int		nViewTopLine;
	nViewTopLine = m_nCaretPosY - ( m_nViewRowNum / 2 );

	// sui 02/08/09
	if( 0 > nViewTopLine )	nViewTopLine = 0;
	
	int nScrollLines = nViewTopLine - m_nViewTopLine;	//Sep. 11, 2004 genta 同期用に行数を記憶
	m_nViewTopLine = nViewTopLine;
	/* フォーカス移動時の再描画 */
	RedrawAll();
	// sui 02/08/09

	//	Sep. 11, 2004 genta 同期スクロールの関数化
	SyncScrollV( nScrollLines );
}




/* Base64デコードして保存 */
void CEditView::Command_BASE64DECODE( void )
{
	CMemory		cmemBuf;
	char		szPath[_MAX_PATH];
	HFILE		hFile;

	/* テキストが選択されているか */
	if( !IsTextSelected() ){
		ErrorBeep();
		return;
	}
	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
	if( !GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}

	/* Base64デコード */
	cmemBuf.BASE64Decode();
	strcpy( szPath, _T("") );

	/* 保存ダイアログ モーダルダイアログの表示 */
	if( !m_pcEditDoc->SaveFileDialog( (char*)szPath,  NULL ) ){
		return;
	}
	if(HFILE_ERROR == (hFile = _lcreat( szPath, 0 ) ) ){
		ErrorBeep();
		ErrorMessage( m_hWnd, _T("ファイルの作成に失敗しました。\n\n%s"), szPath );
		return;
	}
	if( HFILE_ERROR == _lwrite( hFile, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() ) ){
		ErrorBeep();
		ErrorMessage( m_hWnd, _T("ファイルの書き込みに失敗しました。\n\n%s"), szPath );
	}
	_lclose( hFile );
	return;
}




/* uudecodeして保存 */
void CEditView::Command_UUDECODE( void )
{
	CMemory		cmemBuf;
	char		szPath[_MAX_PATH];
	HFILE		hFile;
	/* テキストが選択されているか */
	if( !IsTextSelected() ){
		ErrorBeep();
		return;
	}

	// 選択範囲のデータを取得
	// 正常時はTRUE,範囲未選択の場合はFALSEを返す
	if( !GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}
	strcpy( szPath, _T("") );

	// uudecode(デコード) 	//Oct. 17, 2000 jepro 説明を「UUENCODE復号化(デコード) 」から変更
	cmemBuf.UUDECODE( szPath );

	/* 保存ダイアログ モーダルダイアログの表示 */
	if( !m_pcEditDoc->SaveFileDialog( (char*)szPath,  NULL ) ){
		return;
	}
	if(HFILE_ERROR == (hFile = _lcreat( szPath, 0 ) ) ){
		ErrorBeep();
		ErrorMessage( m_hWnd, _T("ファイルの作成に失敗しました。\n\n%s"), szPath );
		return;
	}
	if( HFILE_ERROR == _lwrite( hFile, cmemBuf.GetStringPtr(), cmemBuf.GetStringLength() ) ){
		ErrorBeep();
		ErrorMessage( m_hWnd, _T("ファイルの書き込みに失敗しました。\n\n%s"), szPath );
	}
	_lclose( hFile );
	return;
}




/* 再描画 */
void CEditView::Command_REDRAW( void )
{
	/* フォーカス移動時の再描画 */
	RedrawAll();
	return;
}




/* Oracle SQL*Plusで実行 */
void CEditView::Command_PLSQL_COMPILE_ON_SQLPLUS( void )
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
		ErrorMessage( m_hWnd, _T("Oracle SQL*Plusで実行します。\n\n\nOracle SQL*Plusが起動されていません。\n") );
		return;
	}
	/* テキストが変更されている場合 */
	if( m_pcEditDoc->IsModified() ){
		nRet = ::MYMESSAGEBOX(
			m_hWnd,
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			_T("%s\nは変更されています。 Oracle SQL*Plusで実行する前に保存しますか？"),
			m_pcEditDoc->IsValidPath() ? m_pcEditDoc->GetFilePath() : _T("(無題)")
		);
		switch( nRet ){
		case IDYES:
			if( m_pcEditDoc->IsValidPath() ){
				//nBool = HandleCommand( F_FILESAVE, true, 0, 0, 0, 0 );
				nBool = Command_FILESAVE();
			}else{
				//nBool = HandleCommand( F_FILESAVEAS_DIALOG, true, 0, 0, 0, 0 );
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
	if( m_pcEditDoc->IsValidPath() ){
		/* ファイルパスに空白が含まれている場合はダブルクォーテーションで囲む */
		//	2003.10.20 MIK コード簡略化
		if( _tcschr( m_pcEditDoc->GetFilePath(), SPACE ) ? TRUE : FALSE ){
			wsprintf( szPath, _T("@\"%s\"\r\n"), m_pcEditDoc->GetFilePath() );
		}else{
			wsprintf( szPath, _T("@%s\r\n"), m_pcEditDoc->GetFilePath() );
		}
		/* クリップボードにデータを設定 */
		MySetClipboardData( szPath, lstrlen( szPath ), false );

		/* Oracle SQL*Plusをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( hwndSQLPLUS );

		/* Oracle SQL*Plusにペーストのコマンドを送る */
		ULONG_PTR	dwResult;
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
			TopErrorMessage( m_hWnd, _T("Oracle SQL*Plusからの反応がありません。\nしばらく待ってから再び実行してください。") );
		}
	}else{
		ErrorBeep();
		ErrorMessage( m_hWnd, _T("SQLをファイルに保存しないとOracle SQL*Plusで実行できません。\n") );
		return;
	}
	return;
}




/* Oracle SQL*Plusをアクティブ表示 */
void CEditView::Command_ACTIVATE_SQLPLUS( void )
{
	HWND		hwndSQLPLUS;
	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_hWnd, _T("Oracle SQL*Plusをアクティブ表示します。\n\n\nOracle SQL*Plusが起動されていません。\n") );
		return;
	}
	/* Oracle SQL*Plusをアクティブにする */
	/* アクティブにする */
	ActivateFrameWindow( hwndSQLPLUS );
	return;
}




/* 読み取り専用 */
void CEditView::Command_READONLY( void )
{
	m_pcEditDoc->m_bReadOnly = !m_pcEditDoc->m_bReadOnly;

	// 親ウィンドウのタイトルを更新
	m_pcEditDoc->UpdateCaption();
}

/* ファイルのプロパティ */
void CEditView::Command_PROPERTY_FILE( void )
{
#ifdef _DEBUG
	{
		/* 全行データを返すテスト */
		char*	pDataAll;
		int		nDataAllLen;
		CRunningTimer cRunningTimer( "CEditView::Command_PROPERTY_FILE 全行データを返すテスト" );
		cRunningTimer.Reset();
		pDataAll = m_pcEditDoc->m_cDocLineMgr.GetAllData( &nDataAllLen );
//		MYTRACE( _T("全データ取得             (%dバイト) 所要時間(ミリ秒) = %d\n"), nDataAllLen, cRunningTimer.Read() );
		free( pDataAll );
		pDataAll = NULL;
//		MYTRACE( _T("全データ取得のメモリ開放 (%dバイト) 所要時間(ミリ秒) = %d\n"), nDataAllLen, cRunningTimer.Read() );
	}
#endif


	CDlgProperty	cDlgProperty;
//	cDlgProperty.Create( m_hInstance, m_hWnd, (void *)m_pcEditDoc );
	cDlgProperty.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc );
	return;
}




/* 編集の全終了 */	// 2007.02.13 ryoji 追加
void CEditView::Command_EXITALLEDITORS( void )
{
	CControlTray::CloseAllEditor( TRUE, ::GetParent(m_hwndParent), TRUE, 0 );
	return;
}

/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加
void CEditView::Command_EXITALL( void )
{
	CControlTray::TerminateApplication( ::GetParent(m_hwndParent) );	// 2006.12.25 ryoji 引数追加
	return;
}




/* グループを閉じる */	// 2007.06.20 ryoji 追加
void CEditView::Command_GROUPCLOSE( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		int nGroup = CShareData::getInstance()->GetGroupId( ::GetParent(m_hwndParent) );
		CControlTray::CloseAllEditor( TRUE, ::GetParent(m_hwndParent), TRUE, nGroup );
	}
	return;
}

/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
void CEditView::Command_FILECLOSEALL( void )
{
	int nGroup = CShareData::getInstance()->GetGroupId( ::GetParent(m_hwndParent) );
	CControlTray::CloseAllEditor( TRUE, ::GetParent(m_hwndParent), FALSE, nGroup );	// 2006.12.25, 2007.02.13 ryoji 引数追加
	return;
}




/* ウィンドウを閉じる */
void CEditView::Command_WINCLOSE( void )
{
	/* 閉じる */
	::PostMessage( ::GetParent( m_hwndParent ), MYWM_CLOSE, FALSE, 0 );	// 2007.02.13 ryoji WM_CLOSE→MYWM_CLOSEに変更
	return;
}

//アウトプットウィンドウ表示
void CEditView::Command_WIN_OUTPUT( void )
{
	if( NULL == m_pShareData->m_sHandles.m_hwndDebug
		|| !IsSakuraMainWindow( m_pShareData->m_sHandles.m_hwndDebug )
	){
		CControlTray::OpenNewEditor( NULL, m_hWnd, "-DEBUGMODE", CODE_SJIS, false, true );
	}else{
		/* 開いているウィンドウをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
	}
	return;
}




/* カスタムメニュー表示 */
int CEditView::Command_CUSTMENU( int nMenuIdx )
{
	HMENU		hMenu;
	int			nId;
	POINT		po;
	int			i;
	char		szLabel[300];
	char		szLabel2[300];
	UINT		uFlags;

	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();
	
	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	if( nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx ){
		return 0;
	}
	if( 0 == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] ){
		return 0;
	}
	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( F_0 == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, F_0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
//			::LoadString( m_hInstance, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* キー */
			if( '\0' == m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				strcpy( szLabel2, szLabel );
			}else{
				wsprintf( szLabel2, "%s (&%c)",
					szLabel,
					m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i]
				);
			}
			/* 機能が利用可能か調べる */
			if( IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
			pCEditWnd->m_CMenuDrawer.MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2, _T("") );
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
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}




//選択範囲内全行コピー
void CEditView::Command_COPYLINES( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	CopySelectedAllLines(
		NULL,	/* 引用符 */
		FALSE	/* 行番号を付与する */
	);
	return;
}




//選択範囲内全行引用符付きコピー
void CEditView::Command_COPYLINESASPASSAGE( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	CopySelectedAllLines(
		m_pShareData->m_Common.m_sFormat.m_szInyouKigou,	/* 引用符 */
		FALSE 									/* 行番号を付与する */
	);
	return;
}




//選択範囲内全行行番号付きコピー
void CEditView::Command_COPYLINESWITHLINENUMBER( void )
{
	/* 選択範囲内の全行をクリップボードにコピーする */
	CopySelectedAllLines(
		NULL,	/* 引用符 */
		TRUE	/* 行番号を付与する */
	);
	return;
}




////キー割り当て一覧をコピー
	//Dec. 26, 2000 JEPRO //Jan. 24, 2001 JEPRO debug version (directed by genta)
void CEditView::Command_CREATEKEYBINDLIST( void )
{
	CMemory		cMemKeyList;

	CKeyBind::CreateKeyBindList(
		m_hInstance,
		m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
		m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
		cMemKeyList,
		&m_pcEditDoc->m_cFuncLookup,	//	Oct. 31, 2001 genta 追加
		FALSE	// 2007.02.22 ryoji 追加
	);

	// Windowsクリップボードにコピー
	//2004.02.17 Moca 関数化
	SetClipboardText( m_pcEditDoc->m_hWnd, cMemKeyList.GetStringPtr(), cMemKeyList.GetStringLength() );
}

/* ファイル内容比較 */
void CEditView::Command_COMPARE( void )
{
	HWND		hwndCompareWnd;
	TCHAR		szPath[_MAX_PATH + 1];
	POINT		poSrc;
	POINT		poDes;
	CDlgCompare	cDlgCompare;
	BOOL		bDefferent;
	const char*	pLineSrc;
	int			nLineLenSrc;
	const char*	pLineDes;
	int			nLineLenDes;
	POINT*		ppoCaretDes;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK

	/* 比較後、左右に並べて表示 */
	cDlgCompare.m_bCompareAndTileHorz = m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz;
	BOOL bDlgCompareResult = cDlgCompare.DoModal(
		m_hInstance,
		m_hWnd,
		(LPARAM)m_pcEditDoc,
		m_pcEditDoc->GetFilePath(),
		m_pcEditDoc->IsModified(),
		szPath,
		&hwndCompareWnd
	);
	if( !bDlgCompareResult ){
		return;
	}
	/* 比較後、左右に並べて表示 */
	m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;

	//タブウインドウ時は禁止	//@@@ 2003.06.12 MIK
	if( TRUE  == m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
	 && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		hwndMsgBox = m_hWnd;
		m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz = FALSE;
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
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_nCaretPosX,
		m_nCaretPosY,
		(int*)&poSrc.x,
		(int*)&poSrc.y
	);
	// カーソル位置取得要求
	{
		::SendMessage( hwndCompareWnd, MYWM_GETCARETPOS, 0, 0 );
		ppoCaretDes = (POINT*)m_pShareData->m_sWorkBuffer.m_szWork;
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	bDefferent = TRUE;
	pLineDes = m_pShareData->m_sWorkBuffer.m_szWork;
	pLineSrc = m_pcEditDoc->m_cDocLineMgr.GetLineStr( poSrc.y, &nLineLenSrc );
	/* 行(改行単位)データの要求 */
	nLineLenDes = ::SendMessage( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	while( 1 ){
		if( pLineSrc == NULL &&	0 == nLineLenDes ){
			bDefferent = FALSE;
			break;
		}
		if( pLineSrc == NULL || 0 == nLineLenDes ){
			break;
		}
		if( nLineLenDes > sizeof( m_pShareData->m_sWorkBuffer.m_szWork ) ){
			TopErrorMessage( m_hWnd,
				_T("比較先のファイル\n%s\n%dバイトを超える行があります。\n")
				_T("比較できません。"),
				szPath,
				sizeof( m_pShareData->m_sWorkBuffer.m_szWork )
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
		pLineSrc = m_pcEditDoc->m_cDocLineMgr.GetLineStr( poSrc.y, &nLineLenSrc );
		/* 行(改行単位)データの要求 */
		nLineLenDes = ::SendMessage( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	}
end_of_compare:;
	/* 比較後、左右に並べて表示 */
//From Here Oct. 10, 2000 JEPRO	チェックボックスをボタン化すれば以下の行(To Here まで)は不要のはずだが
//	うまくいかなかったので元に戻してある…
	if( m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz ){
		HWND* phwndArr = new HWND[2];
		phwndArr[0] = ::GetParent( m_hwndParent );
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
		TopInfoMessage( hwndMsgBox, _T("異なる箇所が見つかりました。") );
		/* カーソルを移動させる
			比較相手は、別プロセスなのでメッセージを飛ばす。
		*/
		memcpy( m_pShareData->m_sWorkBuffer.m_szWork, (void*)&poDes, sizeof( poDes ) );
		::SendMessage( hwndCompareWnd, MYWM_SETCARETPOS, 0, 0 );

		/* カーソルを移動させる */
		memcpy( m_pShareData->m_sWorkBuffer.m_szWork, (void*)&poSrc, sizeof( poSrc ) );
		::PostMessage( ::GetParent( m_hwndParent ), MYWM_SETCARETPOS, 0, 0 );
	}

	/* 開いているウィンドウをアクティブにする */
	/* アクティブにする */
	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
	return;
}




/*! ツールバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutToolBar(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CEditView::Command_SHOWTOOLBAR( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR = ((NULL == pCEditWnd->m_hwndToolBar)? TRUE: FALSE);	/* ツールバー表示 */
	pCEditWnd->LayoutToolBar();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TOOLBAR,
		(LPARAM)pCEditWnd->m_hWnd,
		pCEditWnd->m_hWnd
	);
}




/*! ステータスバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutStatusBar(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CEditView::Command_SHOWSTATUSBAR( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR = ((NULL == pCEditWnd->m_hwndStatusBar)? TRUE: FALSE);	/* ステータスバー表示 */
	pCEditWnd->LayoutStatusBar();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_STATUSBAR,
		(LPARAM)pCEditWnd->m_hWnd,
		pCEditWnd->m_hWnd
	);
}




/*! ファンクションキーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutFuncKey(), CEditWnd::EndLayoutBars() で行うように変更
*/
void CEditView::Command_SHOWFUNCKEY( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND = ((NULL == pCEditWnd->m_CFuncKeyWnd.m_hWnd)? TRUE: FALSE);	/* ファンクションキー表示 */
	pCEditWnd->LayoutFuncKey();
	pCEditWnd->EndLayoutBars();

	//全ウインドウに変更を通知する。
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_FUNCKEY,
		(LPARAM)pCEditWnd->m_hWnd,
		pCEditWnd->m_hWnd
	);
}

//@@@ From Here 2003.06.10 MIK
/*! タブ(ウインドウ)の表示/非表示

	@author MIK
	@date 2003.06.10 新規作成
	@date 2006.12.19 ryoji 表示切替は CEditWnd::LayoutTabBar(), CEditWnd::EndLayoutBars() で行うように変更
	@date 2007.06.20 ryoji グループIDリセット
 */
void CEditView::Command_SHOWTAB( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd = ((NULL == pCEditWnd->m_cTabWnd.m_hWnd)? TRUE: FALSE);	/* タブバー表示 */
	pCEditWnd->LayoutTabBar();
	pCEditWnd->EndLayoutBars();

	// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		m_pcEditDoc->m_pcEditWnd->WindowTopMost(
			( (DWORD)::GetWindowLongPtr( m_pcEditDoc->m_pcEditWnd->m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
		);
	}

	//全ウインドウに変更を通知する。
	CShareData::getInstance()->ResetGroupId();
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TAB,
		(LPARAM)pCEditWnd->m_hWnd,
		pCEditWnd->m_hWnd
	);
}
//@@@ To Here 2003.06.10 MIK



/* 印刷 */
void CEditView::Command_PRINT( void )
{
	// 使っていない処理を削除 2003.05.04 かろと
	Command_PRINT_PREVIEW();
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* 印刷実行 */
	pCEditWnd->m_pPrintPreview->OnPrint();
}




/* 印刷プレビュー */
void CEditView::Command_PRINT_PREVIEW( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* 印刷プレビューモードのオン/オフ */
	pCEditWnd->PrintPreviewModeONOFF();
	return;
}




/* 印刷のページレイアウトの設定 */
void CEditView::Command_PRINT_PAGESETUP( void )
{
	BOOL		bRes;
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* 印刷ページ設定 */
	bRes = pCEditWnd->OnPrintPageSetting();
	return;
}




/* ブラウズ */
void CEditView::Command_BROWSE( void )
{
	if( !m_pcEditDoc->IsValidPath() ){
		ErrorBeep();
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	wsprintf( szURL, "%s", m_pcEditDoc->GetFilePath() );
	/* URLを開く */
//	::ShellExecuteEx( NULL, "open", szURL, NULL, NULL, SW_SHOW );

    SHELLEXECUTEINFO info; 
    info.cbSize =sizeof(SHELLEXECUTEINFO);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = m_pcEditDoc->GetFilePath();
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
void CEditView::Command_RECKEYMACRO( void )
{
	if( m_pShareData->m_sFlags.m_bRecordingKeyMacro ){									/* キーボードマクロの記録中 */
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;							/* キーボードマクロを記録中のウィンドウ */
		//@@@ 2002.1.24 YAZAKI キーマクロをマクロ用フォルダに「RecKey.mac」という名で保存
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca 記録用キーマクロのフルパスをCShareData経由で取得
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			ErrorMessage( m_hWnd, _T("マクロファイルを作成できませんでした。\nファイル名の取得エラー nRet=%d"), nRet );
			return;
		}else{
			_tcscpy( m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		int nSaveResult=m_pcEditDoc->m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			m_hInstance,
			m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName
		);
		if ( !nSaveResult ){
			ErrorMessage(	m_hWnd, _T("マクロファイルを作成できませんでした。\n\n%s"), m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName );
		}
	}else{
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = TRUE;
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = ::GetParent( m_hwndParent );	/* キーボードマクロを記録中のウィンドウ */
		/* キーマクロのバッファをクリアする */
		//@@@ 2002.1.24 m_CKeyMacroMgrをCEditDocへ移動
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		m_pcEditDoc->m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		m_pcEditDoc->m_CKeyMacroMgr.ClearAll();
//		m_pShareData->m_CKeyMacroMgr.Clear();
	}
	/* 親ウィンドウのタイトルを更新 */
	m_pcEditDoc->UpdateCaption();

	/* キャレットの行桁位置を表示する */
	DrawCaretPosInfo();
}




/* キーマクロの保存 */
void CEditView::Command_SAVEKEYMACRO( void )
{
	m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;
	m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	//	Jun. 16, 2002 genta
	if( !m_pcEditDoc->m_pcSMacroMgr->IsSaveOk() ){
		//	保存不可
		ErrorMessage( m_hWnd, _T("保存可能なマクロがありません．キーボードマクロ以外は保存できません．") );
	}

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	_tcscpy( szPath, _T("") );
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ) ){
		GetInidirOrExedir( szInitDir, m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER );
	}else{
		_tcscpy( szInitDir, m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		m_hWnd,
		_T("*.mac"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, NULL );
//	strcat( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, "\\" );

	/* キーボードマクロの保存 */
	//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
	//@@@ 2002.1.24 YAZAKI
	if ( !m_pcEditDoc->m_pcSMacroMgr->Save( STAND_KEYMACRO, m_hInstance, szPath ) ){
		ErrorMessage( m_hWnd, _T("マクロファイルを作成できませんでした。\n\n%s"), szPath );
	}
	return;
}




/* キーマクロの実行 */
void CEditView::Command_EXECKEYMACRO( void )
{
	//@@@ 2002.1.24 YAZAKI 記録中は終了してから実行
	if (m_pShareData->m_sFlags.m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;
	m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	/* キーボードマクロの実行 */
	//@@@ 2002.1.24 YAZAKI
	if ( m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName[0] ){
		//	ファイルが保存されていたら
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		BOOL bLoadResult = m_pcEditDoc->m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			m_hInstance,
			m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName,
			NULL
		);
		if ( !bLoadResult ){
			ErrorMessage( m_hWnd, _T("ファイルを開けませんでした。\n\n%s"), m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName );
		}
		else {
			//	2007.07.20 genta : flagsオプション追加
			m_pcEditDoc->m_pcSMacroMgr->Exec( STAND_KEYMACRO, m_hInstance, this, 0 );
		}
	}

	/* フォーカス移動時の再描画 */
	RedrawAll();

	return;
}




/*! キーマクロの読み込み
	@date 2005.02.20 novice デフォルトの拡張子変更
 */
void CEditView::Command_LOADKEYMACRO( void )
{
	m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;
	m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	const TCHAR*		pszFolder;
	_tcscpy( szPath, _T("") );
	pszFolder = m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( pszFolder ) ){
		GetInidirOrExedir( szInitDir, pszFolder );
	}else{
		_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		m_hWnd,
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
	_tcscpy(m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName, szPath);
//	m_pShareData->m_CKeyMacroMgr.LoadKeyMacro( m_hInstance, m_hWnd, szPath );
	return;
}


/*! 名前を指定してマクロ実行
	@param pszPath	マクロのファイルパス、またはマクロのコード。
	@param pszType	種別。NULLの場合ファイル指定、それ以外の場合は言語の拡張子を指定

	@date 2008.10.23 syat 新規作成
	@date 2009.07.19 syat pszType追加
 */
void CEditView::Command_EXECEXTMACRO( const char* pszPath, const char* pszType )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];	//ファイル選択ダイアログの初期フォルダ
	const TCHAR*	pszFolder;					//マクロフォルダ
	HWND			hwndRecordingKeyMacro = NULL;
	strcpy( szPath, "" );

	if( pszType == NULL && pszPath == NULL ) {
		// ファイルが指定されていない場合、ダイアログを表示する
		pszFolder = m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER;

		if( _IS_REL_PATH( pszFolder ) ){
			GetInidirOrExedir( szInitDir, pszFolder );
		}else{
			_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
		}
		/* ファイルオープンダイアログの初期化 */
		cDlgOpenFile.Create(
			m_hInstance,
			m_hWnd,
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
	if( m_pShareData->m_sFlags.m_bRecordingKeyMacro &&									/* キーボードマクロの記録中 */
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == ::GetParent( m_hwndParent )	/* キーボードマクロを記録中のウィンドウ */
	){
		m_pcEditDoc->m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, (LPARAM)pszPath, this );

		//キーマクロの記録を一時停止する
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = m_pShareData->m_sFlags.m_hwndRecordingKeyMacro;
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */
	}

	//古い一時マクロの退避
	CMacroManagerBase* oldMacro = m_pcEditDoc->m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = m_pcEditDoc->m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		m_hInstance,
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_hWnd, _T("マクロの読み込みに失敗しました。\n\n%s"), pszPath );
	}
	else {
		m_pcEditDoc->m_pcSMacroMgr->Exec( TEMP_KEYMACRO, m_hInstance, this, FA_NONRECORD | FA_FROMMACRO );
	}

	// 終わったら開放
	m_pcEditDoc->m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		m_pcEditDoc->m_pcSMacroMgr->SetTempMacro( oldMacro );
	}

	// キーマクロ記録中だった場合は再開する
	if ( hwndRecordingKeyMacro != NULL ) {
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = TRUE;
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
	}

	/* フォーカス移動時の再描画 */
	RedrawAll();

	return;
}


/*!	@brief 折り返しの動作を決定

	トグルコマンド「現在のウィンドウ幅で折り返し」を行った場合の動作を決定する
	
	@retval TGWRAP_NONE No action
	@retval TGWRAP_FULL 最大値
	@retval TGWRAP_WINDOW ウィンドウ幅
	@retval TGWRAP_PROP 設定値

	@date 2006.01.08 genta メニュー表示で同一の判定を使うため，Command_WRAPWINDOWWIDTH()より分離．
	@date 2006.01.08 genta 判定条件を見直し
	@date 2008.06.08 ryoji ウィンドウ幅設定にぶら下げ余白を追加
*/
CEditView::TOGGLE_WRAP_ACTION CEditView::GetWrapMode( int& newWidth )
{
	//@@@ 2002.01.14 YAZAKI 現在のウィンドウ幅で折り返されているときは、最大値にするコマンド。
	//2002/04/08 YAZAKI ときどきウィンドウ幅で折り返されないことがあるバグ修正。
	// 20051022 aroka 現在のウィンドウ幅→最大値→文書タイプの初期値 をトグルにするコマンド
	// ウィンドウ幅==文書タイプ||最大値==文書タイプ の場合があるため判定順序に注意する。
	/*	Jan.  8, 2006 genta
		じゅうじさんの要望により判定方法を再考．現在の幅に合わせるのを最優先に．
	
		基本動作： 設定値→ウィンドウ幅
			→(ウィンドウ幅と合っていなければ)→ウィンドウ幅→上へ戻る
			→(ウィンドウ幅と合っていたら)→最大値→設定値
			ただし，最大値==設定値の場合には最大値→設定値の遷移が省略されて上に戻る
			
			ウィンドウ幅が極端に狭い場合にはウィンドウ幅に合わせることは出来ないが，
			設定値と最大値のトグルは可能．

		1)現在の折り返し幅==ウィンドウ幅 : 最大値
		2)現在の折り返し幅!=ウィンドウ幅
		3)→ウィンドウ幅が極端に狭い場合
		4)　└→折り返し幅!=最大値 : 最大値
		5)　└→折り返し幅==最大値
		6)　　　└→最大値==設定値 : 変更不能
		7)　　　└→最大値!=設定値 : 設定値
		8)→ウィンドウ幅が十分にある
		9)　└→折り返し幅==最大値
		a)　　　└→最大値!=設定値 : 設定値
	 	b)　　　└→最大値==設定値 : ウィンドウ幅
		c)　└→ウィンドウ幅
	*/
	
	if (m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() == ViewColNumToWrapColNum( m_nViewColNum ) ){
		// a)
		newWidth = MAXLINEKETAS;
		return TGWRAP_FULL;
	}
	else if( MINLINEKETAS > m_nViewColNum - GetWrapOverhang() ){ // 2)
		// 3)
		if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() != MAXLINEKETAS ){
			// 4)
			newWidth = MAXLINEKETAS;
			return TGWRAP_FULL;
		}
		else if( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas == MAXLINEKETAS ){ // 5)
			// 6)
			return TGWRAP_NONE;
		}
		else { // 7)
			newWidth = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas;
			return TGWRAP_PROP;
		}
	}
	else { // 8)
		if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() == MAXLINEKETAS && // 9)
			m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas != MAXLINEKETAS ){
			// a)
			newWidth = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas;
			return TGWRAP_PROP;
			
		}
		else {	// b) c)
			//	現在のウィンドウ幅
			newWidth = ViewColNumToWrapColNum( m_nViewColNum );
			return TGWRAP_WINDOW;
		}
	}
}

/*! 現在のウィンドウ幅で折り返し

	@date 2002.01.14 YAZAKI 現在のウィンドウ幅で折り返されているときは、最大値にするように
	@date 2002.04.08 YAZAKI ときどきウィンドウ幅で折り返されないことがあるバグ修正。
	@date 2005.08.14 genta ここでの設定は共通設定に反映しない．
	@date 2005.10.22 aroka 現在のウィンドウ幅→最大値→文書タイプの初期値 をトグルにする

	@note 変更する順序を変更したときはCEditWnd::InitMenu()も変更すること
	@sa CEditWnd::InitMenu()
*/
void CEditView::Command_WRAPWINDOWWIDTH( void )	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
{
	// Jan. 8, 2006 genta 判定処理をGetWrapMode()へ移動
	TOGGLE_WRAP_ACTION nWrapMode;
	int newWidth;

	nWrapMode = GetWrapMode( newWidth );
	m_pcEditDoc->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
	m_pcEditDoc->m_bTextWrapMethodCurTemp = !( m_pcEditDoc->m_nTextWrapMethodCur == m_pcEditDoc->GetDocumentAttribute().m_nTextWrapMethod );
	if( nWrapMode == TGWRAP_NONE ){
		return;	// 折り返し桁は元のまま
	}

	m_pcEditDoc->ChangeLayoutParam( true, m_pcEditDoc->m_cLayoutMgr.GetTabSpace(), newWidth );


	//	Aug. 14, 2005 genta 共通設定へは反映させない
//	m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas = m_nViewColNum;

	m_nViewLeftCol = 0;		/* 表示域の一番左の桁(0開始) */

	/* フォーカス移動時の再描画 */
	RedrawAll();
	return;
}




//検索マークの切替え	// 2001.12.03 hor クリア を 切替え に変更
void CEditView::Command_SEARCH_CLEARMARK( void )
{
// From Here 2001.12.03 hor

	//検索マークのセット

	if(IsTextSelected()){

		// 検索文字列取得
		CMemory	cmemCurText;
		GetCurrentTextForSearch( cmemCurText );

		// 検索文字列設定
		int i,j;
		strcpy( m_szCurSrchKey, cmemCurText.GetStringPtr() );
		for( i = 0; i < m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum; ++i ){
			if( 0 == strcmp( m_szCurSrchKey, m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[j], m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[j - 1] );
			}
		}
		else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[j], m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[j - 1] );
			}
			++m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum;
			if( m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum > MAX_SEARCHKEY ){
				m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum = MAX_SEARCHKEY;
			}
		}
		strcpy( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0], cmemCurText.GetStringPtr() );

		// 検索オプション設定
		m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp=false;	//正規表現使わない
		m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly=false;		//単語で検索しない
		// 2010.06.30 Moca ChangeCurRegexpに再描画フラグ追加。2回再描画しないように
		ChangeCurRegexp(false); // 2002.11.11 Moca 正規表現で検索した後，色分けができていなかった

		// 再描画
		RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	//検索マークのクリア

	m_bCurSrchKeyMark = false;	/* 検索文字列のマーク */
	/* フォーカス移動時の再描画 */
	RedrawAll();
	return;
}




//! ファイルの再オープン
void CEditView::Command_FILE_REOPEN(
	ECodeType	nCharCode,	//!< [in] 開き直す際の文字コード
	bool 		bNoConfirm	//!< [in] ファイルが更新された場合に確認を行わ「ない」かどうか。true:確認しない false:確認する
)
{
	if( !bNoConfirm && fexist( m_pcEditDoc->GetFilePath() ) && m_pcEditDoc->IsModified() ){
		int nDlgResult = MYMESSAGEBOX(
			m_hWnd,
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%s\n\nこのファイルは変更されています。\n再ロードを行うと変更が失われますが、よろしいですか?"),
			m_pcEditDoc->GetFilePath()
		);
		if( IDOK == nDlgResult ){
			//継続。下へ進む
		}else{
			return; //中断
		}
	}

	// 同一ファイルの再オープン
	 m_pcEditDoc->ReloadCurrentFile(
		nCharCode,					/* 文字コード種別 */
		m_pcEditDoc->m_bReadOnly	/* 読み取り専用モード */
	);
	/* キャレットの行桁位置を表示する */
	DrawCaretPosInfo();
}




//日付挿入
void CEditView::Command_INS_DATE( void )
{
	// 日付をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CShareData::getInstance()->MyGetDateFormat( systime, szText, _countof( szText ) - 1 );

	// テキストを貼り付け ver1
	Command_INSTEXT( true, szText, -1, TRUE );
}




//時刻挿入
void CEditView::Command_INS_TIME( void )
{
	// 時刻をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CShareData::getInstance()->MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );

	// テキストを貼り付け ver1
	Command_INSTEXT( true, szText, -1, TRUE );
}


/*! 外部コマンド実行ダイアログ表示
	@date 2002.02.02 YAZAKI.
	@date 2007.01.02 maru	引数は使っていないようなのでvoidに変更．
							HandleCommand(F_EXECMD,,,,,)実行の引数変更
*/
void CEditView::Command_EXECCOMMAND_DIALOG( void )
{
	CDlgExec cDlgExec;

	/* モードレスダイアログの表示 */
	cDlgExec.m_bEditable = m_pcEditDoc->IsEditable();	// 編集禁止
	if( !cDlgExec.DoModal( m_hInstance, m_hWnd, 0 ) ){
		return;
	}

	AddToCmdArr( cDlgExec.m_szCommand );
	const char* cmd_string = cDlgExec.m_szCommand;

	//HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	//	外部コマンド実行コマンドの発行
	HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(m_pShareData->m_nExecFlgOpt), 0, 0);	//	外部コマンド実行コマンドの発行	
}

//外部コマンド実行
//	Sept. 20, 2000 JEPRO  名称CMMANDをCOMMANDに変更
//	Oct. 9, 2001   genta  マクロ対応のため引数追加
//  2002.2.2       YAZAKI ダイアログ呼び出し部とコマンド実行部を分離
//void CEditView::Command_EXECCOMMAND( const char *cmd_string )
void CEditView::Command_EXECCOMMAND( const char *cmd_string, const int nFlgOpt)	//	2006.12.03 maru 引数の拡張
{
	//	From Here Aug. 21, 2001 genta
	//	パラメータ置換 (超暫定)
	const int bufmax = 1024;
	char buf[bufmax + 1];
	m_pcEditDoc->ExpandParameter(cmd_string, buf, bufmax);
	
	// 子プロセスの標準出力をリダイレクトする
	ExecCmd( buf, nFlgOpt );
	//	To Here Aug. 21, 2001 genta
	return;
}




void CEditView::AddToCmdArr( const TCHAR* szCmd )
{
	CRecent	cRecentCmd;

	cRecentCmd.EasyCreate( RECENT_FOR_CMD );
	cRecentCmd.AppendItem( szCmd );
	cRecentCmd.Terminate();
}




//	Jun. 16, 2000 genta
//	対括弧の検索
void CEditView::Command_BRACKETPAIR( void )
{
	int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	if( SearchBracket( m_nCaretPosX, m_nCaretPosY, &nCol, &nLine, &mode ) ){	// 02/09/18 ai
		//	2005.06.24 Moca
		//	2006.07.09 genta 表示更新漏れ：新規関数にて対応
		MoveCursorSelecting( nCol, nLine, m_bSelectingLock );
	}
	else{
		//	失敗した場合は nCol/nLineには有効な値が入っていない.
		//	何もしない
	}
}


//	現在位置を移動履歴に登録する
void CEditView::Command_JUMPHIST_SET( void )
{
	AddCurrentLineToHistory();
}


//	From HERE Sep. 8, 2000 genta
//	移動履歴を前へたどる
//
void CEditView::Command_JUMPHIST_PREV( void )
{
	// 2001.12.13 hor
	// 移動履歴の最後に現在の位置を記憶する
	// ( 次の履歴が取得できないときは追加して戻る )
	if( !m_cHistory->CheckNext() ){
		AddCurrentLineToHistory();
		m_cHistory->PrevValid();
	}

	if( m_cHistory->CheckPrev() ){
		int x, y;
		if( ! m_cHistory->PrevValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK );
		}
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			m_cHistory->GetCurrent().GetPos(),
			m_cHistory->GetCurrent().GetLine(),
			&x, &y
		);
		//	2006.07.09 genta 選択を考慮
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
}

//	移動履歴を次へたどる
void CEditView::Command_JUMPHIST_NEXT( void )
{
	if( m_cHistory->CheckNext() ){
		int x, y;
		if( ! m_cHistory->NextValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK );
		}
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			m_cHistory->GetCurrent().GetPos(),
			m_cHistory->GetCurrent().GetLine(),
			&x, &y
		);
		//	2006.07.09 genta 選択を考慮
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
}
//	To HERE Sep. 8, 2000 genta

/* 次のグループ */			// 2007.06.20 ryoji
void CEditView::Command_NEXTGROUP( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->NextGroup();
}

/* 前のグループ */			// 2007.06.20 ryoji
void CEditView::Command_PREVGROUP( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->PrevGroup();
}

/* タブを右に移動 */		// 2007.06.20 ryoji
void CEditView::Command_TAB_MOVERIGHT( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->MoveRight();
}

/* タブを左に移動 */		// 2007.06.20 ryoji
void CEditView::Command_TAB_MOVELEFT( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->MoveLeft();
}

/* 新規グループ */			// 2007.06.20 ryoji
void CEditView::Command_TAB_SEPARATE( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->Separate();
}

/* 次のグループに移動 */	// 2007.06.20 ryoji
void CEditView::Command_TAB_JOINTNEXT( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->JoinNext();
}

/* 前のグループに移動 */	// 2007.06.20 ryoji
void CEditView::Command_TAB_JOINTPREV( void )
{
	CTabWnd* pcTabWnd = &m_pcEditDoc->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->m_hWnd == NULL )
		return;
	pcTabWnd->JoinPrev();
}

/* このタブ／ウィンドウ以外を閉じる */	// 2009.07.20 syat
// 2009.12.26 syat このウィンドウ以外を閉じるとの兼用化
void CEditView::Command_TAB_CLOSEOTHER( void )
{
	int nGroup = 0;

	// ウィンドウ一覧を取得する
	EditNode* pEditNode;
	int nCount = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
	if( 0 >= nCount )return;

	for( int i = 0; i < nCount; i++ ){
		if( pEditNode[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
			pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
			nGroup = pEditNode[i].m_nGroup;
		}
	}

	//終了要求を出す
	CShareData::getInstance()->RequestCloseEditor( pEditNode, nCount, FALSE, nGroup, TRUE, m_pcEditDoc->m_hwndParent );
	delete []pEditNode;
	return;
}

/* 左をすべて閉じる */		// 2009.07.20 syat
void CEditView::Command_TAB_CLOSELEFT( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// ウィンドウ一覧を取得する
		EditNode* pEditNode;
		int nCount = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//右は閉じない
			}
		}

		//終了要求を出す
		CShareData::getInstance()->RequestCloseEditor( pEditNode, nCount, FALSE, nGroup, TRUE, m_pcEditDoc->m_hwndParent );
		delete []pEditNode;
	}
	return;
}

/* 右をすべて閉じる */		// 2009.07.20 syat
void CEditView::Command_TAB_CLOSERIGHT( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// ウィンドウ一覧を取得する
		EditNode* pEditNode;
		int nCount = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				pEditNode[i].m_hWnd = NULL;		//自分自身は閉じない
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( !bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//左は閉じない
			}
		}

		//終了要求を出す
		CShareData::getInstance()->RequestCloseEditor( pEditNode, nCount, FALSE, nGroup, TRUE, m_pcEditDoc->m_hwndParent );
		delete []pEditNode;
	}
	return;
}

/* 	上書き用の一文字削除	2009.04.11 ryoji */
void CEditView::DelCharForOverwrite( void )
{
	bool bEol = false;
	BOOL bDelete = TRUE;
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( m_nCaretPosY );
	if( NULL != pcLayout ){
		/* 指定された桁に対応する行のデータ内の位置を調べる */
		int nIdxTo = LineColmnToIndex( pcLayout, m_nCaretPosX );
		if( nIdxTo >= pcLayout->GetLengthWithoutEOL() ){
			bEol = true;	// 現在位置は改行または折り返し以後
			if( pcLayout->m_cEol != EOL_NONE ){
				if( m_pShareData->m_Common.m_sEdit.m_bNotOverWriteCRLF ){	/* 改行は上書きしない */
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
			DeleteData( false );
		}
	}
}

/*!
	@brief 検索で見つからないときの警告（メッセージボックス／サウンド）

	@date 2010.04.21 ryoji	新規作成（数カ所で用いられていた類似コードの共通化）
*/
void CEditView::AlertNotFound(HWND hwnd, LPCTSTR format, ...)
{
	if( m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND
		&& m_bDrawSWITCH	// ← たぶん「全て置換」実行中判定の代用品（もとは Command_SEARCH_NEXT() の中でだけ使用されていた）
	){
		if( NULL == hwnd ){
			hwnd = m_hWnd;
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

/*[EOF]*/
