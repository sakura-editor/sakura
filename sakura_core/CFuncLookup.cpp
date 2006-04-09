/*!	@file
	@brief 表示用文字列等の取得

	機能名，機能分類，機能番号などの変換．設定画面での表示用文字列を用意する．

	@author genta
	@date Oct.  1, 2001 マクロ
	@date Oct. 15, 2001 カスタムメニュー
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

#include "stdafx.h"
#include "CFuncLookup.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka

//	オフセット値
const int LUOFFSET_MACRO = 0;
const int LUOFFSET_CUSTMENU = 1;

//! 動的に内容が変わる分類の名前
const char *DynCategory[] = {
	"外部マクロ",
	"カスタムメニュー"
};

/*!	@brief 分類中の位置に対応する機能番号を返す．

	@param category [in] 分類番号 (0-)
	@param position [in] 分類中のindex (0-)
*/
int CFuncLookup::Pos2FuncCode( int category, int position ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppnFuncListArr[category][position];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO){
		//	キー割り当てマクロ
		if( m_pcSMacroMgr->IsEnabled(position))
			return F_USERMACRO_0 + position;
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU){
		//	キー割り当てマクロ
		if( position == 0 )
			return F_MENU_RBUTTON;
		else if( 1 <= position && position <= MAX_CUSTOM_MENU - 1 )
			return F_CUSTMENU_BASE + position;
	}
	return 0;
}

/*!	@brief 分類中の位置に対応する機能名称を返す．

	@param category [in] 分類番号 (0-)
	@param position [in] 分類中のindex (0-)
	@param ptr [out] 文字列を格納するバッファの先頭
	@param bufsize [in] 文字列を格納するバッファのサイズ

	@retval true 名称の設定に成功
	@retval false 失敗。文字列は格納されていない
*/
bool CFuncLookup::Pos2FuncName( int category, int position, char *ptr, int bufsize ) const
{
	int func;
	if( category < nsFuncCode::nFuncKindNum ){
		func = nsFuncCode::ppnFuncListArr[category][position];
		return ( ::LoadString( m_hInstance, func, ptr, bufsize ) > 0 );
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO){
		//	キー割り当てマクロ
		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU){
		//	キー割り当てマクロ
		if( 0 <= position && position < MAX_CUSTOM_MENU )
		{
			strncpy( ptr, m_pCommon->m_szCustMenuNameArr[position], bufsize );
			ptr[bufsize-1] = '\0';
		}
		else
			return false;
	}
	return true;
}

/*!	@brief 機能番号に対応する機能名称を返す．

	@param funccode [in] 機能番号
	@param ptr [out] 文字列を格納するバッファの先頭
	@param bufsize [in] 文字列を格納するバッファのサイズ
	
	@retval true 名称の設定に成功
	@retval false 失敗。文字列は格納されていない
*/
bool CFuncLookup::Funccode2Name( int funccode, char *ptr, int bufsize ) const
{
	if( F_USERMACRO_0 <= funccode && funccode < F_USERMACRO_0 + MAX_CUSTMACRO ){
		int position = funccode - F_USERMACRO_0;
		if( !m_pcSMacroMgr->IsEnabled( position )){
			*ptr = '\0';
			return false;
		}

		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	else if( funccode == F_MENU_RBUTTON ){
		strncpy( ptr, m_pCommon->m_szCustMenuNameArr[0], bufsize );
		ptr[bufsize-1] = '\0';
	}
	else if( F_CUSTMENU_1 <= funccode && funccode < F_CUSTMENU_BASE + MAX_CUSTMACRO ){
		strncpy( ptr, m_pCommon->m_szCustMenuNameArr[ funccode - F_CUSTMENU_BASE ], bufsize );
		ptr[bufsize-1] = '\0';
	}
	else {
		return ( ::LoadString( m_hInstance, funccode, ptr, bufsize ) > 0 );
	}
	return true;
}

/*!	@brief 機能分類番号に対応する機能名称を返す．

	@param category [in] 機能分類番号
	
	@return NULL 分類名称．取得に失敗したらNULL．
*/
const char* CFuncLookup::Category2Name( int category ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppszFuncKind[category];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		return DynCategory[0];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		return DynCategory[1];
	}
	return NULL;
}

/*!	@brief ComboBoxに利用可能な機能分類一覧を登録する

	@param hComboBox [in(out)] データを設定するコンボボックス
*/
void CFuncLookup::SetCategory2Combo( HWND hComboBox ) const
{
	int i;

	//	リストを初期化する
	::SendMessage( hComboBox, CB_RESETCONTENT, 0, (LPARAM)0 );

	//	固定機能リスト
	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
		::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
	}

	//	ユーザマクロ
	::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)DynCategory[0] );
	//	カスタムメニュー
	::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)DynCategory[1] );
}

/*!	@brief 指定された分類に属する機能リストをListBoxに登録する．
	
	@param hListBox [in(out)] 値を設定するリストボックス
	@param category [in] 機能分類番号
*/
void CFuncLookup::SetListItem( HWND hListBox, int category ) const
{
	char pszLabel[256];
	int i;

	//	リストを初期化する
	::SendMessage( hListBox, LB_RESETCONTENT , 0, (LPARAM)0 );

	if( category < nsFuncCode::nFuncKindNum ){
		for( i = 0; i < nsFuncCode::pnFuncListNumArr[category]; ++i ){
			if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[category])[i], pszLabel, 255 ) ){
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)pszLabel );
			}else{
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)"--未定義--" );
			}
		}
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		//	マクロ
		for( i = 0; i < MAX_CUSTMACRO ; ++i ){
			if( m_pcSMacroMgr->IsEnabled(i)){
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)m_pcSMacroMgr->GetTitle(i));
			}
			else {
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)"unavailable" );
			}
		}
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		for( i = 0; i < MAX_CUSTOM_MENU ; ++i ){
			::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)m_pCommon->m_szCustMenuNameArr[i] );
		}
	}
}

/*!
	指定分類中の機能数を取得する．
	
	@param category [in] 機能分類番号
*/
int CFuncLookup::GetItemCount(int category) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::pnFuncListNumArr[category];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		//	マクロ
		return MAX_CUSTMACRO;
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		//	マクロ
		return MAX_CUSTOM_MENU;
	}
	return 0;
}
