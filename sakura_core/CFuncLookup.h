/*!	@file
	@brief 表示用文字列等の取得

	機能名，機能分類，機能番号などの変換．設定画面での表示用文字列を用意する．

	@author genta
	@date Oct. 1, 2001
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
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

#ifndef __CNAMELOOKUP_H__
#define __CNAMELOOKUP_H__

#include <windows.h>
#include "global.h"
#include "Funccode.h"
struct MacroRec;// 2007.11.02 ryoji
struct Common;// 2002/2/10 aroka

/*!
	@brief 表示用文字列等の取得

	機能，機能分類と位置，機能番号，文字列などの対応を集約する．
*/
class SAKURA_CORE_API CFuncLookup {

public:
	//	Oct. 15, 2001 genta 引数追加
	// 2007.11.02 ryoji 引数変更（CSMacroMgr->MacroRec）
	CFuncLookup( HINSTANCE hInst, MacroRec* pMacroRec, Common* pCom )
		: m_pMacroRec( pMacroRec ), m_hInstance( hInst ), m_pCommon( pCom ) {}
	CFuncLookup() : m_pMacroRec( NULL ), m_hInstance( NULL ) {}

	void Init( HINSTANCE hInst, MacroRec* pMacroRec, Common* pCom ){
		m_pMacroRec = pMacroRec;
		m_hInstance = hInst;
		m_pCommon = pCom;
	}

	int Pos2FuncCode( int category, int position, bool bGetUnavailable = true ) const;	// 2007.10.31 ryoji bGetUnavailableパラメータ追加
	bool Pos2FuncName( int category, int position, char *ptr, int bufsize ) const;
	bool Funccode2Name( int funccode, char *ptr, int bufsize ) const ;
	const char* Category2Name( int category ) const;

	void SetCategory2Combo( HWND hComboBox ) const ;
	void SetListItem( HWND hListBox, int category ) const;
	
	int GetCategoryCount(void) const {
		return nsFuncCode::nFuncKindNum + 2;
	}
	
	int GetItemCount(int category) const;


private:
	HINSTANCE m_hInstance;	//!< 文字列リソースを持つインスタンス
	MacroRec* m_pMacroRec;	//!< マクロ情報	// 2007.11.02 ryoji メンバ変更（CSMacroMgr->MacroRec）
	
	Common* m_pCommon;	//! 共通設定データ領域へのポインタ

};

#endif
/* [EOF] */
