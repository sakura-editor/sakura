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

class CSMacroMgr;

#ifndef _CSMACROMGR_H_
#define _CSMACROMGR_H_

#include <windows.h>
#include <vector>
#include "CMemory.h"
#include "CShareData.h"
#include "CKeyMacroMgr.h"

using namespace std;

class CEditView;

//#define MAX_STRLEN			70
//#define MAX_SMacroNUM		10000
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CSMacroMgr
{
	//	データの型宣言
	typedef vector<KeyMacroData> KeyMacroList;

	/*! マクロ１つの情報を格納する */
	struct Macro1 {
		KeyMacroList m_mac; //!< マクロ本体
		vector<string> m_strlist; //!< 補助文字列
		bool m_flag; //!< 読み込み済みかどうかを表すフラグ
	
		Macro1() : m_flag(false){}
		
		void Reset(void){
			m_flag = false;
			m_strlist.clear();
			m_mac.clear();
		}
		
		bool IsReady(void) const { return m_flag; }
	};
	
public:

	/*
	||  Constructors
	*/
	CSMacroMgr();
	~CSMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll( void );	/* キーマクロのバッファをクリアする */

	/*! キーボードマクロの実行 */
	BOOL Exec( HINSTANCE hInstance, CEditView* pViewClass, int idx );
	
	//	登録インターフェース
	//  有効・無効の切り替え
	// Oct. 4, 2001 deleted by genta
//	void Enable(int idx, bool state){
//		if( 0 <= idx || idx < MAX_CUSTMACRO )
//			m_pShareData->m_MacroTable[idx].Enable( state );
//	}
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
	
	//!	表示名の設定
	BOOL SetName(int idx, const char *);
	
	//!	表示名の取得
	const char* GetName(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szName : NULL;
	}
	
	//!	ファイル名の設定
	BOOL SetFile(int idx, const char *);
	
	/*!	@brief ファイル名の取得
	
		@param idx [in] マクロ番号
	*/
	const char* GetFile(int idx) const {
		return ( 0 <= idx || idx < MAX_CUSTMACRO ) ?
		m_pShareData->m_MacroTable[idx].m_szFile : NULL;
	}

protected:
	/*! キーボードマクロの読み込み */
	BOOL Load( CSMacroMgr::Macro1& mbuf, HINSTANCE hInstance, const char* pszPath );
	
	/*! キーマクロのバッファにデータ追加 */
	int Append( CSMacroMgr::Macro1& mbuf, int nFuncID, LPARAM lParam1 );

private:
	vector<Macro1> m_vMacro;

	DLLSHAREDATA*	m_pShareData;

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSMacroMGR_H_ */


/*[EOF]*/
