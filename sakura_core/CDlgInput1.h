//	$Id$
/*!	@file
	1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
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

class CDlgInput1;

#ifndef _CDLGINPUT1_H_
#define _CDLGINPUT1_H_

#include <windows.h>
#include "CMemory.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgInput1
{
public:
	/*
	||  Constructors
	*/
	CDlgInput1();
	~CDlgInput1();
	BOOL DoModal( HINSTANCE , HWND , const char* , const char* , int , char*  );	/* モードレスダイアログの表示 */
	/*
	||  Attributes & Operations
	*/
	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND		m_hWnd;			/* このダイアログのハンドル */

	const char*	m_pszTitle;		/* ダイアログタイトル */
	const char*	m_pszMessage;	/* メッセージ */
	int			m_nMaxTextLen;	/* 入力サイズ上限 */
//	char*		m_pszText;		/* テキスト */
	CMemory		m_cmemText;		/* テキスト */
protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGINPUT1_H_ */

/*[EOF]*/
