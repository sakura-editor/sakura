//	$Id$
/*!	@file

	機能名，機能分類，機能番号などの変換．設定画面での表示用文字列を用意する．

	@author genta
	@date Oct. 1, 2001
*/

#ifndef __CNAMELOOKUP_H__
#define __CNAMELOOKUP_H__

#include <windows.h>
#include "global.h"
#include "Funccode.h"
#include "CSMacroMgr.h"

/*!
	機能，機能分類と位置，機能番号，文字列などの対応を集約する．
*/
class SAKURA_CORE_API CFuncLookup {

public:
	CFuncLookup( HINSTANCE hInst, CSMacroMgr* SMacroMgr )
		: m_pcSMacroMgr( SMacroMgr ), m_hInstance( hInst ) {}
	CFuncLookup() : m_pcSMacroMgr( NULL ), m_hInstance( NULL ) {}

	void Init( HINSTANCE hInst, CSMacroMgr* SMacroMgr ){
		m_pcSMacroMgr = SMacroMgr;
		m_hInstance = hInst;
	}

	int Pos2FuncCode( int category, int position ) const;
	bool Pos2FuncName( int category, int position, char *ptr, int bufsize ) const;
	bool Funccode2Name( int funccode, char *ptr, int bufsize ) const ;
	const char* Category2Name( int category ) const;

	void SetCategory2Combo( HWND hComboBox ) const ;
	void SetListItem( HWND hListBox, int category ) const;
	
	int GetCategoryCount(void) const {
		return nsFuncCode::nFuncKindNum + 1;
	}
	
	int GetItemCount(int category) const;


private:
	HINSTANCE m_hInstance;	//!< 文字列リソースを持つインスタンス
	CSMacroMgr* m_pcSMacroMgr;	//!< マクロ管理クラス

};

#endif
/* [EOF] */
