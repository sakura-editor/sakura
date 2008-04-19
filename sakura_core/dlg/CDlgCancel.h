/*!	@file
	@brief キャンセルボタンダイアログ

	@author Norio Nakatani
	@date 1998/09/09 作成
    @date 1999/12/02 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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
	int DoModal( HINSTANCE, HWND, int );	/* モードレスダイアログの表示 */
	HWND DoModeless( HINSTANCE, HWND, int );	/* モードレスダイアログの表示 */

//	HWND Open( LPCTSTR );
//	void Close( void );	/* モードレスダイアログの削除 */
	BOOL IsCanceled( void ){ return m_bCANCEL; } /* IDCANCELボタンが押されたか？ */
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

//	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
//	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
//	HWND		m_hWnd;			/* このダイアログのハンドル */
	BOOL		m_bCANCEL;		/* IDCANCELボタンが押された */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCANCEL_H_ */



