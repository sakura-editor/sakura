/*!	@file
	@brief ウィンドウの基本クラス

	@author Norio Nakatani
	@date 2000/01/11 新規作成
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_
#define SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_
#pragma once

#include "apiwrap/window/CGenericWnd.hpp"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//!	ウィンドウの基本クラス
/*!
	@par CWndクラスの基本的な機能
	@li ウィンドウ作成
	@li ウィンドウメッセージ配送

	@par 普通?のウィンドウの使用方法は以下の手順
	@li RegisterWC()	ウィンドウクラス登録
	@li Create()		ウィンドウ作成
*/
class CWnd : public apiwrap::window::CGenericWnd {
	using Me = CWnd;

protected:
	friend LRESULT CALLBACK CWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
public:
	/* Constructors */
	CWnd(const WCHAR* pszInheritanceAppend = L"");
	CWnd(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	~CWnd() override;

	using CGenericWnd::_SetHwnd;

	/*
	||  Attributes & Operations
	*/

	// ウィンドウクラス登録
	ATOM RegisterWC(
		HINSTANCE	hInstance,
		HICON		hIcon,			// Handle to the class icon.
		HICON		hIconSm,		// Handle to a small icon
		HCURSOR		hCursor,		// Handle to the class cursor.
		HBRUSH		hbrBackground,	// Handle to the class background brush.
		LPCWSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		LPCWSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
	);

	//ウィンドウ作成
	HWND Create(
		HWND		hwndParent,
		DWORD		dwExStyle,		// extended window style
		LPCWSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
		LPCWSTR		lpWindowName,	// pointer to window name
		DWORD		dwStyle,		// window style
		int			x,				// horizontal position of window
		int			y,				// vertical position of window
		int			nWidth,			// window width
		int			nHeight,		// window height
		HMENU		hMenu			// handle to menu, or child-window identifier
	);

	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
	/* 仮想関数 */
	virtual LRESULT DispatchEvent_WM_APP( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */

	/* 仮想関数 メッセージ処理(デフォルト動作) */
	#define DECLH(method) LRESULT method( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ){return CallDefWndProc( hwnd, msg, wp, lp );}
	virtual DECLH( OnCommand		);	// WM_COMMAND
	virtual DECLH( OnPaint			);	// WM_PAINT
	virtual DECLH( OnLButtonDown	);	// WM_LBUTTONDOWN
	virtual DECLH( OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH( OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH( OnRButtonDown	);	// WM_RBUTTONDOWN
	virtual DECLH( OnMButtonDown	);	// WM_MBUTTONDOWN
	virtual DECLH( OnMouseMove		);	// WM_MOUSEMOVE
	virtual DECLH( OnTimer			);	// WM_TIMER
	virtual DECLH( OnSize			);	// WM_SIZE
	virtual DECLH( OnDestroy		);	// WM_DSESTROY

	virtual DECLH( OnMeasureItem	);	// WM_MEASUREITEM
	virtual DECLH( OnNotify			);	// WM_NOTIFY	//@@@ 2003.05.31 MIK
	virtual DECLH( OnDrawItem		);	// WM_DRAWITEM	// 2006.02.01 ryoji
	virtual DECLH( OnCaptureChanged	);	// WM_CAPTURECHANGED	// 2006.11.30 ryoji

	/* デフォルトメッセージ処理 */
	virtual LRESULT CallDefWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );

public:
	//インターフェース
	HWND GetParentHwnd() const{ return m_hwndParent; }
	HINSTANCE GetAppInstance() const{ return m_hInstance; }

	//ウィンドウ標準操作
	void DestroyWindow();

private: // 2002/2/10 aroka アクセス権変更
	HINSTANCE	m_hInstance;	// アプリケーションインスタンスのハンドル
	HWND		m_hwndParent;	// オーナーウィンドウのハンドル

#ifdef _DEBUG
	WCHAR		m_szClassInheritances[1024];
#endif
};
#endif /* SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_ */
