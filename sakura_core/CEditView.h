//	$Id$
/*!	@file
	文書ウィンドウの管理
	
	@author Norio Nakatani
	@date	1998/03/13 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CEditView;

#include "CEditDoc.h"


#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_


#define _CARETMARGINRATE 20

#include <windows.h>
#include "CDropTarget.h"
#include "CMemory.h"
#include "CDocLineMgr.h"
#include "CLayoutMgr.h"
#include "COpe.h"
#include "COpeBlk.h"
#include "COpeBuf.h"
#include "CDlgFind.h"
//@@#include "CProp1.h"
#include "CShareData.h"
#include "CFuncInfoArr.h"
#include "CSplitBoxWnd.h"
#include "CSplitterWnd.h"
#include "CDlgCancel.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
#include "CHokanMgr.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"
#include "CDropTarget.h"
#include "CMarkMgr.h"

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif

struct GrepParam {
	void*	pCEditView;
	char*	pszGrepKey;
	char*	pszGrepFile;
	char*	pszGrepFolder;
	BOOL	bGrepSubFolder;
	BOOL	bGrepLoHiCase;
	BOOL	bGrepRegularExp;
	BOOL	bKanjiCode_AutoDetect;
	BOOL	bGrepOutputLine;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

class SAKURA_CORE_API CEditView/* : public CDropTarget*/
{
public:
	/* Constructors */
	CEditView();
	~CEditView();
	/* 初期化系メンバ関数 */
	BOOL Create( HINSTANCE, HWND, CEditDoc*, int,/* BOOL,*/ BOOL );
	/* 状態 */
	BOOL IsTextSelected( void );	/* テキストが選択されているか */
	BOOL IsTextSelecting( void );	/* テキストの選択中か */
	/* メッセージディスパッチャ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void OnMove( int, int, int, int );
	void OnSetFocus( void );
	void OnKillFocus( void );
	VOID OnTimer( HWND, UINT, UINT, DWORD );
	BOOL HandleCommand( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );
	/* コマンド操作 */
	void CaretUnderLineON( BOOL );								/* カーソル行アンダーラインのON */
	void CaretUnderLineOFF( BOOL );								/* カーソル行アンダーラインのOFF */
	void AdjustScrollBars( void );								/* スクロールバーの状態を更新する */
	int  MoveCursor( int, int, BOOL, int = _CARETMARGINRATE );	/* 行桁指定によるカーソル移動 */
	BOOL DetectWidthOfLineNumberArea( BOOL );					/* 行番号表示に必要な幅を設定 */
	int DetectWidthOfLineNumberArea_calculate( void );			/* 行番号表示に必要な桁数を計算 */
	void DisableSelectArea( BOOL );								/* 現在の選択範囲を非選択状態に戻す */
	void OnChangeSetting( void );								/* 設定変更を反映させる */
	void SetFont( void );										/* フォントの変更 */
	void RedrawAll( void );										/* フォーカス移動時の再描画 */
	void Redraw( void );										// 2001/06/21 asa-o 再描画
	void CopyViewStatus( CEditView* );							/* 自分の表示状態を他のビューにコピー */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */
	DWORD DoGrep( CMemory*, CMemory*, CMemory*, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, int );/* Grep実行 */
	/* Grep実行 */	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int DoGrepTree( CDlgCancel*, HWND, const char*, int*, const char*, const char*, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, int, CBregexp*, int, int* );
	/* Grep実行 */	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int DoGrepFile( CDlgCancel*, HWND, const char*, int*, const char*, const char*, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, int, CBregexp*, int, int*, const char*, CMemory& );
	/* Grep実行 */
	void CEditView::DoGrep_Thread(
		DWORD	dwGrepParam
	);

	//	Jun. 16, 2000 genta
	bool  SearchBracket( int PosX, int PosY, int* NewX, int* NewY );	//	対括弧の検索
	bool  SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
						int upChar, int dnChar );	//	対括弧の前方検索
	bool  SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
									int dnChar, int upChar );
//@@@ 2001.02.03 Start by MIK: 全角の対括弧
	bool  SearchBracketForward2( int PosX, int PosY, int* NewX, int* NewY,
						         char* upChar, char* dnChar );	//	対括弧の前方検索
	bool  SearchBracketBackward2( int PosX, int PosY, int* NewX, int* NewY,
								  char* dnChar, char* upChar );
//@@@ 2001.02.03 End

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin);	// 補完ウィンドウ用のキーワードヘルプ表示


public: /* テスト用にアクセス属性を変更 */
	CDropTarget*	m_pcDropTarget;
	BOOL			m_bDrawSWITCH;
	BOOL			m_bDragSource;	/* 選択テキストのドラッグ中か */
	BOOL			m_bDragMode;	/* 選択テキストのドラッグ中か */

	/* 単語検索の状態 */
	
	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	CBregexp	m_CurRegexp;			/*!< コンパイルデータ */
	BOOL	m_bCurSrchKeyMark;			/* 検索文字列のマーク */
	char	m_szCurSrchKey[_MAX_PATH];	/* 検索文字列 */
	int		m_bCurSrchRegularExp;		/* 検索／置換  1==正規表現 */
	int		m_bCurSrchLoHiCase;			/* 検索／置換  1==英大文字小文字の区別 */
	int		m_bCurSrchWordOnly;			/* 検索／置換  1==単語のみ検索 */

	BOOL	m_bExecutingKeyMacro;		/* キーボードマクロの実行中 */
//	BOOL	m_bGrepRunning;
	HANDLE	m_hThreadGrep;
	HWND	m_hWnd;				/* 編集ウィンドウハンドル */
	int		m_nViewTopLine;		/* 表示域の一番上の行(0開始) */
	int		m_nViewLeftCol;		/* 表示域の一番左の桁(0開始) */
	int		m_nCaretPosX_Prev;	/* ビュー左端からのカーソル桁位置（０オリジン）*/
	int		m_nCaretPosX;		/* ビュー左端からのカーソル桁位置（０開始）*/
	int		m_nCaretPosY;		/* ビュー上端からのカーソル行位置（０開始）*/
	int		m_nCaretPosX_PHY;	/* カーソル位置  改行単位行先頭からのバイト数（０開始）*/
	int		m_nCaretPosY_PHY;	/* カーソル位置  改行単位行の行番号（０開始）*/
	/*
	||  メンバ変数
	*/
	char*			m_pszAppName;	/* Mutex作成用・ウィンドウクラス名 */
	HINSTANCE		m_hInstance;	/* インスタンスハンドル */
	HWND			m_hwndParent;	/* 親ウィンドウハンドル */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	CEditDoc*		m_pcEditDoc;		/* ドキュメント */
	COpeBlk*		m_pcOpeBlk;			/* 操作ブロック */
	BOOL			m_bDoing_UndoRedo;	/* アンドゥ・リドゥの実行中か */
	HWND			m_hwndVScrollBar;	/* 垂直スクロールバーウィンドウハンドル */
	int				m_nVScrollRate;		/* 垂直スクロールバーの縮尺 */
	HWND			m_hwndHScrollBar;	/* 水平スクロールバーウィンドウハンドル */
	HWND			m_hwndSizeBox;		/* サイズボックスウィンドウハンドル */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* 垂直分割ボックス */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* 水平分割ボックス */
	HFONT			m_hFontCaretPosInfo;/* キャレットの行桁位置表示用フォント */
	int				m_nCaretPosInfoCharWidth;
	int				m_nCaretPosInfoCharHeight;
	int				m_pnCaretPosInfoDx[64];	/* 文字列描画用文字幅配列 */
	HDC				m_hdcCompatDC;		/* 再描画用コンパチブルＤＣ */
	HBITMAP			m_hbmpCompatBMP;	/* 再描画用メモリＢＭＰ */
	HBITMAP			m_hbmpCompatBMPOld;	/* 再描画用メモリＢＭＰ(OLD) */
	int				m_pnDx[10240 + 10];	/* 文字列描画用文字幅配列 */
	HFONT			m_hFont_HAN;		/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_FAT;	/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_UL;		/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_FAT_UL;	/* 現在のフォントハンドル */
	HFONT			m_hFont_ZEN;		/* 現在のフォントハンドル */
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* コマンドの実行中 */
	/* 分割状態 */
	int				m_nMyIndex;
	/* キーリピート状態 */
	int				m_bPrevCommand;
	/* 入力状態 */
	int		m_nCaretWidth;			/* キャレットの幅 */
	int		m_nCaretHeight;			/* キャレットの高さ */
	int		m_nOldUnderLineY;
	/* キー状態 */
	int		m_bSelectingLock;		/* 選択状態のロック */
	int		m_bBeginSelect;			/* 範囲選択中 */
	int		m_bBeginBoxSelect;		/* 矩形範囲選択中 */
	int		m_bBeginLineSelect;		/* 行単位選択中 */
	int		m_bBeginWordSelect;		/* 単語単位選択中 */
//	int		m_nSelectLineBgn;		/* 範囲選択開始行(原点) */
//	int		m_nSelectColmBgn;		/* 範囲選択開始桁(原点) */
	int		m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
	int		m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
	int		m_nSelectLineBgnTo;		/* 範囲選択開始行(原点) */
	int		m_nSelectColmBgnTo;		/* 範囲選択開始桁(原点) */
	int		m_nSelectLineFrom;		/* 範囲選択開始行 */
	int		m_nSelectColmFrom;		/* 範囲選択開始桁 */
	int		m_nSelectLineTo;		/* 範囲選択終了行 */
	int		m_nSelectColmTo;		/* 範囲選択終了桁 */
	int		m_nSelectLineFromOld;	/* 範囲選択開始行 */
	int		m_nSelectColmFromOld;	/* 範囲選択開始桁 */
	int		m_nSelectLineToOld;		/* 範囲選択終了行 */
	int		m_nSelectColmToOld;		/* 範囲選択終了桁 */
	int		m_nMouseRollPosXOld;	/* マウス範囲選択前回位置(X座標) */
	int		m_nMouseRollPosYOld;	/* マウス範囲選択前回位置(Y座標) */
	/* 画面情報 */
	int		m_nViewAlignLeft;		/* 表示域の左端座標 */
	int		m_nViewAlignLeftCols;	/* 行番号域の桁数 */
	int		m_nViewAlignTop;		/* 表示域の上端座標 */
	int		m_nTopYohaku;
	int		m_nViewCx;				/* 表示域の幅 */
	int		m_nViewCy;				/* 表示域の高さ */
	int		m_nViewColNum;			/* 表示域の桁数 */
	int		m_nViewRowNum;			/* 表示域の行数 */
	/* 表示方法 */
	int		m_nCharWidth;			/* 半角文字の幅 */
	int		m_nCharHeight;			/* 文字の高さ */
	/* 辞書Tip関連 */
	DWORD	m_dwTipTimer;			/* Tip起動タイマー */
	CTipWnd	m_cTipWnd;				/* Tip表示ウィンドウ */
	POINT	m_poTipCurPos;			/* Tip起動時のマウスカーソル位置 */
	BOOL	m_bInMenuLoop;			/* メニュー モーダル ループに入っています */
	CDicMgr	m_cDicMgr;				/* 辞書マネージャ */
	/* 入力補完 */
//	CHokanMgr	m_cHokanMgr;
	BOOL		m_bHokan;
	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump履歴
	/*
	||  実装ヘルパ関数
	*/
protected:
	CEOL GetCurrentInsertEOL( void );					/* 現在、Enterなどで挿入する改行コードの種類を取得 */

	void GetCurrentTextForSearch( CMemory& );			/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	BOOL MyGetClipboardData( CMemory&, BOOL* );			/* クリップボードからデータを取得 */
	BOOL MySetClipboardData( const char*, int, BOOL );	/* クリップボードにデータを設定 */
	int GetLeftWord( CMemory*, int );					/* カーソル直前の単語を取得 */
//	void PrintBitmap( HDC, int, int, const char* );		/* ビットマップファイル表示 */
//	HANDLE OpenDIB ( LPCSTR );							/* DIBファイルを開いてメモリDIBを作成 */
//	HANDLE ReadDibBitmapInfo ( int );					/* DIB形式のファイルを読む */
//	BOOL DibInfo ( HANDLE, LPBITMAPINFOHEADER );		/* CF_DIB形式のメモリブロックに関連付けられているDIB情報を取得します */
//	WORD PaletteSize ( VOID FAR * pv );					/* パレットのバイト数を計算します */
//	WORD DibNumColors ( VOID FAR * );					/* 情報ブロックのBitCountメンバを参照して、DIBの色数を判断します */
//	DWORD lread ( int, void*, DWORD );					/* データをすべて読み取る */
//	void TraceRgn( HRGN );								/* デバッグ用 リージョン矩形のダンプ */
//	void OnPaintOld( HDC, PAINTSTRUCT *, BOOL );		/* 通常の描画処理 */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* 通常の描画処理 */
//	int DispLine( HDC, int, int, int, const unsigned char*, int, BOOL );/		* 行のテキスト／選択状態の描画 */
	int DispLineNew( HDC, const CLayout*, int&, int, int&, BOOL, int, BOOL );	/* 行のテキスト／選択状態の描画 */
	void DispLineNumber( HDC, const CLayout*, int, int );		/* 行番号表示 */
	void SetCurrentColor( HDC, int );							/* 現在の色を指定 */
	void DispRuler( HDC );										/* ルーラー描画 */
//	void DispRulerEx( HDC );									/* ルーラー描画 */
	int	DispText( HDC, int, int, const unsigned char*, int );	/* テキスト表示 */
	void DispTextSelected( HDC, int, int, int, int );			/* テキスト反転 */
	void TwoPointToRect( RECT*, int, int, int, int );			/* 2点を対角とする矩形を求める */
	void DrawSelectArea( void );								/* 指定行の選択領域の描画 */
	void DrawSelectAreaLine( HDC, int, int, int, int, int );	/* 指定行の選択領域の描画 */
	int  LineColmnToIndex( const char*, int, int );				/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
	int  LineColmnToIndex2( const char*, int, int, int* );		/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */
	int  LineIndexToColmn( const char*, int, int );				/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
	BOOL GetSelectedData( CMemory&, BOOL, const char*, BOOL, enumEOLType neweol = EOL_UNKNOWN);/* 選択範囲のデータを取得 */
	void CopySelectedAllLines( const char*, BOOL );				/* 選択範囲内の全行をクリップボードにコピーする */
	void ConvSelectedArea( int );								/* 選択エリアのテキストを指定方法で変換 */
	void ConvMemory( CMemory*, int );							/* 機能種別によるバッファの変換 */
	void ShowEditCaret( void );									/* キャレットの表示・更新 */
	void OnSize( int, int );									/* ウィンドウサイズの変更処理 */
	void OnVScroll( int, int, HWND );							/* 垂直スクロールバーメッセージ処理 */
	void OnHScroll( int, int, HWND );							/* 水平スクロールバーメッセージ処理 */
	void OnLBUTTONDOWN( WPARAM, int, int );						/* マウス左ボタン押下 */
	void OnMOUSEMOVE( WPARAM, int, int );						/* マウス移動のメッセージ処理 */
	void OnLBUTTONUP( WPARAM, int, int );						/* マウス左ボタン開放のメッセージ処理 */
	void OnLBUTTONDBLCLK( WPARAM, int , int );					/* マウス左ボタンダブルクリック */
	void OnRBUTTONDOWN( WPARAM, int, int );						/* マウス右ボタン押下 */
	void OnRBUTTONUP( WPARAM, int, int );						/* マウス右ボタン開放 */
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );						/* マウスホイールのメッセージ処理 */
	void BeginSelectArea( void );								/* 現在のカーソル位置から選択を開始する */
	void ChangeSelectAreaByCurrentCursor( int, int );			/* 現在のカーソル位置によって選択範囲を変更 */
	void ChangeSelectAreaByCurrentCursorTEST( int, int, int&, int&, int&, int& );/* 現在のカーソル位置によって選択範囲を変更 */
	int  MoveCursorToPoint( int, int );							/* マウス等による座標指定によるカーソル移動 */
	void ScrollAtV( int );										/* 指定上端行位置へスクロール */
	void ScrollAtH( int );										/* 指定左端桁位置へスクロール */
	int Cursor_UPDOWN( int, int );								/* カーソル上下移動処理 */
	void SetIMECompFormPos( void );								/* IME編集エリアの位置を変更 */
	void SetIMECompFormFont( void );							/* IME編集エリアの表示フォントを変更 */
	void SetParentCaption( BOOL = FALSE );						/* 親ウィンドウのタイトルを更新 */
	void DrawCaretPosInfo( void );								/* キャレットの行桁位置を表示する */
//	void Draw3dRect( HDC, int, int, int, int, COLORREF, COLORREF );
//	void FillSolidRect( HDC, int, int, int, int, COLORREF );
	BOOL IsCurrentPositionURL( int, int, int*, int*, int*, char** );/* カーソル位置にURLが有る場合のその範囲を調べる */
	int IsCurrentPositionSelected( int, int );					/* 指定カーソル位置が選択エリア内にあるか */
	int IsCurrentPositionSelectedTEST( int, int, int, int, int, int );/* 指定カーソル位置が選択エリア内にあるか */
	BOOL IsSeaechString( const char*, int, int, int* );			/* 現在位置が検索文字列に該当するか */
	HFONT ChooseFontHandle( BOOL bFat, BOOL bUnderLine );		/* フォントを選ぶ */
	void ExecCmd(const char*, BOOL ) ;							// 子プロセスの標準出力をリダイレクトする
	void AddToCmdArr( const char* );


public: /* テスト用にアクセス属性を変更 */
	/* IDropTarget実装 */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
protected:
//	LPDATAOBJECT	m_pDataObject;
//	REFIID			m_owniid;

	/* ファイル操作系 */
	void Command_FILENEW( void );				/* 新規作成 */
	void Command_FILEOPEN( void );				/* ファイルを開く */
	BOOL Command_FILESAVE( void );				/* 上書き保存 */
	BOOL Command_FILESAVEAS( void );			/* 名前を付けて保存 */
	void Command_FILECLOSE( void );				/* 開じて(無題) */	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	void Command_FILECLOSE_OPEN( void );		/* 閉じて開く */
	void ReOpen_XXX( int );			/* 再オープン */
	void Command_FILE_REOPEN_SJIS( void );		/* SJISで開き直す */
	void Command_FILE_REOPEN_JIS( void );		/* JISで開き直す */
	void Command_FILE_REOPEN_EUC( void );		/* EUCで開き直す */
	void Command_FILE_REOPEN_UNICODE( void );	/* Unicodeで開き直す */
	void Command_FILE_REOPEN_UTF8( void );		/* UTF-8で開き直す */
	void Command_FILE_REOPEN_UTF7( void );		/* UTF-7で開き直す */
	void Command_PRINT( void );					/* 印刷*/
	void Command_PRINT_PREVIEW( void );			/* 印刷プレビュー*/
	void Command_PRINT_PAGESETUP( void );		/* 印刷ページ設定 */	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	BOOL Command_OPEN_HfromtoC( BOOL );			/* 同名のC/C++ヘッダ(ソース)を開く */	//Feb. 7, 2001 JEPRO 追加
	BOOL Command_OPEN_HHPP( BOOL );				/* 同名のC/C++ヘッダファイルを開く */	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	BOOL Command_OPEN_CCPP( BOOL );				/* 同名のC/C++ソースファイルを開く */	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plusをアクティブ表示 */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plusで実行 */
	void Command_BROWSE( void );				/* ブラウズ */
	void Command_PROPERTY_FILE( void );			/* ファイルのプロパティ */
	void Command_EXITALL( void );				/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加

	/* 編集系 */
	void Command_CHAR( char );				/* 文字入力 */
	void Command_IME_CHAR( WORD );			/* 全角文字入力 */
	void Command_UNDO( void );				/* 元に戻す(Undo) */
	void Command_REDO( void );				/* やり直し(Redo) */
	void Command_DELETE( void );			/* カーソル位置または選択エリアを削除 */
	void Command_DELETE_BACK( void );		/* カーソルの前を削除 */
	void Command_WordDeleteToStart( void );	/* 単語の左端まで削除 */
	void Command_WordDeleteToEnd( void );	/* 単語の右端まで削除 */
	void Command_WordCut( void );			/* 単語切り取り */
	void Command_WordDelete( void );		/* 単語削除 */
	void Command_LineCutToStart( void );   	//行頭まで切り取り(改行単位)
	void Command_LineCutToEnd( void );   	//行末まで切り取り(改行単位)
	void Command_LineDeleteToStart( void );	/* 行頭まで削除(改行単位) */
	void Command_LineDeleteToEnd( void );  	//行末まで削除(改行単位)
	void Command_CUT_LINE( void );			/* 行切り取り(折り返し単位) */
	void Command_DELETE_LINE( void );		/* 行削除(折り返し単位) */
	void Command_DUPLICATELINE( void );		/* 行の二重化(折り返し単位) */
	void Command_INDENT( char cChar );		/* インデント ver 1 */
	void Command_INDENT( const char*, int );/* インデント ver0 */
	void Command_UNINDENT( char cChar );	/* 逆インデント */
	void Command_WORDSREFERENCE( void );	/* 単語リファレンス */

	void DeleteData2( int, int, int, CMemory*, COpe*, BOOL, BOOL, BOOL = FALSE );/* 指定位置の指定長データ削除 */
	void DeleteData( BOOL, BOOL = FALSE );/* 現在位置のデータ削除 */
	void InsertData_CEditView( int, int, const char*, int, int*, int*, COpe*, BOOL, BOOL = FALSE );/* 現在位置にデータを挿入 */
	void SmartIndent_CPP( char );	/* C/C++スマートインデント処理 */

	/* カーソル移動系 */
	int Command_UP( int, BOOL );			/* カーソル上移動 */
	int Command_DOWN( int, BOOL );			/* カーソル下移動 */
	int  Command_LEFT( int, BOOL );			/* カーソル左移動 */
	void Command_RIGHT( int, int, BOOL );	/* カーソル右移動 */
	void Command_UP2( int );				/* カーソル上移動（２行づつ） */
	void Command_DOWN2( int );				/* カーソル下移動（２行づつ） */
	void Command_WORDLEFT( int );			/* 単語の左端に移動 */
	void Command_WORDRIGHT( int );			/* 単語の右端に移動 */
	void Command_GOLINETOP( int, BOOL );	/* 行頭に移動（折り返し単位） */
	void Command_GOLINEEND( int, int );		/* 行末に移動（折り返し単位） */
//	void Command_ROLLDOWN( int );			/* スクロールダウン */
//	void Command_ROLLUP( int );				/* スクロールアップ */
	void Command_HalfPageUp( int );			//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	void Command_HalfPageDown( int );		//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	void Command_1PageUp( int );			//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	void Command_1PageDown( int );			//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	void Command_GOFILETOP( int );			/* ファイルの先頭に移動 */
	void Command_GOFILEEND( int );			/* ファイルの最後に移動 */
	void Command_CURLINECENTER( void );		/* カーソル行をウィンドウ中央へ */
	void Command_JUMPPREV(void);			// 移動履歴: 前へ
	void Command_JUMPNEXT(void);			// 移動履歴: 次へ
	void Command_WndScrollDown(void);		// テキストを１行下へスクロール	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// テキストを１行上へスクロール	// 2001/06/20 asa-o

	/* 選択系 */
	void Command_SELECTWORD( void );		/* 現在位置の単語選択 */
	void Command_SELECTALL( void );			/* すべて選択 */
	void Command_BEGIN_SELECT( void );		/* 範囲選択開始 */

	/* 矩形選択系 */
//	void Command_BOXSELECTALL( void );		/* 矩形ですべて選択 */
	void Command_BEGIN_BOXSELECT( void );	/* 矩形範囲選択開始 */
	int Command_UP_BOX( BOOL );				/* (矩形選択)カーソル上移動 */
//	int Command_DOWN( int, BOOL );			/* カーソル下移動 */
//	int  Command_LEFT( int, BOOL );			/* カーソル左移動 */
//	void Command_RIGHT( int, int, BOOL );	/* カーソル右移動 */
//	void Command_UP2( int );				/* カーソル上移動（２行づつ） */
//	void Command_DOWN2( int );				/* カーソル下移動（２行づつ） */
//	void Command_WORDLEFT( int );			/* 単語の左端に移動 */
//	void Command_WORDRIGHT( int );			/* 単語の右端に移動 */
//	void Command_GOLINETOP( int, BOOL );	/* 行頭に移動（折り返し単位） */
//	void Command_GOLINEEND( int, int );		/* 行末に移動（折り返し単位） */
//	void Command_HalfPageUp( int );			//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	void Command_HalfPageDown( int );		//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	void Command_1PageUp( int );			//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
//	void Command_1PageDown( int );			//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//	void Command_GOFILETOP( int );			/* ファイルの先頭に移動 */
//	void Command_GOFILEEND( int );			/* ファイルの最後に移動 */

	/* クリップボード系 */
	void Command_CUT( void );						/* 切り取り（選択範囲をクリップボードにコピーして削除）*/
	void Command_COPY( int, enumEOLType neweol = EOL_UNKNOWN );/* コピー(選択範囲をクリップボードにコピー) */
	void Command_PASTE( void );						/* 貼り付け（クリップボードから貼り付け）*/
	void Command_PASTEBOX( void );					/* 矩形貼り付け（クリップボードから矩形貼り付け）*/
//	void Command_INSTEXT( BOOL, const char*, int );	/* テキストを貼り付け ver0 */
	void Command_INSTEXT( BOOL, const char*, BOOL );/* テキストを貼り付け ver1 */
	void Command_ADDTAIL( const char*, int );		/* 最後にテキストを追加 */
	void Command_COPYPATH( void );					/* このファイルのパス名をクリップボードにコピー */
	void Command_COPYTAG( void );					/* このファイルのパス名とカーソル位置をコピー */
	void Command_COPYLINES( void );					/* 選択範囲内全行コピー */
	void Command_COPYLINESASPASSAGE( void );		/* 選択範囲内全行引用符付きコピー */
	void Command_COPYLINESWITHLINENUMBER( void );	/* 選択範囲内全行行番号付きコピー */
	void Command_CREATEKEYBINDLIST( void );			// キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO	Command_の作り方がわからないので殺してある


	/* データ置換 削除&挿入にも使える */
void ReplaceData_CEditView(
	int			nDelLineFrom,		/* 削除範囲行  From レイアウト行番号 */
	int			nDelColmFrom,		/* 削除範囲位置From レイアウト行桁位置 */
	int			nDelLineTo,			/* 削除範囲行  To   レイアウト行番号 */
	int			nDelColmTo,			/* 削除範囲位置To   レイアウト行桁位置 */
	CMemory*	pcmemCopyOfDeleted,	/* 削除されたデータのコピー(NULL可能) */
	const char*	pInsData,			/* 挿入するデータ */
	int			nInsDataLen,		/* 挿入するデータの長さ */
//	int*		pnAddLineNum,		/* 再描画ヒント レイアウト行の増減 */
//	int*		pnModLineFrom,		/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	int*		pnModLineTo,		/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	BOOL		bDispSSTRING,		/* シングルクォーテーション文字列を表示する */
//	BOOL		bDispWSTRING,		/* ダブルクォーテーション文字列を表示する */
//	BOOL		bUndo				/* Undo操作かどうか */
//	int			nX,
//	int			nY,
//	const char*	pData,
//	int			nDataLen,
//	int*		pnNewLine,	/* 挿入された部分の次の位置の行 */
//	int*		pnNewPos,	/* 挿入された部分の次の位置のデータ位置 */
//	COpe*		pcOpe,		/* 編集操作要素 COpe */
	BOOL		bRedraw
//	BOOL		bUndo		/* Undo操作かどうか */
);

	/* 挿入系 */
	void Command_INS_DATE( void );	//日付挿入
	void Command_INS_TIME( void );	//時刻挿入

	/* 変換系 */
	void Command_TOLOWER( void );				/* 英大文字→英小文字 */
	void Command_TOUPPER( void );				/* 英小文字→英大文字 */
	void Command_TOZENKAKUKATA( void );			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	void Command_TOZENKAKUHIRA( void );			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	void Command_TOHANKAKU( void );				/* 全角→半角 */
	void Command_HANKATATOZENKAKUKATA( void );	/* 半角カタカナ→全角カタカナ */
	void Command_HANKATATOZENKAKUHIRA( void );	/* 半角カタカナ→全角ひらがな */
	void Command_TABTOSPACE( void );			/* TAB→空白 */
	void Command_SPACETOTAB( void );			/* 空白→TAB */  //#### Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* 自動判別→SJISコード変換 */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS→SJIS)コード変換 */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC→SJISコード変換 */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode→SJISコード変換 */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8→SJISコード変換 */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7→SJISコード変換 */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS→JISコード変換 */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS→EUCコード変換 */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS→UTF-8コード変換 */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS→UTF-7コード変換 */
	void Command_BASE64DECODE( void );			/* Base64デコードして保存 */
	void Command_UUDECODE( void );				/* uudecodeして保存	*/	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
	void Command_SEARCH_DIALOG( void );					/* 検索(単語検索ダイアログ) */
	void Command_SEARCH_NEXT( BOOL, HWND, const char* );/* 次を検索 */
	void Command_SEARCH_PREV( BOOL, HWND );				/* 前を検索 */
	void Command_REPLACE( void );						/* 置換(置換ダイアログ) */
	void Command_SEARCH_CLEARMARK( void );				/* 検索マークのクリア */
	void Command_GREP( void );							/* Grep */
	void Command_JUMP( void );							/* 指定行ヘジャンプ */
	BOOL Command_FUNCLIST( BOOL );						/* アウトライン解析 */
	void /*BOOL*/ Command_TAGJUMP( void/*BOOL*/ );		/* タグジャンプ機能 */
	void/*BOOL*/ Command_TAGJUMPBACK( void/*BOOL*/ );	/* タグジャンプバック機能 */
	void Command_COMPARE( void );						/* ファイル内容比較 */
	void Command_BRACKETPAIR( void );					/* 対括弧の検索 */

	/* モード切り替え系 */
	void Command_CHGMOD_INS( void );	/* 挿入／上書きモード切り替え */
	void Command_CANCEL_MODE( void );	/* 各種モードの取り消し */

	/* 設定系 */
	void Command_SHOWTOOLBAR( void );		/* ツールバーの表示/非表示 */
	void Command_SHOWFUNCKEY( void );		/* ファンクションキーの表示/非表示 */
	void Command_SHOWSTATUSBAR( void );		/* ステータスバーの表示/非表示 */
	void Command_TYPE_LIST( void );			/* タイプ別設定一覧 */
	void Command_OPTION_TYPE( void );		/* タイプ別設定 */
	void Command_OPTION( void );			/* 共通設定 */
	void Command_FONT( void );				/* フォント設定 */
	void Command_WRAPWINDOWWIDTH( void );	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更

	/* マクロ系 */
	void Command_RECKEYMACRO( void );	/* キーマクロの記録開始／終了 */
	void Command_SAVEKEYMACRO( void );	/* キーマクロの保存 */
	void Command_LOADKEYMACRO( void );	/* キーマクロの読み込み */
	void Command_EXECKEYMACRO( void );	/* キーマクロの実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	void Command_EXECCMMAND( void );	/* 外部コマンド実行 */
	void Command_EXECCOMMAND( void );	/* 外部コマンド実行 */
//	To Here Sept. 20, 2000

	/* カスタムメニュー */
	void Command_MENU_RBUTTON( void );	/* 右クリックメニュー */
	int	CreatePopUpMenu_R( void );		/* ポップアップメニュー(右クリック) */
	int Command_CUSTMENU( int );		/* カスタムメニュー表示 */

	/* ウィンドウ系 */
	void Command_SPLIT_V( void );		/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	void Command_SPLIT_H( void );		/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	void Command_SPLIT_VH( void );		/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
	void Command_WINCLOSE( void );		/* ウィンドウを閉じる */
	void Command_FILECLOSEALL( void );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	void Command_CASCADE( void );		/* 重ねて表示 */
	void Command_TILE_V( void );		/* 上下に並べて表示 */
	void Command_TILE_H( void );		/* 左右に並べて表示 */
	void Command_MAXIMIZE_V( void );	/* 縦方向に最大化 */
	void Command_MAXIMIZE_H( void );	/* 横方向に最大化 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* すべて最小化 */
	void Command_REDRAW( void );		/* 再描画 */
	void Command_WIN_OUTPUT( void );	//アウトプットウィンドウ表示

	/* 支援 */
	void Command_HOKAN( void );			/* 入力補完	*/
	void Command_HELP_CONTENTS( void );	/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* コマンド一覧 */
	void Command_EXTHELP1( void );		/* 外部ヘルプ１ */
	void Command_EXTHTMLHELP( void );	/* 外部HTMLヘルプ */
	void Command_ABOUT( void );			/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */
	void Command_SENDMAIL( void );		/* メール送信 */

	//	May 23, 2000 genta
	//	画面描画補助関数
	void DrawEOL(HDC hdc, HANDLE hPen, int nPosX, int nPosY, int nWidth, int nHeight,
		CEOL cEol, int bBold );

	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//現在行を履歴に追加する

};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
