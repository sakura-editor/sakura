//	$Id$
/************************************************************************

	CKeyBind.h

    キー割り当てに関するクラス
	for Windows
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
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
