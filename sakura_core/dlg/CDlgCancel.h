/*!	@file
	@brief キャンセルボタンダイアログ

	@author Norio Nakatani
	@date 1998/09/09 作成
    @date 1999/12/02 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgCancel;

#ifndef _CDLGCANCEL_H_
#define _CDLGCANCEL_H_

#include "dlg/CDialog.h"

/*!
	@brief キャンセルボタンダイアログ
*/
class CDlgCancel : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCancel();
//	void Create( HINSTANCE, HWND );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete);	/* モードレスダイアログの表示 */
	HWND DoModeless(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete);	/* モードレスダイアログの表示 */

//	HWND Open( LPCTSTR );
//	void Close( void );	/* モードレスダイアログの削除 */
	BOOL IsCanceled( void ){ return m_bCANCEL; } /* IDCANCELボタンが押されたか？ */
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	/* ダイアログのメッセージ処理 *//* BOOL->INT_PTR 2008/7/18 Uchi*/
	void DeleteAsync( void );	/* 自動破棄を遅延実行する */	// 2008.05.28 ryoji

//	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
//	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
//	HWND		m_hWnd;			/* このダイアログのハンドル */
	BOOL		m_bCANCEL;		/* IDCANCELボタンが押された */
	bool		m_bAutoCleanup;	/* 自動後処理型 */	// 2008.05.28 ryoji

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnBnClicked(int wID);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCANCEL_H_ */

