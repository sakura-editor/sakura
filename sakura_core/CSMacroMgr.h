//	$Id$
/*!	@file
	キーボードマクロ(直接実行用)

	@author genta
	
	@date Sep. 29, 2001
	$Revision$
*/
/*
	Copyright (C) 2001, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _CSMACROMGR_H_
#define _CSMACROMGR_H_

#include <windows.h>

#include "CKeyMacroMgr.h"
#include "CShareData.h"

//using namespace std;

class CEditView;

#define STAND_KEYMACRO	-1	//	標準マクロ（保存ができる）
#define SAVED_KEYMACRO	0	//	保存されたマクロ

//	関数名はCSMacroMgrが持つ
struct MacroFuncInfo {
	int  	m_nFuncID;
	char *	m_pszFuncName;
//		char *	m_pszFuncParam;
	VARTYPE	m_varArguments[4]; //引数の型の配列
//		int		m_ArgumentCount; //引数の数
	VARTYPE	m_varResult; //戻り値の型 VT_EMPTYならprocedureということで
	char *	m_pszData;
};

/*-----------------------------------------------------------------------
クラスの宣言

@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
-----------------------------------------------------------------------*/
class CSMacroMgr
{
	//	データの型宣言
	CMacroManagerBase* m_cSavedKeyMacro[MAX_CUSTMACRO];	//	キーマクロをカスタムメニューの数だけ管理
	//	Jun. 16, 2002 genta
	//	キーマクロに標準マクロ以外のマクロを読み込めるように
	CMacroManagerBase* m_pKeyMacro;	//	標準の（保存ができる）キーマクロも管理

public:

	/*
	||  Constructors
	*/
	CSMacroMgr();
	~CSMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void Clear( int idx );
	void ClearAll( void );	/* キーマクロのバッファをクリアする */

	/*! キーボードマクロの実行 */
	BOOL Exec( int idx, HINSTANCE hInstance, CEditView* pcEditView );
	
	//!	実行可能か？CShareDataに問い合わせ
	bool IsEnabled(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].IsEnabled() : false;
	}
	
	//!	表示する名前の取得
	const char* GetTitle(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
			( m_pShareData->m_MacroTable[idx].m_szName[0] == '\0' ?
				m_pShareData->m_MacroTable[idx].m_szFile : 
				m_pShareData->m_MacroTable[idx].m_szName)
			: NULL;
	}
	
	//!	表示名の取得
	const char* GetName(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szName : NULL;
	}
	
	/*!	@brief ファイル名の取得
	
		@param idx [in] マクロ番号
	*/
	const char* GetFile(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szFile : NULL;
	}

	/*! キーボードマクロの読み込み */
	BOOL Load( int idx, HINSTANCE hInstance, const char* pszPath );
	BOOL Save( int idx, HINSTANCE hInstance, const char* pszPath );
	
	/*! キーマクロのバッファにデータ追加 */
	int Append( int idx, int nFuncID, LPARAM lParam1, CEditView* pcEditView );

	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID( HINSTANCE , int , char* , char* );	/* 機能ID→関数名，機能名日本語 */
	static int GetFuncInfoByName( HINSTANCE , const char* , char* );	/* 関数名→機能ID，機能名日本語 */
	static BOOL CanFuncIsKeyMacro( int );	/* キーマクロに記録可能な機能かどうかを調べる */
	
	//	Jun. 16, 2002 genta
	static const MacroFuncInfo* GetFuncInfoByID( int );
	
	BOOL IsSaveOk(void);

private:
	DLLSHAREDATA*	m_pShareData;
	CMacroManagerBase** Idx2Ptr(int idx);

public:
	static MacroFuncInfo	m_MacroFuncInfoArr[];
	static MacroFuncInfo	m_MacroFuncInfoNotCommandArr[];
};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */


/*[EOF]*/
