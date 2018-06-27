/*!	@file
	@brief キーボードマクロ(直接実行用)

	@author genta
	@date Sep. 29, 2001 作成
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2005, FILE
	Copyright (C) 2007, ryoji

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

#include <Windows.h>
#include <WTypes.h> //VARTYPE

#include "CMacroManagerBase.h"
#include "env/DLLSHAREDATA.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class CEditView;


const int STAND_KEYMACRO	= -1;	//!< 標準マクロ(キーマクロ)
const int TEMP_KEYMACRO		= -2;	//!< 一時マクロ(名前を指定してマクロ実行)
const int INVALID_MACRO_IDX	= -3;	//!< 無効なマクロのインデックス番号 @date Sep. 15, 2005 FILE

struct MacroFuncInfoEx
{
	int			m_nArgMinSize;
	int			m_nArgMaxSize;
	VARTYPE*	m_pVarArgEx;
};

//マクロ関数情報構造体
//	関数名はCSMacroMgrが持つ
struct MacroFuncInfo {
	int				m_nFuncID;
	const WCHAR*	m_pszFuncName;
	VARTYPE			m_varArguments[4];	//!< 引数の型の配列
	VARTYPE			m_varResult;		//!< 戻り値の型 VT_EMPTYならprocedureということで
	MacroFuncInfoEx*	m_pData;
};
//マクロ関数情報構造体配列
typedef MacroFuncInfo* MacroFuncInfoArray;

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

	//　一時マクロ（名前を指定してマクロ実行）を管理
	CMacroManagerBase* m_pTempMacro;

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

	//! キーボードマクロの実行
	BOOL Exec( int idx, HINSTANCE hInstance, CEditView* pcEditView, int flags );
	
	//!	実行可能か？CShareDataに問い合わせ
	bool IsEnabled(int idx) const {
		return ( 0 <= idx && idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() : false;
	}
	
	//!	表示する名前の取得
	const TCHAR* GetTitle(int idx) const
	{
		return ( 0 <= idx && idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].GetTitle() : NULL;	// 2007.11.02 ryoji
	}
	
	//!	表示名の取得
	const TCHAR* GetName(int idx) const
	{
		return ( 0 <= idx && idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szName : NULL;
	}
	
	/*!	@brief ファイル名の取得
	
		@param idx [in] マクロ番号
	*/
	const TCHAR* GetFile(int idx) const
	{
		return ( 0 <= idx && idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szFile : 
		( (idx == STAND_KEYMACRO || idx == TEMP_KEYMACRO) && m_sMacroPath != _T("") ) ?
		m_sMacroPath.c_str() : NULL;
	}

	/*! キーボードマクロの読み込み */
	BOOL Load( int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType );
	BOOL Save( int idx, HINSTANCE hInstance, const TCHAR* pszPath );
	void UnloadAll(void);

	/*! キーマクロのバッファにデータ追加 */
	int Append( int idx, EFunctionCode nFuncID, const LPARAM* lParams, CEditView* pcEditView );

	/*
	||  Attributes & Operations
	*/
	static WCHAR* GetFuncInfoByID( HINSTANCE , int , WCHAR* , WCHAR* );	/* 機能ID→関数名，機能名日本語 */
	static EFunctionCode GetFuncInfoByName( HINSTANCE , const WCHAR* , WCHAR* );	/* 関数名→機能ID，機能名日本語 */
	static BOOL CanFuncIsKeyMacro( int );	/* キーマクロに記録可能な機能かどうかを調べる */
	
	//	Jun. 16, 2002 genta
	static const MacroFuncInfo* GetFuncInfoByID( int );
	
	bool IsSaveOk(void);

	//	Sep. 15, 2005 FILE	実行中マクロのインデックス番号操作 (INVALID_MACRO_IDX:無効)
	int GetCurrentIdx( void ) const {
		return m_CurrentIdx;
	}
	int SetCurrentIdx( int idx ) {
		int oldIdx = m_CurrentIdx;
		m_CurrentIdx = idx;
		return oldIdx;
	}

	//  Oct. 22, 2008 syat 一時マクロ導入
	CMacroManagerBase* SetTempMacro( CMacroManagerBase *newMacro );

private:
	DLLSHAREDATA*	m_pShareData;
	CMacroManagerBase** Idx2Ptr(int idx);

	/*!	実行中マクロのインデックス番号 (INVALID_MACRO_IDX:無効)
		@date Sep. 15, 2005 FILE
	*/
	int m_CurrentIdx;

	std::tstring	m_sMacroPath;	// Loadしたマクロ名

public:
	static MacroFuncInfo	m_MacroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo	m_MacroFuncInfoArr[];		// 関数情報(戻り値あり)

private:
	DISALLOW_COPY_AND_ASSIGN(CSMacroMgr);
};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */



