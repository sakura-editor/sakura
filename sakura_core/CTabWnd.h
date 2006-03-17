//	$Id$
/*!	@file
	@brief タブウィンドウ

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK
	Copyright (C) 2005, ryoji

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

class CTabWnd;

#ifndef _CTABWND_H_
#define _CTABWND_H_

#include "CWnd.h"
#include "CEditDoc.h"
#include "CShareData.h"
#include <string>
#include <map>

//! ファンクションキーウィンドウ
class SAKURA_CORE_API CTabWnd : public CWnd
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
	void ForceActiveWindow( HWND hwnd );
	void TabWnd_ActivateFrameWindow( HWND hwnd, bool bForce = true );	//2004.08.27 Kazika 引数追加

	LRESULT TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	/*
	|| メンバ変数
	*/
	LPCTSTR			m_pszClassName;	/*!< クラス名 */
	DLLSHAREDATA*	m_pShareData;	/*!< 共有データ */
	HFONT			m_hFont;		/*!< 表示用フォント */
	HWND			m_hwndTab;		/*!< タブコントロール */
	HWND			m_hwndToolTip;	/*!< ツールチップ */
	TCHAR			m_szTextTip1[1024];
#ifdef UNICODE
	//※現在扱っている文字コードとは逆の文字コードを指定する。
	char			m_szTextTip2[1024];	//!< SJIS文字列でのツールチップ
#else
	//※現在扱っている文字コードとは逆の文字コードを指定する。
	wchar_t			m_szTextTip2[1024];	//!< UNICODE文字列でのツールチップ
#endif	//UNICODE

protected:
	/*
	|| 実装ヘルパ系
	*/
	void Refresh( void );	// 2006.02.06 ryoji 引数削除
	int FindTabIndexByHWND( HWND hWnd );
	void ShowHideWindow( HWND hwnd, BOOL bDisp );
	int GetFirstOpenedWindow( void );

	/* 仮想関数 メッセージ処理 */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );		/*!< WM_SIZE処理 */
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );	/*!< WM_DSESTROY処理 */
	virtual LRESULT OnNotify( HWND, UINT, WPARAM, LPARAM );		/*!< WM_NOTIFY処理 */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );		/*!< WM_PAINT処理 */
	virtual LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONDOWN処理 */
	virtual LRESULT OnRButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_RBUTTONDOWN処理 */
	virtual LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM );	/*!< WM_MOUSEMOVE処理 */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );		/*!< WM_TIMER処理 */
	virtual LRESULT OnMeasureItem( HWND, UINT, WPARAM, LPARAM );	/*!< WM_MEASUREITEM処理 */
	virtual LRESULT OnDrawItem( HWND, UINT, WPARAM, LPARAM );		/*!< WM_DRAWITEM処理 */

	// 2005.09.01 ryoji ドラッグアンドドロップでタブの順序変更を可能に
	/* サブクラス化した Tab でのメッセージ処理 */
	LRESULT OnTabLButtonDown( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_LBUTTONDOWN 処理 */
	LRESULT OnTabLButtonUp( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_LBUTTONUP 処理 */
	LRESULT OnTabMouseMove( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_MOUSEMOVE 処理 */
	LRESULT OnTabCaptureChanged( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_CAPTURECHANGED 処理 */
	LRESULT OnTabRButtonDown( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_RBUTTONDOWN 処理 */
	LRESULT OnTabRButtonUp( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_RBUTTONUP 処理 */
	LRESULT OnTabMButtonDown( WPARAM wParam, LPARAM lParam );	/*!< タブ部 WM_MBUTTONDOWN 処理 */
	LRESULT OnTabMButtonUp( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_MBUTTONUP 処理 */
	LRESULT OnTabNotify( WPARAM wParam, LPARAM lParam );		/*!< タブ部 WM_NOTIFY 処理 */

	void BreakDrag( void ) { if( ::GetCapture() == m_hwndTab ) ::ReleaseCapture(); m_eDragState = DRAG_NONE; }	/*!< ドラッグ状態解除処理 */
	BOOL ReorderTab( int nSrcTab, int nDstTab );	/*!< タブ順序変更処理 */
	LRESULT ExecTabCommand( int nId, POINTS pts );	/*!< タブ部 コマンド実行処理 */
	void LayoutTab( void );							/*!< タブのレイアウト調整処理 */

	enum DragState { DRAG_NONE, DRAG_CHECK, DRAG_DRAG };

	DragState m_eDragState;		 //!< ドラッグ状態
	int	m_nSrcTab;				 //!< 移動元タブ

	// 2006.01.28 ryoji タブへのアイコン表示を可能に
	HIMAGELIST (WINAPI *m_RealImageList_Duplicate)(HIMAGELIST himl);
	HIMAGELIST m_hIml;								/*!< イメージリスト */
	HICON m_hIconApp;								/*!< アプリケーションアイコン */
	HICON m_hIconGrep;								/*!< Grepアイコン */
	int m_iIconApp;									/*!< アプリケーションアイコンのインデックス */
	int m_iIconGrep;								/*!< Grepアイコンのインデックス */
	HIMAGELIST InitImageList( void );				/*!< イメージリストの初期化処理 */
	int GetImageIndex( EditNode* pNode );			/*!< イメージリストのインデックス取得処理 */
	HIMAGELIST ImageList_Duplicate( HIMAGELIST himl );	/*!< イメージリストの複製処理 */

	// 2006.02.01 ryoji タブ一覧を追加
	void DrawListBtn( HDC hdc, const LPRECT lprcClient );			/*!< 一覧ボタン描画処理 */
	void GetListBtnRect( const LPRECT lprcClient, LPRECT lprc );	/*!< 一覧ボタンの矩形取得処理 */
	LRESULT OnListBtnClick( POINTS pts, BOOL bLeft );				/*!< 一覧ボタンクリック処理 */
	BOOL m_bHovering;
	BOOL m_bListBtnHilighted;
	HFONT CreateMenuFont( void )
	{
		// メニュー用フォント作成
		NONCLIENTMETRICS	ncm;
		ncm.cbSize = sizeof( NONCLIENTMETRICS );
		::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), (PVOID)&ncm, 0 );
		return ::CreateFontIndirect( &ncm.lfMenuFont );
	}
};

#endif /* _CTABWND_H_ */

/*[EOF]*/
