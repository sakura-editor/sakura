/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_


#include <windows.h>
#include <objidl.h>  // LPDATAOBJECT
#include "CShareData.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
#include "CHokanMgr.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"
#include "CEOL.h"
#include "CTextMetrics.h"
#include "CTextDrawer.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CCaret.h"
#include "CRuler.h"
#include "CViewCalc.h"
#include "CViewCommander.h"
#include "CViewParser.h"
#include "CViewSelect.h"
#include "basis/SakuraBasis.h"
#include "CEditView_Paint.h"

class CDropTarget; /// 2002/2/3 aroka ヘッダ軽量化
class CMemory;///
class COpe;///
class COpeBlk;///
class CSplitBoxWnd;///
class CDlgCancel;///
class CRegexKeyword;///
class CAutoMarkMgr; /// 2002/2/3 aroka ヘッダ軽量化 to here
class CEditDoc;	//	2002/5/13 YAZAKI ヘッダ軽量化
class CLayout;	//	2002/5/13 YAZAKI ヘッダ軽量化
class CDocLine;
class CMigemo;	// 2004.09.14 isearch

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

#if !defined(RECONVERTSTRING) && (WINVER < 0x040A)
typedef struct tagRECONVERTSTRING {
    DWORD dwSize;
    DWORD dwVersion;
    DWORD dwStrLen;
    DWORD dwStrOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwTargetStrLen;
    DWORD dwTargetStrOffset;
} RECONVERTSTRING, *PRECONVERTSTRING;
#endif // RECONVERTSTRING

///	マウスからコマンドが実行された場合の上位ビット
///	@date 2006.05.19 genta
const int CMD_FROM_MOUSE = 2;

class CEditView;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 文書ウィンドウの管理
	
	1つの文書ウィンドウにつき1つのCEditDocオブジェクトが割り当てられ、
	1つのCEditDocオブジェクトにつき、4つのCEditViweオブジェクトが割り当てられる。
	ウィンドウメッセージの処理、コマンドメッセージの処理、
	画面表示などを行う。
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
//2007.08.25 kobake 文字間隔配列の機能をCTextMetricsに移動
//2007.10.02 kobake Command_TRIM2をCConvertに移動

class SAKURA_CORE_API CEditView
: public CViewCalc //$$ これが親クラスである必要は無いが、このクラスのメソッド呼び出しが多いので、暫定的に親クラスとする。
, public CEditView_Paint
{
public:
	const CEditDoc* GetDocument() const
	{
		return m_pcEditDoc;
	}
	CEditDoc* GetDocument()
	{
		return m_pcEditDoc;
	}
	HWND GetHwnd() const
	{
		return m_hWnd;
	}
public:
	//! 背景にビットマップを使用するかどうか(事実上は常にFALSE。TRUEの場合背景作画しない)
	bool IsBkBitmap() const{ return false; }

public:
	bool GetDrawSwitch() const
	{
		return m_bDrawSWITCH;
	}
public:
	void SetDrawSwitch(bool b)
	{
		m_bDrawSWITCH = b;
	}
	CEditView* GetEditView()
	{
		return this;
	}
	const CEditView* GetEditView() const
	{
		return this;
	}


public:
	/* Constructors */
	CEditView(CEditWnd* pcEditWnd);
	~CEditView();
	/* 初期化系メンバ関数 */
	BOOL Create( HINSTANCE, HWND, CEditDoc*, int,/* BOOL,*/ BOOL );
	BOOL CreateScrollBar( void );		/* スクロールバー作成 */	// 2006.12.19 ryoji
	void DestroyScrollBar( void );		/* スクロールバー破棄 */	// 2006.12.19 ryoji

	//	Oct. 2, 2005 genta 挿入モードの設定・取得
	bool IsInsMode() const;
	void SetInsMode(bool);

	/* メッセージディスパッチャ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void OnMove( int, int, int, int );
	void OnSetFocus( void );
	void OnKillFocus( void );
	void OnTimer( HWND, UINT, UINT, DWORD );
	/* コマンド操作 */
	void CaretUnderLineON( bool );								/* カーソル行アンダーラインのON */
	void CaretUnderLineOFF( bool );								/* カーソル行アンダーラインのOFF */
	void AdjustScrollBars( void );								/* スクロールバーの状態を更新する */
	// 2006.07.09 genta 行桁指定によるカーソル移動(選択領域を考慮)
	void MoveCursorSelecting( CLayoutPoint ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE );
	void OnChangeSetting( void );								/* 設定変更を反映させる */
	void SetFont( void );										/* フォントの変更 */
	void RedrawAll( void );										/* フォーカス移動時の再描画 */
	void Redraw( void );										// 2001/06/21 asa-o 再描画
	void CopyViewStatus( CEditView* );							/* 自分の表示状態を他のビューにコピー */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */

	DWORD DoGrep( const CNativeW*, const CNativeT*, const CNativeT*, BOOL, const SSearchOption&, ECodeType, BOOL, int );	/* Grep実行 */
	/* Grep実行 */	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int DoGrepTree( CDlgCancel*, HWND, const wchar_t*, int*, const TCHAR*, const TCHAR*, BOOL, const SSearchOption&, ECodeType, BOOL, int, CBregexp*, int, int* );
	/* Grep実行 */	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	//	Mar. 28, 2004 genta 不要な引数を削除
	int DoGrepFile( CDlgCancel*, HWND, const wchar_t*, int*, const TCHAR*, const SSearchOption&, ECodeType, BOOL, int, CBregexp*, int*, const TCHAR*, CNativeW& );
	/* Grep結果をpszWorkに格納 */
	void SetGrepResult(
		/* データ格納先 */
		wchar_t*	pWork,
		int*		pnWorkLen,
		/* マッチしたファイルの情報 */
		const TCHAR*		pszFullPath,	//	フルパス
		const TCHAR*		pszCodeName,	//	文字コード情報"[SJIS]"とか
		/* マッチした行の情報 */
		int			nLine,			//	マッチした行番号
		int			nColm,			//	マッチした桁番号
		const wchar_t*	pCompareData,	//	行の文字列
		int			nLineLen,		//	行の文字列の長さ
		int			nEolCodeLen,	//	EOLの長さ
		/* マッチした文字列の情報 */
		const wchar_t*	pMatchData,		//	マッチした文字列
		int			nMatchLen,		//	マッチした文字列の長さ
		/* オプション */
		BOOL		bGrepOutputLine,
		int			nGrepOutputStyle
	);
	
	//	Jun. 16, 2000 genta
	bool  SearchBracket( const CLayoutPoint& ptPos, CLayoutPoint* pptLayoutNew, int* mode );	// 対括弧の検索		// modeの追加 02/09/18 ai
	bool  SearchBracketForward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const wchar_t* upChar, const wchar_t* dnChar, int* mode );	//	対括弧の前方検索	// modeの追加 02/09/19 ai
	bool  SearchBracketBackward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const wchar_t* dnChar, const wchar_t* upChar, int* mode );	//	対括弧の後方検索	// modeの追加 02/09/19 ai
//@@@ 2001.02.03 Start by MIK: 全角の対括弧
	bool  SearchBracketForward2( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
								 const wchar_t* upChar, const wchar_t* dnChar, int* mode );	//	対括弧の前方検索	// modeの追加 02/09/19 ai
	bool  SearchBracketBackward2( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
								  const wchar_t* dnChar, const wchar_t* upChar, int* mode );	//	対括弧の後方検索	// modeの追加 02/09/19 ai
//@@@ 2001.02.03 End

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// 補完ウィンドウ用のキーワードヘルプ表示

// 2002/01/19 novice public属性に変更
	BOOL GetSelectedData( CNativeW*, BOOL, const wchar_t*, BOOL, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN);/* 選択範囲のデータを取得 */
	//	Aug. 25, 2002 genta protected->publicに移動
	bool IsImeON( void );	// IME ONか	// 2006.12.04 ryoji
	int HokanSearchByFile( const wchar_t*, BOOL, CNativeW**, int, int ); // 2003.06.25 Moca

	/*!	CEditView::KeyWordHelpSearchDictのコール元指定用ローカルID
		@date 2006.04.10 fon 新規作成
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon
	BOOL KeySearchCore( const CNativeW* pcmemCurText );	// 2006.04.10 fon
	
	//	Jan. 10, 2005 インクリメンタルサーチ
	bool IsISearchEnabled(int nCommand) const;

	//	Jan.  8, 2006 genta 折り返しトグル動作判定
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( CLayoutInt* newKetas );

	void _SetDragMode(BOOL b)
	{
		m_bDragMode = b;
	}
public:
	/*
	||  実装ヘルパ関数
	*/
	void GetCurrentTextForSearch( CNativeW& );			/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	void GetCurrentTextForSearchDlg( CNativeW& );		/* 現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用） 2006.08.23 ryoji */
public:
//	CEOL GetCurrentInsertEOL( void );					/* 現在、Enterなどで挿入する改行コードの種類を取得 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      クリップボード                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//取得
	BOOL MyGetClipboardData( CNativeW&, BOOL*, BOOL* = NULL );			/* クリップボードからデータを取得 */

	//設定
	BOOL MySetClipboardData( const ACHAR*, int, bool bColmnSelect, bool = FALSE );	/* クリップボードにデータを設定 */
	BOOL MySetClipboardData( const WCHAR*, int, bool bColmnSelect, bool = FALSE );	/* クリップボードにデータを設定 */

	void CopyCurLine( bool bAddCRLFWhenCopy, enumEOLType neweol, bool bEnableLineModePaste );	/* カーソル行をクリップボードにコピーする */	// 2007.10.08 ryoji


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	void OnPaintOld( HDC, PAINTSTRUCT *, BOOL );	/* 通常の描画処理 */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* 通常の描画処理 */
	void OnSize( int, int );							/* ウィンドウサイズの変更処理 */
	CLayoutInt  OnVScroll( int, int );							/* 垂直スクロールバーメッセージ処理 */
	CLayoutInt  OnHScroll( int, int );							/* 水平スクロールバーメッセージ処理 */
	void OnLBUTTONDOWN( WPARAM, int, int );				/* マウス左ボタン押下 */
	void OnMOUSEMOVE( WPARAM, int, int );				/* マウス移動のメッセージ処理 */
	void OnLBUTTONUP( WPARAM, int, int );				/* マウス左ボタン開放のメッセージ処理 */
	void OnLBUTTONDBLCLK( WPARAM, int , int );			/* マウス左ボタンダブルクリック */
	void OnRBUTTONDOWN( WPARAM, int, int );				/* マウス右ボタン押下 */
	void OnRBUTTONUP( WPARAM, int, int );				/* マウス右ボタン開放 */
// novice 2004/10/11 マウス中ボタン対応
	void OnMBUTTONDOWN( WPARAM, int, int );				/* マウス中ボタン押下 */
// novice 2004/10/10 マウスサイドボタン対応
	void OnXLBUTTONDOWN( WPARAM, int, int );			/* マウス左サイドボタン押下 */
	void OnXRBUTTONDOWN( WPARAM, int, int );			/* マウス右サイドボタン押下 */
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );				/* マウスホイールのメッセージ処理 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void SetCurrentColor( HDC, int );							/* 現在の色を指定 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// 行のテキスト／選択状態の描画
	//	int DispLine( HDC, int, int, int, const unsigned char*, int, BOOL );
	//2007.08.25 kobake 戻り値をintからboolに変更
	bool DispLineNew(
		HDC				hdc,			//!< [in]     作画対象
		const CLayout*	pcLayout,		//!< [in]     表示を開始するレイアウト
		DispPos*		pDispPos,		//!< [in/out] 描画する箇所、描画元ソース
		CLayoutInt		nLineTo,		//!< [in]     作画終了するレイアウト行番号
		bool			bSelected		//!< [in]     選択中か
	);

	void DispTextSelected( HDC hdc, CLayoutInt nLineNum, const CMyPoint& ptXY, CLayoutInt nX_Layout );	/* テキスト反転 */
//	int	DispCtrlCode( HDC, int, int, const unsigned char*, int );	/* コントロールコード表示 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           計算                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void CopySelectedAllLines( const wchar_t*, BOOL );			/* 選択範囲内の全行をクリップボードにコピーする */
	void ConvSelectedArea( EFunctionCode );								/* 選択エリアのテキストを指定方法で変換 */

public:
	

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt  ScrollAtV( CLayoutInt );										/* 指定上端行位置へスクロール */
	CLayoutInt  ScrollAtH( CLayoutInt );										/* 指定左端桁位置へスクロール */
	//	From Here Sep. 11, 2004 genta ずれ維持の同期スクロール
	CLayoutInt  ScrollByV( CLayoutInt vl ){	return ScrollAtV( GetTextArea().GetViewTopLine() + vl );}			/* 指定行スクロール*/
	CLayoutInt  ScrollByH( CLayoutInt hl ){	return ScrollAtH( GetTextArea().GetViewLeftCol() + hl );}					/* 指定桁スクロール */
public:
	void SyncScrollV( CLayoutInt );									/* 垂直同期スクロール */
	void SyncScrollH( CLayoutInt );									/* 水平同期スクロール */

	void DrawBracketPair( bool );								/* 対括弧の強調表示 02/09/18 ai */
	void SetBracketPairPos( bool );								/* 対括弧の強調表示位置設定 03/02/18 ai */
protected:
	int GetColorIndex( HDC, const CLayout*, int );				/* 指定位置のColorIndexの取得 02/12/13 ai */
	bool IsBracket( const wchar_t*, CLogicInt, CLogicInt );					/* 括弧判定 03/01/09 ai */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        過去の遺産                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

public:
	void SetIMECompFormPos( void );								/* IME編集エリアの位置を変更 */
public:
	void SetIMECompFormFont( void );							/* IME編集エリアの表示フォントを変更 */
	void SetParentCaption( void );								/* 親ウィンドウのタイトルを更新 */	// 2007.03.08 ryoji bKillFocusパラメータを除去
public:

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         各種判定                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool IsCurrentPositionURL( const CLayoutPoint& ptCaretPos, CLogicRange* pUrlRange, std::wstring* pwstrURL );/* カーソル位置にURLが有る場合のその範囲を調べる */
	BOOL CheckTripleClick( CMyPoint ptMouse );							/* トリプルクリックをチェックする */	// 2007.10.02 nasukoji
	int IsCurrentPositionSelected( CLayoutPoint ptCaretPos );					/* 指定カーソル位置が選択エリア内にあるか */
	int IsCurrentPositionSelectedTEST( const CLayoutPoint& ptCaretPos, const CLayoutRange& sSelect ) const;/* 指定カーソル位置が選択エリア内にあるか */
	bool IsSearchString( const wchar_t*, CLogicInt, CLogicInt, CLogicInt*, CLogicInt* );	/* 現在位置が検索文字列に該当するか */	//2002.02.08 hor 引数追加


	void ExecCmd(const TCHAR*, int ) ;							// 子プロセスの標準出力をリダイレクトする
	void AddToCmdArr( const TCHAR* );
	BOOL ChangeCurRegexp(void);									// 2002.01.16 hor 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときはFALSEを返す)
	void SendStatusMessage( const TCHAR* msg );					// 2002.01.26 hor 検索／置換／ブックマーク検索時の状態をステータスバーに表示する
	void SendStatusMessage2( const TCHAR* msg );					// Jul. 9, 2005 genta
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode);	/* 再変換用構造体を設定する 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode);				/* 再変換用構造体の情報を元に選択範囲を変更する 2002.04.09 minfu */

public: /* テスト用にアクセス属性を変更 */
	/* IDropTarget実装 */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );


public:


	/* 指定位置の指定長データ削除 */
	void CEditView::DeleteData2(
		const CLayoutPoint&	ptCaretPos,
		CLogicInt			nDelLen,
		CNativeW*			pcMem
	);

	void DeleteData( bool bRedraw );/* 現在位置のデータ削除 */

	/* 現在位置にデータを挿入 */
	void InsertData_CEditView(
		CLayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		int				nDataLen,
		CLayoutPoint*	pptNewPos,	//挿入された部分の次の位置のデータ位置
		bool			bRedraw
	);
	void SmartIndent_CPP( wchar_t );	/* C/C++スマートインデント処理 */
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji 前の行にある末尾の空白を削除 */


	/* データ置換 削除&挿入にも使える */
	void ReplaceData_CEditView(
		CLayoutRange	sDelRange,			// 削除範囲。レイアウト単位。
		CNativeW*		pcmemCopyOfDeleted,	// 削除されたデータのコピー(NULL可能)
		const wchar_t*	pInsData,			// 挿入するデータ
		CLogicInt		nInsDataLen,		// 挿入するデータの長さ
		bool			bRedraw
	);

	//	Jan. 10, 2005 genta HandleCommandからgrep関連処理を分離
	void TranslateCommand_grep( EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );


	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose追加
	//	Feb. 17, 2007 genta 相対パスの基準ディレクトリ指示を追加
	bool TagJumpSub( const TCHAR* pszJumpToFile, CMyPoint ptJumpTo, bool bClose = false, bool bRelFromIni = false );


//2004.10.13 インクリメンタルサーチ関係
	void TranslateCommand_isearch( EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	bool ProcessCommand_isearch( int, bool, LPARAM, LPARAM, LPARAM, LPARAM );

	int	CreatePopUpMenu_R( void );		/* ポップアップメニュー(右クリック) */

	/* 支援 */
	//	Jan. 10, 2005 genta HandleCommandから補完関連処理を分離
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);

	void ShowHokanMgr( CNativeW& cmemData, BOOL bAutoDecided );	//	補完ウィンドウを表示する。Ctrl+Spaceや、文字の入力/削除時に呼び出されます。 YAZAKI 2002/03/11


	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//現在行を履歴に追加する

	void AnalyzeDiffInfo( const char*, int );	/* DIFF情報の解析 */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( TCHAR*, HWND );	/* DIFF一時ファイル作成 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	void ViewDiffInfo( const TCHAR*, const TCHAR*, int );		/* DIFF差分表示 */		//2005.10.29 maru
	
	BOOL OPEN_ExtFromtoExt( BOOL, BOOL, const TCHAR* [], const TCHAR* [], int, int, const TCHAR* ); // 指定拡張子のファイルに対応するファイルを開く補助関数 // 2003.08.12 Moca


public:
	//ATOK専用再変換のAPI
	HMODULE m_hAtokModule;
	BOOL (WINAPI *AT_ImmSetReconvertString)( HIMC , int ,PRECONVERTSTRING , DWORD  );
private:

	/* インクリメンタルサーチ */ 
	//2004.10.24 isearch migemo
	void ISearchEnter( int mode, ESearchDirection direction);
	void ISearchExit();
	void ISearchExec(WORD wChar);
	void ISearchExec(const char* pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CNativeT* msg) const;



public:
	//主要構成部品アクセス
	CTextArea& GetTextArea(){ assert(m_pcTextArea); return *m_pcTextArea; }
	const CTextArea& GetTextArea() const{ assert(m_pcTextArea); return *m_pcTextArea; }
	CCaret& GetCaret(){ assert(m_pcCaret); return *m_pcCaret; }
	const CCaret& GetCaret() const{ assert(m_pcCaret); return *m_pcCaret; }
	CRuler& GetRuler(){ assert(m_pcRuler); return *m_pcRuler; }
	const CRuler& GetRuler() const{ assert(m_pcRuler); return *m_pcRuler; }

	//主要属性アクセス
	CTextMetrics& GetTextMetrics(){ return m_cTextMetrics; }
	const CTextMetrics& GetTextMetrics() const{ return m_cTextMetrics; }
	CViewSelect& GetSelectionInfo(){ return m_cViewSelect; }
	const CViewSelect& GetSelectionInfo() const{ return m_cViewSelect; }

	//主要オブジェクトアクセス
	CViewFont& GetFontset(){ assert(m_pcFontset); return *m_pcFontset; }
	const CViewFont& GetFontset() const{ assert(m_pcFontset); return *m_pcFontset; }

	//主要ヘルパアクセス
	const CViewParser& GetParser() const{ return m_cParser; }
	const CTextDrawer& GetTextDrawer() const{ return m_cTextDrawer; }
	CViewCommander& GetCommander(){ return m_cCommander; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//参照
	CEditWnd*	m_pcEditWnd;	//!< ウィンドウ
	CEditDoc*	m_pcEditDoc;	//!< ドキュメント

	//主要構成部品
	CTextArea*		m_pcTextArea;
	CCaret*			m_pcCaret;
	CRuler*			m_pcRuler;

	//主要属性
	CTextMetrics	m_cTextMetrics;
	CViewSelect		m_cViewSelect;

	//主要オブジェクト
	CViewFont*		m_pcFontset;

	//主要ヘルパ
	CViewParser	m_cParser;
	CTextDrawer		m_cTextDrawer;
	CViewCommander	m_cCommander;


public: /* テスト用にアクセス属性を変更 */
	CDropTarget*	m_pcDropTarget;
private:
	bool			m_bDrawSWITCH;
public:
	BOOL			m_bDragSource;	/* 選択テキストのドラッグ中か */
	BOOL			m_bDragMode;	/* 選択テキストのドラッグ中か */

	/* 単語検索の状態 */

	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	CBregexp	m_CurRegexp;			/*!< コンパイルデータ */
	bool		m_bCurSrchKeyMark;			/* 検索文字列のマーク */
	wchar_t		m_szCurSrchKey[_MAX_PATH];	/* 検索文字列 */
	
	SSearchOption m_sCurSearchOption;	// 検索／置換  オプション

	bool	m_bExecutingKeyMacro;		/* キーボードマクロの実行中 */
	HWND	m_hWnd;				/* 編集ウィンドウハンドル */

	CLogicPoint	m_ptSrchStartPos_PHY;	// 検索/置換開始時のカーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))

	BOOL		m_bSearch;				/* 検索/置換開始位置を登録するか */											// 02/06/26 ai
//	CLogicInt	m_nCharSize;			/* 対括弧の文字サイズ */	// 02/09/18 ai 
	
	CLogicPoint	m_ptBracketCaretPos_PHY;// 前カーソル位置の括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	CLogicPoint	m_ptBracketPairPos_PHY;	// 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	BOOL	m_bDrawBracketPairFlag;	/* 対括弧の強調表示を行なうか */						// 03/02/18 ai

	TCHAR*			m_pszAppName;	/* Mutex作成用・ウィンドウクラス名 */
	HINSTANCE		m_hInstance;	/* インスタンスハンドル */
	HWND			m_hwndParent;	/* 親ウィンドウハンドル */

	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	COpeBlk*		m_pcOpeBlk;			/* 操作ブロック */
	BOOL			m_bDoing_UndoRedo;	/* アンドゥ・リドゥの実行中か */
	HWND			m_hwndVScrollBar;	/* 垂直スクロールバーウィンドウハンドル */
	int				m_nVScrollRate;		/* 垂直スクロールバーの縮尺 */
	HWND			m_hwndHScrollBar;	/* 水平スクロールバーウィンドウハンドル */
	HWND			m_hwndSizeBox;		/* サイズボックスウィンドウハンドル */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* 垂直分割ボックス */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* 水平分割ボックス */

	HDC				m_hdcCompatDC;		/* 再描画用コンパチブルＤＣ */
	HBITMAP			m_hbmpCompatBMP;	/* 再描画用メモリＢＭＰ */
	HBITMAP			m_hbmpCompatBMPOld;	/* 再描画用メモリＢＭＰ(OLD) */
public:
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* コマンドの実行中 */

	/* 分割状態 */
	int				m_nMyIndex;

	/* 入力状態 */
	COLORREF	m_crBack;			/* テキストの背景色 */			// 2006.12.07 ryoji
	int		m_nOldUnderLineY;




	/* 表示方法 */
public:
	/* 辞書Tip関連 */
	DWORD	m_dwTipTimer;			/* Tip起動タイマー */
	CTipWnd	m_cTipWnd;				/* Tip表示ウィンドウ */
	POINT	m_poTipCurPos;			/* Tip起動時のマウスカーソル位置 */
	BOOL	m_bInMenuLoop;			/* メニュー モーダル ループに入っています */
	CDicMgr	m_cDicMgr;				/* 辞書マネージャ */

	/* 入力補完 */
	BOOL		m_bHokan;			//	補完中か？＝補完ウィンドウが表示されているか？かな？

	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump履歴
	CRegexKeyword	*m_cRegexKeyword;	//@@@ 2001.11.17 add MIK

private:
	UINT	m_uMSIMEReconvertMsg;
	UINT	m_uATOKReconvertMsg;
public:
	UINT	m_uWM_MSIME_RECONVERTREQUEST;
private:
	
	int		m_nLastReconvLine;             //2002.04.09 minfu 再変換情報保存用;
	int		m_nLastReconvIndex;            //2002.04.09 minfu 再変換情報保存用;

	bool	m_bUnderLineON;

	CMigemo* m_pcmigemo;

	ESearchDirection m_nISearchDirection;
	int m_nISearchMode;
	bool m_bISearchWrap;

	CLayoutRange m_sISearchHistory[256];

	bool m_bISearchFlagHistory[256];
	int m_nISearchHistoryCount;
	bool m_bISearchFirst;

	// 2007.10.02 nasukoji
	bool	m_bActivateByMouse;		//!< マウスによるアクティベート

	// 2007.10.02 nasukoji
	DWORD	m_dwTripleClickCheck;	//!< トリプルクリックチェック用時刻
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
