//	$Id$
/*!	@file
	ファイルタイプ一覧ダイアログ

	@author Norio Nakatani
	@date 1998/12/23 新規作成
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

class CDlgTypeList;

#ifndef _CDLGTYPELIST_H_
#define _CDLGTYPELIST_H_

const int PROP_TEMPCHANGE_FLAG = 0x10000;

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgTypeList : public CDialog
{
public:
	BOOL DoModal( HINSTANCE, HWND, int* );	/* モーダルダイアログの表示 */

	int				m_nSettingType;
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	void SetData();	/* ダイアログデータの設定 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */


/*[EOF]*/
