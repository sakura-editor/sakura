/*!	@file
	@brief タブウィンドウ

	@author MIK
	@date 2003.5.30
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK, Kazika
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2012, Moca, syat
	Copyright (C) 2013, Uchi, aroka, novice, syat, ryoji

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


#ifndef SAKURA_WINDOW_CTABWND_H_
#define SAKURA_WINDOW_CTABWND_H_

#include "CWnd.h"
#include "util/design_template.h"

class CGraphics;
struct EditNode;
struct DLLSHAREDATA;

//! タブバーウィンドウ
class CTabWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTabWnd();
	virtual ~CTabWnd();

	/*
	|| メンバ関数
	*/
	HWND Open( HINSTANCE, HWND );		/*!< ウィンドウ オープン */
	void Close( void );					/*!< ウィンドウ クローズ */
	void TabWindowNotify( WPARAM wParam, LPARAM lParam );
	void Refresh( BOOL bEnsureVisible = TRUE, BOOL bRebuild = FALSE );			// 2006.02.06 ryoji 引数削除
	void NextGroup( void );			/* 次のグループ */			// 2007.06.20 ryoji
	void PrevGroup( void );			/* 前のグループ */			// 2007.06.20 ryoji
	void MoveRight( void );			/* タブを右に移動 */		// 2007.06.20 ryoji
	void MoveLeft( void );			/* タブを左に移動 */		// 2007.06.20 ryoji
	void Separate( void );			/* 新規グループ */			// 2007.06.20 ryoji
	void JoinNext( void );			/* 次のグループに移動 */	// 2007.06.20 ryoji
	void JoinPrev( void );			/* 前のグループに移動 */	// 2007.06.20 ryoji

	LRESULT TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT TabListMenu( POINT pt, BOOL bSel = TRUE, BOOL bFull = FALSE, BOOL bOtherGroup = TRUE );	/*!< タブ一覧メニュー作成処理 */	// 2006.03.23 fon

	void SizeBox_ONOFF( bool bSizeBox );
	HWND GetHwndSizeBox(){
		return m_hwndSizeBox;
	}
	void OnSize(){
		OnSize( GetHwnd(), WM_SIZE, 0, 0 );
	}
	void UpdateStyle();
protected:
	/*
	|| 実装ヘルパ系
	*/
	int FindTabIndexByHWND( HWND hWnd );
	void AdjustWindowPlacement( void );							/*!< 編集ウィンドウの位置合わせ */	// 2007.04.03 ryoji
	int SetCarmWindowPlacement( HWND hwnd, const WINDOWPLACEMENT* pWndpl );	/* アクティブ化の少ない SetWindowPlacement() を実行する */	// 2007.11.30 ryoji
	void ShowHideWindow( HWND hwnd, BOOL bDisp );
	void HideOtherWindows( HWND hwndExclude );					/*!< 他の編集ウィンドウを隠す */	// 2007.05.17 ryoji
	void ForceActiveWindow( HWND hwnd );
	void TabWnd_ActivateFrameWindow( HWND hwnd, bool bForce = true );	//2004.08.27 Kazika 引数追加
	HWND GetNextGroupWnd( void );	/* 次のグループの先頭ウィンドウを探す */	// 2007.06.20 ryoji
	HWND GetPrevGroupWnd( void );	/* 前のグループの先頭ウィンドウを探す */	// 2007.06.20 ryoji
	void GetTabName( EditNode* pEditNode, BOOL bFull, BOOL bDupamp, LPTSTR pszName, int nLen );	/* タブ名取得処理 */	// 2007.06.28 ryoji 新規作成

	/* 仮想関数 */
	virtual void AfterCreateWindow( void ){}	/*!< ウィンドウ作成後の処理 */	// 2007.03.13 ryoji 可視化しない

	/* 仮想関数 メッセージ処理 */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );		/*!< WM_SIZE処理 */
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );	/*!< WM_DSESTROY処理 */
	virtual LRESULT OnNotify( HWND, UINT, WPARAM, LPARAM );		/*!< WM_NOTIFY処理 */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );		/*!< WM_PAINT処理 */
	virtual LRESULT OnCaptureChanged( HWND, UINT, WPARAM, LPARAM );	/*!< WM_CAPTURECHANGED 処理 */
	virtual LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONDOWN処理 */
	virtual LRESULT OnLButtonUp( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONUP処理 */
	virtual LRESULT OnRButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_RBUTTONDOWN処理 */
	virtual LRESULT OnLButtonDblClk( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONDBLCLK処理 */
	virtual LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM );	/*!< WM_MOUSEMOVE処理 */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );		/*!< WM_TIMER処理 */
	virtual LRESULT OnMeasureItem( HWND, UINT, WPARAM, LPARAM );	/*!< WM_MEASUREITEM処理 */
	virtual LRESULT OnDrawItem( HWND, UINT, WPARAM, LPARAM );		/*!< WM_DRAWITEM処理 */

	// 2005.09.01 ryoji ドラッグアンドドロップでタブの順序変更を可能に
	/* サブクラス化した Tab でのメッセージ処理 */
	LRESULT OnTabLButtonDown( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_LBUTTONDOWN 処理 */
	LRESULT OnTabLButtonUp( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_LBUTTONUP 処理 */
	LRESULT OnTabMouseMove( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_MOUSEMOVE 処理 */
	LRESULT OnTabTimer( WPARAM wParam, LPARAM lParam );			/*!< タブ部 WM_TIMER処理 */
	LRESULT OnTabCaptureChanged( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_CAPTURECHANGED 処理 */
	LRESULT OnTabRButtonDown( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_RBUTTONDOWN 処理 */
	LRESULT OnTabRButtonUp( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_RBUTTONUP 処理 */
	LRESULT OnTabMButtonDown( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_MBUTTONDOWN 処理 */
	LRESULT OnTabMButtonUp( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_MBUTTONUP 処理 */
	LRESULT OnTabNotify( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_NOTIFY 処理 */

	//実装補助インターフェース
	void BreakDrag( void ) { if( ::GetCapture() == m_hwndTab ) ::ReleaseCapture(); m_eDragState = DRAG_NONE; m_nTabCloseCapture = -1; }	/*!< ドラッグ状態解除処理 */
	BOOL ReorderTab( int nSrcTab, int nDstTab );	/*!< タブ順序変更処理 */
	void BroadcastRefreshToGroup( void );
	BOOL SeparateGroup( HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop );	/*!< タブ分離処理 */	// 2007.06.20 ryoji
	LRESULT ExecTabCommand( int nId, POINTS pts );	/*!< タブ部 コマンド実行処理 */
	void LayoutTab( void );							/*!< タブのレイアウト調整処理 */

	HIMAGELIST InitImageList( void );				/*!< イメージリストの初期化処理 */
	int GetImageIndex( EditNode* pNode );			/*!< イメージリストのインデックス取得処理 */
	HIMAGELIST ImageList_Duplicate( HIMAGELIST himl );	/*!< イメージリストの複製処理 */

	// 2006.02.01 ryoji タブ一覧を追加
	void DrawBtnBkgnd( HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted );	/*!< ボタン背景描画処理 */	// 2006.10.21 ryoji
	void DrawListBtn( CGraphics& gr, const LPRECT lprcClient );			/*!< 一覧ボタン描画処理 */
	void DrawCloseFigure( CGraphics& gr, const RECT &btnRect );			/*!< 閉じるマーク描画処理 */
	void DrawCloseBtn( CGraphics& gr, const LPRECT lprcClient );			/*!< 閉じるボタン描画処理 */		// 2006.10.21 ryoji
	void DrawTabCloseBtn( CGraphics& gr, const LPRECT lprcClient, bool selected, bool bHover );	/*!< タブを閉じるボタン描画処理 */		// 2012.04.14 syat
	void GetListBtnRect( const LPRECT lprcClient, LPRECT lprc );	/*!< 一覧ボタンの矩形取得処理 */
	void GetCloseBtnRect( const LPRECT lprcClient, LPRECT lprc );	/*!< 閉じるボタンの矩形取得処理 */	// 2006.10.21 ryoji
	void GetTabCloseBtnRect( const LPRECT lprcClient, LPRECT lprc, bool selected );	/*!< タブを閉じるボタンの矩形取得処理 */	// 2012.04.14 syat

	HFONT CreateMenuFont( void )
	{
		// メニュー用フォント作成
		NONCLIENTMETRICS	ncm;
		// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する	// 2007.12.21 ryoji
		ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );
		::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );
		return ::CreateFontIndirect( &ncm.lfMenuFont );
	}

protected:
	enum DragState { DRAG_NONE, DRAG_CHECK, DRAG_DRAG };
	enum CaptureSrc { CAPT_NONE, CAPT_CLOSE };

	typedef HIMAGELIST (WINAPI *FN_ImageList_Duplicate)(HIMAGELIST himl);

	/*
	|| メンバ変数
	*/
public:
	DLLSHAREDATA*	m_pShareData;	/*!< 共有データ */
	HFONT			m_hFont;		/*!< 表示用フォント */
	HWND			m_hwndTab;		/*!< タブコントロール */
	HWND			m_hwndToolTip;	/*!< ツールチップ（ボタン用） */
	TCHAR			m_szTextTip[1024];	/*!< ツールチップのテキスト（タブ用） */
	ETabPosition	m_eTabPosition;	//!< タブ表示位置

private:
	DragState	m_eDragState;			//!< ドラッグ状態
	int			m_nSrcTab;				//!< 移動元タブ
	POINT		m_ptSrcCursor;			//!< ドラッグ開始カーソル位置
	HCURSOR		m_hDefaultCursor;		//!< ドラッグ開始時のカーソル

	// 2006.01.28 ryoji タブへのアイコン表示を可能に
	FN_ImageList_Duplicate	m_RealImageList_Duplicate;

	HIMAGELIST	m_hIml;					//!< イメージリスト
	HICON		m_hIconApp;				//!< アプリケーションアイコン
	HICON		m_hIconGrep;			//!< Grepアイコン
	int			m_iIconApp;				//!< アプリケーションアイコンのインデックス
	int			m_iIconGrep;			//!< Grepアイコンのインデックス

	BOOL		m_bVisualStyle;			//!< ビジュアルスタイルかどうか	// 2007.04.01 ryoji
	BOOL		m_bHovering;
	BOOL		m_bListBtnHilighted;
	BOOL		m_bCloseBtnHilighted;	//!< 閉じるボタンハイライト状態	// 2006.10.21 ryoji
	CaptureSrc	m_eCaptureSrc;			//!< キャプチャー元
	BOOL		m_bTabSwapped;			//!< ドラッグ中にタブの入れ替えがあったかどうか
	LONG*		m_nTabBorderArray;		//!< ドラッグ前のタブ境界位置配列
	LOGFONT		m_lf;					//!< 表示フォントの特性情報
	bool		m_bMultiLine;			//!< 複数行

	// タブ内の閉じるボタン用変数
	int			m_nTabHover;			//!< マウスカーソル下のタブ（無いときは-1）
	bool		m_bTabCloseHover;		//!< マウスカーソル下にタブ内の閉じるボタンがあるか
	int			m_nTabCloseCapture;		//!< 閉じるボタンがマウス押下されているタブ（無いときは-1）

	HWND		m_hwndSizeBox;
	bool		m_bSizeBox;

private:
	DISALLOW_COPY_AND_ASSIGN(CTabWnd);
};

#endif /* SAKURA_WINDOW_CTABWND_H_ */



