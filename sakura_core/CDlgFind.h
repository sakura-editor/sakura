//	$Id$
/*!	@file
	検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/02 再作成
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
#include "CDialog.h"

#ifndef _CDLGFIND_H_
#define _CDLGFIND_H_


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgFind : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFind();
	/*
	||  Attributes & Operations
	*/
//	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */
	HWND DoModeless( HINSTANCE, HWND, LPARAM );	/* モードレスダイアログの表示 */
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	void ChangeView( LPARAM );

	int		m_bLoHiCase;	/* 英大文字と英小文字を区別する */
	int		m_bWordOnly;	/* 一致する単語のみ検索する */
	int		m_bRegularExp;	/* 正規表現 */
	int		m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */
	char	m_szText[_MAX_PATH + 1];	/* 検索文字列 */


protected:
	void AddToSearchKeyArr( const char* );
	/* オーバーライド? */
	int GetData( void );	/* ダイアログデータの取得 */
	void SetData( void );	/* ダイアログデータの設定 */
	BOOL OnBnClicked( int );
	// virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFIND_H_ */


/*[EOF]*/
