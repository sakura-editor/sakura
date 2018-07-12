/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CVIEWCOMMANDER_5F4F7A80_2BEC_4B1D_A637_B922375FF14C9_H_
#define SAKURA_CVIEWCOMMANDER_5F4F7A80_2BEC_4B1D_A637_B922375FF14C9_H_

class CEditView;
enum EFunctionCode;
class CEditDoc;
struct DLLSHAREDATA;
class COpeBlk;
class CCaret;
class CEditWnd;
class CColorStrategy;
class CColorStrategyPool;
class CSMacroMgr;
#include "CEol.h"

class CViewCommander{
public:
	CViewCommander(CEditView* pEditView);

public:
	//外部依存
	CEditDoc* GetDocument();
	CEditWnd* GetEditWindow();
	HWND GetMainWindow();
	COpeBlk* GetOpeBlk();
	void SetOpeBlk(COpeBlk* p);
	CLayoutRange& GetSelect();
	CCaret& GetCaret();

private:
	CEditView*		m_pCommanderView;
	CSMacroMgr*		m_pcSMacroMgr;

public:
	/* キーリピート状態 */
	int				m_bPrevCommand;



private:
	enum EIndentType {
		INDENT_NONE,
		INDENT_TAB,
		INDENT_SPACE
	};

	// -- -- -- -- 以下、コマンド処理関数群 -- -- -- -- //
public:
	BOOL HandleCommand(
		EFunctionCode	nCommand,
		bool			bRedraw,
		LPARAM			lparam1,
		LPARAM			lparam2,
		LPARAM			lparam3,
		LPARAM			lparam4
	);

	/* ファイル操作系 */
	void Command_FILENEW( void );				/* 新規作成 */
	void Command_FILENEW_NEWWINDOW( void );		/* 新規作成（タブで開く版） */
	/* ファイルを開く */
	// Oct. 2, 2001 genta マクロ用に機能拡張
	// Mar. 30, 2003 genta 引数追加
	void Command_FILEOPEN(
		const WCHAR*	filename		= NULL,
		ECodeType		nCharCode		= CODE_AUTODETECT,
		bool			bViewMode	= false,
		const WCHAR*	defaultName		= NULL
	);

	/* 上書き保存 */ // Feb. 28, 2004 genta 引数追加, Jan. 24, 2005 genta 引数追加
	bool Command_FILESAVE( bool warnbeep = true, bool askname = true );
	bool Command_FILESAVEAS_DIALOG(const WCHAR*, ECodeType, EEolType);		/* 名前を付けて保存 */
	BOOL Command_FILESAVEAS( const WCHAR* filename, EEolType eEolType);	/* 名前を付けて保存 */
	BOOL Command_FILESAVEALL( void );					/* 全て上書き保存 */ // Jan. 23, 2005 genta
	void Command_FILECLOSE( void );						/* 開じて(無題) */	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	/* 閉じて開く*/
	// Mar. 30, 2003 genta 引数追加
	void Command_FILECLOSE_OPEN( LPCWSTR filename = NULL,
		ECodeType nCharCode = CODE_AUTODETECT, bool bViewMode = false );

	void Command_FILE_REOPEN( ECodeType nCharCode, bool bNoConfirm );		/* 再オープン */	//Dec. 4, 2002 genta 引数追加

	void Command_PRINT( void );					/* 印刷*/
	void Command_PRINT_PREVIEW( void );			/* 印刷プレビュー*/
	void Command_PRINT_PAGESETUP( void );		/* 印刷ページ設定 */	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	BOOL Command_OPEN_HfromtoC( BOOL );			/* 同名のC/C++ヘッダ(ソース)を開く */	//Feb. 7, 2001 JEPRO 追加
	BOOL Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* 同名のC/C++ヘッダファイルを開く */	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	BOOL Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* 同名のC/C++ソースファイルを開く */	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plusをアクティブ表示 */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plusで実行 */
	void Command_BROWSE( void );				/* ブラウズ */
	void Command_VIEWMODE( void );				/* ビューモード */
	void Command_PROPERTY_FILE( void );			/* ファイルのプロパティ */
	void Command_PROFILEMGR( void );			// プロファイルマネージャ
	void Command_EXITALLEDITORS( void );		/* 編集の全終了 */	// 2007.02.13 ryoji 追加
	void Command_EXITALL( void );				/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加
	BOOL Command_PUTFILE( LPCWSTR, ECodeType, int );	/* 作業中ファイルの一時出力 maru 2006.12.10 */
	BOOL Command_INSFILE( LPCWSTR, ECodeType, int );	/* キャレット位置にファイル挿入 maru 2006.12.10 */

	/* 編集系 */
	void Command_WCHAR( wchar_t, bool bConvertEOL = true );			/* 文字入力 */ //2007.09.02 kobake Command_CHAR(char)→Command_WCHAR(wchar_t)に変更
	void Command_IME_CHAR( WORD );			/* 全角文字入力 */
	void Command_UNDO( void );				/* 元に戻す(Undo) */
	void Command_REDO( void );				/* やり直し(Redo) */
	void Command_DELETE( void );			/* カーソル位置または選択エリアを削除 */
	void Command_DELETE_BACK( void );		/* カーソル前を削除 */
	void Command_WordDeleteToStart( void );	/* 単語の左端まで削除 */
	void Command_WordDeleteToEnd( void );	/* 単語の右端まで削除 */
	void Command_WordCut( void );			/* 単語切り取り */
	void Command_WordDelete( void );		/* 単語削除 */
	void Command_LineCutToStart( void );	//行頭まで切り取り(改行単位)
	void Command_LineCutToEnd( void );		//行末まで切り取り(改行単位)
	void Command_LineDeleteToStart( void );	/* 行頭まで削除(改行単位) */
	void Command_LineDeleteToEnd( void );  	//行末まで削除(改行単位)
	void Command_CUT_LINE( void );			/* 行切り取り(折り返し単位) */
	void Command_DELETE_LINE( void );		/* 行削除(折り返し単位) */
	void Command_DUPLICATELINE( void );		/* 行の二重化(折り返し単位) */
	void Command_INDENT( wchar_t cChar, EIndentType = INDENT_NONE ); /* インデント ver 1 */
// From Here 2001.12.03 hor
//	void Command_INDENT( const char*, int );/* インデント ver0 */
	void Command_INDENT( const wchar_t*, CLogicInt , EIndentType = INDENT_NONE );/* インデント ver0 */
// To Here 2001.12.03 hor
	void Command_UNINDENT( wchar_t wcChar );	/* 逆インデント */
//	void Command_WORDSREFERENCE( void );	/* 単語リファレンス */
	void Command_TRIM(BOOL);				// 2001.12.03 hor
	void Command_SORT(BOOL);				// 2001.12.06 hor
	void Command_MERGE(void);				// 2001.12.06 hor
	void Command_Reconvert(void);			/* メニューからの再変換対応 minfu 2002.04.09 */
	void Command_CtrlCode_Dialog(void);		/* コントロールコードの入力(ダイアログ) */	//@@@ 2002.06.02 MIK


	/* カーソル移動系 */
	//	Oct. 24, 2001 genta 機能拡張のため引数追加
	void Command_MOVECURSOR(CLogicPoint pos, int option);
	void Command_MOVECURSORLAYOUT(CLayoutPoint pos, int option);
	int Command_UP( bool bSelect, bool bRepeat, int line = 0 );			/* カーソル上移動 */
	int Command_DOWN( bool bSelect, bool bRepeat );			/* カーソル下移動 */
	int  Command_LEFT( bool, bool );			/* カーソル左移動 */
	void Command_RIGHT( bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat );	/* カーソル右移動 */
	void Command_UP2( bool bSelect );				/* カーソル上移動（２行づつ） */
	void Command_DOWN2( bool bSelect );				/* カーソル下移動（２行づつ） */
	void Command_WORDLEFT( bool bSelect );			/* 単語の左端に移動 */
	void Command_WORDRIGHT( bool bSelect );			/* 単語の右端に移動 */
	//	Oct. 29, 2001 genta マクロ向け機能拡張
	void Command_GOLINETOP( bool bSelect, int lparam );	/* 行頭に移動（折り返し単位） */
	void Command_GOLINEEND( bool bSelect, int , int);		/* 行末に移動（折り返し単位） */
//	void Command_ROLLDOWN( int );			/* スクロールダウン */
//	void Command_ROLLUP( int );				/* スクロールアップ */
	void Command_HalfPageUp( bool bSelect, CLayoutYInt );			//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	void Command_HalfPageDown( bool bSelect, CLayoutYInt );		//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	void Command_1PageUp( bool bSelect, CLayoutYInt );			//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	void Command_1PageDown( bool bSelect, CLayoutYInt );			//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	void Command_GOFILETOP( bool bSelect );			/* ファイルの先頭に移動 */
	void Command_GOFILEEND( bool bSelect );			/* ファイルの最後に移動 */
	void Command_CURLINECENTER( void );		/* カーソル行をウィンドウ中央へ */
	void Command_JUMPHIST_PREV(void);		// 移動履歴: 前へ
	void Command_JUMPHIST_NEXT(void);		// 移動履歴: 次へ
	void Command_JUMPHIST_SET(void);		// 現在位置を移動履歴に登録
	void Command_WndScrollDown(void);		// テキストを１行下へスクロール	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// テキストを１行上へスクロール	// 2001/06/20 asa-o
	void Command_GONEXTPARAGRAPH( bool bSelect );	// 次の段落へ進む
	void Command_GOPREVPARAGRAPH( bool bSelect );	// 前の段落へ戻る
	void Command_AUTOSCROLL();		// オートスクロール
	void Command_WHEELUP(int);
	void Command_WHEELDOWN(int);
	void Command_WHEELLEFT(int);
	void Command_WHEELRIGHT(int);
	void Command_WHEELPAGEUP(int);
	void Command_WHEELPAGEDOWN(int);
	void Command_WHEELPAGELEFT(int);
	void Command_WHEELPAGERIGHT(int);
	void Command_MODIFYLINE_NEXT( bool bSelect );	// 次の変更行へ
	void Command_MODIFYLINE_PREV( bool bSelect );	// 前の変更行へ

	/* 選択系 */
	bool Command_SELECTWORD( CLayoutPoint* pptCaretPos = NULL );		/* 現在位置の単語選択 */
	void Command_SELECTALL( void );			/* すべて選択 */
	void Command_SELECTLINE( int lparam );	/* 1行選択 */	// 2007.10.13 nasukoji
	void Command_BEGIN_SELECT( void );		/* 範囲選択開始 */

	/* 矩形選択系 */
//	void Command_BOXSELECTALL( void );		/* 矩形ですべて選択 */
	void Command_BEGIN_BOXSELECT( bool bSelectingLock = false );	/* 矩形範囲選択開始 */
//	int Command_UP_BOX( BOOL );				/* (矩形選択)カーソル上移動 */

	/* クリップボード系 */
	void Command_CUT( void );						/* 切り取り（選択範囲をクリップボードにコピーして削除）*/
	void Command_COPY( bool, bool bAddCRLFWhenCopy, EEolType neweol = EOL_UNKNOWN );/* コピー(選択範囲をクリップボードにコピー) */
	void Command_PASTE( int option );						/* 貼り付け（クリップボードから貼り付け）*/
	void Command_PASTEBOX( int option );					/* 矩形貼り付け（クリップボードから矩形貼り付け）*/
	//<< 2002/03/29 Azumaiya
	/* 矩形貼り付け（引数渡しでの張り付け）*/
	void Command_PASTEBOX( const wchar_t *szPaste, int nPasteSize );
	//>> 2002/03/29 Azumaiya
	void Command_INSBOXTEXT( const wchar_t *, int ); // 矩形貼り付け
	void Command_INSTEXT( bool bRedraw, const wchar_t*, CLogicInt, bool bNoWaitCursor,
		bool bLinePaste = false, bool bFastMode = false, const CLogicRange*	psDelRangeLogicFast = NULL ); // 2004.05.14 Moca テキストを貼り付け '\0'対応
	void Command_ADDTAIL( const wchar_t* pszData, int nDataLen);	/* 最後にテキストを追加 */
	void Command_COPYFILENAME( void );				/* このファイル名をクリップボードにコピー */ //2002/2/3 aroka
	void Command_COPYPATH( void );					/* このファイルのパス名をクリップボードにコピー */
	void Command_COPYTAG( void );					/* このファイルのパス名とカーソル位置をコピー */
	void Command_COPYLINES( void );					/* 選択範囲内全行コピー */
	void Command_COPYLINESASPASSAGE( void );		/* 選択範囲内全行引用符付きコピー */
	void Command_COPYLINESWITHLINENUMBER( void );	/* 選択範囲内全行行番号付きコピー */
	void Command_COPY_COLOR_HTML(bool bLineNumber = false);	//選択範囲内全行行番号付きコピー
	void Command_COPY_COLOR_HTML_LINENUMBER( void );		//選択範囲内色付きHTMLコピー
	CColorStrategy* GetColorStrategyHTML(const CStringRef&, int, const CColorStrategyPool*, CColorStrategy**, CColorStrategy**, bool& bChange);
	void Command_CREATEKEYBINDLIST( void );			// キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO	Command_の作り方がわからないので殺してある


	/* 挿入系 */
	void Command_INS_DATE( void );	//日付挿入
	void Command_INS_TIME( void );	//時刻挿入

	/* 変換系 */
	void Command_TOLOWER( void );				/* 小文字 */
	void Command_TOUPPER( void );				/* 大文字 */
	void Command_TOZENKAKUKATA( void );			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	void Command_TOZENKAKUHIRA( void );			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	void Command_TOHANKAKU( void );				/* 全角→半角 */
	void Command_TOHANKATA( void );				/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	void Command_TOZENEI( void );				/* 半角英数→全角英数 */ //July. 30, 2001 Misaka
	void Command_TOHANEI( void );				/* 全角英数→半角英数 */ //@@@ 2002.2.11 YAZAKI
	void Command_HANKATATOZENKAKUKATA( void );	/* 半角カタカナ→全角カタカナ */
	void Command_HANKATATOZENKAKUHIRA( void );	/* 半角カタカナ→全角ひらがな */
	void Command_TABTOSPACE( void );			/* TAB→空白 */
	void Command_SPACETOTAB( void );			/* 空白→TAB */  //---- Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* 自動判別→SJISコード変換 */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS→SJIS)コード変換 */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC→SJISコード変換 */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode→SJISコード変換 */
	void Command_CODECNV_UNICODEBE2SJIS( void );	/* UnicodeBE→SJISコード変換 */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8→SJISコード変換 */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7→SJISコード変換 */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS→JISコード変換 */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS→EUCコード変換 */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS→UTF-8コード変換 */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS→UTF-7コード変換 */
	void Command_BASE64DECODE( void );			/* Base64デコードして保存 */
	void Command_UUDECODE( void );				/* uudecodeして保存 */	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
	void Command_SEARCH_BOX( void );					/* 検索(ボックス) */	// 2006.06.04 yukihane
	void Command_SEARCH_DIALOG( void );					/* 検索(単語検索ダイアログ) */
	void Command_SEARCH_NEXT( bool, bool, bool, HWND, const WCHAR*, CLogicRange* = NULL );/* 次を検索 */
	void Command_SEARCH_PREV( bool bReDraw, HWND );		/* 前を検索 */
	void Command_REPLACE_DIALOG( void );				/* 置換(置換ダイアログ) */
	void Command_REPLACE( HWND hwndParent );			/* 置換(実行) 2002/04/08 YAZAKI 親ウィンドウを指定するように変更 */
	void Command_REPLACE_ALL();							/* すべて置換(実行) */
	void Command_SEARCH_CLEARMARK( void );				/* 検索マークのクリア */
	void Command_JUMP_SRCHSTARTPOS( void );				/* 検索開始位置へ戻る */	// 02/06/26 ai


	void Command_GREP_DIALOG( void );					/* Grepダイアログの表示 */
	void Command_GREP( void );							/* Grep */
	void Command_GREP_REPLACE_DLG( void );				/* Grep置換ダイアログの表示 */
	void Command_GREP_REPLACE( void );					/* Grep置換 */
	void Command_JUMP_DIALOG( void );					/* 指定行ヘジャンプダイアログの表示 */
	void Command_JUMP( void );							/* 指定行ヘジャンプ */
// From Here 2001.12.03 hor
	BOOL Command_FUNCLIST( int nAction, EOutlineType nOutlineType );	/* アウトライン解析 */ // 20060201 aroka
// To Here 2001.12.03 hor
	// Apr. 03, 2003 genta 引数追加
	bool Command_TAGJUMP( bool bClose = false );		/* タグジャンプ機能 */
	void Command_TAGJUMPBACK( void );					/* タグジャンプバック機能 */
	bool Command_TagJumpByTagsFileMsg( bool );				//ダイレクトタグジャンプ(通知つき)
	bool Command_TagJumpByTagsFile( bool );				//ダイレクトタグジャンプ	//@@@ 2003.04.13 MIK

	bool Command_TagsMake( void );						//タグファイルの作成	//@@@ 2003.04.13 MIK
	bool Command_TagJumpByTagsFileKeyword( const wchar_t* keyword );	//	@@ 2005.03.31 MIK
	void Command_COMPARE( void );						/* ファイル内容比較 */
	void Command_Diff_Dialog( void );					/* DIFF差分表示ダイアログ */	//@@@ 2002.05.25 MIK
	void Command_Diff( const WCHAR* szTmpFile2, int nFlgOpt );	/* DIFF差分表示 */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	void Command_Diff_Next( void );						/* 次の差分へ */	//@@@ 2002.05.25 MIK
	void Command_Diff_Prev( void );						/* 前の差分へ */	//@@@ 2002.05.25 MIK
	void Command_Diff_Reset( void );					/* 差分の全解除 */	//@@@ 2002.05.25 MIK
	void Command_BRACKETPAIR( void );					/* 対括弧の検索 */
// From Here 2001.12.03 hor
	void Command_BOOKMARK_SET( void );					/* ブックマーク設定・解除 */
	void Command_BOOKMARK_NEXT( void );					/* 次のブックマークへ */
	void Command_BOOKMARK_PREV( void );					/* 前のブックマークへ */
	void Command_BOOKMARK_RESET( void );				/* ブックマークの全解除 */
// To Here 2001.12.03 hor
	void Command_BOOKMARK_PATTERN( void );				// 2002.01.16 hor 指定パターンに一致する行をマーク
	void Command_FUNCLIST_NEXT( void );					// 次の関数リストマーク	2014.01.05
	void Command_FUNCLIST_PREV( void );					// 前の関数リストマーク	2014.01.05



	/* モード切り替え系 */
	void Command_CHGMOD_INS( void );	/* 挿入／上書きモード切り替え */
	void Command_CHG_CHARSET( ECodeType, bool );	/* 文字コードセット指定 */	// 2010/6/15 Uchi
	void Command_CHGMOD_EOL( EEolType );	/* 入力する改行コードを設定 2003.06.23 moca */
	void Command_CANCEL_MODE( int whereCursorIs = 0 );	/* 各種モードの取り消し */

	/* 設定系 */
	void Command_SHOWTOOLBAR( void );		/* ツールバーの表示/非表示 */
	void Command_SHOWFUNCKEY( void );		/* ファンクションキーの表示/非表示 */
	void Command_SHOWTAB( void );			/* タブの表示/非表示 */	//@@@ 2003.06.10 MIK
	void Command_SHOWSTATUSBAR( void );		/* ステータスバーの表示/非表示 */
	void Command_SHOWMINIMAP( void );		// ミニマップの表示/非表示
	void Command_TYPE_LIST( void );			/* タイプ別設定一覧 */
	void Command_CHANGETYPE( int nTypePlusOne );	// タイプ別設定一時適用
	void Command_OPTION_TYPE( void );		/* タイプ別設定 */
	void Command_OPTION( void );			/* 共通設定 */
	void Command_FONT( void );				/* フォント設定 */
	void Command_SETFONTSIZE( int, int, int );	/* フォントサイズ設定 */
	void Command_WRAPWINDOWWIDTH( void );	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	void Command_Favorite( void );			//履歴の管理	//@@@ 2003.04.08 MIK
	void Command_SET_QUOTESTRING( const wchar_t* );	//	Jan. 29, 2005 genta 引用符の設定
	void Command_TEXTWRAPMETHOD( int );				/* テキストの折り返し方法を変更する */		// 2008.05.30 nasukoji
	void Command_SELECT_COUNT_MODE( int nMode );	/* 文字カウント方法 */	//2009.07.06 syat

	/* マクロ系 */
	void Command_RECKEYMACRO( void );	/* キーマクロの記録開始／終了 */
	void Command_SAVEKEYMACRO( void );	/* キーマクロの保存 */
	void Command_LOADKEYMACRO( void );	/* キーマクロの読み込み */
	void Command_EXECKEYMACRO( void );	/* キーマクロの実行 */
	void Command_EXECEXTMACRO( const WCHAR* path, const WCHAR* type );	/* 名前を指定してマクロ実行 */
//	From Here 2006.12.03 maru 引数の拡張．
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	void Command_EXECCMMAND( void );	/* 外部コマンド実行 */
	//	Oct. 9, 2001 genta マクロ対応のため機能拡張
//	void Command_EXECCOMMAND_DIALOG( const WCHAR* cmd );	/* 外部コマンド実行ダイアログ表示 */
//	void Command_EXECCOMMAND( const WCHAR* cmd );	/* 外部コマンド実行 */
	void Command_EXECCOMMAND_DIALOG( void );	/* 外部コマンド実行ダイアログ表示 */	//	引数使ってないみたいなので
	//	マクロからの呼び出しではオプションを保存させないため、Command_EXECCOMMAND_DIALOG内で処理しておく．
	void Command_EXECCOMMAND( LPCWSTR cmd, const int nFlgOpt, LPCWSTR );	/* 外部コマンド実行 */
//	To Here Sept. 20, 2000
//	To Here 2006.12.03 maru 引数の拡張

	/* カスタムメニュー */
	void Command_MENU_RBUTTON( void );	/* 右クリックメニュー */
	int Command_CUSTMENU( int );		/* カスタムメニュー表示 */

	/* ウィンドウ系 */
	void Command_SPLIT_V( void );		/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	void Command_SPLIT_H( void );		/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	void Command_SPLIT_VH( void );		/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
	void Command_WINCLOSE( void );		/* ウィンドウを閉じる */
	void Command_FILECLOSEALL( void );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	void Command_BIND_WINDOW( void );	/* 結合して表示 */	//2004.07.14 Kazika 新規追加
	void Command_CASCADE( void );		/* 重ねて表示 */
	void Command_TILE_V( void );		/* 上下に並べて表示 */
	void Command_TILE_H( void );		/* 左右に並べて表示 */
	void Command_MAXIMIZE_V( void );	/* 縦方向に最大化 */
	void Command_MAXIMIZE_H( void );	/* 横方向に最大化 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* すべて最小化 */
	void Command_REDRAW( void );		/* 再描画 */
	void Command_WIN_OUTPUT( void );	//アウトプットウィンドウ表示
	void Command_TRACEOUT( const wchar_t* outputstr , int, int );	//マクロ用アウトプットウィンドウに表示 maru 2006.04.26
	void Command_WINTOPMOST( LPARAM );		// 常に手前に表示 2004.09.21 Moca
	void Command_WINLIST( int nCommandFrom );		/* ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon // 2006.05.19 genta 引数追加
	void Command_DLGWINLIST( void );	// ウィンドウ一覧ダイアログ // 2015.03.07 Moca
	void Command_GROUPCLOSE( void );	/* グループを閉じる */		// 2007.06.20 ryoji
	void Command_NEXTGROUP( void );		/* 次のグループ */			// 2007.06.20 ryoji
	void Command_PREVGROUP( void );		/* 前のグループ */			// 2007.06.20 ryoji
	void Command_TAB_MOVERIGHT( void );	/* タブを右に移動 */		// 2007.06.20 ryoji
	void Command_TAB_MOVELEFT( void );	/* タブを左に移動 */		// 2007.06.20 ryoji
	void Command_TAB_SEPARATE( void );	/* 新規グループ */			// 2007.06.20 ryoji
	void Command_TAB_JOINTNEXT( void );	/* 次のグループに移動 */	// 2007.06.20 ryoji
	void Command_TAB_JOINTPREV( void );	/* 前のグループに移動 */	// 2007.06.20 ryoji
	void Command_TAB_CLOSEOTHER( void );/* このタブ以外を閉じる */	// 2008.11.22 syat
	void Command_TAB_CLOSELEFT( void );	/* 左をすべて閉じる */		// 2008.11.22 syat
	void Command_TAB_CLOSERIGHT( void );/* 右をすべて閉じる */		// 2008.11.22 syat


	void Command_ToggleKeySearch( int );	/* キャレット位置の単語を辞書検索する機能ON-OFF */	// 2006.03.24 fon

	void Command_HOKAN( void );			/* 入力補完 */
	void Command_HELP_CONTENTS( void );	/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* コマンド一覧 */
	void Command_EXTHELP1( void );		/* 外部ヘルプ１ */
	//	Jul. 5, 2002 genta
	void Command_EXTHTMLHELP( const WCHAR* helpfile = NULL, const WCHAR* kwd = NULL );	/* 外部HTMLヘルプ */
	void Command_ABOUT( void );			/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */

private:
	void AlertNotFound(HWND hwnd, bool, LPCTSTR format, ...);
	void DelCharForOverwrite(const wchar_t* pszInput, int nLen);	// 上書き用の一文字削除	// 2009.04.11 ryoji
	bool Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, int count ); // タグジャンプの前処理
public:
	CLogicInt ConvertEol(const wchar_t* pszText, CLogicInt nTextLen, wchar_t* pszConvertedText);
	void Sub_BoxSelectLock( int flags );

};

#endif /* SAKURA_CVIEWCOMMANDER_5F4F7A80_2BEC_4B1D_A637_B922375FF14C9_H_ */
/*[EOF]*/
