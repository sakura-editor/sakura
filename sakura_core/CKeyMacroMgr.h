//	$Id$
/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CKeyMacroMgr;

#ifndef _CKEYMACROMGR_H_
#define _CKEYMACROMGR_H_

#include <windows.h>
#include "CMemory.h"

struct KeyMacroData {
	int		m_nFuncID;
	LPARAM	m_lParam1;
};
#define MAX_STRLEN			70
#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! キーボードマクロ
class CKeyMacroMgr
{
public:
	/*
	||  Constructors
	*/
	CKeyMacroMgr();
	~CKeyMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void Clear( void );	/* キーマクロのバッファをクリアする */
	int Append( int , LPARAM );	/* キーマクロのバッファにデータ追加 */
	BOOL SaveKeyMacro( HINSTANCE , HWND , const char* );	/* キーボードマクロの保存 */
	BOOL ExecKeyMacro( void* );	/* キーボードマクロの実行 */
	BOOL LoadKeyMacro( HINSTANCE, HWND , const char* );	/* キーボードマクロの読み込み */

//	HINSTANCE		m_hInstance;
	int				m_nKeyMacroDataArrNum;
	KeyMacroData	m_pKeyMacroDataArr[MAX_KEYMACRONUM];
//	CMemory*		m_pKeyMacroDataArr_CMem[MAX_KEYMACRONUM];
//	CMemory			m_cmemKeyMacroDataArr[MAX_KEYMACRONUM];
	char			m_szKeyMacroDataArr[MAX_KEYMACRONUM][MAX_STRLEN];

protected:
	/*
	||  実装ヘルパ関数
	*/

};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYMACROMGR_H_ */


/*[EOF]*/
