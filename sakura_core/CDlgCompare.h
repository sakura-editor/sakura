//	$Id$
/*!	@file
	ファイル比較ダイアログボックス

	@author Norio Nakatani
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

class CDlgCompare;

#ifndef _CDLGCOMPARE_H_
#define _CDLGCOMPARE_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgCompare : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCompare();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const char*, BOOL, char*, HWND* );	/* モーダルダイアログの表示 */

	const char*		m_pszPath;
	BOOL			m_bIsModified;
	char*			m_pszComparePath;
	HWND*			m_phwndCompareWnd;
	BOOL			m_bCompareAndTileHorz;/* 左右に並べて表示 */
//	BOOL			m_bCompareAndTileHorz;/* 左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked( int );

	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCOMPARE_H_ */

/*[EOF]*/
