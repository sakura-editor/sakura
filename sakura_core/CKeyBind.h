/*!	@file
	@brief キー割り当てに関するクラス

	@author Norio Nakatani
	@date 1998/03/25 新規作成
	@date 1998/05/16 クラス内にデータを持たないように変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CKeyBind;

#ifndef _CKEYBIND_H_
#define _CKEYBIND_H_

#include <windows.h>
class CMemory;// 2002/2/10 aroka

//! キー情報を保持する
struct KEYDATA {
	/*! キーコード	*/
	short			m_nKeyCode;
//	char*			m_pszKeyName;
	
	/*!	キーの名前	*/
	char			m_szKeyName[64];
	
	/*!	対応する機能番号

		SHIFT, CTRL, ALTの３つのシフト状態のそれぞれに対して
		機能を割り当てるため、配列になっている。
	*/
	/*short*/int	m_nFuncCodeArr[8];
};

class CFuncLookup;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief キー割り当て関連ルーチン
	
	すべての関数はstaticで保持するデータはない。
*/
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
	static int GetFuncCode( WORD nAccelCmd, int nKeyNameArrNum, KEYDATA* pKeyNameArr, BOOL bGetDefFuncCode = TRUE );
	static int GetFuncCodeAt( KEYDATA& KeyData, int nState, BOOL bGetDefFuncCode = TRUE )	/* 特定のキー情報から機能コードを取得する */	// 2007.02.24 ryoji
	{
		if( 0 != KeyData.m_nFuncCodeArr[nState] )
			return KeyData.m_nFuncCodeArr[nState];
		if( bGetDefFuncCode )
			return GetDefFuncCode( KeyData.m_nKeyCode, nState );
		return 0;
	};
	static int GetDefFuncCode( int nKeyCode, int nState );	/* キーのデフォルト機能を取得する */	// 2007.02.22 ryoji

	//! キー割り当て一覧を作成する
	static int CreateKeyBindList( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CMemory& cMemList, CFuncLookup* pcFuncLookup, BOOL bGetDefFuncCode = TRUE );
	static int GetKeyStr( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CMemory& cMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* 機能に対応するキー名の取得 */
	static int GetKeyStrList( HINSTANCE	hInstance, int nKeyNameArrNum,KEYDATA* pKeyNameArr, CMemory*** pppcMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* 機能に対応するキー名の取得(複数) */
	static char* GetMenuLabel( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, int nFuncId, char* pszLabel, BOOL bKeyStr, BOOL bGetDefFuncCode = TRUE );	/* メニューラベルの作成 */

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
