//	$Id$
/*!	@file
	キャンセルボタンダイアログ

	@author Norio Nakatani
	@date 1998/09/09 作成
    @date 1999/12/02 再作成
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
/************************************************************************

	CDlgCancel.h

	UPDATE:	1999.12/02 再作成
	CREATE: 1998.09/09 新規作成
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

class CDlgCancel;

#ifndef _CDLGCANCEL_H_
#define _CDLGCANCEL_H_

//#include <windows.h>
//#include "CMemory.h"
#include "CDialog.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
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
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCANCEL_H_ */


/*[EOF]*/
