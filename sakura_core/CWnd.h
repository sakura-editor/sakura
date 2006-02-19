//	$Id$
/*!	@file
	@brief ウィンドウの基本クラス

	@author Norio Nakatani
	@date 2000/01/11 新規作成
	$Revision$
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CWnd;

#ifndef _CWND_H_
#define _CWND_H_

#include <windows.h>
#include "global.h"
extern CWnd* gm_pCWnd;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//!	ウィンドウの基本クラス
/*!
	@par CWndクラスの基本的な機能
	@li ウィンドウ作成
	@li ウィンドウメッセージ配送

	@par 普通?のウィンドウの使用方法は以下の手順
	@li Init()		初期化
	@li RegisterWC()	ウィンドウクラス登録
	@li Create()		ウィンドウ作成
*/
class SAKURA_CORE_API CWnd
{
public:
	/* Constructors */
	CWnd();
	virtual ~CWnd();
	/*
	||  Attributes & Operations
	*/
protected: // 2002/2/10 aroka アクセス権変更
	char		m_szClassInheritances[1024];
	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
public:
	HWND		m_hWnd;			/* このダイアログのハンドル */

	void Init( HINSTANCE, HWND );/* 初期化 */
	ATOM RegisterWC( HICON, HICON, HCURSOR, HBRUSH, LPCTSTR, LPCTSTR );/* ウィンドウクラス作成 */
	HWND Create( DWORD, LPCTSTR, LPCTSTR, DWORD, int,  int, int, int, HMENU );/* 作成 */
	virtual LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );/* メッセージ配送 */
protected:
	/* 仮想関数 */
	virtual LRESULT DispatchEvent_WM_APP( HWND, UINT, WPARAM, LPARAM );/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
	virtual void PreviCreateWindow( void ){return;};/* ウィンドウ作成前の処理(クラス登録前) ( virtual )*/
	virtual void AfterCreateWindow( void ){::ShowWindow( m_hWnd, SW_SHOW );}/* ウィンドウ作成後の処理 ( virtual )*/

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	virtual LRESULT OnNcDestroy( HWND, UINT, WPARAM, LPARAM );// WM_NCDESTROY

	/* 仮想関数 メッセージ処理(デフォルト動作) */
	#define DECLH(method) LRESULT method( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ){return CallDefWndProc( hwnd, msg, wp, lp );}
	virtual DECLH( OnCreate			);	// WM_CREATE
	virtual DECLH( OnCommand		);	// WM_COMMAND
	virtual DECLH( OnPaint			);	// WM_PAINT
	virtual DECLH( OnLButtonDown	);	// WM_LBUTTONDOWN
	virtual DECLH( OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH( OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH( OnRButtonDown	);	// WM_RBUTTONDOWN
	virtual DECLH( OnRButtonUp		);	// WM_RBUTTONUP
	virtual DECLH( OnRButtonDblClk	);	// WM_RBUTTONDBLCLK
	virtual DECLH( OnMButtonDown	);	// WM_MBUTTONDOWN
	virtual DECLH( OnMButtonUp		);	// WM_MBUTTONUP
	virtual DECLH( OnMButtonDblClk	);	// WM_MBUTTONDBLCLK
	virtual DECLH( OnMouseMove		);	// WM_MOUSEMOVE
	virtual DECLH( OnMouseWheel		);	// WM_MOUSEWHEEL
	virtual DECLH( OnTimer			);	// WM_TIMER
	virtual DECLH( OnSize			);	// WM_SIZE
	virtual DECLH( OnMove			);	// WM_MOVE
	virtual DECLH( OnClose			);	// WM_CLOSE
	virtual DECLH( OnDestroy		);	// WM_DSESTROY
	virtual DECLH( OnQueryEndSession);	// WM_QUERYENDSESSION

	virtual DECLH( OnMeasureItem	);	// WM_MEASUREITEM
	virtual DECLH( OnMenuChar		);	// WM_MENUCHAR
	virtual DECLH( OnNotify			);	// WM_NOTIFY	//@@@ 2003.05.31 MIK
	virtual DECLH( OnDrawItem		);	// WM_DRAWITEM	// 2006.02.01 ryoji

//	virtual DECLH( OnNcCreate			);	// WM_NCCREATE
//	virtual DECLH( OnNcCalcSize			);	// WM_NCCALCSIZE
//	virtual DECLH( OnNcHitTest			);	// WM_NCHITTEST
//	virtual DECLH( OnNcPaint			);	// WM_NCPAINT
//	virtual DECLH( OnNcActivate			);	// WM_NCACTIVATE
//	virtual DECLH( OnNcMouseMove		);	// WM_NCMOUSEMOVE
//	virtual DECLH( OnNcLButtonDown		);	// WM_NCLBUTTONDOWN
//	virtual DECLH( OnNcLButtonUp		);	// WM_NCLBUTTONUP
//	virtual DECLH( OnNcLButtonDblClk	);	// WM_NCLBUTTONDBLCLK
//	virtual DECLH( OnNcRButtonDown		);	// WM_NCRBUTTONDOWN
//	virtual DECLH( OnNcRButtonUp		);	// WM_NCRBUTTONUP
//	virtual DECLH( OnNcRButtonDblClk	);	// WM_NCRBUTTONDBLCLK
//	virtual DECLH( OnNcMButtonDown		);	// WM_NCMBUTTONDOWN
//	virtual DECLH( OnNcMButtonUp		);	// WM_NCMBUTTONUP
//	virtual DECLH( OnNcMButtonDblClk	);	// WM_NCMBUTTONDBLCLK


	/* MDI用 */
	virtual DECLH( OnMDIActivate	);	// WM_MDIACTIVATE
//	virtual DECLH( OnMDICascade		);	// WM_MDICASCADE
//	virtual DECLH( OnMDICreate		);	// WM_MDICREATE
//	virtual DECLH( OnMDIDestroy		);	// WM_MDIDESTROY
//	virtual DECLH( OnMDIGetActive	);	// WM_MDIGETACTIVE
//	virtual DECLH( OnMDIIconArrange	);	// WM_MDIICONARRANGE
//	virtual DECLH( OnMDIMaximize	);	// WM_MDIMAXIMIZE
//	virtual DECLH( OnMDINext		);	// WM_MDINEXT
//	virtual DECLH( OnMDIRefreshMenu	);	// WM_MDIREFRESHMENU
//	virtual DECLH( OnMDIRestore		);	// WM_MDIRESTORE
//	virtual DECLH( OnMDISetMenu		);	// WM_MDISETMENU
//	virtual DECLH( OnMDITile		);	// WM_MDITILE


	/* デフォルトメッセージ処理 */
	virtual LRESULT CallDefWndProc( HWND, UINT, WPARAM, LPARAM );

};

///////////////////////////////////////////////////////////////////////
#endif /* _CWND_H_ */


/*[EOF]*/
