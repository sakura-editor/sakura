//	$Id$
/*!	@file
	ファイルプロパティダイアログ
	
	@author Norio Nakatani
	@date 1999/02/31 新規作成
	@date 1999/12/05 再作成
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

class CDlgProperty;

#ifndef _CDLGPROPERTY_H_
#define _CDLGPROPERTY_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgProperty : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND, LPARAM  );	/* モーダルダイアログの表示 */
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked( int );
	void SetData( void );	/* ダイアログデータの設定 */
};
///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPROPERTY_H_ */

/*[EOF]*/
