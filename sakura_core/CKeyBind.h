//	$Id$
/*!	@file
	キー割り当てに関するクラス
	
	@author Norio Nakatani
	@date 1998/03/25 新規作成
	@date 1998/05/16 クラス内にデータを持たないように変更
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
class CKeyBind;

#ifndef _CKEYBIND_H_
#define _CKEYBIND_H_

#include <windows.h>
#include "CMemory.h"
#include "keycode.h"


struct KEYDATA {
	short			m_nKeyCode;
//	char*			m_pszKeyName;
	char			m_szKeyName[64];	
	/*short*/int	m_nFuncCodeArr[8];
};


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CKeyBind
{
public:
	/*
	||  Constructors
	*/
	CKeyBind();
	~CKeyBind();

	/*
	||  参照系メンバ関数
	*/
	static HACCEL CreateAccerelator( int, KEYDATA* );
	static int GetFuncCode( WORD, int, KEYDATA* );
	static int CreateKeyBindList( HINSTANCE, int, KEYDATA*, CMemory& );
	static int GetKeyStr( HINSTANCE, int, KEYDATA*, CMemory&, int );	/* 機能に対応するキー名の取得 */
	static int CKeyBind::GetKeyStrList( HINSTANCE, int, KEYDATA*, CMemory***, int );	/* 機能に対応するキー名の取得(複数) */
	static char* GetMenuLabel( HINSTANCE, int, KEYDATA*, int, char*, BOOL );	/* メニューラベルの作成 */

	/*
	||  更新系メンバ関数
	*/


protected:
	/*
	||  メンバ変数
	*/
//	HINSTANCE	m_hInstance;
//	CKeyData*	m_pKeyNameArr;
//	int			m_nKeyNameArrNum;

	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYBIND_H_ */

/*[EOF]*/
